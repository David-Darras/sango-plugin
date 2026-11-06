//! Changes a block of the game and writes it back.
//!
//! A block is a pack of parts. The parts with a model hold the ground, the
//! objects and the tall grass. The tool changes the data of the model in the
//! file itself, so the game reads the result like an original block.

use std::collections::{BTreeMap, BTreeSet, HashMap, HashSet};
use std::path::{Path, PathBuf};

use crate::bch::{Bch, Mesh};
use crate::{garc, lz11, pack};

/// The parts of a block that hold a model: the ground with its objects, and
/// the tall grass of the encounters.
const MODEL_PARTS: [usize; 2] = [1, 5];

/// A block that can be changed.
#[derive(Clone)]
pub struct EditableBlock {
    pub id: usize,
    /// True when the block was compressed in the archive.
    compressed: bool,
    /// The data of the pack, not compressed.
    pack: Vec<u8>,
    ranges: Vec<(usize, usize)>,
    /// The meshes, with the part of the pack that holds each of them.
    pub meshes: Vec<(usize, Mesh)>,
}

/// A copy of some vertices of a block, moved by an offset.
pub struct CopyRequest {
    /// The number of a mesh and the vertices of this mesh to copy.
    pub source: Vec<(usize, Vec<usize>)>,
    pub delta: [f32; 3],
}

/// The vertices that a copy added to a mesh: the number of the mesh, the
/// number of the first new vertex and how many there are.
pub type AddedVertices = (usize, usize, usize);

/// The tiles of a block: a grid of attributes. An attribute has the number of
/// the ground in its high byte, and flags in the other bytes.
#[derive(Clone, PartialEq, Debug)]
pub struct Attributes {
    pub width: usize,
    pub height: usize,
    /// Row after row.
    pub cells: Vec<u32>,
}

/// The groups of vertices that touch each other (a tree, a bush, a house).
pub fn mesh_groups(mesh: &Mesh) -> Vec<Vec<usize>> {
    let n = mesh.positions.len();
    let mut parent: Vec<usize> = (0..n).collect();
    fn find(p: &mut [usize], mut x: usize) -> usize {
        while p[x] != x {
            p[x] = p[p[x]];
            x = p[x];
        }
        x
    }
    for t in mesh.indices.chunks_exact(3) {
        let (a, b, c) = (t[0] as usize, t[1] as usize, t[2] as usize);
        let (ra, rb) = (find(&mut parent, a), find(&mut parent, b));
        parent[ra] = rb;
        let (rb, rc) = (find(&mut parent, b), find(&mut parent, c));
        parent[rb] = rc;
    }
    let mut groups: BTreeMap<usize, Vec<usize>> = Default::default();
    for v in 0..n {
        let root = find(&mut parent, v);
        groups.entry(root).or_default().push(v);
    }
    groups.into_values().collect()
}

fn get(b: &[u8], p: usize) -> usize {
    u32::from_le_bytes([b[p], b[p + 1], b[p + 2], b[p + 3]]) as usize
}

fn put(b: &mut [u8], p: usize, v: usize) {
    b[p..p + 4].copy_from_slice(&(v as u32).to_le_bytes());
}

fn align(v: usize, a: usize) -> usize {
    v.div_ceil(a) * a
}

impl EditableBlock {
    pub fn load(arc_dir: &str, block: usize) -> Result<Self, String> {
        let files = garc::read(&PathBuf::from(arc_dir).join("map_land_data.garc"))?;
        let raw = files.get(block).and_then(|f| f.first()).ok_or("no such block")?;
        let compressed = raw.first() == Some(&0x11);
        let pack = if compressed { lz11::decompress(raw)? } else { raw.clone() };
        Self::from_pack(block, compressed, pack)
    }

    /// Opens a block from a file of the folder `land` of the SD card.
    pub fn from_file(path: &Path, block: usize) -> Result<Self, String> {
        let raw = std::fs::read(path).map_err(|e| format!("{}: {e}", path.display()))?;
        let compressed = raw.first() == Some(&0x11);
        let pack = if compressed { lz11::decompress(&raw)? } else { raw };
        Self::from_pack(block, compressed, pack)
    }

