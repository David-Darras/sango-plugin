//! The 3D models of the characters of the map (the overworld models).
//!
//! The archive of the character models has one pack for each model: the first part is a
//! model file (BCH) with its pictures. The archive of the parameters says, for the number of
//! model that an event uses, which pack to read. The tool draws the models as
//! they are in the file, without the pose of the skeleton.

use std::collections::HashMap;
use std::path::PathBuf;
use std::rc::Rc;

use crate::bch::{Bch, Texture};
use crate::{garc, lz11, pack};

/// A mesh of a model.
pub struct ModelMesh {
    pub picture: String,
    pub positions: Vec<[f32; 3]>,
    pub uvs: Vec<[f32; 2]>,
    pub indices: Vec<u32>,
}

/// A model with its pictures.
pub struct LoadedModel {
    pub meshes: Vec<ModelMesh>,
    pub textures: HashMap<String, Texture>,
}

/// The models of the archive, read when they are needed.
pub struct ModelBank {
    arc_dir: String,
    files: Option<Vec<Vec<Vec<u8>>>>,
    params: Option<Vec<Vec<Vec<u8>>>>,
    cache: HashMap<u16, Option<Rc<LoadedModel>>>,
    /// Folders of the SD card with models that replace or add models
    /// (`<number>.bin`, the models of X and Y that the plugin gives to ORAS).
    overrides: Vec<PathBuf>,
    /// The build models (props...): their descriptions, models and pictures.
    build: Option<(Vec<Vec<Vec<u8>>>, Vec<Vec<Vec<u8>>>, Vec<Vec<Vec<u8>>>)>,
    props: HashMap<u32, Option<Rc<LoadedModel>>>,
}

/// The models that have a flag in the high bits are the characters that the
/// game dresses (the player, the trainers of the dress-up): no file.
const DRESSED_FLAG: u16 = 0x2000;

impl ModelBank {
    pub fn new(arc_dir: &str) -> Self {
        ModelBank { arc_dir: arc_dir.to_string(), files: None, params: None, cache: HashMap::new(), overrides: Vec::new(), build: None, props: HashMap::new() }
    }

    /// The 3D model of a build model (a door, a monument...) that a block
    /// places. `kind` is its number: the number of its pack in the archive of the props.
    /// The pictures are in the part 1 of the tileset pack (`tileset`). None when
    /// it cannot be read.
    pub fn load_prop(&mut self, kind: u32, tileset: usize) -> Option<Rc<LoadedModel>> {
        let key = kind | ((tileset as u32) << 16);
        if let Some(found) = self.props.get(&key) {
            return found.clone();
        }
        if self.build.is_none() {
            let packs = garc::read(&garc::locate(&self.arc_dir, crate::garc::Archive::Props)).ok();
            let tilesets = garc::read(&garc::locate(&self.arc_dir, crate::garc::Archive::Tilesets)).ok();
            if let (Some(p), Some(a)) = (packs, tilesets) {
                self.build = Some((Vec::new(), p, a));
            }
        }
        let loaded = self.build.as_ref().and_then(|(_, packs, tilesets)| {
            let mut model = Self::parse(packs.get(kind as usize)?.first()?).ok()?;
            // The pictures of the build models of the tileset.
            let raw = tilesets.get(tileset)?.first()?;
            let data = if raw.first() == Some(&0x11) { lz11::decompress(raw).ok()? } else { raw.clone() };
            let (_, parts) = pack::split(&data).ok()?;
            if let Some(part) = parts.get(1).filter(|p| p.starts_with(b"BCH\0")) {
                if let Ok(bch) = Bch::parse(part) {
                    for t in bch.textures().unwrap_or_default() {
                        model.textures.insert(t.name.clone(), t);
                    }
                }
            }
            Some(Rc::new(model))
        });
        self.props.insert(key, loaded.clone());
        loaded
    }

    /// Tells the folders where models of the SD card are.
    pub fn set_overrides(&mut self, dirs: Vec<PathBuf>) {
        self.overrides = dirs;
        self.cache.clear();
    }

    fn override_file(&self, model: u16) -> Option<PathBuf> {
        self.overrides.iter().map(|d| d.join(format!("{model}.bin"))).find(|p| p.is_file())
    }

    /// Says whether a model number has a model (in the archive or on the SD card).
    pub fn has(&mut self, model: u16) -> bool {
        self.override_file(model).is_some() || self.pack_of(model).is_some()
    }

    fn read(&mut self) -> bool {
        if self.files.is_none() {
            self.files = garc::read(&garc::locate(&self.arc_dir, crate::garc::Archive::CharacterModels)).ok();
            self.params = garc::read(&garc::locate(&self.arc_dir, crate::garc::Archive::CharacterModelParams)).ok();
        }
        self.files.is_some() && self.params.is_some()
    }

    /// The number of the pack of a model number, if the model has a file.
    pub fn pack_of(&mut self, model: u16) -> Option<usize> {
        if model & DRESSED_FLAG != 0 || !self.read() {
            return None;
        }
        let record = self.params.as_ref()?.get(model as usize)?.first()?;
        if record.len() != 24 {
            return None;
        }
        let at = u16::from_le_bytes([record[20], record[21]]) as usize;
        (at < self.files.as_ref()?.len()).then_some(at)
    }

