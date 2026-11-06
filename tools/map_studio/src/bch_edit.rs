//! Adds an object (a mesh) to a model file (BCH).
//!
//! The file has four sections: the content (the tables and the structures),
//! the strings, the commands for the graphics chip, and the raw data (the
//! vertices and the indices). A table says which words the game corrects when
//! it loads the file: for each one, a kind and the place of the word, counted
//! in words from the start of its section.
//!
//! The tool adds an object by copying an object that the model already has:
//! its structures keep their form, and the tool only changes what must change.
//! The new data goes at the end of each section. The sections after it move,
//! and the header follows. Every pointer that the tool writes gets its entry in
//! the table of relocation.

use crate::bch::register_places;

/// The kind of the entries of the table of relocation for the words of the
/// commands: those words are in the commands, and the kinds are 0x20 and more.
const COMMANDS_KIND: u8 = 0x20;
/// The entries for the pointers to the strings (kind 1) count their place in
/// bytes. The other entries count it in words of 4 bytes.
const STRING_KIND: u8 = 1;
/// A list of 16 bit indices, and a list of 8 bit indices.
const INDEX16: u8 = 0x27;
const INDEX8: u8 = 0x28;
/// The size of an entry of the table of the objects, in bytes.
const OBJECT_SIZE: usize = 0x38;
/// The size of the structure of the content that an object has, when the file does not
/// show another one (the objects have 0xd0 bytes, or a little less).
const BLOCK_SIZE: usize = 0xd0;
/// The alignment of the sections and of the new data.
const ALIGNMENT: usize = 0x80;

/// The geometry of the new object.
pub struct NewGeometry {
    /// The vertices, one after the other, in the format of the object that is
    /// copied.
    pub vertices: Vec<u8>,
    pub indices: Vec<u32>,
    /// The size of an index: 1 or 2 bytes.
    pub width: usize,
    /// The centre of the box of the object.
    pub centre: [f32; 3],
}

/// The place of the word of an entry of the table, in bytes from the start of
/// its section.
fn place_of(entry: u32) -> usize {
    let counted = (entry & 0x01ff_ffff) as usize;
    if (entry >> 25) as u8 == STRING_KIND { counted } else { counted * 4 }
}

fn read(b: &[u8], p: usize) -> usize {
    u32::from_le_bytes([b[p], b[p + 1], b[p + 2], b[p + 3]]) as usize
}

fn write(b: &mut [u8], p: usize, v: usize) {
    b[p..p + 4].copy_from_slice(&(v as u32).to_le_bytes());
}

fn align(v: usize, a: usize) -> usize {
    v.div_ceil(a) * a
}

/// A model file that grows.
struct Builder {
    /// The file as it was. The words that already exist change in it.
    bytes: Vec<u8>,
    content: usize,
    strings: usize,
    commands: usize,
    raw: usize,
    raw_ext: usize,
    reloc: usize,
    entries: Vec<u32>,
    content_extra: Vec<u8>,
    commands_extra: Vec<u8>,
    raw_extra: Vec<u8>,
    added: Vec<u32>,
}

impl Builder {
    fn new(part: &[u8]) -> Result<Builder, String> {
        if part.get(..4) != Some(b"BCH\0") || part.len() < 0x44 {
            return Err("not a model file".into());
        }
        let (raw_ext, reloc) = (read(part, 24), read(part, 28));
        if raw_ext != reloc || read(part, 48) != 0 {
            return Err("this model has a layout that the tool cannot grow".into());
        }
        let reloc_length = read(part, 52);
        let entries = (0..reloc_length / 4).map(|i| read(part, reloc + 4 * i) as u32).collect();
        Ok(Builder {
            bytes: part.to_vec(),
            content: read(part, 8),
            strings: read(part, 12),
            commands: read(part, 16),
            raw: read(part, 20),
            raw_ext,
            reloc,
            entries,
            content_extra: Vec::new(),
            commands_extra: Vec::new(),
            raw_extra: Vec::new(),
            added: Vec::new(),
        })
    }

