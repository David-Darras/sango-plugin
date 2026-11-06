//! Loads the data of the game that the tool shows.

use std::collections::{HashMap, HashSet};
use std::path::PathBuf;

use crate::bch::{Bch, Mesh, Texture};
use crate::{garc, lz11, pack};

/// The part of a block that holds the model of the ground and the objects.
const BLOCK_MODEL_PART: usize = 1;
/// The part of a block that holds the model of the tall grass of the
/// encounters. Many blocks have no such model.
const BLOCK_GRASS_PART: usize = 5;
/// The part of an area pack that holds the pictures.
const AREA_TEXTURE_PART: usize = 11;

/// A block with the pictures that its meshes need.
pub struct Scene {
    pub meshes: Vec<Mesh>,
    /// The pictures by name.
    pub textures: HashMap<String, Texture>,
    pub area: Option<usize>,
}

fn open_pack(raw: &[u8]) -> Result<Vec<Vec<u8>>, String> {
    let data = if raw.first() == Some(&0x11) { lz11::decompress(raw)? } else { raw.to_vec() };
    let (_, parts) = pack::split(&data)?;
    Ok(parts.into_iter().map(|p| p.to_vec()).collect())
}

/// Reads the parts of one block of the land archive.
pub fn read_block(arc_dir: &str, block: usize) -> Result<Vec<Vec<u8>>, String> {
    let files = garc::read(&PathBuf::from(arc_dir).join("map_land_data.garc"))?;
    let raw = files.get(block).and_then(|f| f.first()).ok_or("no such block")?;
    open_pack(raw)
}

/// The meshes of the model of a block.
pub fn block_meshes(parts: &[Vec<u8>]) -> Result<Vec<Mesh>, String> {
    let part = parts.get(BLOCK_MODEL_PART).ok_or("the block has no model")?;
    let mut meshes = Bch::parse(part)?.meshes()?;
    if let Some(grass) = parts.get(BLOCK_GRASS_PART).filter(|p| p.starts_with(b"BCH\0")) {
        for mut mesh in Bch::parse(grass)?.meshes()? {
            mesh.encounter = true;
            meshes.push(mesh);
        }
    }
    Ok(meshes)
}

/// The name of the picture of a mesh. When the material does not name one,
/// the name of the material is used without the marks that the tools add.
pub fn picture_of(mesh: &Mesh) -> &str {
    if mesh.texture.is_empty() {
        mesh.material.split(['@', '-']).next().unwrap_or(&mesh.material)
    } else {
        &mesh.texture
    }
}

/// Finds the area pack that has most of the pictures that the meshes need.
fn find_area(files: &[Vec<Vec<u8>>], wanted: &HashSet<&str>) -> Option<usize> {
    let mut best: Option<(usize, usize)> = None;
    for (index, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = open_pack(raw) else { continue };
        let Some(part) = parts.get(AREA_TEXTURE_PART) else { continue };
        let Ok(model) = Bch::parse(part) else { continue };
        let names = model.names();
        let hits = names.iter().filter(|n| wanted.contains(n.as_str())).count();
        if best.map_or(true, |(h, _)| hits > h) {
            best = Some((hits, index));
        }
    }
    best.filter(|(hits, _)| *hits > 0).map(|(_, i)| i)
}

/// The meshes of the model of several blocks (the archive is read once).
pub fn blocks_meshes(arc_dir: &str, ids: &[usize]) -> Result<Vec<(usize, Vec<Mesh>)>, String> {
    let files = garc::read(&PathBuf::from(arc_dir).join("map_land_data.garc"))?;
    let mut out = Vec::new();
    for &id in ids {
        let Some(raw) = files.get(id).and_then(|f| f.first()) else { continue };
        let Ok(parts) = open_pack(raw) else { continue };
        if let Ok(meshes) = block_meshes(&parts) {
            out.push((id, meshes));
        }
    }
    Ok(out)
}

/// The names of the pictures of every area pack, in the order of the archive.
pub fn area_pictures(arc_dir: &str) -> Result<Vec<HashSet<String>>, String> {
    let files = garc::read(&PathBuf::from(arc_dir).join("field_area_bin.garc"))?;
    Ok(files
        .iter()
        .map(|file| {
            let names = file
                .first()
                .and_then(|raw| open_pack(raw).ok())
                .and_then(|parts| parts.get(AREA_TEXTURE_PART).and_then(|p| Bch::parse(p).ok().map(|m| m.names())));
            names.unwrap_or_default().into_iter().collect()
        })
        .collect())
}

