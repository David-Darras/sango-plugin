//! A reader for the model files (BCH) of the 3DS.
//!
//! It finds the meshes of the first model: for each mesh, the name of its
//! material, the vertices and the triangles.

use std::collections::HashMap;

use crate::garc::u32_at;

/// One mesh of a model.
#[derive(Default, Clone)]
pub struct Mesh {
    /// The name of the material. In a map block it also names the kind of
    /// object (ground type, wall of a house, and so on).
    pub material: String,
    /// The name of the picture that the material uses.
    pub texture: String,
    /// True for the tall grass of the encounters.
    pub encounter: bool,
    pub positions: Vec<[f32; 3]>,
    pub normals: Vec<[f32; 3]>,
    pub uvs: Vec<[f32; 2]>,
    pub colors: Vec<[u8; 4]>,
    /// Where the three numbers of each position are in the file, so that
    /// a tool can change them in place.
    pub position_offsets: Vec<usize>,
    /// Where the three numbers of each normal are in the file.
    pub normal_offsets: Vec<usize>,
    /// Where the lists of the mesh are in the file. A tool changes them to
    /// remove or add triangles.
    pub places: Places,
    /// Three indices for each triangle.
    pub indices: Vec<u32>,
}

/// Where the data of a mesh is in the file (offsets from the start of the
/// file).
#[derive(Default, Clone, Debug)]
pub struct Places {
    /// The word that gives the number of indices.
    pub index_count_at: usize,
    /// The word that gives where the list of indices is (from the raw data).
    pub index_start_at: usize,
    /// The word that gives where the vertices are (from the raw data).
    pub vertex_start_at: usize,
    /// The start of the list of indices.
    pub indices_at: usize,
    /// The start of the vertices.
    pub vertices_at: usize,
    /// 1 or 2 bytes for an index.
    pub index_width: usize,
    /// The size of a vertex, in bytes.
    pub stride: usize,
}

/// A picture, with the first row at the top.
pub struct Texture {
    pub name: String,
    pub width: usize,
    pub height: usize,
    pub rgba: Vec<u8>,
}

/// A file read in memory, with the addresses of its sections.
pub struct Bch<'a> {
    pub data: &'a [u8],
    pub content: usize,
    pub strings: usize,
    pub strings_length: usize,
    pub commands: usize,
    pub raw_data: usize,
}

/// The registers of a list of commands of the graphics chip.
fn registers(words: &[u32]) -> HashMap<u16, u32> {
    let mut out = HashMap::new();
    let mut i = 0;
    while i + 1 < words.len() {
        let param = words[i];
        let header = words[i + 1];
        let reg = (header & 0xffff) as u16;
        let extra = ((header >> 20) & 0x7ff) as usize;
        let consecutive = header >> 31 != 0;
        if i + 2 + extra > words.len() {
            break;
        }
        out.insert(reg, param);
        for k in 0..extra {
            let target = if consecutive { reg + 1 + k as u16 } else { reg };
            out.insert(target, words[i + 2 + k]);
        }
        i += 2 + extra + (extra & 1);
    }
    out
}

/// The place of the value of each register in a list of commands: the index
/// of the word. A tool needs it to change a value in the file.
fn register_places(words: &[u32]) -> HashMap<u16, usize> {
    let mut out = HashMap::new();
    let mut i = 0;
    while i + 1 < words.len() {
        let header = words[i + 1];
        let reg = (header & 0xffff) as u16;
        let extra = ((header >> 20) & 0x7ff) as usize;
        let consecutive = header >> 31 != 0;
        if i + 2 + extra > words.len() {
            break;
        }
        out.insert(reg, i);
        for k in 0..extra {
            let target = if consecutive { reg + 1 + k as u16 } else { reg };
            out.insert(target, i + 2 + k);
        }
        i += 2 + extra + (extra & 1);
    }
    out
}

