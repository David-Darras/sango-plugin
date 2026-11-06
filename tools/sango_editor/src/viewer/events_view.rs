//! The events of the map in the viewer: signs, triggers, warps and
//! characters, drawn on the ground, selected, moved, added and saved.

use super::*;
use crate::events::{self, Character, Sign, Trigger, Warp, MapAt, MapEvents};

/// The kinds of events.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Kind {
    Sign,
    Trigger,
    Warp,
    Character,
    /// A door model of the block (it is not an event of the map).
    Prop,
}

impl Kind {
    fn name(self) -> &'static str {
        match self {
            Kind::Sign => "sign",
            Kind::Trigger => "trigger",
            Kind::Warp => "warp",
            Kind::Character => "character",
            Kind::Prop => "door",
        }
    }
    fn colour(self, script: u16) -> [u8; 4] {
        match self {
            Kind::Sign => [40, 220, 235, 120],
            Kind::Trigger => [230, 60, 220, 120],
            Kind::Warp => [120, 240, 40, 130],
            // The scripts 3000 and more (up to 3999) are the trainers.
            Kind::Character if (3000..4000).contains(&script) => [255, 150, 30, 150],
            Kind::Character => [60, 120, 255, 150],
            Kind::Prop => [255, 255, 255, 150],
        }
    }
}

/// The events of the map that the viewer shows.
pub struct EventsState {
    pub loaded: bool,
    pub maps: Vec<MapAt>,
    /// The map that is shown: a place in `maps`.
    pub map: usize,
    /// The events as the game has them, and as the user changed them.
    pub base: MapEvents,
    pub now: MapEvents,
    pub selected: Option<(Kind, usize)>,
    /// The next click puts a new event of this kind.
    pub put: Option<Kind>,
    /// The next click moves the selected event.
    pub moving: bool,
    pub show: [bool; 5],
    /// The character of the game whose values a new character takes.
    pub npc: Option<Character>,
    /// How many blocks around the block are drawn (0: none).
    pub neigh_radius: u32,
    /// What the blocks around were made for (to make them again when it changes).
    pub neigh_key: Option<(bool, u32, usize, Vec<u16>)>,
    /// The blocks of the matrix of the map, and the place that is chosen in it.
    pub matrix: Option<events::MatrixLayout>,
    pub original_matrix: Option<events::MatrixLayout>,
    pub cell: Option<(usize, usize)>,
    /// For each map that is not the one that is edited: its events as the game has
    /// them and as the user has them now (the edited map is in `base` and `now`).
    pub others: Vec<Option<(MapEvents, MapEvents)>>,
    /// For each map: shown or not (all are shown at first).
    pub map_shown: Vec<bool>,
}

impl Default for EventsState {
    fn default() -> Self {
        EventsState {
            loaded: false,
            maps: Vec::new(),
            map: 0,
            base: MapEvents::default(),
            now: MapEvents::default(),
            selected: None,
            put: None,
            moving: false,
            show: [true; 5],
            npc: None,
            neigh_radius: ALL_BLOCKS,
            neigh_key: None,
            matrix: None,
            original_matrix: None,
            cell: None,
            others: Vec::new(),
            map_shown: Vec::new(),
        }
    }
}

/// One event, as a rectangle on the ground of the block.
pub struct EventBox {
    pub kind: Kind,
    pub index: usize,
    /// In units from the middle of the block.
    pub low: [f32; 2],
    pub high: [f32; 2],
    pub label: String,
    pub colour: [u8; 4],
    /// The height of the event (from its data).
    pub height: f32,
    /// The place of its map in the list of the maps.
    pub map: usize,
}

/// A radius that takes all the blocks of the matrix.
const ALL_BLOCKS: u32 = 64;

const MAX_SIGNS: usize = 40;
const MAX_TRIGGERS: usize = 40;
const MAX_WARPS: usize = 24;

impl EventsState {
    pub(super) fn at(&self) -> Option<&MapAt> {
        self.maps.get(self.map)
    }
}

impl Viewer {
    /// Reads the maps of the block and the events of the first one.
    pub(super) fn events_enter(&mut self) {
        if self.ev.loaded {
            return;
        }
        self.ev.loaded = true;
        match crate::events::maps_of_block(&self.arc_dir, self.editor.block_id()) {
            Ok(maps) if !maps.is_empty() => {
                // The map with the most events is the one that the user wants.
                let mut best = (0, 0usize);
                for (i, z) in maps.iter().enumerate() {
                    if let Ok(e) = crate::events::read_map_events(&self.arc_dir, z.map) {
                        let n = e.signs.len() + e.characters.len() + e.warps.len() + e.triggers.len();
                        if n > best.1 {
                            best = (i, n);
                        }
                    }
                }
                // All the maps that use the same matrix are in the same space.
                let chosen = maps[best.0].clone();
                let mut all = maps;
                if let Ok(ids) = events::maps_of_matrix(&self.arc_dir, chosen.matrix) {
                    for map in ids {
                        if !all.iter().any(|z| z.map == map) {
                            all.push(MapAt { map, ..chosen.clone() });
                        }
                    }
                }
                all.sort_by_key(|z| z.map);
                self.ev.map_shown = vec![true; all.len()];
                self.ev.map = all.iter().position(|z| z.map == chosen.map).unwrap_or(0);
                self.ev.maps = all;
                self.events_load_map();
            }
            Ok(_) => self.status = "No map of the game uses this block: there are no events here.".into(),
            Err(e) => self.status = format!("Cannot read the maps: {e}"),
        }
    }

    /// The events of a map as the game has them, and as the file of the tool
    /// and the lines of map_events.txt change them.
    fn load_events(&self, map: usize) -> Result<(MapEvents, MapEvents), String> {
        let base = events::read_map_events(&self.arc_dir, map)?;
        // The file of the map that the tool wrote, else the game.
        let mut now = events::read_map_override(&self.export_dir, map).unwrap_or_else(|| base.clone());
        if let Ok(text) = std::fs::read_to_string(events::file_path(&self.export_dir)) {
            events::apply_file(map, &mut now, &text);
        }
        Ok((base, now))
    }

    /// Reads the events of all the maps of the matrix (the edited one and the others).
    pub(super) fn events_load_map(&mut self) {
        let Some(at) = self.ev.at().cloned() else { return };
        match self.load_events(at.map) {
            Ok((base, now)) => {
                self.ev.base = base;
                self.ev.now = now;
                self.ev.selected = None;
                self.ev.matrix = events::read_matrix(&self.arc_dir, &self.export_dir, at.matrix).ok();
                self.ev.original_matrix = events::read_matrix(&self.arc_dir, "", at.matrix).ok();
                self.ev.neigh_key = None;
                let active = self.ev.map;
                let others: Vec<Option<(MapEvents, MapEvents)>> = self
                    .ev
                    .maps
                    .iter()
                    .enumerate()
                    .map(|(i, z)| if i == active { None } else { self.load_events(z.map).ok() })
                    .collect();
                let total = |e: &MapEvents| e.signs.len() + e.characters.len() + e.warps.len() + e.triggers.len();
                let count: usize = total(&self.ev.now) + others.iter().flatten().map(|(_, n)| total(n)).sum::<usize>();
                self.ev.others = others;
                self.status = format!("{} maps of the matrix {}: {} events in all.", self.ev.maps.len(), at.matrix, count);
            }
            Err(e) => self.status = format!("Cannot read the events: {e}"),
        }
    }

    /// Edits another map of the matrix: the changes of the map that was
    /// edited are kept.
    pub(super) fn switch_map(&mut self, map_index: usize) {
        let old = self.ev.map;
        if map_index == old || map_index >= self.ev.maps.len() {
            return;
        }
        let Some(Some((base, now))) = self.ev.others.get_mut(map_index).map(|o| o.take()) else {
            self.ev.map = old;
            return;
        };
        let previous = (std::mem::replace(&mut self.ev.base, base), std::mem::replace(&mut self.ev.now, now));
        if self.ev.others.len() <= old {
            self.ev.others.resize(old + 1, None);
        }
        self.ev.others[old] = Some(previous);
        self.ev.map = map_index;
        self.ev.selected = None;
        let name = self.ev.at().map_or(0, |z| z.map);
        self.status = format!("Editing the map {} ({}).", name, map_name(name as u16));
    }

    /// The height of the ground at a place, if there is ground.
    fn ground_at(&self, x: f32, z: f32) -> Option<f32> {
        self.pick(Vec3::new(x, 4000.0, z), Vec3::NEG_Y, true).map(|h| h.point.y)
    }

    /// The events of a map (the edited map: `now`).
    fn map_events(&self, map_index: usize) -> Option<&MapEvents> {
        if map_index == self.ev.map {
            Some(&self.ev.now)
        } else {
            self.ev.others.get(map_index).and_then(|o| o.as_ref()).map(|(_, now)| now)
        }
    }

