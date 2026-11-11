//! The scripts that the plugin runs from a text file, without code.
//!
//! The plugin has 12 scripts, numbers 32000 to 32011. Each one is a list of at
//! most 8 steps (show a text, ask yes or no, give an item...). They are in the
//! file `scripts.txt` of the folder of the SD card; the plugin reads the file
//! again when it changes. A sign, a trigger or a character that has one of
//! these numbers as its script starts the script.
//!
//! A line of the file is `script step action parameter1 parameter2 text`.

use std::path::{Path, PathBuf};
use std::sync::Mutex;

/// How many scripts the plugin has, and the number of the first one.
pub const COUNT: usize = 12;
pub const FIRST: u16 = 32000;
/// How many steps a script has.
pub const STEPS: usize = 8;
/// The text of a step: two parts of 31 characters.
pub const TEXT_LENGTH: usize = 62;

/// What a step does, in the order of the numbers of the file.
pub const ACTIONS: [&str; 12] = [
    "(end of the script)",
    "Say a text",
    "Ask yes or no (no ends the script)",
    "Give an item",
    "Give or take money",
    "Set a flag",
    "Reset a flag",
    "Stop when a flag is not set",
    "Play a sound",
    "Wait",
    "Start a script of the game",
    "Give a Pokemon of the table of gifts",
];

/// One step of a script.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Step {
    pub action: u8,
    pub parameter_1: u16,
    pub parameter_2: u16,
    pub text: String,
}

/// The twelve scripts.
#[derive(Clone, Debug, PartialEq)]
pub struct DataScripts {
    pub scripts: Vec<Vec<Step>>,
}

impl Default for DataScripts {
    fn default() -> Self {
        DataScripts { scripts: vec![vec![Step::default(); STEPS]; COUNT] }
    }
}

impl DataScripts {
    /// Reads the text of the file. A line that is not understood is left out.
    pub fn parse(text: &str) -> DataScripts {
        let mut out = DataScripts::default();
        for line in text.lines() {
            let mut rest = line.trim_end();
            let mut numbers = [0u32; 5];
            let mut ok = true;
            for slot in numbers.iter_mut() {
                let trimmed = rest.trim_start();
                let end = trimmed.find(' ').unwrap_or(trimmed.len());
                match trimmed[..end].parse::<u32>() {
                    Ok(v) => *slot = v,
                    Err(_) => {
                        ok = false;
                        break;
                    }
                }
                rest = &trimmed[end..];
            }
            if !ok || numbers[0] as usize >= COUNT || numbers[1] as usize >= STEPS || numbers[2] as usize >= ACTIONS.len() {
                continue;
            }
            let text: String = rest.strip_prefix(' ').unwrap_or(rest).chars().take(TEXT_LENGTH).collect();
            out.scripts[numbers[0] as usize][numbers[1] as usize] =
                Step { action: numbers[2] as u8, parameter_1: numbers[3].min(65535) as u16, parameter_2: numbers[4].min(65535) as u16, text };
        }
        out
    }

    /// The text of the file: only the steps that do something.
    pub fn to_text(&self) -> String {
        let mut out = String::from("# Sango scripts. script step action parameter1 parameter2 text\n");
        for (s, script) in self.scripts.iter().enumerate() {
            for (i, step) in script.iter().enumerate() {
                if step.action == 0 {
                    continue;
                }
                let text: String = step.text.chars().take(TEXT_LENGTH).collect();
                out += &format!("{s} {i} {} {} {} {text}\n", step.action, step.parameter_1, step.parameter_2);
            }
        }
        out
    }

    /// True when at least one step does something.
    pub fn is_used(&self, script: usize) -> bool {
        self.scripts.get(script).is_some_and(|steps| steps.iter().any(|s| s.action != 0))
    }