/// What an attribute of a vertex means. The file does not say it, so the
/// kind comes from the format.
#[derive(Clone, Copy, PartialEq, Debug)]
enum Field {
    Position,
    Normal,
    Uv,
    Color,
    Other,
}

/// How the vertices of one mesh are stored.
struct Layout {
    stride: usize,
    buffer_offset: usize,
    slots: Vec<usize>,
    formats: Vec<u32>,
    kinds: Vec<Field>,
}

struct RawMesh {
    mesh: Mesh,
    layout: Layout,
    index_offset: usize,
    index_count: usize,
    index_count_at: usize,
    index_start_at: usize,
    vertex_start_at: usize,
}

impl<'a> Bch<'a> {
    pub fn parse(data: &'a [u8]) -> Result<Self, String> {
        if data.get(..4) != Some(b"BCH\0") {
            return Err("not a BCH file".into());
        }
        Ok(Bch {
            data,
            content: u32_at(data, 8)? as usize,
            strings: u32_at(data, 12)? as usize,
            commands: u32_at(data, 16)? as usize,
            raw_data: u32_at(data, 20)? as usize,
            strings_length: u32_at(data, 36)? as usize,
        })
    }

    fn u32(&self, pos: usize) -> Result<u32, String> {
        u32_at(self.data, pos)
    }

    fn text_at(&self, pos: usize) -> String {
        let tail = self.data.get(pos..).unwrap_or(&[]);
        let end = tail.iter().position(|&b| b == 0).unwrap_or(tail.len()).min(80);
        String::from_utf8_lossy(&tail[..end]).into_owned()
    }

    /// All the names of the file.
    pub fn names(&self) -> Vec<String> {
        let end = (self.strings + self.strings_length).min(self.data.len());
        self.data[self.strings..end]
            .split(|&b| b == 0)
            .filter(|s| !s.is_empty())
            .map(|s| String::from_utf8_lossy(s).into_owned())
            .collect()
    }

    fn command_words(&self, offset: usize, count: usize) -> Result<Vec<u32>, String> {
        (0..count).map(|i| self.u32(self.commands + offset + 4 * i)).collect()
    }

    /// The meshes of the first model.
    pub fn meshes(&self) -> Result<Vec<Mesh>, String> {
        let table = self.content + self.u32(self.content)? as usize;
        let model = self.content + self.u32(table)? as usize;
        let material_count = self.u32(model + 0x38)? as usize;
        let name_table = self.content + self.u32(model + 0x3c)? as usize;
        let object_table = self.content + self.u32(model + 0x40)? as usize;
        let object_count = self.u32(model + 0x44)? as usize;

        // The names are in a tree: the first entry is the root.
        let material_names: Vec<String> = (0..material_count)
            .map(|i| {
                let entry = name_table + 12 * (i + 1);
                self.u32(entry + 8).map(|o| self.text_at(self.strings + o as usize)).unwrap_or_default()
            })
            .collect();

        let material_table = self.content + self.u32(model + 0x34)? as usize;
        // A material has three places for a picture. A place can hold a
        // dummy picture that the game replaces at run time (for example the
        // picture of a projected shadow), so the first real one is taken.
        let texture_names: Vec<String> = (0..material_count)
            .map(|i| {
                (0..3)
                    .filter_map(|unit| self.u32(material_table + 0x2c * i + 0x1c + 4 * unit).ok())
                    .filter(|&offset| offset != 0)
                    .map(|offset| self.text_at(self.strings + offset as usize))
                    .find(|name| !name.is_empty() && !name.contains("dummy"))
                    .unwrap_or_default()
            })
            .collect();

        let mut raws = Vec::with_capacity(object_count);
        for i in 0..object_count {
            raws.push(self.read_object(object_table + 0x38 * i, &material_names, &texture_names)?);
        }
        self.read_indices(&mut raws)?;
        Ok(raws.into_iter().map(|r| r.mesh).collect())
    }