    /// The events of all the maps of the matrix as rectangles on the ground.
    pub(super) fn event_boxes(&self) -> Vec<EventBox> {
        let mut out = Vec::new();
        for zi in 0..self.ev.maps.len() {
            let (col, row) = (self.ev.maps[zi].column, self.ev.maps[zi].row);
            if !self.ev.map_shown.get(zi).copied().unwrap_or(true) && zi != self.ev.map {
                continue;
            }
            let Some(events) = self.map_events(zi) else { continue };
            let active = zi == self.ev.map;
            // The maps that are not edited are a little more transparent, with their number.
            let fade = |mut c: [u8; 4]| {
                if !active {
                    c[3] = (c[3] as f32 * 0.6) as u8;
                }
                c
            };
            let tag = |label: String| if active { label } else { format!("[{}] {label}", self.ev.maps[zi].map) };
            let rect = |x: f32, z: f32, w: f32, d: f32| {
                ([events::tile_to_local(x, col), events::tile_to_local(z, row)], [events::tile_to_local(x + w, col), events::tile_to_local(z + d, row)])
            };
            if self.ev.show[0] {
                for (i, s) in events.signs.iter().enumerate() {
                    let (low, high) = rect(s.x as f32, s.z as f32, s.width.max(1) as f32, s.depth.max(1) as f32);
                    out.push(EventBox { kind: Kind::Sign, index: i, low, high, label: tag(format!("S{i} id {}", s.id)), colour: fade(Kind::Sign.colour(0)), height: s.height as f32, map: zi });
                }
            }
            if self.ev.show[1] {
                for (i, t) in events.triggers.iter().enumerate() {
                    let (low, high) = rect(t.x as f32, t.z as f32, t.width.max(1) as f32, t.depth.max(1) as f32);
                    out.push(EventBox { kind: Kind::Trigger, index: i, low, high, label: tag(format!("T{i} id {}", t.id)), colour: fade(Kind::Trigger.colour(0)), height: t.height as f32, map: zi });
                }
            }
            if self.ev.show[2] {
                for (i, w) in events.warps.iter().enumerate() {
                    let (wd, dp) = (w.width.max(1) as f32, w.depth.max(1) as f32);
                    let (low, high) = rect(w.x as f32 / events::TILE - 0.5, w.z as f32 / events::TILE - 0.5, wd, dp);
                    out.push(EventBox { kind: Kind::Warp, index: i, low, high, label: tag(format!("W{i} to {} ({})", map_name(w.map), w.exit)), colour: fade(Kind::Warp.colour(0)), height: w.y as f32, map: zi });
                }
            }
            if self.ev.show[3] {
                for (i, c) in events.characters.iter().enumerate() {
                    let (low, high) = rect(c.x as f32, c.z as f32, 1.0, 1.0);
                    let label = if (3000..4000).contains(&c.script) {
                        format!("C{i} {} (trainer {})", model_name(c.model), c.script - 3000)
                    } else {
                        format!("C{i} {} (script {})", model_name(c.model), c.script)
                    };
                    out.push(EventBox { kind: Kind::Character, index: i, low, high, label: tag(label), colour: fade(Kind::Character.colour(c.script)), height: c.height, map: zi });
                }
            }
        }
        if self.ev.show[4] {
            if let Some(at) = self.ev.at() {
                let (col, row) = (at.column, at.row);
                for (i, d) in self.editor.props.iter().enumerate() {
                    let (low, high) = (
                        [events::tile_to_local(d.x / events::TILE - 0.5, col), events::tile_to_local(d.z / events::TILE - 0.5, row)],
                        [events::tile_to_local(d.x / events::TILE + 0.5, col), events::tile_to_local(d.z / events::TILE + 0.5, row)],
                    );
                    out.push(EventBox { kind: Kind::Prop, index: i, low, high, label: format!("D{i} model {}", d.kind), colour: Kind::Prop.colour(0), height: d.y, map: self.ev.map });
                }
            }
        }
        out
    }

    /// True when the events are drawn on the map: in every tool but the one of the tiles.
    pub(super) fn overlay_on(&self) -> bool {
        !self.tile_mode
    }

    /// The object whose blue mark (the square on the ground or the pole) a ray goes
    /// through: the nearest one.
    pub(super) fn object_mark_under(&self, origin: Vec3, direction: Vec3) -> Option<usize> {
        if !self.show_objects {
            return None;
        }
        let mut best: Option<(f32, usize)> = None;
        for (i, instance) in self.editor.instances.iter().enumerate() {
            if instance.deleted || instance.replaces.is_some() {
                continue;
            }
            let (low, high) = self.editor.instance_bounds(i);
            let y = low[1] + 1.5;
            let (mx, mz) = ((low[0] + high[0]) / 2.0, (low[2] + high[2]) / 2.0);
            // The square on the ground (a little thick, to be easy to hit) and the pole in the middle.
            let square = ray_box(origin, direction, Vec3::new(low[0], y - 2.0, low[2]), Vec3::new(high[0], y + 3.0, high[2]));
            let pole = ray_box(origin, direction, Vec3::new(mx - 4.0, y - 2.0, mz - 4.0), Vec3::new(mx + 4.0, y + 24.0, mz + 4.0));
            let t = match (square, pole) {
                (Some(a), Some(b)) => a.min(b),
                (Some(a), None) | (None, Some(a)) => a,
                (None, None) => continue,
            };
            if best.map_or(true, |(d, _)| t < d) {
                best = Some((t, i));
            }
        }
        best.map(|(_, i)| i)
    }

    /// The name of the nearest event that a ray goes through.
    pub(super) fn event_label_under(&self, origin: Vec3, direction: Vec3) -> Option<String> {
        self.event_boxes()
            .into_iter()
            .filter_map(|b| ray_box(origin, direction, Vec3::new(b.low[0], b.height, b.low[1]), Vec3::new(b.high[0], b.height + 34.0, b.high[1])).map(|t| (t, b.label)))
            .min_by(|a, b| a.0.partial_cmp(&b.0).unwrap_or(std::cmp::Ordering::Equal))
            .map(|(_, label)| label)
    }

    /// The distance to the nearest event that a ray goes through.
    pub(super) fn event_hit(&self, origin: Vec3, direction: Vec3) -> Option<f32> {
        self.event_boxes()
            .iter()
            .filter_map(|b| ray_box(origin, direction, Vec3::new(b.low[0], b.height, b.low[1]), Vec3::new(b.high[0], b.height + 34.0, b.high[1])))
            .fold(None, |best: Option<f32>, t| Some(best.map_or(t, |x| x.min(t))))
    }

    /// The coloured squares and poles that show the events.
    pub(super) fn event_vertices(&self) -> Vec<Vertex> {
        if !self.overlay_on() {
            return Vec::new();
        }
        let mut out = Vec::new();
        // The objects that can be moved: a mark on the ground under each, in the open block and in the blocks around.
        if self.show_objects {
            object_marks(&self.editor, [0.0, 0.0], &self.selected, &mut out);
            let current = self.editor.block_id();
            for (id, dx, dz) in self.world_cells() {
                let editor = if id == current { Some(&self.editor) } else { self.world.get(&id) };
                if let Some(editor) = editor {
                    object_marks(editor, [dx as f32 * 720.0, dz as f32 * 720.0], &[], &mut out);
                }
            }
        }
        for b in self.event_boxes() {
            let selected = self.ev.selected == Some((b.kind, b.index)) && b.map == self.ev.map;
            let colour = if selected { [255, 235, 40, 210] } else { b.colour };
            let centre = [(b.low[0] + b.high[0]) / 2.0, (b.low[1] + b.high[1]) / 2.0];
            let y = b.height + 1.5;
            let v = |x: f32, y: f32, z: f32| Vertex { position: [x, y, z], uv: [0.0, 0.0], colour };
            let (a, b2, c, d) = (v(b.low[0], y, b.low[1]), v(b.high[0], y, b.low[1]), v(b.high[0], y, b.high[1]), v(b.low[0], y, b.high[1]));
            out.extend_from_slice(&[a, b2, c, a, c, d]);
            // A pole, so that the event can be seen from far.
            let top = y + if selected { 40.0 } else { 26.0 };
            for (dx, dz) in [(1.6, 0.0), (0.0, 1.6)] {
                let (p0, p1, p2, p3) = (
                    v(centre[0] - dx, y, centre[1] - dz),
                    v(centre[0] + dx, y, centre[1] + dz),
                    v(centre[0] + dx, top, centre[1] + dz),
                    v(centre[0] - dx, top, centre[1] - dz),
                );
                out.extend_from_slice(&[p0, p1, p2, p0, p2, p3]);
            }
        }
        // The door models of the blocks around (the ones of the open block are in the boxes above).
        // Their places are in units of the world, like the ones of the open block.
        if self.ev.show[4] {
            if let Some(at) = self.ev.at() {
                let (col, row) = (at.column, at.row);
                for (id, _, _) in self.world_cells() {
                    let Some(editor) = self.world.get(&id) else { continue };
                    for d in &editor.props {
                        let (x0, x1) = (events::tile_to_local(d.x / events::TILE - 0.5, col), events::tile_to_local(d.x / events::TILE + 0.5, col));
                        let (z0, z1) = (events::tile_to_local(d.z / events::TILE - 0.5, row), events::tile_to_local(d.z / events::TILE + 0.5, row));
                        let colour = Kind::Prop.colour(0);
                        let y = d.y + 1.5;
                        let v = |x: f32, y: f32, z: f32| Vertex { position: [x, y, z], uv: [0.0, 0.0], colour };
                        out.extend_from_slice(&[v(x0, y, z0), v(x1, y, z0), v(x1, y, z1), v(x0, y, z0), v(x1, y, z1), v(x0, y, z1)]);
                        let (mx, mz) = ((x0 + x1) / 2.0, (z0 + z1) / 2.0);
                        for (dx, dz) in [(1.6, 0.0), (0.0, 1.6)] {
                            let top = y + 26.0;
                            out.extend_from_slice(&[v(mx - dx, y, mz - dz), v(mx + dx, y, mz + dz), v(mx + dx, top, mz + dz), v(mx - dx, y, mz - dz), v(mx + dx, top, mz + dz), v(mx - dx, top, mz - dz)]);
                        }
                    }
                }
            }
        }
        out
    }

