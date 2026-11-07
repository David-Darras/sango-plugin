//! Changes a block of the game and writes it back.
//!
//! A block is a pack of parts. The parts with a model hold the ground, the
//! objects and the tall grass. The tool changes the data of the model in the
//! file itself, so the game reads the result like an original block.

use std::collections::{BTreeMap, BTreeSet, HashMap, HashSet};
use std::path::{Path, PathBuf};

use crate::bch::{Bch, Mesh};
use crate::bch_edit::{self, MaterialBlob, NewGeometry};
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

/// A piece of an object, with the data of its vertices as a file stores them.
#[derive(Clone, Debug)]
pub struct PortablePart {
    pub material: String,
    /// How a vertex is made. A block can take the piece only in a mesh that
    /// has the same material and the same layout.
    pub layout: String,
    pub stride: usize,
    pub position_inside: usize,
    pub normal_inside: Option<usize>,
    /// The vertices, one after the other.
    pub vertices: Vec<u8>,
    /// Three numbers for each triangle, from the first vertex of the piece.
    pub indices: Vec<u32>,
    /// The group of drawing of the mesh (0 to 3).
    pub group: usize,
    /// The material itself, so that a block that does not have it can get it.
    pub blob: Option<MaterialBlob>,
}

/// An object taken from a block, to be put in a block. It holds data of the
/// game: do not share it.
#[derive(Clone, Debug)]
pub struct Portable {
    /// The block and the number of the object where it was taken.
    pub origin: (usize, usize),
    pub low: [f32; 3],
    pub high: [f32; 3],
    pub parts: Vec<PortablePart>,
}

const PORTABLE_MAGIC: &[u8; 5] = b"MSOB2";

/// The block of an object that does not come from a block of the game.
pub const NO_ORIGIN: usize = u32::MAX as usize;

/// Writes numbers and texts one after the other.
struct Writer(Vec<u8>);

impl Writer {
    fn number(&mut self, v: usize) {
        self.0.extend_from_slice(&(v as u32).to_le_bytes());
    }
    fn bytes(&mut self, b: &[u8]) {
        self.number(b.len());
        self.0.extend_from_slice(b);
    }
    fn text(&mut self, t: &str) {
        self.bytes(t.as_bytes());
    }
    fn entries(&mut self, list: &[(u8, usize)]) {
        self.number(list.len());
        for (kind, at) in list {
            self.number(*kind as usize);
            self.number(*at);
        }
    }
    fn strings(&mut self, list: &[(usize, String)]) {
        self.number(list.len());
        for (at, text) in list {
            self.number(*at);
            self.text(text);
        }
    }
}

/// Reads what a `Writer` wrote.
struct Reader<'a> {
    data: &'a [u8],
    at: usize,
}

impl<'a> Reader<'a> {
    fn take(&mut self, size: usize) -> Result<&'a [u8], String> {
        let bytes = self.data.get(self.at..self.at + size).ok_or("the object is cut")?;
        self.at += size;
        Ok(bytes)
    }
    fn number(&mut self) -> Result<usize, String> {
        let b = self.take(4)?;
        Ok(u32::from_le_bytes([b[0], b[1], b[2], b[3]]) as usize)
    }
    fn float(&mut self) -> Result<f32, String> {
        let b = self.take(4)?;
        Ok(f32::from_le_bytes([b[0], b[1], b[2], b[3]]))
    }
    fn bytes(&mut self) -> Result<Vec<u8>, String> {
        let size = self.number()?;
        Ok(self.take(size)?.to_vec())
    }
    fn text(&mut self) -> Result<String, String> {
        Ok(String::from_utf8_lossy(&self.bytes()?).into_owned())
    }
    fn entries(&mut self) -> Result<Vec<(u8, usize)>, String> {
        let count = self.number()?;
        (0..count).map(|_| Ok((self.number()? as u8, self.number()?))).collect()
    }
    fn strings(&mut self) -> Result<Vec<(usize, String)>, String> {
        let count = self.number()?;
        (0..count).map(|_| Ok((self.number()?, self.text()?))).collect()
    }
}

impl Portable {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut w = Writer(PORTABLE_MAGIC.to_vec());
        w.number(self.origin.0);
        w.number(self.origin.1);
        for v in self.low.iter().chain(&self.high) {
            w.0.extend_from_slice(&v.to_le_bytes());
        }
        w.number(self.parts.len());
        for part in &self.parts {
            w.text(&part.material);
            w.text(&part.layout);
            w.number(part.stride);
            w.number(part.position_inside);
            w.number(part.normal_inside.map_or(u32::MAX as usize, |n| n));
            w.bytes(&part.vertices);
            w.number(part.indices.len());
            for i in &part.indices {
                w.0.extend_from_slice(&i.to_le_bytes());
            }
            w.number(part.group);
            match &part.blob {
                None => w.number(0),
                Some(m) => {
                    w.number(1);
                    w.text(&m.name);
                    w.text(&m.label);
                    w.number(m.params_at);
                    w.bytes(&m.header);
                    w.entries(&m.header_entries);
                    w.strings(&m.header_strings);
                    w.bytes(&m.region);
                    w.entries(&m.region_entries);
                    w.strings(&m.region_strings);
                    w.bytes(&m.commands);
                    w.number(m.second_words);
                }
            }
        }
        w.0
    }

    pub fn from_bytes(data: &[u8]) -> Result<Portable, String> {
        if data.get(..5) != Some(PORTABLE_MAGIC) {
            return Err("this is not an object of Sango Editor (or it is from an older version: copy it again)".into());
        }
        let mut r = Reader { data, at: 5 };
        let origin = (r.number()?, r.number()?);
        let mut low = [0.0; 3];
        let mut high = [0.0; 3];
        for v in low.iter_mut().chain(high.iter_mut()) {
            *v = r.float()?;
        }
        let count = r.number()?;
        let mut parts = Vec::new();
        for _ in 0..count {
            let material = r.text()?;
            let layout = r.text()?;
            let stride = r.number()?;
            let position_inside = r.number()?;
            let normal = r.number()?;
            let vertices = r.bytes()?;
            let index_count = r.number()?;
            let mut indices = Vec::with_capacity(index_count);
            for _ in 0..index_count {
                indices.push(r.number()? as u32);
            }
            let group = r.number()?;
            let blob = if r.number()? == 0 {
                None
            } else {
                let name = r.text()?;
                let label = r.text()?;
                let params_at = r.number()?;
                let header = r.bytes()?;
                let header_entries = r.entries()?;
                let header_strings = r.strings()?;
                let region = r.bytes()?;
                let region_entries = r.entries()?;
                let region_strings = r.strings()?;
                let commands = r.bytes()?;
                let second_words = r.number()?;
                Some(MaterialBlob { name, label, params_at, header, header_entries, header_strings, region, region_entries, region_strings, commands, second_words })
            };
            parts.push(PortablePart {
                material,
                layout,
                stride,
                position_inside,
                normal_inside: (normal != u32::MAX as usize).then_some(normal),
                vertices,
                indices,
                group,
                blob,
            });
        }
        Ok(Portable { origin, low, high, parts })
    }
}