    /// The pictures of the file, decoded to RGBA. A picture with a format
    /// that is not known is left out.
    pub fn textures(&self) -> Result<Vec<Texture>, String> {
        let table = self.content + self.u32(self.content + 36)? as usize;
        let count = self.u32(self.content + 40)? as usize;
        let mut out = Vec::with_capacity(count);
        for i in 0..count {
            let desc = self.content + self.u32(table + 4 * i)? as usize;
            let words = self.u32(desc + 4)? as usize;
            let regs = registers(&self.command_words(self.u32(desc)? as usize, words)?);
            let (Some(&dim), Some(&address), Some(&kind)) = (regs.get(&0x82), regs.get(&0x85), regs.get(&0x8e)) else {
                continue;
            };
            let (width, height) = ((dim >> 16) as usize, (dim & 0xffff) as usize);
            let format = kind & 0xf;
            let name = self.text_at(self.strings + self.u32(desc + 28)? as usize);
            let start = self.raw_data + address as usize;
            let size = crate::tex::byte_size(format, width, height);
            let Some(bytes) = self.data.get(start..start + size) else { continue };
            if let Some(rgba) = crate::tex::decode(format, width, height, bytes) {
                out.push(Texture { name, width, height, rgba });
            }
        }
        Ok(out)
    }

    /// Reads the vertices of one mesh. The indices come later.
    fn read_object(&self, pos: usize, material_names: &[String], texture_names: &[String]) -> Result<RawMesh, String> {
        let material = (self.u32(pos)? & 0xffff) as usize;
        let vertex_commands = self.u32(pos + 8)? as usize;
        let vertex_words = self.u32(pos + 12)? as usize;
        let face_header = self.content + self.u32(pos + 16)? as usize;

        let regs = registers(&self.command_words(vertex_commands, vertex_words)?);
        let get = |r: u16| regs.get(&r).copied().ok_or(format!("register {r:#x} is missing"));
        let format_low = get(0x201)?;
        let format_high = get(0x202)?;
        let buffer_offset = get(0x203)? as usize;
        let order = get(0x204)?;
        let info = get(0x205)?;
        let stride = ((info >> 16) & 0xff) as usize;
        let attribute_count = (info >> 28) as usize;

        let formats: Vec<u32> = (0..8)
            .map(|k| (format_low >> (4 * k)) & 0xf)
            .chain((0..4).map(|k| (format_high >> (4 * k)) & 0xf))
            .collect();
        let slots: Vec<usize> = (0..attribute_count).map(|k| ((order >> (4 * k)) & 0xf) as usize).collect();

        // The kind of each attribute, from its format.
        let mut kinds = Vec::new();
        let (mut has_normal, mut has_uv) = (false, false);
        for (n, &slot) in slots.iter().enumerate() {
            let f = formats[slot];
            let (ty, count) = (f & 3, (f >> 2) + 1);
            kinds.push(match (ty, count) {
                (3, 3) if n == 0 => Field::Position,
                (3, 3) if !has_normal => { has_normal = true; Field::Normal }
                (3, 2) if !has_uv => { has_uv = true; Field::Uv }
                (1, 4) => Field::Color,
                _ => Field::Other,
            });
        }

        // The commands of the face give the indices.
        let face_commands = self.u32(face_header + 0x2c)? as usize;
        let face_words = self.u32(face_header + 0x30)? as usize;
        let face_list = self.command_words(face_commands, face_words)?;
        let face_places = register_places(&face_list);
        let vertex_places = register_places(&self.command_words(vertex_commands, vertex_words)?);
        let word_at = |base: usize, places: &HashMap<u16, usize>, reg: u16| {
            places.get(&reg).map(|w| self.commands + base + 4 * w).ok_or(format!("register {reg:#x} is missing"))
        };
        let index_count_at = word_at(face_commands, &face_places, 0x228)?;
        let index_start_at = word_at(face_commands, &face_places, 0x227)?;
        let vertex_start_at = word_at(vertex_commands, &vertex_places, 0x203)?;
        let face_regs = registers(&face_list);
        let index_offset = (face_regs.get(&0x227).copied().ok_or("no index register")? & 0x7fff_ffff) as usize;
        let index_count = face_regs.get(&0x228).copied().ok_or("no index count")? as usize;

        // The vertex count is known only with the indices, so the vertices
        // are read later.
        let mesh = Mesh {
            material: material_names.get(material).cloned().unwrap_or_else(|| format!("material{material}")),
            texture: texture_names.get(material).cloned().unwrap_or_default(),
            ..Default::default()
        };
        Ok(RawMesh {
            mesh,
            layout: Layout { stride, buffer_offset, slots, formats, kinds },
            index_offset,
            index_count,
            index_count_at,
            index_start_at,
            vertex_start_at,
        })
    }

