//! What the tool remembers between two runs: the folder of the archives and the
//! block that was open.

use std::path::PathBuf;

/// The folder of the archives that the tool uses when none is given.
const DEFAULT_ARCHIVES: &str = "C:/Users/David/AppData/Roaming/Azahar/dump/romfs/000400000011C500";

pub struct Config {
    pub arc_dir: Option<String>,
    pub block: Option<usize>,
}

fn path() -> PathBuf {
    let base = std::env::var("LOCALAPPDATA").map(PathBuf::from).unwrap_or_else(|_| std::env::temp_dir());
    base.join("sango_editor").join("config.txt")
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

/// Says whether a folder is a dump of the RomFS (it has the file `a/0/3/9`, the blocks).
fn is_dump(dir: &str) -> bool {
    std::path::Path::new(dir).join("a").join("0").join("3").join("9").is_file()
}

/// The folder of the data of the game: the dump of the RomFS. A folder that the
/// user gives is used when it is a dump too, else the usual dump.
pub fn archives(given: Option<&str>) -> Result<String, String> {
    let config = load();
    if let Some(g) = given.filter(|g| is_dump(g)) {
        return Ok(g.to_string());
    }
    if is_dump(DEFAULT_ARCHIVES) {
        if let Some(g) = given {
            println!("{g} is not a dump of the RomFS: the tool reads {DEFAULT_ARCHIVES}");
        }
        return Ok(DEFAULT_ARCHIVES.to_string());
    }
    config
        .arc_dir
        .filter(|d| is_dump(d))
        .ok_or_else(|| "cannot find the dump of the RomFS: give its folder (sango-editor view <folder>)".to_string())
}

/// Brings what the tool kept under its old name (the folder `map_studio` of
/// the data of the user) to the folder of the new name, once.
pub fn migrate_old_files() {
    let base = std::env::var("LOCALAPPDATA").map(PathBuf::from).unwrap_or_else(|_| std::env::temp_dir());
    let (old, new) = (base.join("map_studio"), base.join("sango_editor"));
    if !old.is_dir() || new.exists() {
        return;
    }
    if std::fs::create_dir_all(&new).is_err() {
        return;
    }
    if let Ok(list) = std::fs::read_dir(&old) {
        for entry in list.flatten() {
            let _ = std::fs::copy(entry.path(), new.join(entry.file_name()));
        }
    }
}
