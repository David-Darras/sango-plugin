//! The catalogue of the objects of the game: every tree, bush, building of
//! every block, one time for each different model.

use std::collections::BTreeMap;

use crate::edit::Portable;
use crate::editor::Editor;
use crate::garc;

/// The shape of one mesh of an object, to draw a small picture of it.
#[derive(Clone, Debug)]
pub struct PreviewPart {
    /// The name of the picture of the mesh.
    pub picture: String,
    pub positions: Vec<[f32; 3]>,
    pub uvs: Vec<[f32; 2]>,
    /// Three numbers for each triangle.
    pub indices: Vec<u32>,
}

/// One model of the catalogue.
pub struct Entry {
    /// A name for the list: the meshes that it is made of.
    pub name: String,
    pub portable: Portable,
    pub preview: Vec<PreviewPart>,
    /// The tileset pack that has the pictures of the model, if one was found.
    pub tileset: Option<usize>,
    /// The block where the model was taken, and how many times the models of
    /// all the blocks look like it.
    pub donor: usize,
    pub occurrences: usize,
    /// How many different shapes the objects of this name have.
    pub variants: usize,
    /// The blocks that have it.
    pub blocks: Vec<usize>,
}

/// What makes two objects the same model: the meshes with their numbers of
/// vertices and triangles, and the size of the box.
fn signature(object: &Portable) -> String {
    let mut parts: Vec<String> = object
        .parts
        .iter()
        .map(|p| format!("{}:{}:{}", p.material, p.vertices.len() / p.stride.max(1), p.indices.len()))
        .collect();
    parts.sort();
    format!(
        "{}|{:.0}x{:.0}x{:.0}",
        parts.join("+"),
        object.high[0] - object.low[0],
        object.high[1] - object.low[1],
        object.high[2] - object.low[2]
    )
}

/// Builds the catalogue of all the blocks, or of the blocks that are given.
/// All the objects that have the same meshes make one entry: the entry shows
/// the shape that is the most common. `progress` is called with the number of
/// the block that is being read.
pub fn build(arc_dir: &str, blocks: Option<&[usize]>, mut progress: impl FnMut(usize)) -> Result<Vec<Entry>, String> {
    let count = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?.len();
    let wanted: Vec<usize> = match blocks {
        Some(list) => list.to_vec(),
        None => (0..count).collect(),
    };
    // For each name: the shapes with how many times each was seen.
    struct Shape {
        seen: usize,
        donor: usize,
        portable: Portable,
        preview: Vec<PreviewPart>,
    }
    struct Group {
        shapes: BTreeMap<String, Shape>,
        blocks: Vec<usize>,
    }
    let mut groups: BTreeMap<String, Group> = BTreeMap::new();
    for block in wanted {
        progress(block);
        let Ok(editor) = Editor::new(arc_dir, block) else { continue };
        for (index, object) in editor.catalog_objects().iter().enumerate() {
            let Ok(portable) = editor.portable_of_catalog(object, index) else { continue };
            if portable.parts.is_empty() || portable.parts.iter().any(|p| p.blob.is_none()) {
                continue;
            }
            let mut names: Vec<&str> = portable.parts.iter().map(|p| p.material.as_str()).collect();
            names.sort();
            let name = names.join(" + ");
            let group = groups.entry(name).or_insert_with(|| Group { shapes: BTreeMap::new(), blocks: Vec::new() });
            if !group.blocks.contains(&block) {
                group.blocks.push(block);
            }
            let key = signature(&portable);
            match group.shapes.get_mut(&key) {
                Some(shape) => shape.seen += 1,
                None => {
                    let preview = editor.preview_of(object);
                    group.shapes.insert(key, Shape { seen: 1, donor: block, portable, preview });
                }
            }
        }
    }
    let tilesets = crate::data::tileset_pictures(arc_dir)?;
    let mut entries = Vec::new();
    for (name, group) in groups {
        let occurrences = group.shapes.values().map(|s| s.seen).sum();
        let variants = group.shapes.len();
        let Some(best) = group.shapes.into_values().max_by_key(|s| s.seen) else { continue };
        let tileset = best_tileset(&tilesets, &best.preview);
        entries.push(Entry { name, portable: best.portable, preview: best.preview, tileset, donor: best.donor, occurrences, variants, blocks: group.blocks });
    }
    Ok(entries)
}

