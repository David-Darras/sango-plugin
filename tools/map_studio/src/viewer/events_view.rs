//! The events of the zone in the viewer: signs, triggers, warps and
//! characters, drawn on the ground, selected, moved, added and saved.

use super::*;
use crate::events::{self, Character, Sign, Trigger, Warp, ZoneAt, ZoneEvents};

/// The kinds of events.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Kind {
    Sign,
    Trigger,
    Warp,
    Character,
}

impl Kind {
    fn name(self) -> &'static str {
        match self {
            Kind::Sign => "sign",
            Kind::Trigger => "trigger",
            Kind::Warp => "warp",
            Kind::Character => "character",
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
        }
    }
}

/// The events of the zone that the viewer shows.
pub struct EventsState {
    pub loaded: bool,
    pub zones: Vec<ZoneAt>,
    /// The zone that is shown: a place in `zones`.
    pub zone: usize,
    /// The events as the game has them, and as the user changed them.
    pub base: ZoneEvents,
    pub now: ZoneEvents,
    pub selected: Option<(Kind, usize)>,
    /// The next click puts a new event of this kind.
    pub put: Option<Kind>,
    /// The next click moves the selected event.
    pub moving: bool,
    pub show: [bool; 4],
    pub labels: bool,
}

impl Default for EventsState {
    fn default() -> Self {
        EventsState {
            loaded: false,
            zones: Vec::new(),
            zone: 0,
            base: ZoneEvents::default(),
            now: ZoneEvents::default(),
            selected: None,
            put: None,
            moving: false,
            show: [true; 4],
            labels: true,
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
}

const MAX_SIGNS: usize = 40;
const MAX_TRIGGERS: usize = 40;
const MAX_WARPS: usize = 24;

impl EventsState {
    fn at(&self) -> Option<&ZoneAt> {
        self.zones.get(self.zone)
    }
}

impl Viewer {
    /// Reads the zones of the block and the events of the first one.
    pub(super) fn events_enter(&mut self) {
        if self.ev.loaded {
            return;
        }
        self.ev.loaded = true;
        match crate::events::zones_of_block(&self.arc_dir, self.editor.block_id()) {
            Ok(zones) if !zones.is_empty() => {
                // The zone with the most events is the one that the user wants.
                let mut best = (0, 0usize);
                for (i, z) in zones.iter().enumerate() {
                    if let Ok(e) = crate::events::read_zone_events(&self.arc_dir, z.zone) {
                        let n = e.signs.len() + e.characters.len() + e.warps.len() + e.triggers.len();
                        if n > best.1 {
                            best = (i, n);
                        }
                    }
                }
                self.ev.zones = zones;
                self.ev.zone = best.0;
                self.events_load_zone();
            }
            Ok(_) => self.status = "No zone of the game uses this block: there are no events here.".into(),
            Err(e) => self.status = format!("Cannot read the zones: {e}"),
        }
    }

    /// Reads the events of the chosen zone, with the changes of the file.
    pub(super) fn events_load_zone(&mut self) {
        let Some(at) = self.ev.at().cloned() else { return };
        match crate::events::read_zone_events(&self.arc_dir, at.zone) {
            Ok(base) => {
                // The file of the zone that the tool wrote, else the game.
                let mut now = events::read_zone_override(&self.export_dir, at.zone).unwrap_or_else(|| base.clone());
                if let Ok(text) = std::fs::read_to_string(events::file_path(&self.export_dir)) {
                    events::apply_file(at.zone, &mut now, &text);
                }
                self.ev.base = base;
                self.ev.now = now;
                self.ev.selected = None;
                self.status = format!(
                    "Zone {}: {} signs, {} characters, {} warps, {} triggers.",
                    at.zone,
                    self.ev.now.signs.len(),
                    self.ev.now.characters.len(),
                    self.ev.now.warps.len(),
                    self.ev.now.triggers.len()
                );
            }
            Err(e) => self.status = format!("Cannot read the events: {e}"),
        }
    }

    /// The height of the ground at a place, if there is ground.
    fn ground_at(&self, x: f32, z: f32) -> Option<f32> {
        self.pick(Vec3::new(x, 4000.0, z), Vec3::NEG_Y, true).map(|h| h.point.y)
    }

    /// The events as rectangles on the ground of the block.
    pub(super) fn event_boxes(&self) -> Vec<EventBox> {
        let Some(at) = self.ev.at() else { return Vec::new() };
        let (col, row) = (at.column, at.row);
        let mut out = Vec::new();
        let rect = |x: f32, z: f32, w: f32, d: f32| {
            ([events::tile_to_local(x, col), events::tile_to_local(z, row)], [events::tile_to_local(x + w, col), events::tile_to_local(z + d, row)])
        };
        if self.ev.show[0] {
            for (i, s) in self.ev.now.signs.iter().enumerate() {
                let (low, high) = rect(s.x as f32, s.z as f32, s.width.max(1) as f32, s.depth.max(1) as f32);
                out.push(EventBox { kind: Kind::Sign, index: i, low, high, label: format!("S{i} id {}", s.id), colour: Kind::Sign.colour(0) });
            }
        }
        if self.ev.show[1] {
            for (i, t) in self.ev.now.triggers.iter().enumerate() {
                let (low, high) = rect(t.x as f32, t.z as f32, t.width.max(1) as f32, t.depth.max(1) as f32);
                out.push(EventBox { kind: Kind::Trigger, index: i, low, high, label: format!("T{i} id {}", t.id), colour: Kind::Trigger.colour(0) });
            }
        }
        if self.ev.show[2] {
            for (i, w) in self.ev.now.warps.iter().enumerate() {
                let (wd, dp) = (w.width.max(1) as f32, w.depth.max(1) as f32);
                let (low, high) = rect(w.x as f32 / events::TILE - wd / 2.0, w.z as f32 / events::TILE - dp / 2.0, wd, dp);
                out.push(EventBox { kind: Kind::Warp, index: i, low, high, label: format!("W{i} to {}.{}", w.zone, w.exit), colour: Kind::Warp.colour(0) });
            }
        }
        if self.ev.show[3] {
            for (i, c) in self.ev.now.characters.iter().enumerate() {
                let (low, high) = rect(c.x as f32, c.z as f32, 1.0, 1.0);
                let label = if (3000..4000).contains(&c.script) {
                    format!("C{i} trainer {} m{}", c.script - 3000, c.model)
                } else {
                    format!("C{i} s{} m{}", c.script, c.model)
                };
                out.push(EventBox { kind: Kind::Character, index: i, low, high, label, colour: Kind::Character.colour(c.script) });
            }
        }
        out
    }

    /// The coloured squares and poles that show the events.
    pub(super) fn event_vertices(&self) -> Vec<Vertex> {
        if !self.events_mode {
            return Vec::new();
        }
        let mut out = Vec::new();
        for b in self.event_boxes() {
            let selected = self.ev.selected == Some((b.kind, b.index));
            let colour = if selected { [255, 235, 40, 210] } else { b.colour };
            let centre = [(b.low[0] + b.high[0]) / 2.0, (b.low[1] + b.high[1]) / 2.0];
            let y = self.ground_at(centre[0], centre[1]).unwrap_or(0.0) + 1.5;
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
        out
    }

    /// Writes the names of the events over the scene.
    pub(super) fn events_labels(&self, painter: &egui::Painter, rect: egui::Rect) {
        if !self.events_mode || !self.ev.labels {
            return;
        }
        for b in self.event_boxes() {
            let centre = [(b.low[0] + b.high[0]) / 2.0, (b.low[1] + b.high[1]) / 2.0];
            let y = self.ground_at(centre[0], centre[1]).unwrap_or(0.0) + 34.0;
            let clip = self.matrix * Vec4::new(centre[0], y, centre[1], 1.0);
            if clip.w <= 0.0 {
                continue;
            }
            let (nx, ny) = (clip.x / clip.w, clip.y / clip.w);
            if nx.abs() > 1.0 || ny.abs() > 1.0 {
                continue;
            }
            let pos = egui::pos2(rect.min.x + (nx * 0.5 + 0.5) * rect.width(), rect.min.y + (1.0 - (ny * 0.5 + 0.5)) * rect.height());
            let selected = self.ev.selected == Some((b.kind, b.index));
            let colour = if selected { egui::Color32::from_rgb(255, 235, 40) } else { egui::Color32::WHITE };
            let font = egui::FontId::proportional(12.0);
            let galley = painter.layout_no_wrap(b.label.clone(), font, colour);
            let shade = egui::Rect::from_center_size(pos, galley.size() + egui::vec2(6.0, 2.0));
            painter.rect_filled(shade, 3.0, egui::Color32::from_black_alpha(170));
            painter.galley(pos - galley.size() / 2.0, galley, colour);
        }
    }

    /// The tile of the zone under a point of the block.
    fn tile_under(&self, point: Vec3) -> Option<(i32, i32)> {
        let at = self.ev.at()?;
        Some((events::local_to_tile(point.x, at.column).floor() as i32, events::local_to_tile(point.z, at.row).floor() as i32))
    }

    /// A click in the mode of the events: put, move or select.
    pub(super) fn events_click(&mut self, origin: Vec3, direction: Vec3, frame: &eframe::Frame) {
        let Some(hit) = self.pick(origin, direction, true) else { return };
        let Some((tx, tz)) = self.tile_under(hit.point) else { return };
        if tx < 0 || tz < 0 {
            self.status = "This is outside the zone.".into();
            return;
        }
        if let Some(kind) = self.ev.put.take() {
            self.events_add(kind, tx, tz, hit.point.y);
        } else if self.ev.moving {
            self.ev.moving = false;
            if let Some((kind, index)) = self.ev.selected {
                self.events_set_tile(kind, index, tx, tz, Some(hit.point.y));
                self.status = format!("The {} is in tile ({tx}, {tz}).", kind.name());
            }
        } else {
            // The smallest event under the click; a character before the others.
            let mut found: Option<(f32, Kind, usize)> = None;
            for b in self.event_boxes() {
                let inside = hit.point.x >= b.low[0] && hit.point.x <= b.high[0] && hit.point.z >= b.low[1] && hit.point.z <= b.high[1];
                if inside {
                    let weight = (b.high[0] - b.low[0]) * (b.high[1] - b.low[1]) - if b.kind == Kind::Character { 1.0 } else { 0.0 };
                    if found.map_or(true, |(w, _, _)| weight < w) {
                        found = Some((weight, b.kind, b.index));
                    }
                }
            }
            self.ev.selected = found.map(|(_, k, i)| (k, i));
            self.status = match found {
                Some((_, k, i)) => format!("Selected the {} {i}.", k.name()),
                None => "No event here.".into(),
            };
        }
        self.refresh_events(frame);
    }

    /// Puts a new event in a tile.
    fn events_add(&mut self, kind: Kind, tx: i32, tz: i32, ground: f32) {
        let zone = self.ev.at().map_or(0, |z| z.zone) as u16;
        let now = &mut self.ev.now;
        let (x, z) = (tx as u16, tz as u16);
        let index = match kind {
            Kind::Sign => {
                if now.signs.len() >= MAX_SIGNS {
                    self.status = "A zone has at most 40 signs.".into();
                    return;
                }
                now.signs.push(Sign { raw: Vec::new(), id: 65535, kind: 0, facing: 4, x: tx as i16, z: tz as i16, width: 1, depth: 1, height: ground as i32 });
                now.signs.len() - 1
            }
            Kind::Trigger => {
                if now.triggers.len() >= MAX_TRIGGERS {
                    self.status = "A zone has at most 40 triggers.".into();
                    return;
                }
                now.triggers.push(Trigger { raw: Vec::new(), id: 65535, param: 0, work_id: 0, check_kind: 0, x, z, width: 1, depth: 1, height: ground as i16 });
                now.triggers.len() - 1
            }
            Kind::Warp => {
                if now.warps.len() >= MAX_WARPS {
                    self.status = "A zone has at most 24 warps.".into();
                    return;
                }
                now.warps.push(Warp {
                    raw: Vec::new(),
                    zone,
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
                let model = now.characters.first().map_or(204, |c| c.model);
                let local = now.characters.iter().map(|c| c.local_id + 1).max().unwrap_or(0);
                let mut c = events::new_character(zone, local, x, z, ground);
                c.model = model;
                now.characters.push(c);
                now.characters.len() - 1
            }
        };
        self.ev.selected = Some((kind, index));
        self.status = format!("New {} in tile ({tx}, {tz}). Its script is 65535: the game ignores it until you give it one.", kind.name());
    }

    fn events_set_tile(&mut self, kind: Kind, index: usize, tx: i32, tz: i32, ground: Option<f32>) {
        let now = &mut self.ev.now;
        match kind {
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
        let base = &self.ev.base;
        let now = &mut self.ev.now;
        let (shipped, len) = match kind {
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

    /// Writes the pack of the zone with the new events on the SD card. The
    /// lines of this zone leave `map_events.txt`: the pack has the changes now.
    fn events_save(&mut self) {
        let Some(at) = self.ev.at().cloned() else { return };
        let written = events::write_zone_override(&self.arc_dir, &self.export_dir, at.zone, &self.ev.now);
        self.status = match written {
            Ok(message) => {
                let path = events::file_path(&self.export_dir);
                if let Ok(old) = std::fs::read_to_string(&path) {
                    let text = events::merge_file(&old, at.zone, &self.ev.now, &self.ev.now);
                    let _ = std::fs::write(&path, text);
                }
                format!("Zone {} written in {message}. Start the game again.", at.zone)
            }
            Err(e) => format!("Cannot write the zone: {e}"),
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
        if let Some((kind, index)) = self.ev.selected {
            if dx != 0 || dz != 0 || dy != 0 {
                let now = &mut self.ev.now;
                match kind {
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
        if escape || delete || dx != 0 || dz != 0 || dy != 0 || grab {
            self.refresh_events(frame);
        }
    }

    /// The panel of the events, in the panel on the left. Returns true when
    /// an event changed (the squares must be drawn again).
    pub(super) fn events_ui(&mut self, ui: &mut egui::Ui) -> bool {
        let mut changed = false;
        ui.label("Click an event to select it. Arrows: move one tile. Page up/down: height. G: click a tile to move it there. Delete: take away the event. Esc: select nothing.");
        if self.ev.zones.len() > 1 {
            let before = self.ev.zone;
            let text = |z: &ZoneAt| format!("{} (matrix {}, {}x{})", z.zone, z.matrix, z.width, z.height);
            egui::ComboBox::from_label("Zone")
                .selected_text(self.ev.at().map_or("-".to_string(), |z| text(z)))
                .show_ui(ui, |ui| {
                    for (i, z) in self.ev.zones.iter().enumerate() {
                        ui.selectable_value(&mut self.ev.zone, i, text(z));
                    }
                });
            if self.ev.zone != before {
                self.events_load_zone();
                changed = true;
            }
        } else if let Some(z) = self.ev.at() {
            ui.label(format!("Zone {} (matrix {}, {}x{} blocks, this block is at column {} row {}).", z.zone, z.matrix, z.width, z.height, z.column, z.row));
        }
        ui.horizontal(|ui| {
            for (k, name) in ["Signs", "Triggers", "Warps", "Characters"].iter().enumerate() {
                if ui.checkbox(&mut self.ev.show[k], *name).changed() {
                    changed = true;
                }
            }
        });
        ui.checkbox(&mut self.ev.labels, "Names over the events");
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
        if let Some((kind, index)) = self.ev.selected {
            ui.separator();
            let shipped = match kind {
                Kind::Sign => self.ev.base.signs.len(),
                Kind::Trigger => self.ev.base.triggers.len(),
                Kind::Warp => self.ev.base.warps.len(),
                Kind::Character => self.ev.base.characters.len(),
            };
            ui.label(format!("{} {index} ({})", kind.name(), if index < shipped { "from the game" } else { "added by you" }));
            let now = &mut self.ev.now;
            changed |= match kind {
                Kind::Sign => now.signs.get_mut(index).map_or(false, |s| sign_fields(ui, s)),
                Kind::Trigger => now.triggers.get_mut(index).map_or(false, |t| trigger_fields(ui, t)),
                Kind::Warp => now.warps.get_mut(index).map_or(false, |w| warp_fields(ui, w)),
                Kind::Character => now.characters.get_mut(index).map_or(false, |c| character_fields(ui, c)),
            };
        }
        ui.horizontal(|ui| {
            if ui.button("Save the events").on_hover_text("Writes the pack of the zone (sango/zone/<number>.bin) on the SD card: the plugin gives it to the game").clicked() {
                self.events_save();
            }
            if ui.button("Read again").on_hover_text("Reads the events of the file again (the changes that you did not save are lost)").clicked() {
                self.events_load_zone();
                changed = true;
            }
        });
        ui.horizontal(|ui| {
            if ui.button("Reset the zone to the game").on_hover_text("Takes away the file of the zone and its lines of map_events.txt: the game has its own events again").clicked() {
                if let Some(at) = self.ev.at().cloned() {
                    let result = events::reset_zone(&self.export_dir, at.zone);
                    self.events_load_zone();
                    self.status = match result {
                        Ok(message) => format!("Zone {}: {message}. Start the game again.", at.zone),
                        Err(e) => format!("Cannot reset the zone: {e}"),
                    };
                    changed = true;
                }
            }
            if let Some((kind, index)) = self.ev.selected {
                if ui.button("Restore this event").on_hover_text("Gives this event the values of the game (only an event of the game)").clicked() {
                    let (base, now) = (&self.ev.base, &mut self.ev.now);
                    let ok = match kind {
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
        ui.add(egui::DragValue::new(value)).changed()
    })
    .inner
}

fn sign_fields(ui: &mut egui::Ui, s: &mut Sign) -> bool {
    let mut c = false;
    c |= number(ui, "Script (id)", &mut s.id);
    c |= number(ui, "Kind (0..10)", &mut s.kind);
    c |= number(ui, "Talk from (0 down 1 left 2 right 3 up 4 all 5 sides 6 up/down)", &mut s.facing);
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
    c |= number(ui, "Check (0 step, 5..12 push)", &mut t.check_kind);
    c |= number(ui, "Tile x", &mut t.x);
    c |= number(ui, "Tile z", &mut t.z);
    c |= number(ui, "Width (tiles)", &mut t.width);
    c |= number(ui, "Depth (tiles)", &mut t.depth);
    c |= number(ui, "Height", &mut t.height);
    c
}

fn warp_fields(ui: &mut egui::Ui, w: &mut Warp) -> bool {
    let mut c = false;
    c |= number(ui, "Goes to the zone", &mut w.zone);
    c |= number(ui, "Exit number", &mut w.exit);
    c |= number(ui, "Facing at the exit", &mut w.facing);
    c |= number(ui, "Kind of exit", &mut w.kind);
    c |= number(ui, "x (units, tile x 18 + 9)", &mut w.x);
    c |= number(ui, "y (height)", &mut w.y);
    c |= number(ui, "z (units, tile x 18 + 9)", &mut w.z);
    c |= number(ui, "Width (tiles)", &mut w.width);
    c |= number(ui, "Depth (tiles)", &mut w.depth);
    ui.label("A zone or an exit that does not exist can freeze the game.");
    c
}

fn character_fields(ui: &mut egui::Ui, ch: &mut Character) -> bool {
    let mut c = false;
    c |= number(ui, "Script (3000 + trainer)", &mut ch.script);
    c |= number(ui, "Model", &mut ch.model);
    c |= number(ui, "Facing", &mut ch.facing);
    c |= number(ui, "Movement", &mut ch.movement);
    c |= number(ui, "Interaction kind", &mut ch.interaction);
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