    /// Arms a character: the next click on a tile puts it there.
    pub(super) fn arm_npc(&mut self, model: u16, frame: &eframe::Frame) {
        if !self.events_mode {
            self.events_mode = true;
            self.events_toggled(frame);
        }
        self.ev.npc = self.npcs.templates.get(&model).cloned().or_else(|| {
            let mut c = events::new_character(0, 0, 0, 0, 0.0);
            c.model = model;
            Some(c)
        });
        self.ev.put = Some(Kind::Character);
        self.ev.moving = false;
        self.status = format!("Character with the model {model}: click a tile to put it (Esc to stop).");
    }

    /// The list of the models of the characters, with a small picture of each.
    pub(super) fn npc_ui(&mut self, ctx: &egui::Context, ui: &mut egui::Ui) -> Option<u16> {
        const THUMB: usize = 96;
        ui.heading("Characters (NPCs)");
        if !self.npcs.ready {
            self.npcs.ready = true;
            self.npcs.templates = events::character_templates(&self.arc_dir).unwrap_or_default();
            let mut ids = Vec::new();
            for id in 0..600u16 {
                if self.models.has(id) {
                    ids.push(id);
                }
            }
            for id in self.models.override_ids() {
                if !ids.contains(&id) {
                    ids.push(id);
                }
            }
            ids.sort_unstable();
            self.npcs.ids = ids;
        }
        ui.horizontal(|ui| {
            ui.label("Search (name or number):");
            ui.text_edit_singleline(&mut self.npcs.search);
        });
        let words: Vec<String> = self.npcs.search.to_lowercase().split_whitespace().map(String::from).collect();
        let shown: Vec<u16> = self
            .npcs
            .ids
            .iter()
            .copied()
            .filter(|&id| {
                let name = model_name(id).to_lowercase();
                words.iter().all(|w| id.to_string() == *w || name.contains(w.as_str()))
            })
            .collect();
        ui.label(format!("{} models. Click one, then click a tile of the map. The new character has the script 65535 until you give it one.", shown.len()));
        let columns = ((ui.available_width() / (THUMB as f32 + 8.0)).floor() as usize).max(1);
        let rows = shown.len().div_ceil(columns);
        let mut clicked = None;
        let mut made = 0;
        egui::ScrollArea::vertical().auto_shrink([false, false]).show_rows(ui, THUMB as f32 + 8.0, rows, |ui, range| {
            for row in range {
                ui.horizontal(|ui| {
                    for &id in shown.iter().skip(row * columns).take(columns) {
                        let handle = match self.npcs.thumbs.get(&id) {
                            Some(h) => Some(h.clone()),
                            None if made < 4 => {
                                made += 1;
                                match self.models.load(id) {
                                    Some(model) => {
                                        let parts: Vec<crate::catalog::PreviewPart> = model
                                            .meshes
                                            .iter()
                                            .map(|m| crate::catalog::PreviewPart {
                                                picture: m.picture.clone(),
                                                // The characters look toward +z: turn them to face the picture.
                                                positions: m.positions.iter().map(|p| [-p[0], p[1], -p[2]]).collect(),
                                                uvs: m.uvs.clone(),
                                                indices: m.indices.chunks_exact(3).flat_map(|t| [t[0], t[2], t[1]]).collect(),
                                            })
                                            .collect();
                                        let pixels = crate::thumb::render(&parts, &model.textures, THUMB);
                                        let image = egui::ColorImage::from_rgba_unmultiplied([THUMB, THUMB], &pixels);
                                        let h = ctx.load_texture(format!("npc{id}"), image, egui::TextureOptions::LINEAR);
                                        self.npcs.thumbs.insert(id, h.clone());
                                        Some(h)
                                    }
                                    None => None,
                                }
                            }
                            None => None,
                        };
                        let tip = self
                            .npcs
                            .templates
                            .get(&id)
                            .map_or(format!("{} ({id})", model_name(id)), |c| format!("{} ({id})\nmovement {}, interaction {}", model_name(id), c.movement, c.interaction));
                        let response = match handle {
                            Some(h) => ui.add(egui::ImageButton::new(egui::load::SizedTexture::new(h.id(), [THUMB as f32, THUMB as f32]))),
                            None => {
                                ctx.request_repaint();
                                ui.add_sized([THUMB as f32, THUMB as f32], egui::Button::new(format!("{id}")))
                            }
                        };
                        if response.on_hover_text(tip).clicked() {
                            clicked = Some(id);
                        }
                    }
                });
            }
        });
        clicked
    }

    /// The 3D models of the characters, put on their tiles and turned as the
    /// characters are. A character without a model is not drawn.
    pub(super) fn placed_meshes(&mut self) -> Vec<PlacedMesh> {
        if !self.overlay_on() || !(self.ev.show[3] || self.ev.show[4]) {
            return Vec::new();
        }
        let mut out = Vec::new();
        // (the place in the block, the height, the model, the direction) of each character of all the maps
        let mut list: Vec<([f32; 2], f32, u16, u8)> = Vec::new();
        for zi in 0..self.ev.maps.len() {
            if !self.ev.map_shown.get(zi).copied().unwrap_or(true) && zi != self.ev.map {
                continue;
            }
            let (col, row) = (self.ev.maps[zi].column, self.ev.maps[zi].row);
            if let Some(events) = self.map_events(zi) {
                for c in &events.characters {
                    list.push(([events::tile_to_local(c.x as f32 + 0.5, col), events::tile_to_local(c.z as f32 + 0.5, row)], c.height, c.model, c.facing));
                }
            }
        }
        if !self.ev.show[3] {
            list.clear();
        }
        // The door models of the block, drawn as the 3D models of the game.
        let mut prop_parts: Vec<PlacedMesh> = Vec::new();
        if self.ev.show[4] {
            if let Some(at) = self.ev.at().cloned() {
                let props = self.editor.props.clone();
                let tileset = events::map_tileset(&self.arc_dir, at.map).unwrap_or(0);
                for d in props {
                    let Some(model) = self.models.load_prop(d.kind, tileset) else { continue };
                    let centre = [events::tile_to_local(d.x / events::TILE, at.column), events::tile_to_local(d.z / events::TILE, at.row)];
                    let (sin, cos) = d.turn[1].sin_cos();
                    for mesh in &model.meshes {
                        let vertices: Vec<Vertex> = mesh
                            .positions
                            .iter()
                            .enumerate()
                            .map(|(v, p)| {
                                let (x, y, z) = (p[0] * d.scale[0], p[1] * d.scale[1], p[2] * d.scale[2]);
                                Vertex {
                                    position: [centre[0] + x * cos + z * sin, d.y + y, centre[1] - x * sin + z * cos],
                                    uv: mesh.uvs.get(v).copied().unwrap_or([0.0, 0.0]),
                                    colour: [255; 4],
                                }
                            })
                            .collect();
                        let count = vertices.len() as u32;
                        prop_parts.push(PlacedMesh {
                            key: format!("door{}:{}", d.kind, mesh.picture),
                            vertices,
                            indices: mesh.indices.iter().copied().filter(|&i| i < count).collect(),
                            model: model.clone(),
                            picture: mesh.picture.clone(),
                        });
                    }
                }
            }
        }
        out.extend(prop_parts);
        for (centre, ground, model_id, facing) in list {
            let Some(model) = self.models.load(model_id) else { continue };
            // The model looks toward +z. The numbers are the ones of the plugin:
            // 0 up (-z), 1 down (+z), 2 left (-x), 3 right (+x), 4 up-left, 5 up-right, 6 down-left, 7 down-right.
            let pi = std::f32::consts::PI;
            let yaw = match facing {
                0 => pi,
                2 => -pi / 2.0,
                3 => pi / 2.0,
                4 => 5.0 * pi / 4.0,
                5 => 3.0 * pi / 4.0,
                6 => -pi / 4.0,
                7 => pi / 4.0,
                _ => 0.0,
            };
            let (sin, cos) = yaw.sin_cos();
            for (m, mesh) in model.meshes.iter().enumerate() {
                let vertices: Vec<Vertex> = mesh
                    .positions
                    .iter()
                    .enumerate()
                    .map(|(v, p)| Vertex {
                        position: [centre[0] + p[0] * cos + p[2] * sin, ground + p[1], centre[1] - p[0] * sin + p[2] * cos],
                        uv: mesh.uvs.get(v).copied().unwrap_or([0.0, 0.0]),
                        colour: [255; 4],
                    })
                    .collect();
                let count = vertices.len() as u32;
                out.push(PlacedMesh {
                    key: format!("{model_id}:{}", mesh.picture),
                    vertices,
                    indices: mesh.indices.iter().copied().filter(|&i| i < count).collect(),
                    model: model.clone(),
                    picture: mesh.picture.clone(),
                });
                let _ = m;
            }
        }
        out
    }

    /// The middle of an event on the ground of the block (units).
    fn event_centre(&self, kind: Kind, index: usize) -> Option<[f32; 2]> {
        let at = self.ev.at()?;
        let now = &self.ev.now;
        let (x, z) = match kind {
            Kind::Sign => {
                let s = now.signs.get(index)?;
                (s.x as f32 + s.width.max(1) as f32 / 2.0, s.z as f32 + s.depth.max(1) as f32 / 2.0)
            }
            Kind::Trigger => {
                let t = now.triggers.get(index)?;
                (t.x as f32 + t.width.max(1) as f32 / 2.0, t.z as f32 + t.depth.max(1) as f32 / 2.0)
            }
            Kind::Warp => {
                let w = now.warps.get(index)?;
                (w.x as f32 / events::TILE + w.width.max(1) as f32 / 2.0 - 0.5, w.z as f32 / events::TILE + w.depth.max(1) as f32 / 2.0 - 0.5)
            }
            Kind::Character => {
                let c = now.characters.get(index)?;
                (c.x as f32 + 0.5, c.z as f32 + 0.5)
            }
            Kind::Prop => {
                let d = self.editor.props.get(index)?;
                (d.x / events::TILE, d.z / events::TILE)
            }
        };
        Some([events::tile_to_local(x, at.column), events::tile_to_local(z, at.row)])
    }