    fn from_pack(id: usize, compressed: bool, pack: Vec<u8>) -> Result<Self, String> {
        let ranges = pack::ranges(&pack)?;
        let mut meshes = Vec::new();
        for part in MODEL_PARTS {
            let Some(&(start, end)) = ranges.get(part) else { continue };
            let bytes = &pack[start..end];
            if !bytes.starts_with(b"BCH\0") {
                continue;
            }
            for mesh in Bch::parse(bytes)?.meshes()? {
                meshes.push((part, mesh));
            }
        }
        Ok(EditableBlock { id, compressed, pack, ranges, meshes })
    }

    /// The grid of tiles of the block (the part 0 of the pack).
    pub fn attributes(&self) -> Result<Attributes, String> {
        let &(start, end) = self.ranges.first().ok_or("the block has no part")?;
        let bytes = &self.pack[start..end];
        let width = u16::from_le_bytes([bytes[0], bytes[1]]) as usize;
        let height = u16::from_le_bytes([bytes[2], bytes[3]]) as usize;
        if 4 + 4 * width * height > bytes.len() {
            return Err("the grid of tiles is cut".into());
        }
        let cells = (0..width * height).map(|i| get(bytes, 4 + 4 * i) as u32).collect();
        Ok(Attributes { width, height, cells })
    }

    /// Writes the grid of tiles in place. The size of the file does not change.
    pub fn set_attributes(&mut self, attributes: &Attributes) -> Result<(), String> {
        let current = self.attributes()?;
        if current.width != attributes.width || current.height != attributes.height {
            return Err("the grid has another size".into());
        }
        let start = self.ranges[0].0;
        for (i, &value) in attributes.cells.iter().enumerate() {
            put(&mut self.pack, start + 4 + 4 * i, value as usize);
        }
        Ok(())
    }

    /// Moves a group of vertices of a mesh.
    pub fn translate(&mut self, mesh: usize, vertices: &[usize], delta: [f32; 3]) -> Result<(), String> {
        let (part, mesh) = self.meshes.get_mut(mesh).ok_or("no such mesh")?;
        let base = self.ranges[*part].0;
        for &v in vertices {
            let at = base + *mesh.position_offsets.get(v).ok_or("no such vertex")?;
            for axis in 0..3 {
                let place = at + 4 * axis;
                let bytes: [u8; 4] = self.pack[place..place + 4].try_into().map_err(|_| "bad offset")?;
                let value = f32::from_le_bytes(bytes) + delta[axis];
                self.pack[place..place + 4].copy_from_slice(&value.to_le_bytes());
                mesh.positions[v][axis] = value;
            }
        }
        Ok(())
    }

    /// Removes the triangles of a group of vertices from a mesh.
    ///
    /// The list of indices keeps its place and its size: the kept triangles
    /// move to the start of the list and the number of indices gets smaller.
    /// The vertices stay in the file, but nothing draws them. Returns the
    /// number of triangles that were removed.
    pub fn delete_group(&mut self, mesh: usize, vertices: &[usize]) -> Result<usize, String> {
        let (part, mesh) = self.meshes.get_mut(mesh).ok_or("no such mesh")?;
        let base = self.ranges[*part].0;
        let doomed: HashSet<usize> = vertices.iter().copied().collect();
        let kept: Vec<u32> = mesh
            .indices
            .chunks_exact(3)
            .filter(|t| !t.iter().any(|&i| doomed.contains(&(i as usize))))
            .flatten()
            .copied()
            .collect();
        let removed = (mesh.indices.len() - kept.len()) / 3;
        let width = mesh.places.index_width;
        let start = base + mesh.places.indices_at;
        for k in 0..mesh.indices.len() {
            let value = kept.get(k).copied().unwrap_or(0);
            let at = start + k * width;
            if width == 2 {
                self.pack[at..at + 2].copy_from_slice(&(value as u16).to_le_bytes());
            } else {
                self.pack[at] = value as u8;
            }
        }
        let count_at = base + mesh.places.index_count_at;
        self.pack[count_at..count_at + 4].copy_from_slice(&(kept.len() as u32).to_le_bytes());
        mesh.indices = kept;
        Ok(removed)
    }

