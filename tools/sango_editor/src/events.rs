//! The events of a map: signs, characters, warps and triggers.
//!
//! The game keeps the events of a map in the part 1 of the map pack. The
//! part starts with 12 bytes (the size, the number of signs, characters, warps
//! and triggers, and 4 bytes that are not used). The records follow in this
//! order, and each kind has its own size. A tile is 18 units wide, and the
//! tiles are counted from the corner of the map.
//!
//! The plugin reads the file `map_events.txt` of the SD card. A line starts
//! with the number of the map. `S=`, `T=`, `W=` and `C=` change a record of the
//! game, `S+`, `T+`, `W+` and `C+` add one.

use std::collections::BTreeMap;
use std::path::PathBuf;

use crate::{garc, lz11, pack};

/// The size of a tile, and of a block in tiles.
pub const TILE: f32 = 18.0;
pub const BLOCK_TILES: usize = 40;

/// A sign: a place where the player presses A to start a script.
#[derive(Clone, Debug, PartialEq)]
pub struct Sign {
    /// The bytes of the record in the game (the bytes that the tool does not
    /// know stay the same when the record is written again).
    pub raw: Vec<u8>,
    pub id: u16,
    pub kind: u16,
    pub facing: u8,
    pub x: i16,
    pub z: i16,
    pub width: u16,
    pub depth: u16,
    pub height: i32,
}

/// A trigger: a place that starts a script when the player walks on it.
#[derive(Clone, Debug, PartialEq)]
pub struct Trigger {
    /// The bytes of the record in the game (the bytes that the tool does not
    /// know stay the same when the record is written again).
    pub raw: Vec<u8>,
    pub id: u16,
    pub param: u16,
    pub work_id: u16,
    pub check_kind: u16,
    pub x: u16,
    pub z: u16,
    pub width: u16,
    pub depth: u16,
    pub height: i16,
}

/// A warp: a place that takes the player to another map.
#[derive(Clone, Debug, PartialEq)]
pub struct Warp {
    /// The bytes of the record in the game (the bytes that the tool does not
    /// know stay the same when the record is written again).
    pub raw: Vec<u8>,
    pub map: u16,
    pub exit: u16,
    pub facing: u8,
    pub kind: u8,
    /// The middle of the place, in units (a tile is 18 units).
    pub x: i16,
    pub y: i16,
    pub z: i16,
    /// The size in tiles.
    pub width: u16,
    pub depth: u16,
}

