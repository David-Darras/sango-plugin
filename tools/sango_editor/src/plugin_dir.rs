//! The folder where the plugin reads its text files (the events and the scripts).
//!
//! The plugin that one installs is made for a product, and each product has its own data folder on the
//! SD card: `sango` for the full plugin, `sango/maker` for Sango Maker. The blocks, the maps and the
//! tilesets that the tool writes are always in `sango`; but the text files must be in the folder of
//! the product, else the plugin does not see them.

use std::path::{Path, PathBuf};
use std::sync::Mutex;

static FOLDER: Mutex<Option<PathBuf>> = Mutex::new(None);

/// Says which folder the plugin reads its text files in.
pub fn set(folder: &Path) {
    if let Ok(mut slot) = FOLDER.lock() {
        *slot = Some(folder.to_path_buf());
    }
}

/// The folder of the text files: the one that was chosen, else `default`.
pub fn get(default: &Path) -> PathBuf {
    FOLDER.lock().ok().and_then(|slot| slot.clone()).unwrap_or_else(|| default.to_path_buf())
}

/// Finds the folder that the plugin which is installed reads: Sango Maker has `sango/maker`, the
/// full plugin has `sango`. The tool looks in the newest plugin file of the SD card for the text
/// of the path (the paths of the plugin are in UTF-16).
pub fn detect(sd_dir: &str) -> PathBuf {
    let sango = PathBuf::from(sd_dir);
    let Some(sd_root) = sango.parent() else { return sango };
    let plugins = sd_root.join("luma").join("plugins");
    let mut newest: Option<(std::time::SystemTime, PathBuf)> = None;
    if let Ok(titles) = std::fs::read_dir(&plugins) {
        for title in titles.flatten() {
            let Ok(files) = std::fs::read_dir(title.path()) else { continue };
            for file in files.flatten() {
                let path = file.path();
                if path.extension().is_some_and(|e| e == "3gx") {
                    if let Ok(time) = file.metadata().and_then(|m| m.modified()) {
                        if newest.as_ref().map_or(true, |(t, _)| time > *t) {
                            newest = Some((time, path));
                        }
                    }
                }
            }
        }
    }
    let Some((_, plugin)) = newest else { return sango };
    let Ok(bytes) = std::fs::read(&plugin) else { return sango };
    let wide: Vec<u8> = "sdmc:/sango/maker/events_log.txt".encode_utf16().flat_map(|c| c.to_le_bytes()).collect();
    if bytes.windows(wide.len()).any(|w| w == wide.as_slice()) {
        sango.join("maker")
    } else {
        sango
    }
}

/// Moves the text files that an earlier run wrote in `from` to `to` (the files that are already
/// in `to` stay).
pub fn migrate(from: &Path, to: &Path) {
    if from == to {
        return;
    }
    for name in ["map_events.txt", "scripts.txt"] {
        let (old, new) = (from.join(name), to.join(name));
        if old.is_file() && !new.exists() {
            let _ = std::fs::create_dir_all(to);
            if std::fs::copy(&old, &new).is_ok() {
                let _ = std::fs::remove_file(&old);
            }
        }
    }
}