    /// The door model that stands at the place of a warp moves with the warp.
    fn move_warp_prop(&mut self, old: Option<(i16, i16)>, new: Option<(i16, i16)>) {
        let (Some(old), Some(new)) = (old, new) else { return };
        if old == new {
            return;
        }
        let (ox, oz) = (old.0 as f32, old.1 as f32);
        let mut props = self.editor.props.clone();
        let mut moved = 0;
        for d in props.iter_mut() {
            if (d.x - ox).abs() <= 9.5 && (d.z - oz).abs() <= 9.5 {
                d.x += (new.0 - old.0) as f32;
                d.z += (new.1 - old.1) as f32;
                moved += 1;
            }
        }
        if moved > 0 && self.editor.set_props(props).is_ok() {
            self.status = format!("{} The door model of the warp moved with it (export the block to keep it).", self.status);
        }
    }

    /// The tile of the block under a warp.
    fn warp_block_tile(&self, index: usize) -> Option<(i32, i32)> {
        let at = self.ev.at()?;
        let w = self.ev.now.warps.get(index)?;
        Some(((w.x as i32).div_euclid(18) - (at.column * 40) as i32, (w.z as i32).div_euclid(18) - (at.row * 40) as i32))
    }

    /// A door is a wall tile that has a warp: when the warp moves, the wall
    /// moves with it, and the old tile is free. Does nothing when the old tile
    /// is not a wall.
    fn move_door_wall(&mut self, old: Option<(i32, i32)>, new: Option<(i32, i32)>) {
        let (Some(old), Some(new)) = (old, new) else { return };
        if old == new || !self.walls_follow {
            return;
        }
        let (width, height) = self.editor.grid();
        let inside = |t: (i32, i32)| t.0 >= 0 && t.1 >= 0 && (t.0 as usize) < width && (t.1 as usize) < height;
        if !inside(old) || !inside(new) {
            return;
        }
        let value = self.editor.tiles[old.1 as usize * width + old.0 as usize];
        if value >> 24 != 1 {
            return;
        }
        let plain = self.plain_value();
        self.editor.begin_stroke();
        self.editor.paint(old.0 as usize, old.1 as usize, 1, plain);
        self.editor.paint(new.0 as usize, new.1 as usize, 1, value);
        self.editor.end_stroke();
        self.status = "The door warp took its wall tile with it.".into();
    }

    /// Gives an event the height of the ground where it is: the game looks for
    /// a warp only a little above and under its height.
    fn event_to_ground(&mut self, kind: Kind, index: usize) {
        let Some(c) = self.event_centre(kind, index) else { return };
        let Some(g) = self.ground_at(c[0], c[1]) else { return };
        let now = &mut self.ev.now;
        match kind {
            Kind::Sign => now.signs[index].height = g as i32,
            Kind::Trigger => now.triggers[index].height = g as i16,
            Kind::Warp => now.warps[index].y = g as i16,
            Kind::Character => now.characters[index].height = g,
            Kind::Prop => {
                let mut props = self.editor.props.clone();
                if let Some(d) = props.get_mut(index) {
                    d.y = g;
                }
                let _ = self.editor.set_props(props);
            }
        }
    }

    /// The events whose middle is under objects that moved move with them,
    /// by whole tiles. `low` and `high` are the box of the objects before the
    /// move, `delta` the move in units. Returns how many events moved.
    pub(super) fn events_copy_with_objects(&mut self, low: [f32; 3], high: [f32; 3], delta: [f32; 3]) -> usize {
        if !self.events_follow || !self.ev.loaded {
            return 0;
        }
        let (dx, dz) = ((delta[0] / events::TILE).round() as i32, (delta[2] / events::TILE).round() as i32);
        let dy = delta[1];
        let inside = |c: [f32; 2]| c[0] >= low[0] && c[0] <= high[0] && c[1] >= low[2] && c[1] <= high[2];
        let mut copied = 0;
        // (The characters stay where they are: a copy of a house does not copy the people.)
        let signs: Vec<Sign> = (0..self.ev.now.signs.len()).filter(|&i| self.event_centre(Kind::Sign, i).is_some_and(inside)).map(|i| self.ev.now.signs[i].clone()).collect();
        for mut s in signs {
            s.x += dx as i16;
            s.z += dz as i16;
            s.height += dy as i32;
            self.ev.now.signs.push(s);
            copied += 1;
        }
        let triggers: Vec<Trigger> = (0..self.ev.now.triggers.len()).filter(|&i| self.event_centre(Kind::Trigger, i).is_some_and(inside)).map(|i| self.ev.now.triggers[i].clone()).collect();
        for mut t in triggers {
            t.x = (t.x as i32 + dx).max(0) as u16;
            t.z = (t.z as i32 + dz).max(0) as u16;
            t.height += dy as i16;
            self.ev.now.triggers.push(t);
            copied += 1;
        }
        let warps: Vec<Warp> = (0..self.ev.now.warps.len()).filter(|&i| self.event_centre(Kind::Warp, i).is_some_and(inside)).map(|i| self.ev.now.warps[i].clone()).collect();
        for mut w in warps {
            w.x += (events::TILE as i32 * dx) as i16;
            w.z += (events::TILE as i32 * dz) as i16;
            w.y += dy as i16;
            self.ev.now.warps.push(w);
            copied += 1;
        }
        // The door models: the copy of the building gets its own.
        let mut props = self.editor.props.clone();
        let wanted: Vec<usize> = (0..props.len()).filter(|&i| self.event_centre(Kind::Prop, i).is_some_and(inside)).collect();
        for i in wanted {
            let mut p = props[i].clone();
            p.x += events::TILE * dx as f32;
            p.z += events::TILE * dz as f32;
            p.y += dy;
            props.push(p);
            copied += 1;
        }
        if props.len() != self.editor.props.len() {
            let _ = self.editor.set_props(props);
        }
        copied
    }

    pub(super) fn events_follow_objects(&mut self, low: [f32; 3], high: [f32; 3], delta: [f32; 3]) -> usize {
        if !self.events_follow || !self.ev.loaded {
            return 0;
        }
        let (dx, dz) = ((delta[0] / events::TILE).round() as i32, (delta[2] / events::TILE).round() as i32);
        let dy = delta[1];
        if dx == 0 && dz == 0 && dy.abs() < 1.0 {
            return 0;
        }
        let inside = |c: [f32; 2]| c[0] >= low[0] && c[0] <= high[0] && c[1] >= low[2] && c[1] <= high[2];
        let mut moved = 0;
        for i in 0..self.ev.now.signs.len() {
            if self.event_centre(Kind::Sign, i).is_some_and(inside) {
                let s = &mut self.ev.now.signs[i];
                s.x += dx as i16;
                s.z += dz as i16;
                s.height += dy as i32;
                moved += 1;
            }
        }
        for i in 0..self.ev.now.triggers.len() {
            if self.event_centre(Kind::Trigger, i).is_some_and(inside) {
                let t = &mut self.ev.now.triggers[i];
                t.x = (t.x as i32 + dx).max(0) as u16;
                t.z = (t.z as i32 + dz).max(0) as u16;
                t.height += dy as i16;
                moved += 1;
            }
        }
        for i in 0..self.ev.now.warps.len() {
            if self.event_centre(Kind::Warp, i).is_some_and(inside) {
                let w = &mut self.ev.now.warps[i];
                w.x += (events::TILE as i32 * dx) as i16;
                w.z += (events::TILE as i32 * dz) as i16;
                w.y += dy as i16;
                moved += 1;
            }
        }
        let mut props = self.editor.props.clone();
        let mut door_moved = false;
        for i in 0..props.len() {
            if self.event_centre(Kind::Prop, i).is_some_and(inside) {
                props[i].x += events::TILE * dx as f32;
                props[i].z += events::TILE * dz as f32;
                props[i].y += dy;
                door_moved = true;
                moved += 1;
            }
        }
        if door_moved {
            let _ = self.editor.set_props(props);
        }
        for i in 0..self.ev.now.characters.len() {
            if self.event_centre(Kind::Character, i).is_some_and(inside) {
                let c = &mut self.ev.now.characters[i];
                c.x = (c.x as i32 + dx).max(0) as u16;
                c.z = (c.z as i32 + dz).max(0) as u16;
                c.height += dy;
                moved += 1;
            }
        }
        moved
    }

    /// The blocks of the world that are drawn around the open block: the
    /// number of the block and how many blocks away it is on each side.
    pub(super) fn world_cells(&self) -> Vec<(usize, i32, i32)> {
        let mut out = Vec::new();
        if self.ev.neigh_radius == 0 {
            return out;
        }
        let (Some(at), Some(matrix)) = (self.ev.at(), self.ev.matrix.as_ref()) else { return out };
        let radius = self.ev.neigh_radius as usize;
        for r in at.row.saturating_sub(radius)..=(at.row + radius).min(matrix.height.saturating_sub(1)) {
            for c in at.column.saturating_sub(radius)..=(at.column + radius).min(matrix.width.saturating_sub(1)) {
                let id = matrix.ids.get(r * matrix.width + c).copied().unwrap_or(0xFFFF) as usize;
                if (c, r) != (at.column, at.row) && id < 857 {
                    out.push((id, c as i32 - at.column as i32, r as i32 - at.row as i32));
                }
            }
        }
        out
    }