/// A character of the map.
#[derive(Clone, Debug, PartialEq)]
pub struct Character {
    /// The bytes of the record in the game (the bytes that the tool does not
    /// know stay the same when the record is written again).
    pub raw: Vec<u8>,
    pub local_id: u16,
    pub model: u16,
    pub movement: u16,
    pub interaction: u16,
    pub hide_flag: u16,
    pub script: u16,
    pub facing: u8,
    pub param: [u16; 3],
    pub trigger: [i16; 4],
    /// The three numbers of map that the record keeps, and the kind of position.
    pub maps: [u16; 3],
    pub alias: u16,
    pub position_kind: u32,
    pub x: u16,
    pub z: u16,
    pub height: f32,
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct MapEvents {
    pub signs: Vec<Sign>,
    pub characters: Vec<Character>,
    pub warps: Vec<Warp>,
    pub triggers: Vec<Trigger>,
    /// What follows the triggers (the interrupts): kept as it is.
    pub tail: Vec<u8>,
    /// The first 12 bytes of the part, as they are in the game.
    pub head: [u8; 12],
}

fn put16(b: &mut [u8], at: usize, v: u16) {
    b[at..at + 2].copy_from_slice(&v.to_le_bytes());
}
fn put32(b: &mut [u8], at: usize, v: u32) {
    b[at..at + 4].copy_from_slice(&v.to_le_bytes());
}

/// The bytes of a record: the bytes of the game, with the known fields on top.
fn record_bytes(raw: &[u8], size: usize) -> Vec<u8> {
    let mut b = raw.to_vec();
    b.resize(size, 0);
    b
}

fn u16_at(b: &[u8], at: usize) -> u16 {
    u16::from_le_bytes([b[at], b[at + 1]])
}
fn i16_at(b: &[u8], at: usize) -> i16 {
    u16_at(b, at) as i16
}
fn u32_at(b: &[u8], at: usize) -> u32 {
    u32::from_le_bytes([b[at], b[at + 1], b[at + 2], b[at + 3]])
}

impl MapEvents {
    /// The part 1 of a map pack: the head with the new numbers, the records,
    /// and what follows them in the game (the interrupts and the scripts).
    pub fn to_bytes(&self) -> Vec<u8> {
        let old_records = 20 * self.head[4] as usize + 48 * self.head[5] as usize + 24 * self.head[6] as usize + 24 * self.head[7] as usize;
        let new_records = 20 * self.signs.len() + 48 * self.characters.len() + 24 * self.warps.len() + 24 * self.triggers.len();
        let old_size = u32::from_le_bytes([self.head[0], self.head[1], self.head[2], self.head[3]]) as usize;
        let mut head = self.head;
        put32(&mut head, 0, (old_size + new_records - old_records.min(old_size + new_records)) as u32);
        head[4] = self.signs.len() as u8;
        head[5] = self.characters.len() as u8;
        head[6] = self.warps.len() as u8;
        head[7] = self.triggers.len() as u8;
        let mut out = head.to_vec();
        for s in &self.signs {
            let mut b = record_bytes(&s.raw, 20);
            put16(&mut b, 0, s.id);
            put16(&mut b, 2, s.kind);
            b[4] = s.facing;
            put16(&mut b, 8, s.x as u16);
            put16(&mut b, 10, s.z as u16);
            put16(&mut b, 12, s.width);
            put16(&mut b, 14, s.depth);
            put32(&mut b, 16, s.height as u32);
            out.extend_from_slice(&b);
        }
        for c in &self.characters {
            let mut b = record_bytes(&c.raw, 48);
            put16(&mut b, 0, c.local_id);
            put16(&mut b, 2, c.model);
            put16(&mut b, 4, c.movement);
            put16(&mut b, 6, c.interaction);
            put16(&mut b, 8, c.hide_flag);
            put16(&mut b, 10, c.script);
            b[12] = c.facing;
            for k in 0..3 {
                put16(&mut b, 14 + 2 * k, c.param[k]);
            }
            for k in 0..4 {
                put16(&mut b, 20 + 2 * k, c.trigger[k] as u16);
            }
            for k in 0..3 {
                put16(&mut b, 28 + 2 * k, c.maps[k]);
            }
            put16(&mut b, 34, c.alias);
            put32(&mut b, 36, c.position_kind);
            put16(&mut b, 40, c.x);
            put16(&mut b, 42, c.z);
            put32(&mut b, 44, c.height.to_bits());
            out.extend_from_slice(&b);
        }
        for w in &self.warps {
            let mut b = record_bytes(&w.raw, 24);
            put16(&mut b, 0, w.map);
            put16(&mut b, 2, w.exit);
            b[4] = w.facing;
            b[5] = w.kind;
            put16(&mut b, 8, w.x as u16);
            put16(&mut b, 10, w.y as u16);
            put16(&mut b, 12, w.z as u16);
            put16(&mut b, 14, w.width);
            put16(&mut b, 16, w.depth);
            out.extend_from_slice(&b);
        }
        for t in &self.triggers {
            let mut b = record_bytes(&t.raw, 24);
            put16(&mut b, 0, t.id);
            put16(&mut b, 2, t.param);
            put16(&mut b, 4, t.work_id);
            put16(&mut b, 6, t.check_kind);
            put16(&mut b, 12, t.x);
            put16(&mut b, 14, t.z);
            put16(&mut b, 16, t.width);
            put16(&mut b, 18, t.depth);
            put16(&mut b, 20, t.height as u16);
            out.extend_from_slice(&b);
        }
        out.extend_from_slice(&self.tail);
        while out.len() % 4 != 0 {
            out.push(0);
        }
        out
    }