    fn content_span(&self) -> usize {
        self.strings - self.content
    }

    fn commands_span(&self) -> usize {
        self.raw - self.commands
    }

    fn raw_first(&self) -> usize {
        self.raw_ext - self.raw
    }

    fn content_word(&self, offset: usize) -> usize {
        read(&self.bytes, self.content + offset)
    }

    fn set_content_word(&mut self, offset: usize, value: usize) {
        write(&mut self.bytes, self.content + offset, value);
    }

    /// Adds data at the end of the content. Returns the place of the data,
    /// from the start of the content.
    fn push_content(&mut self, data: &[u8]) -> usize {
        while (self.content_span() + self.content_extra.len()) % 16 != 0 {
            self.content_extra.push(0);
        }
        let at = self.content_span() + self.content_extra.len();
        self.content_extra.extend_from_slice(data);
        at
    }

    fn push_commands(&mut self, data: &[u8]) -> usize {
        while (self.commands_span() + self.commands_extra.len()) % 16 != 0 {
            self.commands_extra.push(0);
        }
        let at = self.commands_span() + self.commands_extra.len();
        self.commands_extra.extend_from_slice(data);
        at
    }

    fn push_raw(&mut self, data: &[u8]) -> usize {
        while (self.raw_first() + self.raw_extra.len()) % 16 != 0 {
            self.raw_extra.push(0);
        }
        let at = self.raw_first() + self.raw_extra.len();
        self.raw_extra.extend_from_slice(data);
        at
    }

    /// The entries of the table of relocation for words of one section, in a
    /// range of bytes from the start of the section: the kind and the place.
    fn entries_in(&self, commands: bool, from: usize, to: usize) -> Vec<(u8, usize)> {
        self.entries
            .iter()
            .filter_map(|&e| {
                let kind = (e >> 25) as u8;
                let place = place_of(e);
                ((kind >= COMMANDS_KIND) == commands && place >= from && place < to).then_some((kind, place))
            })
            .collect()
    }

    fn add_entry(&mut self, kind: u8, place: usize) {
        let counted = if kind == STRING_KIND { place } else { place / 4 };
        self.added.push((u32::from(kind) << 25) | counted as u32);
    }

    /// Builds the new file.
    fn finish(self) -> Vec<u8> {
        let content_pad = align(self.content_extra.len(), ALIGNMENT);
        let commands_pad = align(self.commands_extra.len(), ALIGNMENT);
        let old = &self.bytes;
        let mut out = old[..self.strings].to_vec();
        out.extend_from_slice(&self.content_extra);
        out.resize(self.strings + content_pad, 0);
        let strings = out.len();
        out.extend_from_slice(&old[self.strings..self.commands]);
        let commands = out.len();
        out.extend_from_slice(&old[self.commands..self.raw]);
        out.extend_from_slice(&self.commands_extra);
        out.resize(commands + self.commands_span() + commands_pad, 0);
        let raw = out.len();
        out.extend_from_slice(&old[self.raw..self.raw_ext]);
        out.extend_from_slice(&self.raw_extra);
        let raw_length = self.raw_first() + self.raw_extra.len();
        out.resize(align(raw + raw_length, ALIGNMENT), 0);
        let reloc = out.len();

        // The entries of the commands come first, in the order of their place.
        let mut entries: Vec<u32> = self.entries.iter().chain(&self.added).copied().collect();
        let (mut in_commands, in_content): (Vec<u32>, Vec<u32>) = entries.drain(..).partition(|e| (e >> 25) as u8 >= COMMANDS_KIND);
        in_commands.sort_by_key(|e| e & 0x01ff_ffff);
        for e in in_commands.iter().chain(&in_content) {
            out.extend_from_slice(&e.to_le_bytes());
        }
        let reloc_length = 4 * (in_commands.len() + in_content.len());
        out.resize(align(out.len(), ALIGNMENT), 0);

        write(&mut out, 12, strings);
        write(&mut out, 16, commands);
        write(&mut out, 20, raw);
        write(&mut out, 24, reloc);
        write(&mut out, 28, reloc);
        write(&mut out, 32, self.content_span() + content_pad);
        write(&mut out, 40, self.commands_span() + commands_pad);
        write(&mut out, 44, raw_length);
        write(&mut out, 52, reloc_length);
        let _ = self.reloc;
        out
    }
}