    /// Draws again the blocks around the block when something changed.
    pub(super) fn refresh_neighbours(&mut self, frame: &eframe::Frame) {
        let ids_key = self.ev.matrix.as_ref().map(|m| m.ids.clone()).unwrap_or_default();
        let key = (true, self.ev.neigh_radius, self.ev.map, ids_key);
        if self.ev.neigh_key.as_ref() == Some(&key) {
            return;
        }
        self.ev.neigh_key = Some(key);
        let Some(state) = frame.wgpu_render_state() else { return };
        let mut shifted: Vec<(usize, Mesh)> = Vec::new();
        let mut textures: HashMap<String, Texture> = HashMap::new();
        if self.ev.neigh_radius > 0 {
            if let Some(at) = self.ev.at().cloned() {
                let wanted = self.world_cells();
                let current = self.editor.block_id();
                // The blocks around are opened as editors: they keep their changes, and they can be edited.
                // (A few blocks for each frame, so that the window does not freeze.)
                let mut opened = 0;
                let mut missing = 0;
                for &(id, _, _) in &wanted {
                    if id != current && !self.world.contains_key(&id) {
                        if opened < 4 {
                            opened += 1;
                            if let Ok(editor) = crate::editor::Editor::new(&self.arc_dir, id) {
                                self.world.insert(id, editor);
                            }
                        } else {
                            missing += 1;
                        }
                    }
                }
                if missing > 0 {
                    // Come back at the next frame for the other blocks.
                    self.ev.neigh_key = None;
                    self.status = format!("Opening the blocks of the world ({missing} to go)...");
                }
                // The pictures are the ones of the tileset of the map.
                if let Some(tileset) = events::map_tileset(&self.arc_dir, at.map) {
                    textures = crate::data::tileset_textures(&self.arc_dir, tileset).unwrap_or_default();
                }
                for &(id, dx, dz) in &wanted {
                    let editor = if id == current { Some(&self.editor) } else { self.world.get(&id) };
                    let Some(editor) = editor else { continue };
                    for (_, mesh) in editor.meshes() {
                        let mut m = mesh.clone();
                        for p in m.positions.iter_mut() {
                            p[0] += dx as f32 * 720.0;
                            p[2] += dz as f32 * 720.0;
                        }
                        shifted.push((1, m));
                    }
                }
            }
        }
        {
            let mut renderer = state.renderer.write();
            let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
            gpu.neighbours = make_gpu_meshes(&state.device, &state.queue, &gpu.picture_layout, &gpu.sampler, &shifted, &textures);
        }
        // The marks of the objects of the blocks around come with the blocks.
        self.refresh_events(frame);
    }

    /// The tile of the map under a point of the block.
    fn tile_under(&self, point: Vec3) -> Option<(i32, i32)> {
        let at = self.ev.at()?;
        Some((events::local_to_tile(point.x, at.column).floor() as i32, events::local_to_tile(point.z, at.row).floor() as i32))
    }

    /// A click in the mode of the events: put, move or select.
    pub(super) fn events_click(&mut self, origin: Vec3, direction: Vec3, frame: &eframe::Frame) {
        // The ground of the block, or (outside of it) the plane of the middle height.
        let point = match self.pick(origin, direction, true) {
            Some(hit) => hit.point,
            None if direction.y.abs() > 1e-4 => {
                let height = self.ev.now.characters.first().map_or(0.0, |c| c.height);
                origin + direction * ((height - origin.y) / direction.y)
            }
            None => return,
        };
        let hit = Hit { point, owner: None, mesh: 0 };
        let Some((tx, tz)) = self.tile_under(hit.point) else { return };
        if tx < 0 || tz < 0 {
            self.status = "This is outside the map.".into();
            return;
        }
        if let Some(kind) = self.ev.put.take() {
            self.events_add(kind, tx, tz, hit.point.y);
        } else if self.ev.moving {
            self.ev.moving = false;
            if let Some((kind, index)) = self.ev.selected {
                let old = if kind == Kind::Warp { self.warp_block_tile(index) } else { None };
                let old_pos = if kind == Kind::Warp { self.ev.now.warps.get(index).map(|w| (w.x, w.z)) } else { None };
                self.events_set_tile(kind, index, tx, tz, Some(hit.point.y));
                self.status = format!("The {} is in tile ({tx}, {tz}).", kind.name());
                if kind == Kind::Warp {
                    let new = self.warp_block_tile(index);
                    self.move_door_wall(old, new);
                    let new_pos = self.ev.now.warps.get(index).map(|w| (w.x, w.z));
                    self.move_warp_prop(old_pos, new_pos);
                }
            }
        } else {
            // The event that the ray of the click reaches first: each event is a
            // volume above its square (as high as the pole).
            let mut found: Option<(f32, Kind, usize, usize)> = None;
            for b in self.event_boxes() {
                let (lo, hi) = (Vec3::new(b.low[0], b.height, b.low[1]), Vec3::new(b.high[0], b.height + 34.0, b.high[1]));
                if let Some(t) = ray_box(origin, direction, lo, hi) {
                    if found.map_or(true, |(best, _, _, _)| t < best) {
                        found = Some((t, b.kind, b.index, b.map));
                    }
                }
            }
            if let Some((_, _, _, map)) = found {
                self.switch_map(map);
            }
            self.ev.selected = found.map(|(_, k, i, _)| (k, i));
            self.status = match found {
                Some((_, k, i, map)) => format!("Selected the {} {i} of the map {}.", k.name(), self.ev.maps.get(map).map_or(0, |z| z.map)),
                None => "No event here.".into(),
            };
        }
        self.refresh_events(frame);
    }

    /// Puts a new event in a tile.
    fn events_add(&mut self, kind: Kind, tx: i32, tz: i32, ground: f32) {
        let map = self.ev.at().map_or(0, |z| z.map) as u16;
        let now = &mut self.ev.now;
        let (x, z) = (tx as u16, tz as u16);
        let index = match kind {
            Kind::Prop => {
                self.status = "A door model cannot be added: the block has its own list of props.".into();
                return;
            }
            Kind::Sign => {
                if now.signs.len() >= MAX_SIGNS {
                    self.status = "A map has at most 40 signs.".into();
                    return;
                }
                now.signs.push(Sign { raw: Vec::new(), id: 65535, kind: 0, facing: 4, x: tx as i16, z: tz as i16, width: 1, depth: 1, height: ground as i32 });
                now.signs.len() - 1
            }
            Kind::Trigger => {
                if now.triggers.len() >= MAX_TRIGGERS {
                    self.status = "A map has at most 40 triggers.".into();
                    return;
                }
                now.triggers.push(Trigger { raw: Vec::new(), id: 65535, param: 0, work_id: 0, check_kind: 0, x, z, width: 1, depth: 1, height: ground as i16 });
                now.triggers.len() - 1
            }
            Kind::Warp => {
                if now.warps.len() >= MAX_WARPS {
                    self.status = "A map has at most 24 warps.".into();
                    return;
                }
                now.warps.push(Warp {
                    raw: Vec::new(),
                    map,
                    exit: 0,
                    facing: 0,
                    kind: 0,
                    x: (tx * 18 + 9) as i16,
                    y: ground as i16,
                    z: (tz * 18 + 9) as i16,
                    width: 1,
                    depth: 1,
                });
                now.warps.len() - 1
            }
            Kind::Character => {
                let local = now.characters.iter().map(|c| c.local_id + 1).max().unwrap_or(0);
                let mut c = events::new_character(map, local, x, z, ground);
                match self.ev.npc.take() {
                    Some(t) => {
                        c.model = t.model;
                        c.movement = t.movement;
                        c.interaction = t.interaction;
                        c.facing = t.facing;
                        c.param = t.param;
                        c.position_kind = t.position_kind;
                        c.trigger = t.trigger;
                        c.alias = t.alias;
                    }
                    None => c.model = now.characters.first().map_or(204, |c| c.model),
                }
                now.characters.push(c);
                now.characters.len() - 1
            }
        };
        self.ev.selected = Some((kind, index));
        self.status = format!("New {} in tile ({tx}, {tz}). Its script is 65535: the game ignores it until you give it one.", kind.name());
    }

    fn events_set_tile(&mut self, kind: Kind, index: usize, tx: i32, tz: i32, ground: Option<f32>) {
        if kind == Kind::Prop {
            let mut props = self.editor.props.clone();
            if let Some(d) = props.get_mut(index) {
                d.x = (tx * 18 + 9) as f32;
                d.z = (tz * 18 + 9) as f32;
                if let Some(g) = ground {
                    d.y = g;
                }
            }
            let _ = self.editor.set_props(props);
            return;
        }
        let now = &mut self.ev.now;
        match kind {
            Kind::Prop => {}
            Kind::Sign => {
                if let Some(s) = now.signs.get_mut(index) {
                    s.x = tx as i16;
                    s.z = tz as i16;
                    if let Some(g) = ground {
                        s.height = g as i32;
                    }
                }
            }
            Kind::Trigger => {
                if let Some(t) = now.triggers.get_mut(index) {
                    t.x = tx as u16;
                    t.z = tz as u16;
                    if let Some(g) = ground {
                        t.height = g as i16;
                    }
                }
            }
            Kind::Warp => {
                if let Some(w) = now.warps.get_mut(index) {
                    w.x = (tx * 18 + 9) as i16;
                    w.z = (tz * 18 + 9) as i16;
                    if let Some(g) = ground {
                        w.y = g as i16;
                    }
                }
            }
            Kind::Character => {
                if let Some(c) = now.characters.get_mut(index) {
                    c.x = tx as u16;
                    c.z = tz as u16;
                    if let Some(g) = ground {
                        c.height = g;
                    }
                }
            }
        }
    }