    /// Reads the part 1 of a map pack.
    pub fn parse(part: &[u8]) -> Result<MapEvents, String> {
        if part.len() < 12 {
            return Err("the events of the map are too short".into());
        }
        let (signs, characters, warps, triggers) = (part[4] as usize, part[5] as usize, part[6] as usize, part[7] as usize);
        let needed = 12 + 20 * signs + 48 * characters + 24 * warps + 24 * triggers;
        if part.len() < needed {
            return Err("the events of the map are cut".into());
        }
        let mut at = 12;
        let mut out = MapEvents::default();
        out.head.copy_from_slice(&part[..12]);
        for _ in 0..signs {
            out.signs.push(Sign {
                raw: part[at..at + 20].to_vec(),
                id: u16_at(part, at),
                kind: u16_at(part, at + 2),
                facing: part[at + 4],
                x: i16_at(part, at + 8),
                z: i16_at(part, at + 10),
                width: u16_at(part, at + 12),
                depth: u16_at(part, at + 14),
                height: u32_at(part, at + 16) as i32,
            });
            at += 20;
        }
        for _ in 0..characters {
            out.characters.push(Character {
                raw: part[at..at + 48].to_vec(),
                local_id: u16_at(part, at),
                model: u16_at(part, at + 2),
                movement: u16_at(part, at + 4),
                interaction: u16_at(part, at + 6),
                hide_flag: u16_at(part, at + 8),
                script: u16_at(part, at + 10),
                facing: part[at + 12],
                param: [u16_at(part, at + 14), u16_at(part, at + 16), u16_at(part, at + 18)],
                trigger: [i16_at(part, at + 20), i16_at(part, at + 22), i16_at(part, at + 24), i16_at(part, at + 26)],
                maps: [u16_at(part, at + 28), u16_at(part, at + 30), u16_at(part, at + 32)],
                alias: u16_at(part, at + 34),
                position_kind: u32_at(part, at + 36),
                x: u16_at(part, at + 40),
                z: u16_at(part, at + 42),
                height: f32::from_bits(u32_at(part, at + 44)),
            });
            at += 48;
        }
        for _ in 0..warps {
            out.warps.push(Warp {
                raw: part[at..at + 24].to_vec(),
                map: u16_at(part, at),
                exit: u16_at(part, at + 2),
                facing: part[at + 4],
                kind: part[at + 5],
                x: i16_at(part, at + 8),
                y: i16_at(part, at + 10),
                z: i16_at(part, at + 12),
                width: u16_at(part, at + 14),
                depth: u16_at(part, at + 16),
            });
            at += 24;
        }
        for _ in 0..triggers {
            out.triggers.push(Trigger {
                raw: part[at..at + 24].to_vec(),
                id: u16_at(part, at),
                param: u16_at(part, at + 2),
                work_id: u16_at(part, at + 4),
                check_kind: u16_at(part, at + 6),
                x: u16_at(part, at + 12),
                z: u16_at(part, at + 14),
                width: u16_at(part, at + 16),
                depth: u16_at(part, at + 18),
                height: i16_at(part, at + 20),
            });
            at += 24;
        }
        out.tail = part[at..].to_vec();
        Ok(out)
    }
}

/// A map that has the block, and where.
#[derive(Clone, Debug)]
pub struct MapAt {
    pub map: usize,
    pub matrix: usize,
    pub column: usize,
    pub row: usize,
    pub width: usize,
    pub height: usize,
}

fn unpack(raw: &[u8]) -> Result<Vec<u8>, String> {
    if raw.first() == Some(&0x11) { lz11::decompress(raw) } else { Ok(raw.to_vec()) }
}

/// The blocks of a matrix, row after row.
fn matrix_blocks(raw: &[u8]) -> Result<(usize, usize, Vec<u16>), String> {
    let data = unpack(raw)?;
    if data.len() < 24 || &data[..2] != b"MM" {
        return Err("not a matrix".into());
    }
    let (width, height) = (u16_at(&data, 20) as usize, u16_at(&data, 22) as usize);
    if data.len() < 24 + 2 * width * height {
        return Err("a matrix is cut".into());
    }
    Ok((width, height, (0..width * height).map(|i| u16_at(&data, 24 + 2 * i)).collect()))
}

/// The parts of the pack of a map.
fn map_parts(raw: &[u8]) -> Result<Vec<Vec<u8>>, String> {
    let data = unpack(raw)?;
    let (_, parts) = pack::split(&data)?;
    Ok(parts.into_iter().map(|p| p.to_vec()).collect())
}

/// The maps that have a block in their matrix.
pub fn maps_of_block(arc_dir: &str, block: usize) -> Result<Vec<MapAt>, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let matrices = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Matrices))?;
    let mut cache: BTreeMap<usize, Option<(usize, usize, Vec<u16>)>> = BTreeMap::new();
    let mut found = Vec::new();
    for (map, file) in maps.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = map_parts(raw) else { continue };
        let Some(settings) = parts.first().filter(|p| p.len() >= 6) else { continue };
        let matrix = u16_at(settings, 4) as usize;
        let entry = cache.entry(matrix).or_insert_with(|| matrices.get(matrix).and_then(|f| f.first()).and_then(|r| matrix_blocks(r).ok()));
        let Some((width, height, ids)) = entry else { continue };
        if let Some(at) = ids.iter().position(|&b| b as usize == block) {
            found.push(MapAt { map, matrix, column: at % *width, row: at / *width, width: *width, height: *height });
        }
    }
    Ok(found)
}