/// A copy of some vertices, moved by an offset. The vertices are from the
/// block itself (`source`) or from an object that was taken from a block
/// (`foreign`).
pub struct CopyRequest {
    /// The number of a mesh and the vertices of this mesh to copy.
    pub source: Vec<(usize, Vec<usize>)>,
    pub foreign: Option<Portable>,
    /// The copy turns by this angle (radians) around the vertical axis that
    /// goes through this point (x and z), then moves by `delta`.
    pub centre: [f32; 2],
    pub yaw: f32,
    pub delta: [f32; 3],
}

/// Turns the position and the normal of a vertex around a vertical axis,
/// then moves the position. `bytes` is the vertex as the file stores it.
fn transform_vertex(bytes: &mut [u8], position: usize, normal: Option<usize>, centre: [f32; 2], yaw: f32, delta: [f32; 3]) {
    let read = |b: &[u8], at: usize| f32::from_le_bytes([b[at], b[at + 1], b[at + 2], b[at + 3]]);
    if yaw == 0.0 {
        // No turn: only the position moves, and every other byte stays.
        for (axis, d) in delta.iter().enumerate() {
            let at = position + 4 * axis;
            let value = read(bytes, at) + d;
            bytes[at..at + 4].copy_from_slice(&value.to_le_bytes());
        }
        return;
    }
    let (sin, cos) = yaw.sin_cos();
    let (x, y, z) = (read(bytes, position) - centre[0], read(bytes, position + 4), read(bytes, position + 8) - centre[1]);
    let turned = [centre[0] + cos * x - sin * z + delta[0], y + delta[1], centre[1] + sin * x + cos * z + delta[2]];
    for (axis, value) in turned.iter().enumerate() {
        bytes[position + 4 * axis..position + 4 * axis + 4].copy_from_slice(&value.to_le_bytes());
    }
    if let Some(at) = normal {
        let (nx, ny, nz) = (read(bytes, at), read(bytes, at + 4), read(bytes, at + 8));
        let turned = [cos * nx - sin * nz, ny, sin * nx + cos * nz];
        for (axis, value) in turned.iter().enumerate() {
            bytes[at + 4 * axis..at + 4 * axis + 4].copy_from_slice(&value.to_le_bytes());
        }
    }
}

/// The place that a copy takes in the meshes of a block.
enum Piece {
    /// Vertices of the block.
    Own(Vec<usize>),
    /// The piece of a foreign object with this number.
    Foreign(usize),
}

/// The vertices that a copy added to a mesh: the number of the mesh, the
/// number of the first new vertex and how many there are.
pub type AddedVertices = (usize, usize, usize);

/// The size of a door model in the list of the props, in bytes.
const PROP_SIZE: usize = 0x2c;