    /// The mean length of the edges of the first triangles of a mesh, when
    /// its indices have this width (1 or 2 bytes). A very large value means
    /// that the width is wrong.
    fn edge_length(&self, raw: &RawMesh, width: usize) -> f32 {
        let layout = &raw.layout;
        let mut inside = 0;
        for (n, &slot) in layout.slots.iter().enumerate() {
            if layout.kinds[n] == Field::Position {
                break;
            }
            let format = layout.formats[slot];
            let size = match format & 3 {
                3 => 4,
                2 => 2,
                _ => 1,
            };
            inside += size * ((format >> 2) as usize + 1);
        }
        let start = self.raw_data + raw.index_offset;
        let index = |k: usize| -> Option<usize> {
            if width == 2 {
                let b = self.data.get(start + 2 * k..start + 2 * k + 2)?;
                Some(u16::from_le_bytes([b[0], b[1]]) as usize)
            } else {
                self.data.get(start + k).map(|&b| b as usize)
            }
        };
        let position = |v: usize| -> Option<[f32; 3]> {
            let at = self.raw_data + layout.buffer_offset + v * layout.stride + inside;
            let b = self.data.get(at..at + 12)?;
            let f = |o: usize| f32::from_le_bytes([b[o], b[o + 1], b[o + 2], b[o + 3]]);
            Some([f(0), f(4), f(8)])
        };
        let distance = |a: [f32; 3], b: [f32; 3]| ((a[0] - b[0]).powi(2) + (a[1] - b[1]).powi(2) + (a[2] - b[2]).powi(2)).sqrt();
        let mut total = 0.0f32;
        let mut edges = 0;
        for t in 0..(raw.index_count / 3).min(200) {
            let corners: Option<Vec<[f32; 3]>> = (0..3).map(|c| index(3 * t + c).and_then(position)).collect();
            let Some(p) = corners else { return f32::MAX };
            total += distance(p[0], p[1]) + distance(p[1], p[2]) + distance(p[2], p[0]);
            edges += 3;
        }
        if edges == 0 {
            return 0.0;
        }
        let mean = total / edges as f32;
        if mean.is_finite() { mean } else { f32::MAX }
    }