/// A world: the maps that share a matrix (the same grid of blocks).
#[derive(Clone, Debug)]
pub struct WorldInfo {
    pub matrix: usize,
    pub maps: Vec<usize>,
}

/// All the worlds of the game, by number of matrix.
pub fn list_worlds(arc_dir: &str) -> Result<Vec<WorldInfo>, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let mut by_matrix: BTreeMap<usize, Vec<usize>> = BTreeMap::new();
    for (map, file) in maps.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = map_parts(raw) else { continue };
        let Some(settings) = parts.first().filter(|p| p.len() >= 6) else { continue };
        by_matrix.entry(u16_at(settings, 4) as usize).or_default().push(map);
    }
    Ok(by_matrix.into_iter().map(|(matrix, maps)| WorldInfo { matrix, maps }).collect())
}

/// The number of the tileset pack of a map (its pictures and its light).
pub fn map_tileset(arc_dir: &str, map: usize) -> Option<usize> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps)).ok()?;
    let parts = map_parts(maps.get(map)?.first()?).ok()?;
    let settings = parts.first().filter(|p| p.len() >= 4)?;
    Some(u16_at(settings, 2) as usize)
}

/// The blocks of a matrix: the width and the height in blocks, and the number
/// of the block of each place (row after row).
#[derive(Clone, Debug, PartialEq)]
pub struct MatrixLayout {
    pub width: usize,
    pub height: usize,
    pub ids: Vec<u16>,
}

/// Reads a matrix, from the file that the tool wrote on the SD card if there
/// is one, else from the archive.
pub fn read_matrix(arc_dir: &str, sd_dir: &str, matrix: usize) -> Result<MatrixLayout, String> {
    let own = PathBuf::from(sd_dir).join("matrix").join(format!("{matrix}.bin"));
    // An empty folder name means "the archive only".
    let found = if sd_dir.is_empty() { None } else { std::fs::read(&own).ok() };
    let raw = match found {
        Some(bytes) => bytes,
        None => {
            let files = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Matrices))?;
            files.get(matrix).and_then(|f| f.first()).ok_or("no such matrix")?.clone()
        }
    };
    let (width, height, ids) = matrix_blocks(&raw)?;
    Ok(MatrixLayout { width, height, ids })
}