    /// The number and the first text of each script, for the lists.
    pub fn labels(&self) -> Vec<(u16, String)> {
        (0..COUNT)
            .map(|s| {
                let first = self.scripts[s].iter().find(|st| st.action == 1).map(|st| st.text.clone()).unwrap_or_default();
                let label = if self.is_used(s) {
                    if first.is_empty() { format!("script {} of Sango", s + 1) } else { format!("script {} of Sango: \"{first}\"", s + 1) }
                } else {
                    format!("script {} of Sango (empty)", s + 1)
                };
                (FIRST + s as u16, label)
            })
            .collect()
    }
}

fn file_path(sd_dir: &str) -> PathBuf {
    // The folder that was chosen for the plugin, else the folder that was given.
    crate::plugin_dir::get(Path::new(sd_dir)).join("scripts.txt")
}

/// Reads the scripts that the tool or the plugin wrote on the SD card (none when there is no file).
pub fn read(sd_dir: &str) -> DataScripts {
    std::fs::read_to_string(file_path(sd_dir)).map(|t| DataScripts::parse(&t)).unwrap_or_default()
}

/// Writes the scripts. The file is taken away when no script does anything (as the plugin does).
pub fn write(sd_dir: &str, scripts: &DataScripts) -> Result<(), String> {
    let path = file_path(sd_dir);
    if (0..COUNT).all(|s| !scripts.is_used(s)) {
        if path.exists() {
            std::fs::remove_file(&path).map_err(|e| format!("{}: {e}", path.display()))?;
        }
        return Ok(());
    }
    std::fs::create_dir_all(sd_dir).map_err(|e| format!("{sd_dir}: {e}"))?;
    std::fs::write(&path, scripts.to_text()).map_err(|e| format!("{}: {e}", path.display()))
}

/// The scripts of the lists: the ones of Sango (changed by the user) and the ones that
/// the game uses (found once, in the background).
static DATA_LABELS: Mutex<Vec<(u16, String)>> = Mutex::new(Vec::new());
static GAME_LABELS: Mutex<Vec<(u16, String)>> = Mutex::new(Vec::new());

pub fn set_data_labels(scripts: &DataScripts) {
    if let Ok(mut labels) = DATA_LABELS.lock() {
        *labels = scripts.labels();
    }
}

/// Looks in all the maps of the game for the scripts that the signs, the triggers and the
/// characters use, with how many times and an example. Takes a second or two: use it in a thread.
pub fn find_game_scripts(arc_dir: &str) {
    let mut found: std::collections::BTreeMap<u16, (usize, String)> = Default::default();
    for map in 0..crate::names::MAP_NAMES.len() {
        let Ok(events) = crate::events::read_map_events(arc_dir, map) else { continue };
        let place = crate::names::MAP_NAMES.get(map).copied().unwrap_or("?");
        let mut note = |id: u16, what: String| {
            if id == 65535 || (FIRST..FIRST + COUNT as u16).contains(&id) {
                return;
            }
            let entry = found.entry(id).or_insert((0, format!("{what} in {place}")));
            entry.0 += 1;
        };
        for c in &events.characters {
            note(c.script, format!("character {}", crate::names::MODEL_NAMES.get(c.model as usize).copied().unwrap_or("?")));
        }
        for s in &events.signs {
            note(s.id, "sign".into());
        }
        for t in &events.triggers {
            note(t.id, "trigger".into());
        }
    }
    let list: Vec<(u16, String)> = found
        .into_iter()
        .map(|(id, (n, example))| {
            let kind = if (3000..4000).contains(&id) { format!("trainer {}, ", id - 3000) } else { String::new() };
            (id, format!("{kind}used {n} time(s), for example a {example}"))
        })
        .collect();
    if let Ok(mut labels) = GAME_LABELS.lock() {
        *labels = list;
    }
}

/// The choices for the script of an event: none, the scripts of Sango, the scripts of the game.
pub fn choices() -> Vec<(u16, String)> {
    let mut out = vec![(65535u16, "none (the game ignores the event)".to_string())];
    if let Ok(labels) = DATA_LABELS.lock() {
        out.extend(labels.iter().cloned());
    }
    if let Ok(labels) = GAME_LABELS.lock() {
        out.extend(labels.iter().cloned());
    }
    out
}