    /// Takes the selected event away.
    fn events_delete(&mut self) {
        let Some((kind, index)) = self.ev.selected else { return };
        if kind == Kind::Prop {
            self.status = "A door model cannot be taken away (the block has its own list of props). Move it instead.".into();
            return;
        }
        let base = &self.ev.base;
        let now = &mut self.ev.now;
        let (shipped, len) = match kind {
            Kind::Prop => (0, 0),
            Kind::Sign => (base.signs.len(), now.signs.len()),
            Kind::Trigger => (base.triggers.len(), now.triggers.len()),
            Kind::Warp => (base.warps.len(), now.warps.len()),
            Kind::Character => (base.characters.len(), now.characters.len()),
        };
        let _ = shipped;
        if index >= len {
            return;
        }
        match kind {
            Kind::Prop => {}
            Kind::Sign => {
                now.signs.remove(index);
            }
            Kind::Trigger => {
                now.triggers.remove(index);
            }
            Kind::Warp => {
                now.warps.remove(index);
            }
            Kind::Character => {
                now.characters.remove(index);
            }
        }
        self.ev.selected = None;
        self.status = format!("The {} was taken away.", kind.name());
    }

    /// Writes the pack of each map that changed on the SD card (a map that
    /// is as in the game again loses its file). The lines of these maps leave
    /// `map_events.txt`: the packs have the changes now.
    pub(super) fn events_save(&mut self) {
        let mut done: Vec<String> = Vec::new();
        let mut failed: Vec<String> = Vec::new();
        let path = events::file_path(&self.export_dir);
        for zi in 0..self.ev.maps.len() {
            let map = self.ev.maps[zi].map;
            let pair = if zi == self.ev.map { Some((&self.ev.base, &self.ev.now)) } else { self.ev.others.get(zi).and_then(|o| o.as_ref()).map(|(b, n)| (b, n)) };
            let Some((base, now)) = pair else { continue };
            if now != base {
                match events::write_map_override(&self.arc_dir, &self.export_dir, map, now) {
                    Ok(_) => {
                        if let Ok(old) = std::fs::read_to_string(&path) {
                            let text = events::merge_file(&old, map, now, now);
                            let _ = std::fs::write(&path, text);
                        }
                        done.push(map.to_string());
                    }
                    Err(e) => failed.push(format!("{map}: {e}")),
                }
            } else if events::read_map_override(&self.export_dir, map).is_some() {
                let _ = events::reset_map(&self.export_dir, map);
                done.push(format!("{map} (as in the game)"));
            }
        }
        self.status = if !failed.is_empty() {
            format!("Cannot write some maps: {}", failed.join("; "))
        } else if done.is_empty() {
            "Nothing changed: no map was written.".to_string()
        } else {
            format!("Maps written: {}. Start the game again.", done.join(", "))
        };
    }

    /// The keys of the mode of the events.
    pub(super) fn events_keys(&mut self, ctx: &egui::Context, frame: &eframe::Frame) {
        let (left, right, up, down, page_up, page_down, delete, escape, grab) = ctx.input(|i| {
            (
                i.key_pressed(egui::Key::ArrowLeft),
                i.key_pressed(egui::Key::ArrowRight),
                i.key_pressed(egui::Key::ArrowUp),
                i.key_pressed(egui::Key::ArrowDown),
                i.key_pressed(egui::Key::PageUp),
                i.key_pressed(egui::Key::PageDown),
                i.key_pressed(egui::Key::Delete) || i.key_pressed(egui::Key::Backspace) || i.key_pressed(egui::Key::X),
                i.key_pressed(egui::Key::Escape),
                i.key_pressed(egui::Key::G),
            )
        });
        if escape {
            self.ev.put = None;
            self.ev.moving = false;
            self.ev.selected = None;
        }
        if grab && self.ev.selected.is_some() {
            self.ev.moving = !self.ev.moving;
            self.status = if self.ev.moving { "Click on a tile to move the event there.".into() } else { "Cancelled.".into() };
        }
        if delete {
            self.events_delete();
        }
        let (dx, dz) = (right as i32 - left as i32, down as i32 - up as i32);
        let dy = page_up as i32 - page_down as i32;
        let old_warp_tile = match self.ev.selected {
            Some((Kind::Warp, index)) if dx != 0 || dz != 0 => self.warp_block_tile(index),
            _ => None,
        };
        let old_warp_pos = match self.ev.selected {
            Some((Kind::Warp, index)) if dx != 0 || dz != 0 => self.ev.now.warps.get(index).map(|w| (w.x, w.z)),
            _ => None,
        };
        if let Some((kind, index)) = self.ev.selected {
            if kind == Kind::Prop && (dx != 0 || dz != 0 || dy != 0) {
                let mut props = self.editor.props.clone();
                if let Some(d) = props.get_mut(index) {
                    d.x += 18.0 * dx as f32;
                    d.z += 18.0 * dz as f32;
                    d.y += 2.0 * dy as f32;
                }
                let _ = self.editor.set_props(props);
            } else if dx != 0 || dz != 0 || dy != 0 {
                let now = &mut self.ev.now;
                match kind {
                    Kind::Prop => {}
                    Kind::Sign => {
                        if let Some(s) = now.signs.get_mut(index) {
                            s.x += dx as i16;
                            s.z += dz as i16;
                            s.height += 2 * dy;
                        }
                    }
                    Kind::Trigger => {
                        if let Some(t) = now.triggers.get_mut(index) {
                            t.x = (t.x as i32 + dx).max(0) as u16;
                            t.z = (t.z as i32 + dz).max(0) as u16;
                            t.height += 2 * dy as i16;
                        }
                    }
                    Kind::Warp => {
                        if let Some(w) = now.warps.get_mut(index) {
                            w.x += 18 * dx as i16;
                            w.z += 18 * dz as i16;
                            w.y += 2 * dy as i16;
                        }
                    }
                    Kind::Character => {
                        if let Some(c) = now.characters.get_mut(index) {
                            c.x = (c.x as i32 + dx).max(0) as u16;
                            c.z = (c.z as i32 + dz).max(0) as u16;
                            c.height += 2.0 * dy as f32;
                        }
                    }
                }
            }
        }
        if let Some((kind, index)) = self.ev.selected {
            if dx != 0 || dz != 0 {
                self.event_to_ground(kind, index);
                if kind == Kind::Warp {
                    let new = self.warp_block_tile(index);
                    self.move_door_wall(old_warp_tile, new);
                    let new_pos = self.ev.now.warps.get(index).map(|w| (w.x, w.z));
                    self.move_warp_prop(old_warp_pos, new_pos);
                }
            }
        }
        if escape || delete || dx != 0 || dz != 0 || dy != 0 || grab {
            self.refresh_events(frame);
        }
    }

    /// The blocks of the matrix of the map: a grid to see them and to put
    /// another block in a place (to stick a map to another).
    fn matrix_ui(&mut self, ui: &mut egui::Ui) -> bool {
        let mut changed = false;
        let Some(at) = self.ev.at().cloned() else { return false };
        let Some(mut matrix) = self.ev.matrix.clone() else { return false };
        let original = self.ev.original_matrix.clone();
        egui::CollapsingHeader::new(format!("Blocks of the world (matrix {}, {}x{})", at.matrix, matrix.width, matrix.height))
            .default_open(false)
            .show(ui, |ui| {
                ui.label("Each square is a block. Yellow: the block that you edit. Click a square to choose a place.");
                let mut chosen = self.ev.cell;
                egui::Grid::new("matrix_grid").spacing([2.0, 2.0]).show(ui, |ui| {
                    for r in 0..matrix.height {
                        for c in 0..matrix.width {
                            let id = matrix.ids.get(r * matrix.width + c).copied().unwrap_or(0xFFFF);
                            let is_here = (c, r) == (at.column, at.row);
                            let is_chosen = chosen == Some((c, r));
                            let changed_here = original.as_ref().map_or(false, |o| o.ids.get(r * o.width + c) != Some(&id));
                            let text = if id < 857 { format!("{id}") } else { "-".to_string() };
                            let mut rich = egui::RichText::new(text).monospace();
                            if is_here {
                                rich = rich.color(egui::Color32::from_rgb(255, 220, 40)).strong();
                            } else if changed_here {
                                rich = rich.color(egui::Color32::from_rgb(120, 240, 120));
                            }
                            if ui.add(egui::Button::new(rich).selected(is_chosen).min_size(egui::vec2(34.0, 20.0))).clicked() {
                                chosen = Some((c, r));
                            }
                        }
                        ui.end_row();
                    }
                });
                self.ev.cell = chosen;
                if let Some((c, r)) = chosen {
                    let slot = r * matrix.width + c;
                    let mut id = matrix.ids.get(slot).copied().unwrap_or(0xFFFF);
                    ui.horizontal(|ui| {
                        ui.label(format!("Place ({c}, {r}): block"));
                        if super::chrome::spin_with(ui, &mut id, 0.0, 857.0, |d| d) {
                            if let Some(v) = matrix.ids.get_mut(slot) {
                                *v = id;
                            }
                            changed = true;
                        }
                        if ui.button("Empty").clicked() {
                            if let Some(v) = matrix.ids.get_mut(slot) {
                                *v = 0xFFFF;
                            }
                            changed = true;
                        }
                    });
                    if id < 857 && (c, r) != (at.column, at.row) {
                        if ui.button("Edit this block here").on_hover_text("Opens the block in this window (the changes of this block go in its project file)").clicked() {
                            self.pending_block = Some(id as usize);
                        }
                        if ui.button("Open this block in a new window").clicked() {
                            let exe = std::env::current_exe();
                            let started = exe.and_then(|e| std::process::Command::new(e).args(["view", &self.arc_dir, &id.to_string()]).spawn());
                            self.status = match started {
                                Ok(_) => format!("Block {id} opens in a new window."),
                                Err(e) => format!("Cannot open the block: {e}"),
                            };
                        }
                    }
                }
                ui.horizontal(|ui| {
                    if ui.button("Save the world blocks").on_hover_text("Writes the matrix on the SD card (sango/matrix): every map that uses it has the new blocks").clicked() {
                        self.status = match events::write_matrix_override(&self.arc_dir, &self.export_dir, at.matrix, &matrix) {
                            Ok(message) => format!("Matrix {} written in {message}. Start the game again.", at.matrix),
                            Err(e) => format!("Cannot write the matrix: {e}"),
                        };
                    }
                    if ui.button("Reset the world blocks").clicked() {
                        self.confirm = Some(super::chrome::Confirm::ResetMatrix);
                    }
                });
            });
        self.ev.matrix = Some(matrix);
        changed
    }