/// Writes a matrix with other blocks in the folder `matrix` of the SD card.
pub fn write_matrix_override(arc_dir: &str, sd_dir: &str, matrix: usize, layout: &MatrixLayout) -> Result<String, String> {
    let files = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Matrices))?;
    let raw = files.get(matrix).and_then(|f| f.first()).ok_or("no such matrix")?;
    let compressed = raw.first() == Some(&0x11);
    let mut data = unpack(raw)?;
    if data.len() < 24 + 2 * layout.ids.len() {
        return Err("the matrix is too short".into());
    }
    for (i, id) in layout.ids.iter().enumerate() {
        put16(&mut data, 24 + 2 * i, *id);
    }
    let bytes = if compressed { lz11::compress(&data)? } else { data };
    let folder = PathBuf::from(sd_dir).join("matrix");
    std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
    let path = folder.join(format!("{matrix}.bin"));
    std::fs::write(&path, &bytes).map_err(|e| format!("{}: {e}", path.display()))?;
    crate::edit::write_index(&folder, matrix, bytes.len())?;
    Ok(format!("{} ({} bytes)", path.display(), bytes.len()))
}

/// Gives a matrix its blocks of the game again.
pub fn reset_matrix(sd_dir: &str, matrix: usize) -> Result<(), String> {
    let folder = PathBuf::from(sd_dir).join("matrix");
    let file = folder.join(format!("{matrix}.bin"));
    if file.exists() {
        std::fs::remove_file(&file).map_err(|e| format!("{}: {e}", file.display()))?;
        crate::edit::remove_index(&folder, matrix)?;
    }
    Ok(())
}

/// The blocks of a world where a map has events, as the game has them. A map
/// has no block of its own: this is the part of the world that it lives in.
pub fn blocks_of_map(arc_dir: &str, matrix: usize, map: usize) -> Vec<usize> {
    // (A folder that does not exist: the matrix of the game, not the one of the user.)
    let nowhere = std::env::temp_dir().join("sango_editor_no_such_folder");
    let (Ok(layout), Ok(events)) = (read_matrix(arc_dir, &nowhere.to_string_lossy(), matrix), read_map_events(arc_dir, map)) else { return Vec::new() };
    let mut cells: std::collections::BTreeSet<(usize, usize)> = Default::default();
    let mut add = |tx: f32, tz: f32| {
        if tx >= 0.0 && tz >= 0.0 {
            cells.insert((tx as usize / BLOCK_TILES, tz as usize / BLOCK_TILES));
        }
    };
    for s in &events.signs {
        add(s.x as f32, s.z as f32);
    }
    for t in &events.triggers {
        add(t.x as f32, t.z as f32);
    }
    for w in &events.warps {
        add(w.x as f32 / TILE, w.z as f32 / TILE);
    }
    for c in &events.characters {
        add(c.x as f32, c.z as f32);
    }
    let mut blocks: Vec<usize> = Vec::new();
    for (column, row) in cells {
        if column < layout.width && row < layout.height {
            let id = layout.ids[row * layout.width + column] as usize;
            if id < 857 && !blocks.contains(&id) {
                blocks.push(id);
            }
        }
    }
    blocks
}

/// The maps that use a matrix.
pub fn maps_of_matrix(arc_dir: &str, matrix: usize) -> Result<Vec<usize>, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let mut found = Vec::new();
    for (map, file) in maps.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = map_parts(raw) else { continue };
        if parts.first().filter(|p| p.len() >= 6).map_or(false, |p| u16_at(p, 4) as usize == matrix) {
            found.push(map);
        }
    }
    Ok(found)
}

/// The events of a map, as the game has them.
pub fn read_map_events(arc_dir: &str, map: usize) -> Result<MapEvents, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let raw = maps.get(map).and_then(|f| f.first()).ok_or("no such map")?;
    let parts = map_parts(raw)?;
    MapEvents::parse(parts.get(1).ok_or("the map has no events")?)
}