    /// Reads the indices and then the vertices of all the meshes.
    ///
    /// The index lists are in one area for 16 bit numbers and in one area for
    /// 8 bit numbers. The file does not tell which one a list uses, so the
    /// width comes from the distance to the next list.
    fn read_indices(&self, raws: &mut [RawMesh]) -> Result<(), String> {
        let mut order: Vec<usize> = (0..raws.len()).collect();
        order.sort_by_key(|&i| raws[i].index_offset);
        let mut width = vec![0usize; raws.len()];
        for pair in order.windows(2) {
            let (a, b) = (pair[0], pair[1]);
            let gap = raws[b].index_offset.saturating_sub(raws[a].index_offset);
            let count = raws[a].index_count;
            if gap >= count * 2 && gap <= count * 2 + 3 {
                width[a] = 2;
            } else if gap >= count && gap <= count + 3 {
                width[a] = 1;
            }
        }
        let boundary = (0..raws.len())
            .filter(|&i| width[i] == 1)
            .map(|i| raws[i].index_offset)
            .min()
            .unwrap_or(usize::MAX);
        for i in 0..raws.len() {
            if width[i] == 0 {
                width[i] = if raws[i].index_offset < boundary { 2 } else { 1 };
            }
        }

        // The geometry tells better than the distance between the lists: with
        // the right width the triangles are small. A file that a tool wrote
        // can have its lists in any order.
        for (i, raw) in raws.iter().enumerate() {
            let (narrow, wide) = (self.edge_length(raw, 1), self.edge_length(raw, 2));
            if narrow < 0.7 * wide {
                width[i] = 1;
            } else if wide < 0.7 * narrow {
                width[i] = 2;
            }
        }

        for (i, raw) in raws.iter_mut().enumerate() {
            let Layout { stride, buffer_offset, slots, formats, kinds } = &raw.layout;
            let start = self.raw_data + raw.index_offset;
            let mut indices = Vec::with_capacity(raw.index_count);
            for k in 0..raw.index_count {
                let value = if width[i] == 2 {
                    let at = start + 2 * k;
                    let b = self.data.get(at..at + 2).ok_or("index outside the file")?;
                    u16::from_le_bytes([b[0], b[1]]) as u32
                } else {
                    *self.data.get(start + k).ok_or("index outside the file")? as u32
                };
                indices.push(value);
            }
            let vertex_count = indices.iter().max().map(|m| *m as usize + 1).unwrap_or(0);
            let mut mesh = Mesh {
                material: raw.mesh.material.clone(),
                texture: raw.mesh.texture.clone(),
                ..Default::default()
            };
            for v in 0..vertex_count {
                let mut at = self.raw_data + buffer_offset + v * stride;
                for (n, &slot) in slots.iter().enumerate() {
                    let f = formats[slot];
                    let (ty, count) = (f & 3, ((f >> 2) + 1) as usize);
                    let attribute_at = at;
                    let mut values = [0f32; 4];
                    for c in 0..count {
                        let (value, size) = match ty {
                            3 => (f32::from_le_bytes(self.take(at)?), 4),
                            2 => (i16::from_le_bytes([self.byte(at)?, self.byte(at + 1)?]) as f32, 2),
                            1 => (self.byte(at)? as f32, 1),
                            _ => (self.byte(at)? as i8 as f32, 1),
                        };
                        values[c] = value;
                        at += size;
                    }
                    match kinds[n] {
                        Field::Position => {
                            mesh.positions.push([values[0], values[1], values[2]]);
                            mesh.position_offsets.push(attribute_at);
                        }
                        Field::Normal => {
                            mesh.normals.push([values[0], values[1], values[2]]);
                            mesh.normal_offsets.push(attribute_at);
                        }
                        Field::Uv => mesh.uvs.push([values[0], values[1]]),
                        Field::Color => mesh.colors.push([values[0] as u8, values[1] as u8, values[2] as u8, values[3] as u8]),
                        Field::Other => {}
                    }
                }
            }
            mesh.indices = indices;
            mesh.places = Places {
                index_count_at: raw.index_count_at,
                index_start_at: raw.index_start_at,
                vertex_start_at: raw.vertex_start_at,
                indices_at: start,
                vertices_at: self.raw_data + buffer_offset,
                index_width: width[i],
                stride: *stride,
            };
            raw.mesh = mesh;
        }
        Ok(())
    }

    fn byte(&self, at: usize) -> Result<u8, String> {
        self.data.get(at).copied().ok_or_else(|| "vertex outside the file".to_string())
    }

    fn take(&self, at: usize) -> Result<[u8; 4], String> {
        let b = self.data.get(at..at + 4).ok_or("vertex outside the file")?;
        Ok([b[0], b[1], b[2], b[3]])
    }
}