/// Adds an object to the model that is in `part`. The new object is a copy of
/// the object number `source`, with the geometry that is given: it has the same
/// material and goes at the end of the group of the source in the table of the
/// objects. Returns the new file and the number of the new object in the table.
pub fn add_object(part: &[u8], source: usize, geometry: &NewGeometry) -> Result<(Vec<u8>, usize), String> {
    let mut b = Builder::new(part)?;
    let table = b.content_word(0);
    let model = b.content_word(table);
    let objects = b.content_word(model + 0x40);
    let count = b.content_word(model + 0x44);
    if source >= count {
        return Err("no such object".into());
    }
    let old_entry = |b: &Builder, i: usize| -> Vec<usize> { (0..14).map(|j| b.content_word(objects + OBJECT_SIZE * i + 4 * j)).collect() };
    let entry = old_entry(&b, source);

    // The groups of the table: the model keeps the start and the end of each.
    let bounds: Vec<usize> = (0..8).map(|k| b.content_word(model + 0x48 + 4 * k)).collect();
    let source_place = objects + OBJECT_SIZE * source;
    let group = (0..4)
        .find(|&g| bounds[2 * g] <= source_place && source_place < bounds[2 * g + 1])
        .ok_or("the object is in no group")?;
    let insert_place = bounds[2 * group + 1];
    let insert_at = (insert_place - objects) / OBJECT_SIZE;

    // The size of the structure of the content of an object.
    let mut starts: Vec<usize> = (0..count).map(|i| old_entry(&b, i)[4]).collect();
    starts.sort_unstable();
    let block_size = starts.iter().find(|&&s| s > entry[4]).map_or(BLOCK_SIZE, |&next| next - entry[4]);
    if block_size > 4 * BLOCK_SIZE {
        return Err(format!("the object has a structure of {block_size:#x} bytes, which is more than the tool expects"));
    }

    // 1. The data of the geometry, in the raw data.
    let vertices_at = b.push_raw(&geometry.vertices);
    let mut index_bytes = Vec::new();
    for &i in &geometry.indices {
        if geometry.width == 2 {
            index_bytes.extend_from_slice(&(i as u16).to_le_bytes());
        } else {
            index_bytes.push(i as u8);
        }
    }
    let indices_at = b.push_raw(&index_bytes);

    // 2. The commands of the vertices: the same, with the place of the new
    // vertices.
    let (vertex_commands, vertex_words) = (entry[2], entry[3]);
    let words_of = |b: &Builder, at: usize, words: usize| -> Vec<u32> {
        (0..words).map(|k| read(&b.bytes, b.commands + at + 4 * k) as u32).collect()
    };
    let mut vertex_list = words_of(&b, vertex_commands, vertex_words);
    let places = register_places(&vertex_list);
    let start_word = *places.get(&0x203).ok_or("the commands have no place for the vertices")?;
    vertex_list[start_word] = vertices_at as u32;
    let new_vertex_commands = b.push_commands(&to_bytes(&vertex_list));
    for (kind, place) in b.entries_in(true, vertex_commands, vertex_commands + 4 * vertex_words) {
        b.add_entry(kind, new_vertex_commands + (place - vertex_commands));
    }

    // 3. The commands that follow the drawing.
    let (end_commands, end_words) = (entry[6], entry[7]);
    let end_list = words_of(&b, end_commands, end_words);
    let new_end_commands = b.push_commands(&to_bytes(&end_list));
    for (kind, place) in b.entries_in(true, end_commands, end_commands + 4 * end_words) {
        b.add_entry(kind, new_end_commands + (place - end_commands));
    }

    // 4. The structure of the face, and its commands.
    let face_commands = b.content_word(entry[4] + 0x2c);
    let face_words = b.content_word(entry[4] + 0x30);
    let mut face_list = words_of(&b, face_commands, face_words);
    let places = register_places(&face_list);
    face_list[*places.get(&0x227).ok_or("no place for the indices")?] = indices_at as u32;
    face_list[*places.get(&0x228).ok_or("no place for the number of indices")?] = geometry.indices.len() as u32;
    let new_face_commands = b.push_commands(&to_bytes(&face_list));
    let index_kind = if geometry.width == 2 { INDEX16 } else { INDEX8 };
    for (kind, place) in b.entries_in(true, face_commands, face_commands + 4 * face_words) {
        let kind = if kind == INDEX16 || kind == INDEX8 { index_kind } else { kind };
        b.add_entry(kind, new_face_commands + (place - face_commands));
    }
    let mut block = b.bytes[b.content + entry[4]..b.content + entry[4] + block_size].to_vec();
    let content_entries = b.entries_in(false, entry[4], entry[4] + block_size);
    // The pointers that stay inside the structure follow the copy.
    let new_block = b.content_span() + b.content_extra.len();
    let new_block = new_block.div_ceil(16) * 16;
    for &(kind, place) in &content_entries {
        let inside = place - entry[4];
        let value = read(&block, inside);
        if kind == 0 && value >= entry[4] && value < entry[4] + block_size {
            write(&mut block, inside, value - entry[4] + new_block);
        }
    }
    write(&mut block, 0x2c, new_face_commands);
    let placed = b.push_content(&block);
    debug_assert_eq!(placed, new_block);
    for &(kind, place) in &content_entries {
        b.add_entry(kind, new_block + (place - entry[4]));
    }

    // 5. The entry of the new object.
    let mut new_entry = entry.clone();
    new_entry[2] = new_vertex_commands;
    new_entry[4] = new_block;
    new_entry[6] = new_end_commands;
    if entry[13] >= entry[4] && entry[13] < entry[4] + block_size {
        new_entry[13] = entry[13] - entry[4] + new_block;
    }
    for (k, c) in geometry.centre.iter().enumerate() {
        new_entry[8 + k] = c.to_bits() as usize;
    }

    // 6. The new table of the objects, with the new object after the last
    // object of its group.
    let mut table_bytes = Vec::with_capacity((count + 1) * OBJECT_SIZE);
    for i in 0..=count {
        let words = if i == insert_at {
            new_entry.clone()
        } else {
            old_entry(&b, if i > insert_at { i - 1 } else { i })
        };
        for w in words {
            table_bytes.extend_from_slice(&(w as u32).to_le_bytes());
        }
    }
    let new_table = b.push_content(&table_bytes);
    // The words of the table that the game corrects.
    let table_entries = b.entries_in(false, objects, objects + count * OBJECT_SIZE);
    for &(kind, place) in &table_entries {
        let (index, inside) = ((place - objects) / OBJECT_SIZE, (place - objects) % OBJECT_SIZE);
        let new_index = if index >= insert_at { index + 1 } else { index };
        b.add_entry(kind, new_table + new_index * OBJECT_SIZE + inside);
    }
    for (kind, place) in table_entries.iter().filter(|(_, p)| (p - objects) / OBJECT_SIZE == source) {
        b.add_entry(*kind, new_table + insert_at * OBJECT_SIZE + (place - objects) % OBJECT_SIZE);
    }

    // 7. The header of the model: the table, the number of objects and the
    // groups.
    // The game keeps a little memory for each object while it draws: the
    // header says how much (8 bytes) and how many addresses (2). Both grow with
    // the number of objects. If they do not, the game writes outside its
    // memory.
    let memory = read(&b.bytes, 56);
    write(&mut b.bytes, 56, memory + 8);
    let addresses = u16::from_le_bytes([b.bytes[66], b.bytes[67]]);
    b.bytes[66..68].copy_from_slice(&(addresses + 2).to_le_bytes());
    b.set_content_word(model + 0x40, new_table);
    b.set_content_word(model + 0x44, count + 1);
    for (k, &p) in bounds.iter().enumerate() {
        let shifted = if p >= insert_place { p + OBJECT_SIZE } else { p };
        b.set_content_word(model + 0x48 + 4 * k, new_table + (shifted - objects));
    }
    Ok((b.finish(), insert_at))
}