    /// The panel of the events, in the panel on the left. Returns true when
    /// an event changed (the squares must be drawn again).
    pub(super) fn events_ui(&mut self, ui: &mut egui::Ui) -> bool {
        let mut changed = false;
        ui.label(egui::RichText::new("Click an event to select it. Arrows: move one tile. G then click: move it. Delete: remove it.").weak());
        if self.ev.maps.len() > 1 {
            let before = self.ev.map;
            let text = |z: &MapAt| format!("{} [{}]", map_name(z.map as u16), z.map);
            egui::ComboBox::from_label("Map")
                .selected_text(self.ev.at().map_or("-".to_string(), |z| text(z)))
                .show_ui(ui, |ui| {
                    for (i, z) in self.ev.maps.iter().enumerate() {
                        ui.selectable_value(&mut self.ev.map, i, text(z));
                    }
                });
            if self.ev.map != before {
                let target = self.ev.map;
                self.ev.map = before;
                self.switch_map(target);
                changed = true;
            }
        } else if let Some(z) = self.ev.at() {
            ui.label(format!("Map {} (matrix {}, {}x{} blocks, this block is at column {} row {}).", z.map, z.matrix, z.width, z.height, z.column, z.row));
        }
        ui.horizontal(|ui| {
            for (k, name) in ["Signs", "Triggers", "Warps", "Characters", "Doors and placed"].iter().enumerate() {
                if ui.checkbox(&mut self.ev.show[k], *name).changed() {
                    changed = true;
                }
            }
        });
        egui::CollapsingHeader::new(format!("Maps shown ({})", self.ev.map_shown.iter().filter(|s| **s).count())).default_open(false).show(ui, |ui| {
            ui.horizontal(|ui| {
                if ui.button("All").clicked() {
                    self.ev.map_shown.iter_mut().for_each(|s| *s = true);
                    changed = true;
                }
                if ui.button("None").clicked() {
                    self.ev.map_shown.iter_mut().for_each(|s| *s = false);
                    changed = true;
                }
            });
            for i in 0..self.ev.maps.len() {
                let map = self.ev.maps[i].map;
                let mut shown = self.ev.map_shown.get(i).copied().unwrap_or(true);
                let text = format!("{} [{map}]{}", map_name(map as u16).split(" [").next().unwrap_or(""), if i == self.ev.map { "  (edited)" } else { "" });
                if ui.checkbox(&mut shown, text).changed() {
                    if let Some(s) = self.ev.map_shown.get_mut(i) {
                        *s = shown;
                    }
                    changed = true;
                }
            }
        });
        ui.horizontal(|ui| {
            ui.label("Blocks drawn around:");
            let mut radius = self.ev.neigh_radius.min(ALL_BLOCKS);
            if super::chrome::spin_with(ui, &mut radius, 0.0, ALL_BLOCKS as f64, |d| d.custom_formatter(|v, _| if v >= ALL_BLOCKS as f64 { "all".to_string() } else { format!("{v}") })) {
                self.ev.neigh_radius = radius;
                changed = true;
            }
            if ui.button("All the world").clicked() && self.ev.neigh_radius != ALL_BLOCKS {
                self.ev.neigh_radius = ALL_BLOCKS;
                changed = true;
            }
            if ui.button("None").clicked() && self.ev.neigh_radius != 0 {
                self.ev.neigh_radius = 0;
                changed = true;
            }
        });
        changed |= self.matrix_ui(ui);
        ui.horizontal(|ui| {
            for (kind, text) in [(Kind::Sign, "+ Sign"), (Kind::Trigger, "+ Trigger"), (Kind::Warp, "+ Warp"), (Kind::Character, "+ Character")] {
                if ui.selectable_label(self.ev.put == Some(kind), text).clicked() {
                    self.ev.put = if self.ev.put == Some(kind) { None } else { Some(kind) };
                    self.ev.moving = false;
                    if self.ev.put.is_some() {
                        self.status = format!("Click a tile to put the new {}.", kind.name());
                    }
                }
            }
        });
        if let Some((Kind::Prop, index)) = self.ev.selected {
            ui.separator();
            ui.label(format!("door model {index} (a model that the game puts at a door)"));
            if let Some(mut d) = self.editor.props.get(index).cloned() {
                if prop_fields(ui, &mut d) {
                    let mut props = self.editor.props.clone();
                    props[index] = d;
                    let _ = self.editor.set_props(props);
                    changed = true;
                }
            }
        } else if let Some((kind, index)) = self.ev.selected {
            ui.separator();
            let shipped = match kind {
                Kind::Prop => 0,
                Kind::Sign => self.ev.base.signs.len(),
                Kind::Trigger => self.ev.base.triggers.len(),
                Kind::Warp => self.ev.base.warps.len(),
                Kind::Character => self.ev.base.characters.len(),
            };
            ui.label(format!("{} {index} ({})", kind.name(), if index < shipped { "from the game" } else { "added by you" }));
            let now = &mut self.ev.now;
            changed |= match kind {
                Kind::Prop => false,
                Kind::Sign => now.signs.get_mut(index).map_or(false, |s| sign_fields(ui, s)),
                Kind::Trigger => now.triggers.get_mut(index).map_or(false, |t| trigger_fields(ui, t)),
                Kind::Warp => now.warps.get_mut(index).map_or(false, |w| warp_fields(ui, w)),
                Kind::Character => now.characters.get_mut(index).map_or(false, |c| character_fields(ui, c)),
            };
        }
        ui.horizontal(|ui| {
            if ui.button("Save the events").on_hover_text("Writes the pack of the map (sango/zone/<number>.bin) on the SD card: the plugin gives it to the game").clicked() {
                self.events_save();
            }
            if ui.button("Read again").on_hover_text("Reads the events of the file again (the changes that you did not save are lost)").clicked() {
                self.events_load_map();
                changed = true;
            }
        });
        ui.horizontal(|ui| {
            if ui.button("Reset the map to the game").on_hover_text("The map chosen above: its events and the blocks where it has events go back to the game").clicked() {
                self.confirm = Some(super::chrome::Confirm::ResetMap);
            }
            if ui.button("Reset the whole world").on_hover_text("All the maps, all the blocks and the grid of this world go back to the game").clicked() {
                self.confirm = Some(super::chrome::Confirm::ResetWorld);
            }
            if let Some((kind, index)) = self.ev.selected {
                if ui.button("Restore this event").on_hover_text("Gives this event the values of the game (only an event of the game)").clicked() {
                    let (base, now) = (&self.ev.base, &mut self.ev.now);
                    let ok = match kind {
                        Kind::Prop => self.editor.restore_prop(index).is_ok(),
                        Kind::Sign => base.signs.get(index).map(|b| now.signs[index] = b.clone()).is_some(),
                        Kind::Trigger => base.triggers.get(index).map(|b| now.triggers[index] = b.clone()).is_some(),
                        Kind::Warp => base.warps.get(index).map(|b| now.warps[index] = b.clone()).is_some(),
                        Kind::Character => base.characters.get(index).map(|b| now.characters[index] = b.clone()).is_some(),
                    };
                    self.status = if ok { "The event has the values of the game.".into() } else { "This event is not in the game: nothing to restore.".into() };
                    changed = true;
                }
            }
        });
        changed
    }
}

fn number<T: egui::emath::Numeric>(ui: &mut egui::Ui, label: &str, value: &mut T) -> bool {
    ui.horizontal(|ui| {
        ui.label(label);
        super::chrome::spin(ui, value)
    })
    .inner
}

/// The name of a model of a character.
pub fn model_name(model: u16) -> String {
    match crate::names::MODEL_NAMES.get(model as usize) {
        Some(name) => name.to_string(),
        None => format!("Model {model}"),
    }
}

/// The name of a map.
pub fn map_name(map: u16) -> String {
    match crate::names::MAP_NAMES.get(map as usize) {
        Some(name) => format!("{name} [{map}]"),
        None => format!("Map {map}"),
    }
}