/// Builds the catalogue of the ground tiles of the blocks: each different
/// piece (same picture, same shape, same texture coordinates) once, the most
/// common first.
pub fn build_tiles(arc_dir: &str, blocks: Option<&[usize]>, mut progress: impl FnMut(usize)) -> Result<Vec<Entry>, String> {
    let count = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?.len();
    let wanted: Vec<usize> = match blocks {
        Some(list) => list.to_vec(),
        None => (0..count).collect(),
    };
    struct Seen {
        seen: usize,
        donor: usize,
        portable: Portable,
        preview: Vec<PreviewPart>,
        blocks: Vec<usize>,
    }
    let mut shapes: BTreeMap<String, Seen> = BTreeMap::new();
    for block in wanted {
        progress(block);
        let Ok(editor) = Editor::new(arc_dir, block) else { continue };
        for (index, object) in editor.tile_objects(false).iter().enumerate() {
            let preview = editor.preview_of(object);
            if preview.is_empty() {
                continue;
            }
            // Only the whole tiles: a piece that covers a part of the tile (a
            // border) would leave a hole where it replaces the ground.
            let mut covered = 0.0f32;
            for part in &preview {
                for t in part.indices.chunks_exact(3) {
                    let p = |k: usize| part.positions[t[k] as usize];
                    let (a, b, c) = (p(0), p(1), p(2));
                    covered += ((b[0] - a[0]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[0] - a[0])).abs() / 2.0;
                }
            }
            if covered < 18.0 * 18.0 * 0.95 {
                continue;
            }
            // The tiles of one kind: the same pictures, the same number of
            // vertices and triangles, and about the same slope.
            let rise = ((object.high[1] - object.low[1]) * 2.0).round();
            let mut key = String::new();
            for part in &preview {
                key += &format!("{}|{}|{}|", part.picture, part.positions.len(), part.indices.len());
            }
            key += &format!("{rise}");
            match shapes.get_mut(&key) {
                Some(seen) => {
                    seen.seen += 1;
                    if !seen.blocks.contains(&block) {
                        seen.blocks.push(block);
                    }
                }
                None => {
                    let Ok(portable) = editor.portable_of_catalog(object, index) else { continue };
                    if portable.parts.iter().any(|p| p.blob.is_none()) {
                        continue;
                    }
                    // Grass alone would leave a hole in the ground that it replaces.
                    if portable.parts.iter().all(|p| crate::editor::is_grass_material(&p.material)) {
                        continue;
                    }
                    shapes.insert(key, Seen { seen: 1, donor: block, portable, preview, blocks: vec![block] });
                }
            }
        }
    }
    let tilesets = crate::data::tileset_pictures(arc_dir)?;
    let mut entries: Vec<Entry> = shapes
        .into_values()
        .map(|s| {
            let tileset = best_tileset(&tilesets, &s.preview);
            let name = s.portable.parts.iter().map(|p| p.material.as_str()).collect::<Vec<_>>().join(" + ");
            Entry { name, portable: s.portable, preview: s.preview, tileset, donor: s.donor, occurrences: s.seen, variants: 1, blocks: s.blocks }
        })
        .collect();
    entries.sort_by_key(|e| std::cmp::Reverse(e.occurrences));
    Ok(entries)
}

/// The tileset pack that has most of the pictures that a model needs.
fn best_tileset(tilesets: &[std::collections::HashSet<String>], preview: &[PreviewPart]) -> Option<usize> {
    let mut best: Option<(usize, usize)> = None;
    for (index, names) in tilesets.iter().enumerate() {
        let hits = preview.iter().filter(|p| names.contains(&p.picture)).count();
        if hits > 0 && best.map_or(true, |(h, _)| hits > h) {
            best = Some((hits, index));
        }
    }
    best.map(|(_, i)| i)
}

