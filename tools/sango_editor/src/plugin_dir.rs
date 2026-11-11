//! The folder where the plugin reads its text files (the events and the scripts).
//!
//! The blocks, the maps and the tilesets that the tool writes are always in `sango`. The text
//! files also go there, unless the user chooses another folder in the settings.

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