    /// The numbers of the models that the SD card has.
    pub fn override_ids(&self) -> Vec<u16> {
        let mut ids: Vec<u16> = Vec::new();
        for d in &self.overrides {
            if let Ok(list) = std::fs::read_dir(d) {
                for e in list.flatten() {
                    if let Some(n) = e.path().file_stem().and_then(|s| s.to_str()).and_then(|s| s.parse::<u16>().ok()) {
                        if e.path().extension().map_or(false, |x| x == "bin") && !ids.contains(&n) {
                            ids.push(n);
                        }
                    }
                }
            }
        }
        ids
    }

    /// Reads a model (once). None when the model has no file or cannot be read.
    pub fn load(&mut self, model: u16) -> Option<Rc<LoadedModel>> {
        if let Some(found) = self.cache.get(&model) {
            return found.clone();
        }
        let from_sd = self.override_file(model).and_then(|p| std::fs::read(p).ok()).and_then(|raw| Self::parse(&raw).ok());
        let loaded = from_sd.or_else(|| self.pack_of(model).and_then(|at| self.read_pack(at).ok())).map(Rc::new);
        self.cache.insert(model, loaded.clone());
        loaded
    }

    fn read_pack(&self, at: usize) -> Result<LoadedModel, String> {
        let raw = self.files.as_ref().and_then(|f| f.get(at)).and_then(|f| f.first()).ok_or("no such model")?;
        Self::parse(raw)
    }

    /// Reads a pack of the archive (or a file of the SD card).
    fn parse(raw: &[u8]) -> Result<LoadedModel, String> {
        let data = if raw.first() == Some(&0x11) { lz11::decompress(raw)? } else { raw.to_vec() };
        let (_, parts) = pack::split(&data)?;
        let model = parts.first().ok_or("the model has no part")?;
        let bch = Bch::parse(model)?;
        let found = bch.meshes_with_faces()?;
        // Some models keep the vertices of a mesh near the bone that moves them:
        // seen from the file, they float in the air. For those, the vertices
        // go through the bones (the index of the vertex is taken as the number
        // of the bone). A model that has feet near the ground is left as it is.
        let lowest = found.iter().flat_map(|m| m.positions.iter().map(|p| p[1])).fold(f32::MAX, f32::min);
        let worlds = if lowest > 4.0 { bone_worlds(model) } else { Vec::new() };
        let meshes = found
            .into_iter()
            .map(|m| {
                let mut positions = m.positions.clone();
                let stride = m.places.stride;
                if !worlds.is_empty() && stride >= 40 {
                    for (v, p) in positions.iter_mut().enumerate() {
                        let at = m.places.vertices_at + v * stride;
                        let (Some(idx), Some(wt)) = (model.get(at + stride - 8..at + stride - 4), model.get(at + stride - 4..at + stride)) else { continue };
                        let mut out = glam::Vec3::ZERO;
                        let mut total = 0.0;
                        for k in 0..4 {
                            let w = wt[k] as f32;
                            if w <= 0.0 {
                                continue;
                            }
                            let bone = m.bones.get(idx[k] as usize).copied().unwrap_or(0) as usize;
                            let world = worlds.get(bone).copied().unwrap_or(glam::Mat4::IDENTITY);
                            out += (world * glam::Vec4::new(p[0], p[1], p[2], 1.0)).truncate() * w;
                            total += w;
                        }
                        if total > 0.0 {
                            *p = (out / total).to_array();
                        }
                    }
                }
                ModelMesh { picture: crate::data::picture_of(&m).to_string(), positions, uvs: m.uvs, indices: m.indices }
            })
            .collect();
        let textures = bch.textures().unwrap_or_default().into_iter().map(|t| (t.name.clone(), t)).collect();
        Ok(LoadedModel { meshes, textures })
    }
}

/// The world matrices of the bones of a model file, from the inverse matrices
/// that the file keeps for each bone (the bones are 25 words each).
pub fn bone_worlds(model: &[u8]) -> Vec<glam::Mat4> {
    let u = |o: usize| model.get(o..o + 4).map_or(0, |b| u32::from_le_bytes([b[0], b[1], b[2], b[3]]));
    let f = |o: usize| f32::from_bits(u(o));
    let content = u(8) as usize;
    let table = content + u(content) as usize;
    let md = content + u(table) as usize;
    let bones = content + u(md + 0x70) as usize;
    let count = u(md + 0x74) as usize;
    if count == 0 || count > 200 || bones + count * 100 > model.len() {
        return Vec::new();
    }
    (0..count)
        .map(|i| {
            let b = bones + i * 100 + 44;
            // Three rows of four: the inverse of the world matrix of the bone.
            let m = glam::Mat4::from_cols_array(&[
                f(b), f(b + 16), f(b + 32), 0.0,
                f(b + 4), f(b + 20), f(b + 36), 0.0,
                f(b + 8), f(b + 24), f(b + 40), 0.0,
                f(b + 12), f(b + 28), f(b + 44), 1.0,
            ]);
            m.inverse()
        })
        .collect()
}
