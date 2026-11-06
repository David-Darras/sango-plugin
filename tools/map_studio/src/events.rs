//! The events of a zone: signs, characters, warps and triggers.
//!
//! The game keeps the events of a zone in the part 1 of the zone pack. The
//! part starts with 12 bytes (the size, the number of signs, characters, warps
//! and triggers, and 4 bytes that are not used). The records follow in this
//! order, and each kind has its own size. A tile is 18 units wide, and the
//! tiles are counted from the corner of the zone.
//!
//! The plugin reads the file `map_events.txt` of the SD card. A line starts
//! with the number of the zone. `S=`, `T=`, `W=` and `C=` change a record of the
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

/// A warp: a place that takes the player to another zone.
#[derive(Clone, Debug, PartialEq)]
pub struct Warp {
    /// The bytes of the record in the game (the bytes that the tool does not
    /// know stay the same when the record is written again).
    pub raw: Vec<u8>,
    pub zone: u16,
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
pub struct ZoneEvents {
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

impl ZoneEvents {
    /// The part 1 of a zone pack: the head with the new numbers, the records,
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
            put16(&mut b, 0, w.zone);
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

    /// Reads the part 1 of a zone pack.
    pub fn parse(part: &[u8]) -> Result<ZoneEvents, String> {
        if part.len() < 12 {
            return Err("the events of the zone are too short".into());
        }
        let (signs, characters, warps, triggers) = (part[4] as usize, part[5] as usize, part[6] as usize, part[7] as usize);
        let needed = 12 + 20 * signs + 48 * characters + 24 * warps + 24 * triggers;
        if part.len() < needed {
            return Err("the events of the zone are cut".into());
        }
        let mut at = 12;
        let mut out = ZoneEvents::default();
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
                zone: u16_at(part, at),
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

/// A zone that has the block, and where.
#[derive(Clone, Debug)]
pub struct ZoneAt {
    pub zone: usize,
    pub matrix: usize,
    pub column: usize,
    pub row: usize,
    pub width: usize,
    pub height: usize,
}

impl ZoneAt {
    /// The tile of the zone that is the first tile of the block.
    pub fn first_tile(&self) -> (i32, i32) {
        ((self.column * BLOCK_TILES) as i32, (self.row * BLOCK_TILES) as i32)
    }
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

/// The parts of the pack of a zone.
fn zone_parts(raw: &[u8]) -> Result<Vec<Vec<u8>>, String> {
    let data = unpack(raw)?;
    let (_, parts) = pack::split(&data)?;
    Ok(parts.into_iter().map(|p| p.to_vec()).collect())
}

/// The zones that have a block in their matrix.
pub fn zones_of_block(arc_dir: &str, block: usize) -> Result<Vec<ZoneAt>, String> {
    let zones = garc::read(&PathBuf::from(arc_dir).join("zonedata_y.garc"))?;
    let matrices = garc::read(&PathBuf::from(arc_dir).join("map_matrix.garc"))?;
    let mut cache: BTreeMap<usize, Option<(usize, usize, Vec<u16>)>> = BTreeMap::new();
    let mut found = Vec::new();
    for (zone, file) in zones.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = zone_parts(raw) else { continue };
        let Some(settings) = parts.first().filter(|p| p.len() >= 6) else { continue };
        let matrix = u16_at(settings, 4) as usize;
        let entry = cache.entry(matrix).or_insert_with(|| matrices.get(matrix).and_then(|f| f.first()).and_then(|r| matrix_blocks(r).ok()));
        let Some((width, height, ids)) = entry else { continue };
        if let Some(at) = ids.iter().position(|&b| b as usize == block) {
            found.push(ZoneAt { zone, matrix, column: at % *width, row: at / *width, width: *width, height: *height });
        }
    }
    Ok(found)
}

/// The events of a zone, as the game has them.
pub fn read_zone_events(arc_dir: &str, zone: usize) -> Result<ZoneEvents, String> {
    let zones = garc::read(&PathBuf::from(arc_dir).join("zonedata_y.garc"))?;
    let raw = zones.get(zone).and_then(|f| f.first()).ok_or("no such zone")?;
    let parts = zone_parts(raw)?;
    ZoneEvents::parse(parts.get(1).ok_or("the zone has no events")?)
}

/// For each number of model: the character of the game that uses it most
/// often (with the same movement and the same kind of interaction). The tool
/// gives a new character these values.
pub fn character_templates(arc_dir: &str) -> Result<BTreeMap<u16, Character>, String> {
    let zones = garc::read(&PathBuf::from(arc_dir).join("zonedata_y.garc"))?;
    let mut counts: BTreeMap<(u16, u16, u16, u8), (usize, Character)> = BTreeMap::new();
    for file in &zones {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = zone_parts(raw) else { continue };
        let Some(Ok(events)) = parts.get(1).map(|p| ZoneEvents::parse(p)) else { continue };
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

/// Reads the events from the file of a zone that the tool wrote on the SD
/// card, if there is one.
pub fn read_zone_override(sd_dir: &str, zone: usize) -> Option<ZoneEvents> {
    let raw = std::fs::read(PathBuf::from(sd_dir).join("zone").join(format!("{zone}.bin"))).ok()?;
    let parts = zone_parts(&raw).ok()?;
    ZoneEvents::parse(parts.get(1)?).ok()
}

/// Writes the pack of a zone with new events in the folder `zone` of the SD
/// card (the plugin gives it to the game in place of the pack of the archive).
pub fn write_zone_override(arc_dir: &str, sd_dir: &str, zone: usize, events: &ZoneEvents) -> Result<String, String> {
    let zones = garc::read(&PathBuf::from(arc_dir).join("zonedata_y.garc"))?;
    let raw = zones.get(zone).and_then(|f| f.first()).ok_or("no such zone")?;
    let compressed = raw.first() == Some(&0x11);
    let pack_bytes = unpack(raw)?;
    let pack_new = pack::replace_part_with(&pack_bytes, 1, &events.to_bytes(), 4)?;
    let bytes = if compressed { lz11::compress(&pack_new)? } else { pack_new };
    let folder = PathBuf::from(sd_dir).join("zone");
    std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
    let path = folder.join(format!("{zone}.bin"));
    std::fs::write(&path, &bytes).map_err(|e| format!("{}: {e}", path.display()))?;
    crate::edit::write_index(&folder, zone, bytes.len())?;
    Ok(format!("{} ({} bytes)", path.display(), bytes.len()))
}

/// Gives the zone its events of the game again: the file of the zone leaves the
/// SD card, and the lines of the zone leave `map_events.txt`.
pub fn reset_zone(sd_dir: &str, zone: usize) -> Result<String, String> {
    let folder = PathBuf::from(sd_dir).join("zone");
    let file = folder.join(format!("{zone}.bin"));
    let mut done = Vec::new();
    if file.exists() {
        std::fs::remove_file(&file).map_err(|e| format!("{}: {e}", file.display()))?;
        crate::edit::remove_index(&folder, zone)?;
        done.push("the file of the zone");
    }
    let path = file_path(sd_dir);
    if let Ok(old) = std::fs::read_to_string(&path) {
        let empty = ZoneEvents::default();
        let text = merge_file(&old, zone, &empty, &empty);
        if text.lines().any(|l| !l.starts_with('#')) {
            std::fs::write(&path, text).map_err(|e| format!("{}: {e}", path.display()))?;
        } else {
            let _ = std::fs::remove_file(&path);
        }
        if old.lines().filter_map(parse_line).any(|l| l.zone == zone) {
            done.push("the lines of map_events.txt");
        }
    }
    Ok(if done.is_empty() { "there was nothing to remove".to_string() } else { format!("removed {}", done.join(" and ")) })
}

/// The words of a line of the file, from its number of zone.
struct Line {
    zone: usize,
    kind: char,
    add: bool,
    index: usize,
    fields: Vec<i64>,
}

fn parse_line(line: &str) -> Option<Line> {
    let mut words = line.split_whitespace();
    let zone = words.next()?.parse().ok()?;
    let tag = words.next()?;
    let mut chars = tag.chars();
    let (kind, mode) = (chars.next()?, chars.next()?);
    if !matches!(kind, 'S' | 'T' | 'W' | 'C') || !matches!(mode, '+' | '=') {
        return None;
    }
    let index = words.next()?.parse().ok()?;
    let fields: Vec<i64> = words.filter_map(|w| w.parse().ok()).collect();
    Some(Line { zone, kind, add: mode == '+', index, fields })
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
        zone: f[5] as u16,
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
pub fn new_character(zone: u16, local_id: u16, x: u16, z: u16, height: f32) -> Character {
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
        maps: [zone; 3],
        alias: 65535,
        position_kind: 0,
        x,
        z,
        height,
    }
}

/// Applies the lines of the file that are for a zone to its events.
pub fn apply_file(zone: usize, events: &mut ZoneEvents, text: &str) {
    let shipped = (events.signs.len(), events.triggers.len(), events.warps.len(), events.characters.len());
    for line in text.lines().filter_map(parse_line).filter(|l| l.zone == zone) {
        match (line.kind, line.add) {
            ('S', false) if line.index < shipped.0 => {
                if let Some(s) = sign_from(&line.fields) {
                    events.signs[line.index] = s;
                }
            }
            ('S', true) => {
                if let Some(s) = sign_from(&line.fields) {
                    events.signs.push(s);
                }
            }
            ('T', false) if line.index < shipped.1 => {
                if let Some(t) = trigger_from(&line.fields) {
                    events.triggers[line.index] = t;
                }
            }
            ('T', true) => {
                if let Some(t) = trigger_from(&line.fields) {
                    events.triggers.push(t);
                }
            }
            ('W', false) if line.index < shipped.2 => {
                if let Some(w) = warp_from(&line.fields) {
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
                let mut c = new_character(zone as u16, events.characters.len() as u16, 0, 0, 0.0);
                if apply_character(&mut c, &line.fields) {
                    events.characters.push(c);
                }
            }
            _ => {}
        }
    }
}

fn sign_line(zone: usize, add: bool, index: usize, s: &Sign) -> String {
    format!("{zone} S{} {index} {} {} {} {} {} {} {} {}", if add { '+' } else { '=' }, s.x, s.z, s.width, s.depth, s.height, s.id, s.facing, s.kind)
}

fn trigger_line(zone: usize, add: bool, index: usize, t: &Trigger) -> String {
    format!(
        "{zone} T{} {index} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' }, t.x, t.z, t.width, t.depth, t.height, t.id, t.work_id, t.param, t.check_kind
    )
}

fn warp_line(zone: usize, add: bool, index: usize, w: &Warp) -> String {
    format!(
        "{zone} W{} {index} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' }, w.x, w.y, w.z, w.width, w.depth, w.zone, w.exit, w.facing, w.kind
    )
}

fn character_line(zone: usize, add: bool, index: usize, c: &Character) -> String {
    format!(
        "{zone} C{} {index} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
        if add { '+' } else { '=' },
        c.x, c.z, c.height as i32, c.facing, c.script, c.movement, c.interaction, c.param[0], c.param[1], c.param[2],
        c.hide_flag, c.model, c.trigger[0], c.trigger[1], c.trigger[2], c.trigger[3]
    )
}

/// The text of the file with the changes of a zone: the lines of the other
/// zones stay, and the lines of this zone are written again from the
/// difference between the events of the game (`base`) and the events now.
pub fn merge_file(old: &str, zone: usize, base: &ZoneEvents, now: &ZoneEvents) -> String {
    let mut out = String::from("# Sango map events. map S+|S=|T+|T=|W+|W=|C= index fields. See the docs.\n");
    for line in old.lines() {
        if line.trim().is_empty() || line.starts_with('#') {
            continue;
        }
        // The lines of this zone are written again below; the other lines stay.
        if parse_line(line).map_or(true, |l| l.zone != zone) {
            out.push_str(line);
            out.push('\n');
        }
    }
    let mut lines: Vec<String> = Vec::new();
    for (i, s) in now.signs.iter().enumerate() {
        if i >= base.signs.len() {
            lines.push(sign_line(zone, true, i - base.signs.len(), s));
        } else if *s != base.signs[i] {
            lines.push(sign_line(zone, false, i, s));
        }
    }
    for (i, t) in now.triggers.iter().enumerate() {
        if i >= base.triggers.len() {
            lines.push(trigger_line(zone, true, i - base.triggers.len(), t));
        } else if *t != base.triggers[i] {
            lines.push(trigger_line(zone, false, i, t));
        }
    }
    for (i, w) in now.warps.iter().enumerate() {
        if i >= base.warps.len() {
            lines.push(warp_line(zone, true, i - base.warps.len(), w));
        } else if *w != base.warps[i] {
            lines.push(warp_line(zone, false, i, w));
        }
    }
    for (i, c) in now.characters.iter().enumerate() {
        if i >= base.characters.len() {
            lines.push(character_line(zone, true, i - base.characters.len(), c));
        } else if *c != base.characters[i] {
            lines.push(character_line(zone, false, i, c));
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
/// zone, for the block that is at (`column`, `row`) of the matrix.
pub fn tile_to_local(tile: f32, block_index: usize) -> f32 {
    (tile - (block_index * BLOCK_TILES) as f32) * TILE - 360.0
}

/// The tile of the zone that is at a place of the block.
pub fn local_to_tile(local: f32, block_index: usize) -> f32 {
    (local + 360.0) / TILE + (block_index * BLOCK_TILES) as f32
}