/// A list of choices for a number: the name of each choice, with its number.
fn choice(ui: &mut egui::Ui, label: &str, value: &mut u16, options: &[(u16, String)]) -> bool {
    let current = options.iter().find(|(v, _)| v == value).map_or(format!("{value}"), |(v, n)| format!("{v}: {n}"));
    let mut changed = false;
    ui.horizontal(|ui| {
        ui.label(label);
        egui::ComboBox::from_id_salt(label).selected_text(current).width(190.0).show_ui(ui, |ui| {
            egui::ScrollArea::vertical().max_height(320.0).show(ui, |ui| {
                for (v, name) in options {
                    if ui.selectable_label(*value == *v, format!("{v}: {name}")).clicked() {
                        *value = *v;
                        changed = true;
                    }
                }
            });
        });
        // The number can also be typed, for a value that the list does not have.
        changed |= super::chrome::spin(ui, value);
    });
    changed
}

fn choice_u8(ui: &mut egui::Ui, label: &str, value: &mut u8, options: &[(u16, String)]) -> bool {
    let mut v = *value as u16;
    let changed = choice(ui, label, &mut v, options);
    *value = v.min(255) as u8;
    changed
}

fn named(list: &[&str]) -> Vec<(u16, String)> {
    list.iter().enumerate().map(|(i, n)| (i as u16, n.to_string())).collect()
}

/// The direction of a character or of the exit of a warp.
fn facing_options() -> Vec<(u16, String)> {
    named(&["Up", "Down", "Left", "Right", "Up left", "Up right", "Down left", "Down right"])
}

/// The side from which a sign reacts.
fn talk_options() -> Vec<(u16, String)> {
    named(&["From below (the player looks up)", "From the left", "From the right", "From above", "From all sides", "From the sides", "From above and below"])
}

fn check_options() -> Vec<(u16, String)> {
    let mut v = named(&["Step on it", "(unused 1)", "(unused 2)", "(unused 3)", "(unused 4)", "Push up", "Push down", "Push left", "Push right", "Push up-left", "Push up-right", "Push down-left", "Push down-right"]);
    v.truncate(13);
    v
}

fn exit_options() -> Vec<(u16, String)> {
    let mut v = named(&[
        "Nothing", "Fade only", "Mat (walk in)", "Prop", "Script only", "Stairs", "Warp (teleport)", "Secret base", "Mat without fade-in", "Stairs without fade-in",
        "Ladder up", "Ladder down",
    ]);
    for k in 12..50u16 {
        v.push((k, format!("Special {}", k - 12 + 1)));
    }
    v
}

fn interaction_options() -> Vec<(u16, String)> {
    let mut v = vec![(0, "Talk only".to_string()), (1, "Trainer (looks ahead)".to_string()), (2, "Trainer (sees all sides)".to_string())];
    for k in 3..24u16 {
        v.push((k, format!("Kind {k}")));
    }
    v
}

fn model_options() -> Vec<(u16, String)> {
    crate::names::MODEL_NAMES.iter().enumerate().map(|(i, n)| (i as u16, n.to_string())).collect()
}

fn map_options() -> Vec<(u16, String)> {
    crate::names::MAP_NAMES.iter().enumerate().map(|(i, n)| (i as u16, n.to_string())).collect()
}

fn sign_fields(ui: &mut egui::Ui, s: &mut Sign) -> bool {
    let mut c = false;
    c |= number(ui, "Script (id)", &mut s.id);
    c |= number(ui, "Kind (0..10)", &mut s.kind);
    c |= choice_u8(ui, "Talk from", &mut s.facing, &talk_options());
    c |= number(ui, "Tile x", &mut s.x);
    c |= number(ui, "Tile z", &mut s.z);
    c |= number(ui, "Width (tiles)", &mut s.width);
    c |= number(ui, "Depth (tiles)", &mut s.depth);
    c |= number(ui, "Height", &mut s.height);
    c
}

fn trigger_fields(ui: &mut egui::Ui, t: &mut Trigger) -> bool {
    let mut c = false;
    c |= number(ui, "Script (id)", &mut t.id);
    c |= number(ui, "Work (0 = always)", &mut t.work_id);
    c |= number(ui, "Value of the work", &mut t.param);
    c |= choice(ui, "Starts when", &mut t.check_kind, &check_options());
    c |= number(ui, "Tile x", &mut t.x);
    c |= number(ui, "Tile z", &mut t.z);
    c |= number(ui, "Width (tiles)", &mut t.width);
    c |= number(ui, "Depth (tiles)", &mut t.depth);
    c |= number(ui, "Height", &mut t.height);
    c
}

fn warp_fields(ui: &mut egui::Ui, w: &mut Warp) -> bool {
    let mut c = false;
    c |= choice(ui, "Goes to", &mut w.map, &map_options());
    c |= number(ui, "Exit number", &mut w.exit);
    c |= choice_u8(ui, "Facing at the exit", &mut w.facing, &facing_options());
    let mut kind = w.kind as u16;
    if choice(ui, "Kind of exit", &mut kind, &exit_options()) {
        w.kind = kind.min(255) as u8;
        c = true;
    }
    c |= number(ui, "x (units, tile x 18 + 9)", &mut w.x);
    c |= number(ui, "y (height)", &mut w.y);
    c |= number(ui, "z (units, tile x 18 + 9)", &mut w.z);
    c |= number(ui, "Width (tiles)", &mut w.width);
    c |= number(ui, "Depth (tiles)", &mut w.depth);
    ui.label("A map or an exit that does not exist can freeze the game. A door must be on a wall tile.");
    c
}

fn character_fields(ui: &mut egui::Ui, ch: &mut Character) -> bool {
    let mut c = false;
    c |= number(ui, "Script (3000 + trainer)", &mut ch.script);
    c |= choice(ui, "Model", &mut ch.model, &model_options());
    c |= choice_u8(ui, "Facing", &mut ch.facing, &facing_options());
    c |= number(ui, "Movement", &mut ch.movement);
    c |= choice(ui, "Interaction", &mut ch.interaction, &interaction_options());
    c |= number(ui, "Hidden when this flag is set", &mut ch.hide_flag);
    c |= number(ui, "Parameter 0 (sight range)", &mut ch.param[0]);
    c |= number(ui, "Parameter 1", &mut ch.param[1]);
    c |= number(ui, "Parameter 2", &mut ch.param[2]);
    c |= number(ui, "Tile x", &mut ch.x);
    c |= number(ui, "Tile z", &mut ch.z);
    c |= number(ui, "Height", &mut ch.height);
    c |= number(ui, "Area x", &mut ch.trigger[0]);
    c |= number(ui, "Area z", &mut ch.trigger[1]);
    c |= number(ui, "Area width", &mut ch.trigger[2]);
    c |= number(ui, "Area depth", &mut ch.trigger[3]);
    c
}

fn prop_fields(ui: &mut egui::Ui, d: &mut crate::edit::Prop) -> bool {
    let mut c = false;
    c |= number(ui, "Model of the door", &mut d.kind);
    c |= number(ui, "x (units, tile x 18 + 9)", &mut d.x);
    c |= number(ui, "y (height)", &mut d.y);
    c |= number(ui, "z (units, tile x 18 + 9)", &mut d.z);
    c |= number(ui, "Scale x", &mut d.scale[0]);
    c |= number(ui, "Scale y", &mut d.scale[1]);
    c |= number(ui, "Scale z", &mut d.scale[2]);
    ui.label("The props are saved with the block: use Export the block.");
    c
}

/// The list of the models of the characters.
#[derive(Default)]
pub struct NpcPanel {
    pub ready: bool,
    pub ids: Vec<u16>,
    pub templates: std::collections::BTreeMap<u16, Character>,
    pub thumbs: std::collections::HashMap<u16, egui::TextureHandle>,
    pub search: String,
}

/// The distance along a ray to a box (the slab method), if the ray hits it.
fn ray_box(origin: Vec3, direction: Vec3, low: Vec3, high: Vec3) -> Option<f32> {
    let (mut near, mut far) = (f32::MIN, f32::MAX);
    for a in 0..3 {
        let (o, d) = (origin[a], direction[a]);
        if d.abs() < 1e-9 {
            if o < low[a] || o > high[a] {
                return None;
            }
            continue;
        }
        let (t1, t2) = ((low[a] - o) / d, (high[a] - o) / d);
        near = near.max(t1.min(t2));
        far = far.min(t1.max(t2));
    }
    (near <= far && far > 0.0).then_some(near.max(0.0))
}

/// A mark on the ground under each object of a block that can be moved: a
/// square and a pole. `offset` is the place of the block in the world.
fn object_marks(editor: &crate::editor::Editor, offset: [f32; 2], selected: &[usize], out: &mut Vec<Vertex>) {
    for (i, instance) in editor.instances.iter().enumerate() {
        // (A tile of ground or a piece of grass that was put on a tile is not an object.)
        if instance.deleted || instance.replaces.is_some() {
            continue;
        }
        let (low, high) = editor.instance_bounds(i);
        let colour = if selected.contains(&i) { [255, 235, 40, 150] } else { [70, 150, 255, 110] };
        let y = low[1] + 1.5;
        let v = |x: f32, y: f32, z: f32| Vertex { position: [x + offset[0], y, z + offset[1]], uv: [0.0, 0.0], colour };
        let (a, b, c, d) = (v(low[0], y, low[2]), v(high[0], y, low[2]), v(high[0], y, high[2]), v(low[0], y, high[2]));
        out.extend_from_slice(&[a, b, c, a, c, d]);
        let (mx, mz) = ((low[0] + high[0]) / 2.0, (low[2] + high[2]) / 2.0);
        for (dx, dz) in [(1.4, 0.0), (0.0, 1.4)] {
            let top = y + 22.0;
            let (p0, p1, p2, p3) = (v(mx - dx, y, mz - dz), v(mx + dx, y, mz + dz), v(mx + dx, top, mz + dz), v(mx - dx, top, mz - dz));
            out.extend_from_slice(&[p0, p1, p2, p0, p2, p3]);
        }
    }
}