    /// Copies groups of vertices to other places.
    ///
    /// The new vertices and indices go at the end of the raw data of the
    /// model, because the buffers of the other meshes follow each other and
    /// leave no room. A mesh then points to a new buffer that holds the old
    /// vertices and the new ones. The old buffer stays in the file, unused.
    /// The sections after the raw data and the parts after the model move, and
    /// the file is read again so that the offsets of the meshes are right.
    ///
    /// Returns, for each request, the vertices that were added.
    pub fn copy_groups(&mut self, requests: &[CopyRequest]) -> Result<Vec<Vec<AddedVertices>>, String> {
        // For each part and mesh: the requests that touch it.
        type PerMesh = BTreeMap<usize, Vec<(usize, Vec<usize>)>>;
        let mut by_part: BTreeMap<usize, PerMesh> = BTreeMap::new();
        for (r, request) in requests.iter().enumerate() {
            let mut merged: BTreeMap<usize, BTreeSet<usize>> = BTreeMap::new();
            for (mesh, vertices) in &request.source {
                merged.entry(*mesh).or_default().extend(vertices.iter().copied());
            }
            for (mesh, vertices) in merged {
                let part = self.meshes.get(mesh).ok_or("no such mesh")?.0;
                by_part.entry(part).or_default().entry(mesh).or_default().push((r, vertices.into_iter().collect()));
            }
        }
        let mut added: Vec<Vec<AddedVertices>> = vec![Vec::new(); requests.len()];
        for (part, per_mesh) in by_part {
            for (r, range) in self.copy_in_part(part, &per_mesh, requests)? {
                added[r].push(range);
            }
        }
        Ok(added)
    }