/// The pictures of an area as the tool can copy them (see `bch_edit`), for the
/// names that are asked.
pub fn area_texture_blobs(arc_dir: &str, area: usize, names: &[String]) -> Result<Vec<crate::bch_edit::TextureBlob>, String> {
    let areas = garc::read(&PathBuf::from(arc_dir).join("field_area_bin.garc"))?;
    let raw = areas.get(area).and_then(|f| f.first()).ok_or("no such area")?;
    let parts = open_pack(raw)?;
    let part = parts.get(AREA_TEXTURE_PART).ok_or("the area has no pictures")?;
    crate::bch_edit::read_textures(part, names)
}

/// The area pack as the archive has it: the data (not compressed) and whether
/// the archive compresses it.
pub fn area_pack(arc_dir: &str, area: usize) -> Result<(Vec<u8>, bool), String> {
    let areas = garc::read(&PathBuf::from(arc_dir).join("field_area_bin.garc"))?;
    let raw = areas.get(area).and_then(|f| f.first()).ok_or("no such area")?;
    if raw.first() == Some(&0x11) { Ok((lz11::decompress(raw)?, true)) } else { Ok((raw.clone(), false)) }
}

/// The part of an area pack that holds the pictures.
pub const AREA_PICTURES: usize = AREA_TEXTURE_PART;

/// The pictures of one area pack, by name.
pub fn area_textures(arc_dir: &str, area: usize) -> Result<HashMap<String, Texture>, String> {
    let areas = garc::read(&PathBuf::from(arc_dir).join("field_area_bin.garc"))?;
    let raw = areas.get(area).and_then(|f| f.first()).ok_or("no such area")?;
    let parts = open_pack(raw)?;
    let part = parts.get(AREA_TEXTURE_PART).ok_or("the area has no pictures")?;
    Ok(Bch::parse(part)?.textures()?.into_iter().map(|t| (t.name.clone(), t)).collect())
}

/// Loads a block with its pictures. Without an area number, the tool looks
/// for the area pack that fits.
pub fn load_scene(arc_dir: &str, block: usize, area: Option<usize>) -> Result<Scene, String> {
    load_scene_from(arc_dir, block, area, None)
}

/// Like `load_scene`. With a file, the block comes from this file (a block
/// that the tool wrote) and not from the archive.
pub fn load_scene_from(
    arc_dir: &str,
    block: usize,
    area: Option<usize>,
    land_file: Option<&str>,
) -> Result<Scene, String> {
    let parts = match land_file {
        Some(path) => open_pack(&std::fs::read(path).map_err(|e| format!("{path}: {e}"))?)?,
        None => read_block(arc_dir, block)?,
    };
    let meshes = block_meshes(&parts)?;
    let areas = garc::read(&PathBuf::from(arc_dir).join("field_area_bin.garc"))?;
    let area = area.or_else(|| {
        let wanted: HashSet<&str> = meshes.iter().map(|m| picture_of(m)).collect();
        find_area(&areas, &wanted)
    });
    let mut textures = HashMap::new();
    if let Some(index) = area {
        let raw = areas.get(index).and_then(|f| f.first()).ok_or("no such area")?;
        let area_parts = open_pack(raw)?;
        let part = area_parts.get(AREA_TEXTURE_PART).ok_or("the area has no pictures")?;
        for texture in Bch::parse(part)?.textures()? {
            textures.insert(texture.name.clone(), texture);
        }
    }
    Ok(Scene { meshes, textures, area })
}

/// The attributes of the tiles of all the blocks, with how many tiles have
/// each, from the most common. The tool offers them to the brush.
pub fn attribute_values(arc_dir: &str) -> Result<Vec<(u32, usize)>, String> {
    let files = garc::read(&PathBuf::from(arc_dir).join("map_land_data.garc"))?;
    let mut counts: HashMap<u32, usize> = HashMap::new();
    for file in &files {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = open_pack(raw) else { continue };
        let Some(grid) = parts.first() else { continue };
        if grid.len() < 4 {
            continue;
        }
        let cells = u16::from_le_bytes([grid[0], grid[1]]) as usize * u16::from_le_bytes([grid[2], grid[3]]) as usize;
        for i in 0..cells.min((grid.len() - 4) / 4) {
            let value = u32::from_le_bytes([grid[4 + 4 * i], grid[5 + 4 * i], grid[6 + 4 * i], grid[7 + 4 * i]]);
            *counts.entry(value).or_default() += 1;
        }
    }
    let mut list: Vec<(u32, usize)> = counts.into_iter().collect();
    list.sort_by_key(|(value, n)| (std::cmp::Reverse(*n), *value));
    Ok(list)
}

/// The parts of a block from the bytes of its file.
pub fn read_block_from_bytes(raw: &[u8]) -> Result<Vec<Vec<u8>>, String> {
    open_pack(raw)
}