/// For each number of model: the character of the game that uses it most
/// often (with the same movement and the same kind of interaction). The tool
/// gives a new character these values.
pub fn character_templates(arc_dir: &str) -> Result<BTreeMap<u16, Character>, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let mut counts: BTreeMap<(u16, u16, u16, u8), (usize, Character)> = BTreeMap::new();
    for file in &maps {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = map_parts(raw) else { continue };
        let Some(Ok(events)) = parts.get(1).map(|p| MapEvents::parse(p)) else { continue };
        for c in events.characters {
            let key = (c.model, c.movement, c.interaction, c.facing);
            counts.entry(key).and_modify(|e| e.0 += 1).or_insert((1, c));
        }
    }
    let mut best: BTreeMap<u16, (usize, Character)> = BTreeMap::new();
    for ((model, ..), (n, c)) in counts {
        if best.get(&model).map_or(true, |(m, _)| n > *m) {
            best.insert(model, (n, c));
        }
    }
    Ok(best.into_iter().map(|(m, (_, c))| (m, c)).collect())
}

/// Reads the events from the file of a map that the tool wrote on the SD
/// card, if there is one.
pub fn read_map_override(sd_dir: &str, map: usize) -> Option<MapEvents> {
    let raw = std::fs::read(PathBuf::from(sd_dir).join("zone").join(format!("{map}.bin"))).ok()?;
    let parts = map_parts(&raw).ok()?;
    MapEvents::parse(parts.get(1)?).ok()
}

/// Writes the pack of a map with new events in the folder `map` of the SD
/// card (the plugin gives it to the game in place of the pack of the archive).
pub fn write_map_override(arc_dir: &str, sd_dir: &str, map: usize, events: &MapEvents) -> Result<String, String> {
    let maps = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Maps))?;
    let raw = maps.get(map).and_then(|f| f.first()).ok_or("no such map")?;
    let compressed = raw.first() == Some(&0x11);
    let pack_bytes = unpack(raw)?;
    let pack_new = pack::replace_part_with(&pack_bytes, 1, &events.to_bytes(), 4)?;
    let bytes = if compressed { lz11::compress(&pack_new)? } else { pack_new };
    let folder = PathBuf::from(sd_dir).join("zone");
    std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
    let path = folder.join(format!("{map}.bin"));
    std::fs::write(&path, &bytes).map_err(|e| format!("{}: {e}", path.display()))?;
    crate::edit::write_index(&folder, map, bytes.len())?;
    Ok(format!("{} ({} bytes)", path.display(), bytes.len()))
}

/// Gives the map its events of the game again: the file of the map leaves the
/// SD card, and the lines of the map leave `map_events.txt`.
pub fn reset_map(sd_dir: &str, map: usize) -> Result<String, String> {
    let folder = PathBuf::from(sd_dir).join("zone");
    let file = folder.join(format!("{map}.bin"));
    let mut done = Vec::new();
    if file.exists() {
        std::fs::remove_file(&file).map_err(|e| format!("{}: {e}", file.display()))?;
        crate::edit::remove_index(&folder, map)?;
        done.push("the file of the map");
    }
    let path = file_path(sd_dir);
    if let Ok(old) = std::fs::read_to_string(&path) {
        let empty = MapEvents::default();
        let text = merge_file(&old, map, &empty, &empty);
        if text.lines().any(|l| !l.starts_with('#')) {
            std::fs::write(&path, text).map_err(|e| format!("{}: {e}", path.display()))?;
        } else {
            let _ = std::fs::remove_file(&path);
        }
        if old.lines().filter_map(parse_line).any(|l| l.map == map) {
            done.push("the lines of map_events.txt");
        }
    }
    Ok(if done.is_empty() { "there was nothing to remove".to_string() } else { format!("removed {}", done.join(" and ")) })
}

/// The words of a line of the file, from its number of map.
struct Line {
    map: usize,
    kind: char,
    add: bool,
    index: usize,
    fields: Vec<i64>,
}

fn parse_line(line: &str) -> Option<Line> {
    let mut words = line.split_whitespace();
    let map = words.next()?.parse().ok()?;
    let tag = words.next()?;
    let mut chars = tag.chars();
    let (kind, mode) = (chars.next()?, chars.next()?);
    if !matches!(kind, 'S' | 'T' | 'W' | 'C') || !matches!(mode, '+' | '=') {
        return None;
    }
    let index = words.next()?.parse().ok()?;
    let fields: Vec<i64> = words.filter_map(|w| w.parse().ok()).collect();
    Some(Line { map, kind, add: mode == '+', index, fields })
}