/// A door model that the game puts at a door. The place is in units of the
/// map (the matrix), not of the block.
#[derive(Clone, Debug, PartialEq)]
pub struct Prop {
    pub kind: u32,
    pub scale: [f32; 3],
    pub turn: [f32; 3],
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub tail: u32,
}

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
        if a >= n || b >= n || c >= n {
            continue;
        }
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
        let files = garc::read(&crate::garc::locate(arc_dir, crate::garc::Archive::Blocks))?;
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

    /// The door models of the block (the part 3 of the pack): the game puts
    /// them at the props of the buildings, at the places in this list.
    pub fn props(&self) -> Vec<Prop> {
        let Some(&(start, end)) = self.ranges.get(3) else { return Vec::new() };
        let bytes = &self.pack[start..end];
        if bytes.len() < 4 {
            return Vec::new();
        }
        let count = get(bytes, 0);
        if count > 64 || 4 + count * PROP_SIZE > bytes.len() {
            return Vec::new();
        }
        let float = |at: usize| f32::from_bits(get(bytes, at) as u32);
        (0..count)
            .map(|i| {
                let b = 4 + i * PROP_SIZE;
                Prop {
                    kind: get(bytes, b) as u32,
                    scale: [float(b + 4), float(b + 8), float(b + 12)],
                    turn: [float(b + 16), float(b + 20), float(b + 24)],
                    x: float(b + 28),
                    y: float(b + 32),
                    z: float(b + 36),
                    tail: get(bytes, b + 40) as u32,
                }
            })
            .collect()
    }

    /// Writes the props in place (as many as the block has).
    pub fn set_props(&mut self, props: &[Prop]) -> Result<(), String> {
        let Some(&(start, end)) = self.ranges.get(3) else { return Ok(()) };
        let count = get(&self.pack[start..end], 0);
        if 4 + count * PROP_SIZE > end - start {
            return Err("the list of props of the block is cut".into());
        }
        if count != props.len() {
            // The list gets longer or shorter: the part is made again, with the same data after the list.
            let old = &self.pack[start..end];
            let mut part = (props.len() as u32).to_le_bytes().to_vec();
            part.resize(4 + props.len() * PROP_SIZE, 0);
            part.extend_from_slice(&old[4 + count * PROP_SIZE..]);
            part.resize(part.len().div_ceil(pack::ALIGNMENT) * pack::ALIGNMENT, 0);
            let made = pack::replace_part(&self.pack, 3, &part)?;
            *self = Self::from_pack(self.id, self.compressed, made)?;
            if self.ranges.get(3).map_or(true, |&(s, e)| get(&self.pack[s..e], 0) != props.len()) {
                return Err("the list of props could not change its length".into());
            }
        }
        let Some(&(start, _)) = self.ranges.get(3) else { return Ok(()) };
        for (i, d) in props.iter().enumerate() {
            let b = start + 4 + i * PROP_SIZE;
            put(&mut self.pack, b, d.kind as usize);
            for (k, v) in d.scale.iter().chain(&d.turn).chain(&[d.x, d.y, d.z]).enumerate() {
                put(&mut self.pack, b + 4 + 4 * k, v.to_bits() as usize);
            }
            put(&mut self.pack, b + 40, d.tail as usize);
        }
        Ok(())
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

    /// A line that describes the collision part (for the tests).
    pub fn collision_debug(&self) -> String {
        let Some(&(start, end)) = self.ranges.get(2) else { return "no part".into() };
        let d = &self.pack[start..end];
        let w = |i: usize| d.get(4 * i..4 * i + 4).map_or(0, |b| u32::from_le_bytes([b[0], b[1], b[2], b[3]]));
        let spans: Vec<String> = (0..20).map(|i| format!("{}:{}", w(8 + 8 * 20 + 2 * i), w(8 + 8 * 20 + 2 * i + 1))).collect();
        format!("len {} head {:08x} {:08x} {:08x} {:08x} | {:08x} {:08x} {:08x} {:08x} | spans {}", d.len(), w(0), w(1), w(2), w(3), w(4), w(5), w(6), w(7), spans.join(" "))
    }

    /// The triangles of the collision model of the block (in units of the block), each one once.
    pub fn collision_triangles(&self) -> Result<Vec<[[f32; 3]; 3]>, String> {
        let &(start, end) = self.ranges.get(2).ok_or("the block has no collision part")?;
        let data = &self.pack[start..end];
        if data.len() < 32 || &data[0..4] != b"coll" {
            return Err("the block has no collision model".into());
        }
        let word = |at: usize| -> Result<u32, String> {
            data.get(at..at + 4).map(|b| u32::from_le_bytes([b[0], b[1], b[2], b[3]])).ok_or_else(|| "the collision model is cut".to_string())
        };
        let float = |at: usize| -> Result<f32, String> { word(at).map(f32::from_bits) };
        let (levels, x, y, z) = (word(16)? as usize, word(20)? as usize, word(24)? as usize, word(28)? as usize);
        let per_box = x * y * z;
        if levels < 2 || per_box == 0 || per_box > 4096 {
            return Err("the collision model has an unknown shape".into());
        }
        let mut boxes = 0usize;
        let mut leaves = 1usize;
        for _ in 1..levels {
            leaves *= per_box;
            boxes += leaves;
        }
        let spans_at = 32 + 32 * boxes;
        let positions_at = spans_at + 8 * leaves;
        let mut seen: std::collections::HashSet<[u32; 9]> = Default::default();
        let mut out = Vec::new();
        for leaf in 0..leaves {
            let first = word(spans_at + 8 * leaf)? as i32;
            let count = word(spans_at + 8 * leaf + 4)? as i32;
            if first < 0 || count <= 0 {
                continue;
            }
            for t in 0..(count as usize) / 3 {
                let mut tri = [[0f32; 3]; 3];
                for (k, p) in tri.iter_mut().enumerate() {
                    let at = positions_at + 16 * (first as usize + 3 * t + k);
                    *p = [float(at)?, float(at + 4)?, float(at + 8)?];
                }
                let key = [
                    tri[0][0].to_bits(), tri[0][1].to_bits(), tri[0][2].to_bits(),
                    tri[1][0].to_bits(), tri[1][1].to_bits(), tri[1][2].to_bits(),
                    tri[2][0].to_bits(), tri[2][1].to_bits(), tri[2][2].to_bits(),
                ];
                if seen.insert(key) {
                    out.push(tri);
                }
            }
        }
        Ok(out)
    }

    /// Turns the whole block around its centre by quarter turns (1 = a quarter turn so that what
    /// was at the east is now at the south, 3 = the other way): the ground and the objects, the tall
    /// grass, the collision model, the grid of tiles and the door models. `cell` is the place
    /// (column, row) of the block in the grid of the world, which the door models need.
    pub fn rotate_quarters(&mut self, quarters: usize, cell: (usize, usize)) -> Result<(), String> {
        let q = quarters % 4;
        if q == 0 {
            return Ok(());
        }
        let yaw = q as f32 * std::f32::consts::FRAC_PI_2;
        let turn = |x: f32, z: f32| -> (f32, f32) {
            match q {
                1 => (-z, x),
                2 => (-x, -z),
                _ => (z, -x),
            }
        };
        let triangles = self.collision_triangles().ok();
        for mesh in 0..self.meshes.len() {
            if !MODEL_PARTS.contains(&self.meshes[mesh].0) {
                continue;
            }
            let all: Vec<usize> = (0..self.meshes[mesh].1.positions.len()).collect();
            self.transform(mesh, &all, [0.0, 0.0], yaw, [0.0; 3])?;
            // The quarter turns are exact: the numbers that the sine and the cosine left are rounded away.
            let base = self.ranges[self.meshes[mesh].0].0;
            for v in 0..self.meshes[mesh].1.positions.len() {
                let at = base + self.meshes[mesh].1.position_offsets[v];
                for axis in [0usize, 2] {
                    let value = self.meshes[mesh].1.positions[v][axis];
                    let rounded = (value * 1000.0).round() / 1000.0;
                    self.pack[at + 4 * axis..at + 4 * axis + 4].copy_from_slice(&rounded.to_le_bytes());
                    self.meshes[mesh].1.positions[v][axis] = rounded;
                }
            }
        }
        // The grid of tiles.
        let grid = self.attributes()?;
        if grid.width != grid.height {
            return Err("the grid of tiles is not square".into());
        }
        let size = grid.width;
        let mut cells = grid.cells.clone();
        for row in 0..size {
            for column in 0..size {
                let (c, r) = match q {
                    1 => (size - 1 - row, column),
                    2 => (size - 1 - column, size - 1 - row),
                    _ => (row, size - 1 - column),
                };
                cells[r * size + c] = grid.cells[row * size + column];
            }
        }
        self.set_attributes(&Attributes { width: size, height: size, cells })?;
        // The door models: their place is in units of the world.
        let mut props = self.props();
        let (origin_x, origin_z) = (cell.0 as f32 * 720.0 + 360.0, cell.1 as f32 * 720.0 + 360.0);
        for d in props.iter_mut() {
            let (x, z) = turn(d.x - origin_x, d.z - origin_z);
            d.x = origin_x + x;
            d.z = origin_z + z;
            d.turn[1] -= yaw;
        }
        self.set_props(&props)?;
        // The collision model.
        if let Some(triangles) = triangles {
            let turned: Vec<[[f32; 3]; 3]> = triangles
                .iter()
                .map(|t| {
                    let mut out = *t;
                    for p in out.iter_mut() {
                        let (x, z) = turn(p[0], p[2]);
                        p[0] = x;
                        p[2] = z;
                    }
                    out
                })
                .collect();
            self.set_collision(&turned)?;
        }
        Ok(())
    }

    /// Makes the collision of the block again from triangles (in units of the block): the
    /// game walks on this model, not on the pictures. The triangles are put in a grid of boxes
    /// (three levels, five by five boxes) so that the game finds them fast.
    pub fn set_collision(&mut self, triangles: &[[[f32; 3]; 3]]) -> Result<(), String> {
        const DIV: usize = 5;
        const LEVELS: usize = 3;
        const MARGIN: f32 = 0.5;
        let part = 2;
        let Some(&(start, end)) = self.ranges.get(part) else { return Ok(()) };
        let old = self.pack[start..end].to_vec();
        if old.len() < 16 || &old[0..4] != b"coll" {
            return Err("the block has no collision model to change".into());
        }
        let old_size = u32::from_le_bytes([old[4], old[5], old[6], old[7]]) as usize;
        let tail = old.get(16 + old_size..).unwrap_or(&[]).to_vec();
        // The block is cut in 16 x 16 cells; the triangles go in every cell that they touch.
        let (low_edge, cell) = (-364.0f32, 728.0f32 / (DIV * DIV) as f32);
        let side = DIV * DIV;
        let mut leaves: Vec<Vec<usize>> = vec![Vec::new(); side * side];
        for (k, t) in triangles.iter().enumerate() {
            let (mut lo, mut hi) = ([f32::MAX; 3], [f32::MIN; 3]);
            for p in t {
                for a in 0..3 {
                    lo[a] = lo[a].min(p[a]);
                    hi[a] = hi[a].max(p[a]);
                }
            }
            let index = |v: f32| (((v - low_edge) / cell).floor().max(0.0) as usize).min(side - 1);
            for cx in index(lo[0] - MARGIN)..=index(hi[0] + MARGIN) {
                for cz in index(lo[2] - MARGIN)..=index(hi[2] + MARGIN) {
                    leaves[cx * side + cz].push(k);
                }
            }
        }
        // The number of a leaf in the arrays: the number of its box of the first level, then its place in it.
        let leaf_number = |cx: usize, cz: usize| {
            let (px, pz, lx, lz) = (cx / DIV, cz / DIV, cx % DIV, cz % DIV);
            (px * DIV + pz) * (DIV * DIV) + (lx * DIV + lz)
        };
        let leaf_count = side * side;
        let boxes_first = DIV * DIV;
        let empty = ([1.0e30f32; 3], [-1.0e30f32; 3]);
        let mut leaf_boxes = vec![empty; leaf_count];
        let mut spans: Vec<(i32, i32)> = vec![(0, 0); leaf_count];
        let mut positions: Vec<[f32; 3]> = Vec::new();
        // The leaves in the order of their numbers.
        let mut by_number: Vec<(usize, usize, usize)> = Vec::new();
        for cx in 0..side {
            for cz in 0..side {
                by_number.push((leaf_number(cx, cz), cx, cz));
            }
        }
        by_number.sort_unstable();
        for &(number, cx, cz) in &by_number {
            let list = &leaves[cx * side + cz];
            let first = positions.len();
            let (mut lo, mut hi) = empty;
            for &k in list {
                for p in &triangles[k] {
                    positions.push(*p);
                    for a in 0..3 {
                        lo[a] = lo[a].min(p[a] - MARGIN);
                        hi[a] = hi[a].max(p[a] + MARGIN);
                    }
                }
            }
            spans[number] = (first as i32, (positions.len() - first) as i32);
            leaf_boxes[number] = (lo, hi);
        }
        let mut first_boxes = vec![empty; boxes_first];
        for number in 0..leaf_count {
            let parent = number / (DIV * DIV);
            let (lo, hi) = leaf_boxes[number];
            if lo[0] > hi[0] {
                continue;
            }
            for a in 0..3 {
                first_boxes[parent].0[a] = first_boxes[parent].0[a].min(lo[a]);
                first_boxes[parent].1[a] = first_boxes[parent].1[a].max(hi[a]);
            }
        }
        let mut payload: Vec<u8> = Vec::new();
        for v in [LEVELS as u32, DIV as u32, 1, DIV as u32] {
            payload.extend_from_slice(&v.to_le_bytes());
        }
        let put_box = |payload: &mut Vec<u8>, b: &([f32; 3], [f32; 3])| {
            for a in 0..3 {
                payload.extend_from_slice(&b.0[a].to_le_bytes());
            }
            payload.extend_from_slice(&0f32.to_le_bytes());
            for a in 0..3 {
                payload.extend_from_slice(&b.1[a].to_le_bytes());
            }
            payload.extend_from_slice(&0f32.to_le_bytes());
        };
        for b in &first_boxes {
            put_box(&mut payload, b);
        }
        for b in &leaf_boxes {
            put_box(&mut payload, b);
        }
        for &(first, count) in &spans {
            payload.extend_from_slice(&first.to_le_bytes());
            payload.extend_from_slice(&count.to_le_bytes());
        }
        for p in &positions {
            for a in 0..3 {
                payload.extend_from_slice(&p[a].to_le_bytes());
            }
            payload.extend_from_slice(&0f32.to_le_bytes());
        }
        let mut part_bytes = b"coll".to_vec();
        part_bytes.extend_from_slice(&(payload.len() as u32).to_le_bytes());
        part_bytes.extend_from_slice(&old[8..16]);
        part_bytes.extend_from_slice(&payload);
        part_bytes.extend_from_slice(&tail);
        part_bytes.resize(part_bytes.len().div_ceil(pack::ALIGNMENT) * pack::ALIGNMENT, 0);
        let made = pack::replace_part(&self.pack, part, &part_bytes)?;
        *self = Self::from_pack(self.id, self.compressed, made)?;
        Ok(())
    }

    /// Gives the same height to vertices of a mesh (the ground is made flat).
    pub fn set_height(&mut self, mesh: usize, vertices: &[usize], height: f32) -> Result<(), String> {
        let (part, mesh) = self.meshes.get_mut(mesh).ok_or("no such mesh")?;
        let base = self.ranges[*part].0;
        for &v in vertices {
            let at = base + *mesh.position_offsets.get(v).ok_or("no such vertex")? + 4;
            self.pack[at..at + 4].copy_from_slice(&height.to_le_bytes());
            mesh.positions[v][1] = height;
        }
        Ok(())
    }

    /// Turns a group of vertices of a mesh around a vertical axis, then moves
    /// it. The positions and the normals change in the file.
    pub fn transform(
        &mut self,
        mesh: usize,
        vertices: &[usize],
        centre: [f32; 2],
        yaw: f32,
        delta: [f32; 3],
    ) -> Result<(), String> {
        let (part, mesh) = self.meshes.get_mut(mesh).ok_or("no such mesh")?;
        let base = self.ranges[*part].0;
        for &v in vertices {
            let position = base + *mesh.position_offsets.get(v).ok_or("no such vertex")?;
            let normal = mesh.normal_offsets.get(v).map(|n| base + n);
            // The numbers of a vertex are close to each other in the file.
            let from = position.min(normal.unwrap_or(position));
            let to = position.max(normal.unwrap_or(position)) + 12;
            let mut bytes = self.pack[from..to].to_vec();
            transform_vertex(&mut bytes, position - from, normal.map(|n| n - from), centre, yaw, delta);
            self.pack[from..to].copy_from_slice(&bytes);
            let at = position;
            mesh.positions[v] = [
                f32::from_le_bytes(self.pack[at..at + 4].try_into().map_err(|_| "bad offset")?),
                f32::from_le_bytes(self.pack[at + 4..at + 8].try_into().map_err(|_| "bad offset")?),
                f32::from_le_bytes(self.pack[at + 8..at + 12].try_into().map_err(|_| "bad offset")?),
            ];
        }
        Ok(())
    }

    /// Moves a group of vertices of a mesh.
    pub fn translate(&mut self, mesh: usize, vertices: &[usize], delta: [f32; 3]) -> Result<(), String> {
        self.transform(mesh, vertices, [0.0, 0.0], 0.0, delta)
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

    /// Removes the triangles of a mesh whose centre is in the tile (column,
    /// row), seen from above. The vertices stay. Returns how many triangles
    /// were removed.
    pub fn delete_triangles_in_tile(&mut self, mesh: usize, column: usize, row: usize) -> Result<usize, String> {
        let (part, mesh) = self.meshes.get_mut(mesh).ok_or("no such mesh")?;
        let base = self.ranges[*part].0;
        let (x0, z0) = (-360.0 + 18.0 * column as f32, -360.0 + 18.0 * row as f32);
        let in_tile = |t: &[u32]| {
            let (mut x, mut z) = (0.0, 0.0);
            for &i in t {
                let p = mesh.positions.get(i as usize).copied().unwrap_or([0.0; 3]);
                x += p[0] / 3.0;
                z += p[2] / 3.0;
            }
            x >= x0 && x < x0 + 18.0 && z >= z0 && z < z0 + 18.0
        };
        let kept: Vec<u32> = mesh.indices.chunks_exact(3).filter(|t| !in_tile(t)).flatten().copied().collect();
        let removed = (mesh.indices.len() - kept.len()) / 3;
        if removed == 0 {
            return Ok(0);
        }
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

    /// Cuts what goes out of the square of the block (from -limit to +limit): small objects (trees,
    /// buildings) whose centre is outside are removed; for the ground, the triangles that are
    /// completely outside are removed and the vertices that are outside go to the border. The
    /// collision model is cut the same way. Returns how many triangles went away.
    pub fn cut_outside(&mut self, limit: f32) -> Result<usize, String> {
        let outside = |p: &[f32; 3]| p[0].abs() > limit + 0.01 || p[2].abs() > limit + 0.01;
        let side = move |p: &[f32; 3]| -> u8 { (p[0] > limit) as u8 | ((p[0] < -limit) as u8) << 1 | ((p[2] > limit) as u8) << 2 | ((p[2] < -limit) as u8) << 3 };
        let mut removed_total = 0usize;
        for index in 0..self.meshes.len() {
            let part = self.meshes[index].0;
            if !MODEL_PARTS.contains(&part) {
                continue;
            }
            let groups = mesh_groups(&self.meshes[index].1);
            let mesh = &self.meshes[index].1;
            // For each vertex: None for the ground (a big group), or the decision for a small object.
            let mut small: Vec<Option<bool>> = vec![None; mesh.positions.len()];
            for g in &groups {
                let (mut lo, mut hi) = ([f32::MAX; 2], [f32::MIN; 2]);
                for &v in g {
                    let p = mesh.positions[v];
                    lo[0] = lo[0].min(p[0]);
                    hi[0] = hi[0].max(p[0]);
                    lo[1] = lo[1].min(p[2]);
                    hi[1] = hi[1].max(p[2]);
                }
                if (hi[0] - lo[0]).max(hi[1] - lo[1]) < 100.0 {
                    let centre = [(lo[0] + hi[0]) / 2.0, 0.0, (lo[1] + hi[1]) / 2.0];
                    for &v in g {
                        small[v] = Some(outside(&centre));
                    }
                }
            }
            let mut kept: Vec<u32> = Vec::new();
            for t in mesh.indices.chunks_exact(3) {
                let drop = match small.get(t[0] as usize).copied().flatten() {
                    Some(gone) => gone,
                    None => {
                        let sides: Vec<u8> = t.iter().map(|&i| mesh.positions.get(i as usize).map_or(0, side)).collect();
                        sides[0] & sides[1] & sides[2] != 0
                    }
                };
                if !drop {
                    kept.extend_from_slice(t);
                }
            }
            let removed = (mesh.indices.len() - kept.len()) / 3;
            if removed > 0 {
                let (part, mesh) = &mut self.meshes[index];
                let base = self.ranges[*part].0;
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
                removed_total += removed;
            }
            // The vertices of the ground that are still outside go to the border.
            let (part, mesh) = &mut self.meshes[index];
            let base = self.ranges[*part].0;
            let used: HashSet<usize> = mesh.indices.iter().map(|&i| i as usize).collect();
            for v in 0..mesh.positions.len() {
                if small.get(v).copied().flatten().is_some() || !used.contains(&v) {
                    continue;
                }
                let p = mesh.positions[v];
                if !outside(&p) {
                    continue;
                }
                let moved = [p[0].clamp(-limit, limit), p[1], p[2].clamp(-limit, limit)];
                let at = base + mesh.position_offsets[v];
                for axis in [0usize, 2] {
                    self.pack[at + 4 * axis..at + 4 * axis + 4].copy_from_slice(&moved[axis].to_le_bytes());
                }
                mesh.positions[v] = moved;
            }
        }
        if let Ok(triangles) = self.collision_triangles() {
            let mut cut: Vec<[[f32; 3]; 3]> = Vec::new();
            for t in &triangles {
                if side(&t[0]) & side(&t[1]) & side(&t[2]) != 0 {
                    continue;
                }
                let mut out = *t;
                for p in out.iter_mut() {
                    p[0] = p[0].clamp(-limit, limit);
                    p[2] = p[2].clamp(-limit, limit);
                }
                cut.push(out);
            }
            if cut.len() != triangles.len() || cut.iter().zip(&triangles).any(|(a, b)| a != b) {
                self.set_collision(&cut)?;
            }
        }
        Ok(removed_total)
    }

    /// The mesh of this block that can take a piece of a foreign object.
    pub(crate) fn host_of(&self, part: &PortablePart) -> Result<usize, String> {
        self.meshes
            .iter()
            .position(|(_, m)| m.material == part.material && m.places.layout == part.layout)
            .ok_or_else(|| {
                let same_material = self.meshes.iter().any(|(_, m)| m.material == part.material);
                if same_material {
                    format!("this block has the mesh {} but its vertices are made another way", part.material)
                } else {
                    format!("this block has no mesh {}", part.material)
                }
            })
    }

    /// Adds a new mesh to the block. The new mesh is a copy of the mesh
    /// `like` (same material, same group of drawing) that holds copies of
    /// some vertices of meshes that are made like it, moved by `delta`.
    /// Returns the number of the new mesh. The meshes after it have a number
    /// that is one more.
    pub fn add_mesh_from_groups(
        &mut self,
        like: usize,
        source: &[(usize, Vec<usize>)],
        delta: [f32; 3],
    ) -> Result<usize, String> {
        self.add_mesh_from_groups_turned(like, source, [0.0, 0.0], 0.0, delta)
    }

    /// Like `add_mesh_from_groups`, and the vertices turn by `yaw` around the point `centre` first.
    pub fn add_mesh_from_groups_turned(
        &mut self,
        like: usize,
        source: &[(usize, Vec<usize>)],
        centre: [f32; 2],
        yaw: f32,
        delta: [f32; 3],
    ) -> Result<usize, String> {
        let (part, like_mesh) = self.meshes.get(like).ok_or("no such mesh")?;
        let part = *part;
        let layout = like_mesh.places.layout.clone();
        let (stride, width) = (like_mesh.places.stride, like_mesh.places.index_width);
        let base = self.ranges[part].0;

        let mut vertices: Vec<u8> = Vec::new();
        let mut indices: Vec<u32> = Vec::new();
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for (index, group) in source {
            let (p, mesh) = self.meshes.get(*index).ok_or("no such mesh")?;
            if *p != part || mesh.places.layout != layout {
                return Err(format!("the mesh {} is not made like the mesh that is copied", mesh.material));
            }
            let chosen: BTreeSet<usize> = group.iter().copied().collect();
            let first_new = vertices.len() / stride;
            let local: HashMap<usize, u32> =
                chosen.iter().enumerate().map(|(k, &v)| (v, (first_new + k) as u32)).collect();
            for &v in &chosen {
                let from = mesh.places.vertices_at + v * stride;
                let mut bytes = self.pack[base + from..base + from + stride].to_vec();
                let inside = mesh.position_offsets[v] - from;
                let normal = mesh.normal_offsets.get(v).map(|n| n - from);
                transform_vertex(&mut bytes, inside, normal, centre, yaw, delta);
                for a in 0..3 {
                    let at = inside + 4 * a;
                    let value = f32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]]);
                    low[a] = low[a].min(value);
                    high[a] = high[a].max(value);
                }
                vertices.extend_from_slice(&bytes);
            }
            for t in mesh.indices.chunks_exact(3) {
                if t.iter().all(|&i| local.contains_key(&(i as usize))) {
                    indices.extend(t.iter().map(|&i| local[&(i as usize)]));
                }
            }
        }
        if indices.is_empty() {
            return Err("nothing to copy".into());
        }
        if vertices.len() / stride > if width == 2 { 65536 } else { 256 } {
            return Err("too many vertices for the indices of this mesh".into());
        }
        let geometry = NewGeometry {
            vertices,
            indices,
            width,
            centre: [(low[0] + high[0]) / 2.0, (low[1] + high[1]) / 2.0, (low[2] + high[2]) / 2.0],
        };
        // The number of the mesh in the table of its part.
        let in_part = self.meshes[..like].iter().filter(|(p, _)| *p == part).count();
        let before = self.meshes[..like].iter().filter(|(p, _)| *p < part).count();
        let (bytes, inserted) = bch_edit::add_object(&self.pack[self.ranges[part].0..self.ranges[part].1], in_part, &geometry)?;
        let mut padded = bytes;
        padded.resize(padded.len().div_ceil(pack::ALIGNMENT) * pack::ALIGNMENT, 0);
        let pack = pack::replace_part(&self.pack, part, &padded)?;
        *self = Self::from_pack(self.id, self.compressed, pack)?;
        Ok(before + inserted)
    }

    /// Puts the pieces of an object that come from another block in this block
    /// as new meshes. A piece whose mesh the block does not have gets its
    /// material too. The pieces turn by `yaw` around the point `centre`, then
    /// move by `delta`. Returns how many meshes were added.
    pub fn add_foreign_meshes(&mut self, object: &Portable, centre: [f32; 2], yaw: f32, delta: [f32; 3]) -> Result<usize, String> {
        self.add_foreign_meshes_many(object, &[(centre, yaw, delta)])
    }

    /// Like `add_foreign_meshes`, for many copies of the same object: all the copies go in the
    /// same new mesh (or in as few new meshes as the indices allow). Returns how many meshes were added.
    pub fn add_foreign_meshes_many(&mut self, object: &Portable, places: &[([f32; 2], f32, [f32; 3])]) -> Result<usize, String> {
        let mut added = 0;
        for piece in &object.parts {
            let blob = piece.blob.as_ref().ok_or_else(|| format!("the material {} was not copied with the object", piece.material))?;
            let count = piece.vertices.len() / piece.stride.max(1);
            // A mesh of this block that is made like the piece gives the shape of the new mesh. A mesh with
            // big indices (two bytes) is better: it holds many copies. The mesh that has the same material
            // is the model when there are only a few copies.
            let big = |m: &Mesh| m.places.index_width == 2;
            let same_layout = |m: &Mesh| m.places.layout == piece.layout;
            let template = (if places.len() > 1 { self.meshes.iter().position(|(p, m)| *p == 1 && same_layout(m) && big(m) && m.material == piece.material) } else { None })
                .or_else(|| if places.len() > 1 { self.meshes.iter().position(|(p, m)| *p == 1 && same_layout(m) && big(m)) } else { None })
                .or_else(|| self.meshes.iter().position(|(p, m)| *p == 1 && m.material == piece.material && same_layout(m) && (big(m) || count <= 256)))
                .or_else(|| self.meshes.iter().position(|(p, m)| *p == 1 && same_layout(m) && (big(m) || count <= 256)))
                .or_else(|| self.meshes.iter().position(|(p, m)| *p == 1 && same_layout(m)))
                .ok_or_else(|| format!("this block has no mesh with the vertices of {}", piece.material))?;
            let (part, like) = &self.meshes[template];
            let part = *part;
            let width = like.places.index_width;
            let limit = if width == 2 { 65536 } else { 256 };
            if count > limit {
                return Err(format!("{} has too many vertices for the indices of the mesh that serves as a model", piece.material));
            }
            let per_mesh = (limit / count.max(1)).max(1);
            for chunk in places.chunks(per_mesh) {
                let mut vertices = Vec::with_capacity(piece.vertices.len() * chunk.len());
                let mut indices: Vec<u32> = Vec::with_capacity(piece.indices.len() * chunk.len());
                let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
                for (copy, &(centre, yaw, delta)) in chunk.iter().enumerate() {
                    for vertex in piece.vertices.chunks_exact(piece.stride) {
                        let mut bytes = vertex.to_vec();
                        transform_vertex(&mut bytes, piece.position_inside, piece.normal_inside, centre, yaw, delta);
                        for a in 0..3 {
                            let at = piece.position_inside + 4 * a;
                            let v = f32::from_le_bytes([bytes[at], bytes[at + 1], bytes[at + 2], bytes[at + 3]]);
                            low[a] = low[a].min(v);
                            high[a] = high[a].max(v);
                        }
                        vertices.extend_from_slice(&bytes);
                    }
                    indices.extend(piece.indices.iter().map(|&i| i + (copy * count) as u32));
                }
                let geometry = NewGeometry {
                    vertices,
                    indices,
                    width,
                    centre: [(low[0] + high[0]) / 2.0, (low[1] + high[1]) / 2.0, (low[2] + high[2]) / 2.0],
                };
                let template_now = self
                    .meshes
                    .iter()
                    .position(|(p, m)| *p == part && m.material == self.meshes[template].1.material && m.places.layout == piece.layout && m.places.index_width == width)
                    .unwrap_or(template);
                let in_part = self.meshes[..template_now].iter().filter(|(p, _)| *p == part).count();
                // A material that the block already has is not copied again.
                let has_material = self.meshes.iter().any(|(_, m)| m.material == piece.material);
                let (bytes, _) = bch_edit::add_object_with(
                    &self.pack[self.ranges[part].0..self.ranges[part].1],
                    in_part,
                    Some(piece.group),
                    &geometry,
                    if has_material { None } else { Some(blob) },
                )?;
                let mut padded = bytes;
                padded.resize(padded.len().div_ceil(pack::ALIGNMENT) * pack::ALIGNMENT, 0);
                let pack = pack::replace_part(&self.pack, part, &padded)?;
                *self = Self::from_pack(self.id, self.compressed, pack)?;
                added += 1;
            }
        }
        Ok(added)
    }

    /// Says whether the block can take an object from another block.
    pub fn can_host(&self, object: &Portable) -> Result<(), String> {
        for part in &object.parts {
            self.host_of(part)?;
        }
        Ok(())
    }

    /// Takes an object out of the block: the data of the vertices of the
    /// groups, to put them in a block later.
    pub fn portable(
        &self,
        origin: (usize, usize),
        groups: &[(usize, Vec<usize>)],
        low: [f32; 3],
        high: [f32; 3],
    ) -> Result<Portable, String> {
        let mut parts = Vec::new();
        for (index, vertices) in groups {
            let (part, mesh) = self.meshes.get(*index).ok_or("no such mesh")?;
            let base = self.ranges[*part].0;
            let places = &mesh.places;
            let chosen: BTreeSet<usize> = vertices.iter().copied().collect();
            let local: HashMap<usize, u32> = chosen.iter().enumerate().map(|(k, &v)| (v, k as u32)).collect();
            let first = *chosen.iter().next().ok_or("an object without vertices")?;
            let start_of = |v: usize| places.vertices_at + v * places.stride;
            let mut bytes = Vec::with_capacity(chosen.len() * places.stride);
            for &v in &chosen {
                let from = base + start_of(v);
                bytes.extend_from_slice(&self.pack[from..from + places.stride]);
            }
            let mut indices = Vec::new();
            for t in mesh.indices.chunks_exact(3) {
                if t.iter().all(|&i| local.contains_key(&(i as usize))) {
                    indices.extend(t.iter().map(|&i| local[&(i as usize)]));
                }
            }
            parts.push(PortablePart {
                material: mesh.material.clone(),
                layout: places.layout.clone(),
                stride: places.stride,
                position_inside: mesh.position_offsets[first] - start_of(first),
                normal_inside: mesh.normal_offsets.get(first).map(|n| n - start_of(first)),
                vertices: bytes,
                indices,
                group: mesh.group,
                blob: bch_edit::read_material(&self.pack[base..self.ranges[*part].1], mesh.material_index).ok(),
            });
        }
        Ok(Portable { origin, low, high, parts })
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
        // For each part and mesh: the pieces that go there.
        type PerMesh = BTreeMap<usize, Vec<(usize, Piece)>>;
        let mut by_part: BTreeMap<usize, PerMesh> = BTreeMap::new();
        for (r, request) in requests.iter().enumerate() {
            let mut merged: BTreeMap<usize, BTreeSet<usize>> = BTreeMap::new();
            for (mesh, vertices) in &request.source {
                merged.entry(*mesh).or_default().extend(vertices.iter().copied());
            }
            for (mesh, vertices) in merged {
                let part = self.meshes.get(mesh).ok_or("no such mesh")?.0;
                by_part.entry(part).or_default().entry(mesh).or_default().push((r, Piece::Own(vertices.into_iter().collect())));
            }
            if let Some(object) = &request.foreign {
                for (k, piece) in object.parts.iter().enumerate() {
                    let mesh = self.host_of(piece)?;
                    let part = self.meshes[mesh].0;
                    by_part.entry(part).or_default().entry(mesh).or_default().push((r, Piece::Foreign(k)));
                }
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
        per_mesh: &BTreeMap<usize, Vec<(usize, Piece)>>,
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
        for (&mesh_index, pieces) in per_mesh {
            let (_, mesh) = &self.meshes[mesh_index];
            let places = &mesh.places;
            let (stride, vertex_count) = (places.stride, mesh.positions.len());
            let total: usize = pieces
                .iter()
                .map(|(request, piece)| match piece {
                    Piece::Own(v) => v.len(),
                    Piece::Foreign(k) => {
                        requests[*request].foreign.as_ref().map_or(0, |o| o.parts[*k].vertices.len() / o.parts[*k].stride)
                    }
                })
                .sum();
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
            for (request, piece) in pieces {
                let (centre, yaw, delta) = (requests[*request].centre, requests[*request].yaw, requests[*request].delta);
                match piece {
                    Piece::Own(chosen) => {
                        let new_id: HashMap<usize, u32> =
                            chosen.iter().enumerate().map(|(j, &v)| (v, (next + j) as u32)).collect();
                        for &v in chosen {
                            let from = places.vertices_at + v * stride;
                            let mut bytes = old[from..from + stride].to_vec();
                            let inside = mesh.position_offsets[v] - from;
                            let normal = mesh.normal_offsets.get(v).map(|n| n - from);
                            transform_vertex(&mut bytes, inside, normal, centre, yaw, delta);
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
                    Piece::Foreign(k) => {
                        let object = requests[*request].foreign.as_ref().ok_or("no foreign object")?;
                        let piece = &object.parts[*k];
                        let count = piece.vertices.len() / piece.stride;
                        for vertex in piece.vertices.chunks_exact(piece.stride) {
                            let mut bytes = vertex.to_vec();
                            transform_vertex(&mut bytes, piece.position_inside, piece.normal_inside, centre, yaw, delta);
                            data.extend_from_slice(&bytes);
                        }
                        indices.extend(piece.indices.iter().map(|&i| next as u32 + i));
                        added.push((*request, (mesh_index, next, count)));
                        next += count;
                    }
                }
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

        write_index(&folder, self.id, bytes.len())?;
        Ok(path)
    }
}

/// Adds a file to the list `index.bin` of a folder of the SD card (the number
/// of files, then the number and the size of each file, sorted by number).
pub fn write_index(folder: &Path, id: usize, size: usize) -> Result<(), String> {
    let index_path = folder.join("index.bin");
    let mut entries: BTreeMap<u32, u32> = Default::default();
    if let Ok(old) = std::fs::read(&index_path) {
        let count = garc::u32_at(&old, 0)? as usize;
        for i in 0..count {
            entries.insert(garc::u32_at(&old, 4 + 8 * i)?, garc::u32_at(&old, 8 + 8 * i)?);
        }
    }
    entries.insert(id as u32, size as u32);
    let mut index = (entries.len() as u32).to_le_bytes().to_vec();
    for (id, size) in entries {
        index.extend_from_slice(&id.to_le_bytes());
        index.extend_from_slice(&size.to_le_bytes());
    }
    std::fs::write(&index_path, index).map_err(|e| format!("{}: {e}", index_path.display()))
}

/// Takes a file out of the list `index.bin` of a folder of the SD card. The
/// list goes when it has no file left.
pub fn remove_index(folder: &Path, id: usize) -> Result<(), String> {
    let index_path = folder.join("index.bin");
    let Ok(old) = std::fs::read(&index_path) else { return Ok(()) };
    let count = garc::u32_at(&old, 0)? as usize;
    let mut entries: BTreeMap<u32, u32> = Default::default();
    for i in 0..count {
        entries.insert(garc::u32_at(&old, 4 + 8 * i)?, garc::u32_at(&old, 8 + 8 * i)?);
    }
    entries.remove(&(id as u32));
    if entries.is_empty() {
        return std::fs::remove_file(&index_path).map_err(|e| format!("{}: {e}", index_path.display()));
    }
    let mut index = (entries.len() as u32).to_le_bytes().to_vec();
    for (id, size) in entries {
        index.extend_from_slice(&id.to_le_bytes());
        index.extend_from_slice(&size.to_le_bytes());
    }
    std::fs::write(&index_path, index).map_err(|e| format!("{}: {e}", index_path.display()))
}