/// Saves a catalogue in a file, to not build it again.
pub fn save(entries: &[Entry], path: &std::path::Path) -> Result<(), String> {
    let mut out = b"MSCAT3".to_vec();
    let number = |out: &mut Vec<u8>, v: usize| out.extend_from_slice(&(v as u32).to_le_bytes());
    number(&mut out, entries.len());
    for e in entries {
        number(&mut out, e.name.len());
        out.extend_from_slice(e.name.as_bytes());
        number(&mut out, e.donor);
        number(&mut out, e.occurrences);
        number(&mut out, e.variants);
        number(&mut out, e.blocks.len());
        for b in &e.blocks {
            number(&mut out, *b);
        }
        number(&mut out, e.tileset.map_or(u32::MAX as usize, |a| a));
        number(&mut out, e.preview.len());
        for part in &e.preview {
            number(&mut out, part.picture.len());
            out.extend_from_slice(part.picture.as_bytes());
            number(&mut out, part.positions.len());
            for (p, uv) in part.positions.iter().zip(&part.uvs) {
                for v in p.iter().chain(uv) {
                    out.extend_from_slice(&v.to_le_bytes());
                }
            }
            number(&mut out, part.indices.len());
            for i in &part.indices {
                number(&mut out, *i as usize);
            }
        }
        let bytes = e.portable.to_bytes();
        number(&mut out, bytes.len());
        out.extend_from_slice(&bytes);
    }
    if let Some(dir) = path.parent() {
        std::fs::create_dir_all(dir).map_err(|e| e.to_string())?;
    }
    std::fs::write(path, out).map_err(|e| e.to_string())
}

/// Reads a catalogue that `save` wrote.
pub fn load(path: &std::path::Path) -> Result<Vec<Entry>, String> {
    let data = std::fs::read(path).map_err(|e| e.to_string())?;
    if data.get(..6) != Some(b"MSCAT3") {
        return Err("this is not a catalogue (or it is from an older version)".into());
    }
    let position = std::cell::Cell::new(6usize);
    let number = || -> Result<usize, String> {
        let at = position.get();
        let b = data.get(at..at + 4).ok_or("the catalogue is cut")?;
        position.set(at + 4);
        Ok(u32::from_le_bytes([b[0], b[1], b[2], b[3]]) as usize)
    };
    let count = number()?;
    let mut entries = Vec::with_capacity(count);
    for _ in 0..count {
        let name_size = number()?;
        let name_at = position.get();
        position.set(name_at + name_size);
        let name = String::from_utf8_lossy(data.get(name_at..name_at + name_size).ok_or("the catalogue is cut")?).into_owned();
        let donor = number()?;
        let occurrences = number()?;
        let variants = number()?;
        let block_count = number()?;
        let mut blocks = Vec::new();
        for _ in 0..block_count {
            blocks.push(number()?);
        }
        let tileset = number()?;
        let part_count = number()?;
        let mut preview = Vec::new();
        for _ in 0..part_count {
            let size = number()?;
            let at = position.get();
            let picture = String::from_utf8_lossy(data.get(at..at + size).ok_or("the catalogue is cut")?).into_owned();
            position.set(at + size);
            let vertex_count = number()?;
            let mut positions = Vec::with_capacity(vertex_count);
            let mut uvs = Vec::with_capacity(vertex_count);
            for _ in 0..vertex_count {
                let at = position.get();
                let bytes = data.get(at..at + 20).ok_or("the catalogue is cut")?;
                let f = |k: usize| f32::from_le_bytes([bytes[k * 4], bytes[k * 4 + 1], bytes[k * 4 + 2], bytes[k * 4 + 3]]);
                positions.push([f(0), f(1), f(2)]);
                uvs.push([f(3), f(4)]);
                position.set(at + 20);
            }
            let index_count = number()?;
            let mut indices = Vec::with_capacity(index_count);
            for _ in 0..index_count {
                indices.push(number()? as u32);
            }
            preview.push(PreviewPart { picture, positions, uvs, indices });
        }
        let size = number()?;
        let at = position.get();
        let portable = Portable::from_bytes(data.get(at..at + size).ok_or("the catalogue is cut")?)?;
        position.set(at + size);
        entries.push(Entry { name, portable, preview, tileset: (tileset != u32::MAX as usize).then_some(tileset), donor, occurrences, variants, blocks });
    }
    Ok(entries)
}