fn sign_from(f: &[i64]) -> Option<Sign> {
    (f.len() >= 8).then(|| Sign {
        raw: Vec::new(),
        x: f[0] as i16,
        z: f[1] as i16,
        width: f[2] as u16,
        depth: f[3] as u16,
        height: f[4] as i32,
        id: f[5] as u16,
        facing: f[6] as u8,
        kind: f[7] as u16,
    })
}

fn trigger_from(f: &[i64]) -> Option<Trigger> {
    (f.len() >= 9).then(|| Trigger {
        raw: Vec::new(),
        x: f[0] as u16,
        z: f[1] as u16,
        width: f[2] as u16,
        depth: f[3] as u16,
        height: f[4] as i16,
        id: f[5] as u16,
        work_id: f[6] as u16,
        param: f[7] as u16,
        check_kind: f[8] as u16,
    })
}

fn warp_from(f: &[i64]) -> Option<Warp> {
    (f.len() >= 9).then(|| Warp {
        raw: Vec::new(),
        x: f[0] as i16,
        y: f[1] as i16,
        z: f[2] as i16,
        width: f[3] as u16,
        depth: f[4] as u16,
        map: f[5] as u16,
        exit: f[6] as u16,
        facing: f[7] as u8,
        kind: f[8] as u8,
    })
}

fn apply_character(c: &mut Character, f: &[i64]) -> bool {
    if f.len() < 16 {
        return false;
    }
    c.x = f[0] as u16;
    c.z = f[1] as u16;
    c.height = f[2] as f32;
    c.facing = f[3] as u8;
    c.script = f[4] as u16;
    c.movement = f[5] as u16;
    c.interaction = f[6] as u16;
    c.param = [f[7] as u16, f[8] as u16, f[9] as u16];
    c.hide_flag = f[10] as u16;
    c.model = f[11] as u16;
    c.trigger = [f[12] as i16, f[13] as i16, f[14] as i16, f[15] as i16];
    true
}

/// A character that does not come from the game: the same fields as a record
/// of the game, with the usual values.
pub fn new_character(map: u16, local_id: u16, x: u16, z: u16, height: f32) -> Character {
    Character {
        raw: Vec::new(),
        local_id,
        model: 0,
        movement: 0,
        interaction: 0,
        hide_flag: 0,
        script: 65535,
        facing: 0,
        param: [0; 3],
        trigger: [0; 4],
        maps: [map; 3],
        alias: 65535,
        position_kind: 0,
        x,
        z,
        height,
    }
}

/// Applies the lines of the file that are for a map to its events.
pub fn apply_file(map: usize, events: &mut MapEvents, text: &str) {
    let shipped = (events.signs.len(), events.triggers.len(), events.warps.len(), events.characters.len());
    for line in text.lines().filter_map(parse_line).filter(|l| l.map == map) {
        match (line.kind, line.add) {
            ('S', false) if line.index < shipped.0 => {
                if let Some(mut s) = sign_from(&line.fields) {
                    // The bytes of the record that the file does not give stay.
                    s.raw = events.signs[line.index].raw.clone();
                    events.signs[line.index] = s;
                }
            }
            ('S', true) => {
                if let Some(s) = sign_from(&line.fields) {
                    events.signs.push(s);
                }
            }
            ('T', false) if line.index < shipped.1 => {
                if let Some(mut t) = trigger_from(&line.fields) {
                    t.raw = events.triggers[line.index].raw.clone();
                    events.triggers[line.index] = t;
                }
            }
            ('T', true) => {
                if let Some(t) = trigger_from(&line.fields) {
                    events.triggers.push(t);
                }
            }
            ('W', false) if line.index < shipped.2 => {
                if let Some(mut w) = warp_from(&line.fields) {
                    w.raw = events.warps[line.index].raw.clone();
                    events.warps[line.index] = w;
                }
            }
            ('W', true) => {
                if let Some(w) = warp_from(&line.fields) {
                    events.warps.push(w);
                }
            }
            ('C', false) if line.index < shipped.3 => {
                apply_character(&mut events.characters[line.index], &line.fields);
            }
            ('C', true) => {
                let mut c = new_character(map as u16, events.characters.len() as u16, 0, 0, 0.0);
                if apply_character(&mut c, &line.fields) {
                    events.characters.push(c);
                }
            }
            _ => {}
        }
    }
}