    fn copy_in_part(
        &mut self,
        part: usize,
        per_mesh: &BTreeMap<usize, Vec<(usize, Vec<usize>)>>,
        requests: &[CopyRequest],
    ) -> Result<Vec<(usize, AddedVertices)>, String> {
        let (start, end) = self.ranges[part];
        let old = self.pack[start..end].to_vec();
        let (raw, raw_ext, reloc) = (get(&old, 20), get(&old, 24), get(&old, 28));
        let (ext_len, reloc_len) = (get(&old, 48), get(&old, 52));
        if raw_ext != reloc || ext_len != 0 {
            return Err("this model has a layout that the tool cannot grow".into());
        }
        let mut data: Vec<u8> = Vec::new();
        let first = raw_ext - raw; // where the new data starts, from the raw data
        let mut patches: Vec<(usize, usize)> = Vec::new();
        let mut added: Vec<(usize, AddedVertices)> = Vec::new();
        for (&mesh_index, copies) in per_mesh {
            let (_, mesh) = &self.meshes[mesh_index];
            let places = &mesh.places;
            let (stride, vertex_count) = (places.stride, mesh.positions.len());
            let total: usize = copies.iter().map(|(_, v)| v.len()).sum();
            let limit = if places.index_width == 2 { 65536 } else { 256 };
            if vertex_count + total > limit {
                return Err(format!("mesh {} would have too many vertices for its indices", mesh.material));
            }
            // The vertices: the old ones, then the copies.
            while (first + data.len()) % 16 != 0 {
                data.push(0);
            }
            let vertices_at = first + data.len();
            data.extend_from_slice(&old[places.vertices_at..places.vertices_at + vertex_count * stride]);
            let mut indices: Vec<u32> = mesh.indices.clone();
            let mut next = vertex_count;
            for (request, chosen) in copies {
                let delta = requests[*request].delta;
                let new_id: HashMap<usize, u32> =
                    chosen.iter().enumerate().map(|(j, &v)| (v, (next + j) as u32)).collect();
                for &v in chosen {
                    let from = places.vertices_at + v * stride;
                    let mut bytes = old[from..from + stride].to_vec();
                    let inside = mesh.position_offsets[v] - from;
                    for axis in 0..3 {
                        let at = inside + 4 * axis;
                        let value = f32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]]);
                        bytes[at..at + 4].copy_from_slice(&(value + delta[axis]).to_le_bytes());
                    }
                    data.extend_from_slice(&bytes);
                }
                // The triangles of the copy.
                for t in mesh.indices.chunks_exact(3) {
                    if t.iter().all(|&i| new_id.contains_key(&(i as usize))) {
                        indices.extend(t.iter().map(|&i| new_id[&(i as usize)]));
                    }
                }
                added.push((*request, (mesh_index, next, chosen.len())));
                next += chosen.len();
            }
            // The indices.
            while (first + data.len()) % 16 != 0 {
                data.push(0);
            }
            let indices_at = first + data.len();
            for &i in &indices {
                if places.index_width == 2 {
                    data.extend_from_slice(&(i as u16).to_le_bytes());
                } else {
                    data.push(i as u8);
                }
            }
            patches.push((places.vertex_start_at, vertices_at));
            patches.push((places.index_start_at, indices_at));
            patches.push((places.index_count_at, indices.len()));
        }

        let new_raw_len = first + data.len();
        let new_ext = align(raw + new_raw_len, 0x80);
        let mut bytes = old[..raw_ext].to_vec();
        bytes.extend_from_slice(&data);
        bytes.resize(new_ext, 0);
        bytes.extend_from_slice(&old[reloc..reloc + reloc_len]);
        bytes.resize(align(bytes.len(), pack::ALIGNMENT), 0);
        for (at, value) in patches {
            put(&mut bytes, at, value);
        }
        put(&mut bytes, 44, new_raw_len);
        put(&mut bytes, 24, new_ext);
        put(&mut bytes, 28, new_ext);

        let pack = pack::replace_part(&self.pack, part, &bytes)?;
        *self = Self::from_pack(self.id, self.compressed, pack)?;
        Ok(added)
    }

    /// The bytes to put in the folder `land` of the SD card.
    pub fn to_bytes(&self) -> Result<Vec<u8>, String> {
        if self.compressed {
            lz11::compress(&self.pack)
        } else {
            Ok(self.pack.clone())
        }
    }

    /// Writes the block as `<out_dir>/land/<id>.bin` and adds it to the list
    /// `<out_dir>/land/index.bin`. The plugin uses a folder only when it has
    /// this list: the number of files, then the number and the size of each
    /// file, sorted by number.
    pub fn save(&self, out_dir: &str) -> Result<PathBuf, String> {
        let folder = PathBuf::from(out_dir).join("land");
        std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
        let path = folder.join(format!("{}.bin", self.id));
        let bytes = self.to_bytes()?;
        std::fs::write(&path, &bytes).map_err(|e| format!("{}: {e}", path.display()))?;

        let index_path = folder.join("index.bin");
        let mut entries: BTreeMap<u32, u32> = Default::default();
        if let Ok(old) = std::fs::read(&index_path) {
            let count = garc::u32_at(&old, 0)? as usize;
            for i in 0..count {
                entries.insert(garc::u32_at(&old, 4 + 8 * i)?, garc::u32_at(&old, 8 + 8 * i)?);
            }
        }
        entries.insert(self.id as u32, bytes.len() as u32);
        let mut index = (entries.len() as u32).to_le_bytes().to_vec();
        for (id, size) in entries {
            index.extend_from_slice(&id.to_le_bytes());
            index.extend_from_slice(&size.to_le_bytes());
        }
        std::fs::write(&index_path, index).map_err(|e| format!("{}: {e}", index_path.display()))?;
        Ok(path)
    }
}