fn to_bytes(words: &[u32]) -> Vec<u8> {
    words.iter().flat_map(|w| w.to_le_bytes()).collect()
}

/// Looks for what is wrong in a model file, without the game: every word that
/// the table of relocation names must point inside its target section, and every
/// object must have the same words in the table as the others. Returns the
/// problems that it found.
pub fn validate(part: &[u8]) -> Vec<String> {
    let mut problems = Vec::new();
    let b = match Builder::new(part) {
        Ok(b) => b,
        Err(message) => return vec![message],
    };
    let (content_span, commands_span) = (b.content_span(), b.commands_span());
    let raw_length = read(part, 44);
    if read(part, 32) > content_span || read(part, 40) > commands_span {
        problems.push("a length in the header is bigger than its section".into());
    }
    for &e in &b.entries {
        let kind = (e >> 25) as u8;
        let place = place_of(e);
        let (limit, name) = if kind >= COMMANDS_KIND { (commands_span, "commands") } else { (content_span, "content") };
        if place + 4 > limit {
            problems.push(format!("an entry of kind {kind:#x} is outside the {name}: {place:#x}"));
            continue;
        }
        let word = if kind >= COMMANDS_KIND { read(part, b.commands + place) } else { read(part, b.content + place) };
        let bad = match kind {
            0 => word > content_span,
            2 => word > commands_span,
            1 => word > b.commands - b.strings,
            0x26 | 0x27 | 0x28 | 0x2e => word > raw_length,
            _ => false,
        };
        if bad {
            problems.push(format!("a word of kind {kind:#x} at {name}+{place:#x} points outside: {word:#x}"));
        }
    }
    // The memory that the header gives for the objects.
    {
        let table = b.content_word(0);
        let model = b.content_word(table);
        let objects = b.content_word(model + 0x44);
        let addresses = u16::from_le_bytes([part[66], part[67]]) as usize;
        if read(part, 56) != 8 * objects || addresses != 2 * objects {
            problems.push(format!(
                "the header gives {} bytes and {} addresses for {} objects, not {} and {}",
                read(part, 56), addresses, objects, 8 * objects, 2 * objects
            ));
        }
    }
    // The objects must have the same flagged words.
    let table = b.content_word(0);
    let model = b.content_word(table);
    let objects = b.content_word(model + 0x40);
    let count = b.content_word(model + 0x44);
    let flagged = |object: usize| -> Vec<(usize, u8)> {
        let from = objects + object * OBJECT_SIZE;
        let mut found: Vec<(usize, u8)> =
            b.entries_in(false, from, from + OBJECT_SIZE).into_iter().map(|(k, p)| ((p - from) / 4, k)).collect();
        found.sort_unstable();
        found
    };
    let first = flagged(0);
    for i in 0..count {
        if flagged(i) != first {
            problems.push(format!("object {i} has other relocated words than object 0: {:?}", flagged(i)));
        }
        let entry: Vec<usize> = (0..14).map(|j| b.content_word(objects + OBJECT_SIZE * i + 4 * j)).collect();
        // Each object must have its three entries for the commands of the
        // vertices (the place of the vertices) and one for the indices.
        let (vc, vw) = (entry[2], entry[3]);
        if b.entries_in(true, vc, vc + 4 * vw).len() != 2 {
            problems.push(format!("object {i}: its vertex commands have {} relocated words, not 2", b.entries_in(true, vc, vc + 4 * vw).len()));
        }
        let face = b.content_word(entry[4] + 0x2c);
        let words = b.content_word(entry[4] + 0x30);
        let index_entries = b.entries_in(true, face, face + 4 * words);
        if index_entries.len() != 1 || !matches!(index_entries[0].0, INDEX16 | INDEX8) {
            problems.push(format!("object {i}: its face commands have {index_entries:?} as relocated words, not one list of indices"));
        }
    }
    problems
}