fn sign_line(map: usize, add: bool, index: usize, s: &Sign) -> String {
    format!("{map} S{} {index} {} {} {} {} {} {} {} {}", if add { '+' } else { '=' }, s.x, s.z, s.width, s.depth, s.height, s.id, s.facing, s.kind)
}

fn trigger_line(map: usize, add: bool, index: usize, t: &Trigger) -> String {
    format!(
        "{map} T{} {index} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' }, t.x, t.z, t.width, t.depth, t.height, t.id, t.work_id, t.param, t.check_kind
    )
}

fn warp_line(map: usize, add: bool, index: usize, w: &Warp) -> String {
    format!(
        "{map} W{} {index} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' }, w.x, w.y, w.z, w.width, w.depth, w.map, w.exit, w.facing, w.kind
    )
}

fn character_line(map: usize, add: bool, index: usize, c: &Character) -> String {
    format!(
        "{map} C{} {index} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' },
        c.x, c.z, c.height as i32, c.facing, c.script, c.movement, c.interaction, c.param[0], c.param[1], c.param[2],
        c.hide_flag, c.model, c.trigger[0], c.trigger[1], c.trigger[2], c.trigger[3]
    )
}

/// The text of the file with the changes of a map: the lines of the other
/// maps stay, and the lines of this map are written again from the
/// difference between the events of the game (`base`) and the events now.
pub fn merge_file(old: &str, map: usize, base: &MapEvents, now: &MapEvents) -> String {
    let mut out = String::from("# Sango map events. map S+|S=|T+|T=|W+|W=|C= index fields. See the docs.\n");
    for line in old.lines() {
        if line.trim().is_empty() || line.starts_with('#') {
            continue;
        }
        // The lines of this map are written again below; the other lines stay.
        if parse_line(line).map_or(true, |l| l.map != map) {
            out.push_str(line);
            out.push('\n');
        }
    }
    let mut lines: Vec<String> = Vec::new();
    for (i, s) in now.signs.iter().enumerate() {
        if i >= base.signs.len() {
            lines.push(sign_line(map, true, i - base.signs.len(), s));
        } else if *s != base.signs[i] {
            lines.push(sign_line(map, false, i, s));
        }
    }
    for (i, t) in now.triggers.iter().enumerate() {
        if i >= base.triggers.len() {
            lines.push(trigger_line(map, true, i - base.triggers.len(), t));
        } else if *t != base.triggers[i] {
            lines.push(trigger_line(map, false, i, t));
        }
    }
    for (i, w) in now.warps.iter().enumerate() {
        if i >= base.warps.len() {
            lines.push(warp_line(map, true, i - base.warps.len(), w));
        } else if *w != base.warps[i] {
            lines.push(warp_line(map, false, i, w));
        }
    }
    for (i, c) in now.characters.iter().enumerate() {
        if i >= base.characters.len() {
            lines.push(character_line(map, true, i - base.characters.len(), c));
        } else if *c != base.characters[i] {
            lines.push(character_line(map, false, i, c));
        }
    }
    for l in lines {
        out.push_str(&l);
        out.push('\n');
    }
    out
}

/// The place of the file in the folder of the SD card.
pub fn file_path(sd_dir: &str) -> PathBuf {
    PathBuf::from(sd_dir).join("map_events.txt")
}

/// The place in a block (units, from the middle of the block) of a tile of the
/// map, for the block that is at (`column`, `row`) of the matrix.
pub fn tile_to_local(tile: f32, block_index: usize) -> f32 {
    (tile - (block_index * BLOCK_TILES) as f32) * TILE - 360.0
}

/// The tile of the map that is at a place of the block.
pub fn local_to_tile(local: f32, block_index: usize) -> f32 {
    (local + 360.0) / TILE + (block_index * BLOCK_TILES) as f32
}
