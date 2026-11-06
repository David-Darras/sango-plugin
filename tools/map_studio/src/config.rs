//! What the tool remembers between two runs: the folder of the archives and the
//! block that was open.

use std::path::PathBuf;

/// The folder of the archives that the tool uses when none is given.
const DEFAULT_ARCHIVES: &str = "C:/Users/David/Desktop/ctr/sango/sango_project/arc";

pub struct Config {
    pub arc_dir: Option<String>,
    pub block: Option<usize>,
}

fn path() -> PathBuf {
    let base = std::env::var("LOCALAPPDATA").map(PathBuf::from).unwrap_or_else(|_| std::env::temp_dir());
    base.join("map_studio").join("config.txt")
}

pub fn load() -> Config {
    let mut config = Config { arc_dir: None, block: None };
    if let Ok(text) = std::fs::read_to_string(path()) {
        for line in text.lines() {
            if let Some(v) = line.strip_prefix("archives=") {
                config.arc_dir = Some(v.trim().to_string());
            } else if let Some(v) = line.strip_prefix("block=") {
                config.block = v.trim().parse().ok();
            }
        }
    }
    config
}

pub fn save(arc_dir: &str, block: usize) {
    let p = path();
    if let Some(dir) = p.parent() {
        let _ = std::fs::create_dir_all(dir);
    }
    let _ = std::fs::write(p, format!("archives={arc_dir}\nblock={block}\n"));
}

/// The folder of the archives: the one that is remembered, else the usual one.
pub fn archives(given: Option<&str>) -> Result<String, String> {
    let config = load();
    let candidates = [given.map(String::from), config.arc_dir, Some(DEFAULT_ARCHIVES.to_string())];
    candidates
        .into_iter()
        .flatten()
        .find(|d| std::path::Path::new(d).join("map_land_data.garc").is_file())
        .ok_or_else(|| "cannot find the folder of the archives: give it (map-studio view <folder of the archives>)".to_string())
}
