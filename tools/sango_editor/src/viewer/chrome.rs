//! The frame of the window: the theme, the menu, the tool bar, the status bar,
//! the camera of the mouse and of the keyboard, the help and the questions.

use super::*;

/// The tools of the window. Only one tool works at a time.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Tool {
    /// Select, move, copy and delete the objects.
    Select,
    /// Paint the properties of the ground (free, blocked, tall grass, ledge, water).
    Properties,
    /// Put the pieces of ground (the pictures of the tiles) from the catalogue, or copy one from the map.
    Ground,
}

/// A number that can be typed, dragged, or changed with two small arrows (up
/// and down). Shift makes the arrows go by ten. `customize` changes how the
/// number is written. Returns true when the number changed.
pub fn spin_with<N: egui::emath::Numeric>(
    ui: &mut egui::Ui,
    value: &mut N,
    min: f64,
    max: f64,
    customize: impl for<'b> FnOnce(egui::DragValue<'b>) -> egui::DragValue<'b>,
) -> bool {
    let mut changed = false;
    ui.horizontal(|ui| {
        ui.spacing_mut().item_spacing.x = 2.0;
        changed |= ui.add(customize(egui::DragValue::new(&mut *value).range(min..=max))).changed();
        let mut delta = 0.0;
        ui.vertical(|ui| {
            ui.spacing_mut().item_spacing.y = 0.0;
            ui.spacing_mut().button_padding = egui::vec2(3.0, 0.0);
            let arrow = |text: &str| egui::Button::new(egui::RichText::new(text).size(8.0)).min_size(egui::vec2(16.0, 10.0));
            let step = if ui.input(|i| i.modifiers.shift) { 10.0 } else { 1.0 };
            if ui.add(arrow("\u{25B2}")).on_hover_text("More (Shift: by ten)").clicked() {
                delta = step;
            }
            if ui.add(arrow("\u{25BC}")).on_hover_text("Less (Shift: by ten)").clicked() {
                delta = -step;
            }
        });
        if delta != 0.0 {
            let next = (value.to_f64() + delta).clamp(min, max);
            if next != value.to_f64() {
                *value = N::from_f64(next);
                changed = true;
            }
        }
    });
    changed
}

/// A number with its two arrows (see `spin_with`), over all the values of its type.
pub fn spin<N: egui::emath::Numeric>(ui: &mut egui::Ui, value: &mut N) -> bool {
    spin_with(ui, value, N::MIN.to_f64(), N::MAX.to_f64(), |d| d)
}

/// What the menu, the tool bar and the keys ask for in a frame.
#[derive(Default)]
pub struct Commands {
    pub action: Option<Action>,
    pub tool: Option<Tool>,
    /// The scene must be sent again to the graphics card.
    pub refresh: bool,
}

/// A question to answer before something that cannot be undone.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Confirm {
    ResetMap,
    ResetMatrix,
    ResetBlock,
    ResetWorld,
    ClearBlock,
    FlattenBlock,
    ResetEvents,
    RotateRight,
    RotateLeft,
}

/// A move of the camera that runs over a few frames.
#[derive(Clone, Copy, Debug)]
pub struct CamGoal {
    pub target: Vec3,
    pub distance: f32,
    pub yaw: f32,
    pub pitch: f32,
}

/// The closest and the farthest that the camera can be from what it looks at.
const MIN_DISTANCE: f32 = 2.0;
const MAX_DISTANCE: f32 = 14000.0;
/// The angle of view of the camera.
const FIELD_OF_VIEW: f32 = 45.0;
/// The views that the keys 1 and 2 give.
const TOP_PITCH: f32 = 1.5;
const ISO_YAW: f32 = 0.8;
const ISO_PITCH: f32 = 0.7;

/// The colours of the window.
const ACCENT: egui::Color32 = egui::Color32::from_rgb(86, 156, 255);

/// Gives the window its colours, its spacing and its sizes of text.
pub fn apply_theme(ctx: &egui::Context) {
    let mut v = egui::Visuals::dark();
    v.panel_fill = egui::Color32::from_rgb(30, 33, 39);
    v.window_fill = egui::Color32::from_rgb(37, 40, 47);
    v.extreme_bg_color = egui::Color32::from_rgb(22, 24, 28);
    v.faint_bg_color = egui::Color32::from_rgb(40, 44, 52);
    v.window_rounding = egui::Rounding::same(8.0);
    v.menu_rounding = egui::Rounding::same(6.0);
    v.window_shadow = egui::epaint::Shadow { offset: egui::vec2(0.0, 6.0), blur: 18.0, spread: 0.0, color: egui::Color32::from_black_alpha(110) };
    v.selection.bg_fill = ACCENT.linear_multiply(0.45);
    v.selection.stroke = egui::Stroke::new(1.0_f32, ACCENT);
    v.hyperlink_color = ACCENT;
    let rounding = egui::Rounding::same(5.0);
    for w in [&mut v.widgets.noninteractive, &mut v.widgets.inactive, &mut v.widgets.hovered, &mut v.widgets.active, &mut v.widgets.open] {
        w.rounding = rounding;
    }
    v.widgets.inactive.weak_bg_fill = egui::Color32::from_rgb(52, 57, 66);
    v.widgets.inactive.bg_fill = egui::Color32::from_rgb(52, 57, 66);
    v.widgets.hovered.weak_bg_fill = egui::Color32::from_rgb(68, 76, 90);
    v.widgets.hovered.bg_fill = egui::Color32::from_rgb(68, 76, 90);
    v.widgets.hovered.bg_stroke = egui::Stroke::new(1.0_f32, ACCENT.linear_multiply(0.8));
    v.widgets.active.weak_bg_fill = ACCENT.linear_multiply(0.55);
    v.widgets.active.bg_fill = ACCENT.linear_multiply(0.55);
    ctx.set_visuals(v);

    let mut style = (*ctx.style()).clone();
    style.spacing.item_spacing = egui::vec2(8.0, 6.0);
    style.spacing.button_padding = egui::vec2(10.0, 5.0);
    style.spacing.interact_size.y = 26.0;
    style.spacing.menu_margin = egui::Margin::same(6.0);
    style.spacing.window_margin = egui::Margin::same(10.0);
    style.spacing.scroll.bar_width = 8.0;
    use egui::{FontId, TextStyle};
    style.text_styles.insert(TextStyle::Heading, FontId::proportional(19.0));
    style.text_styles.insert(TextStyle::Body, FontId::proportional(14.5));
    style.text_styles.insert(TextStyle::Button, FontId::proportional(14.5));
    style.text_styles.insert(TextStyle::Small, FontId::proportional(12.0));
    style.text_styles.insert(TextStyle::Monospace, FontId::monospace(13.5));
    ctx.set_style(style);
}

/// A title of a section of the panels.
pub fn section_title(text: &str) -> egui::RichText {
    egui::RichText::new(text).strong().size(15.0)
}

/// A button of a tool: pressed when the tool works.
fn tool_button(ui: &mut egui::Ui, current: Tool, tool: Tool, label: &str, tip: &str) -> bool {
    let selected = current == tool;
    let mut text = egui::RichText::new(label).strong();
    if selected {
        text = text.color(egui::Color32::WHITE);
    }
    let button = egui::Button::new(text).selected(selected).min_size(egui::vec2(78.0, 28.0));
    ui.add(button).on_hover_text(tip).clicked()
}

/// A button with its shortcut, in a menu.
fn menu_item(ui: &mut egui::Ui, text: &str, shortcut: &str, enabled: bool) -> bool {
    let clicked = ui.add_enabled(enabled, egui::Button::new(text).shortcut_text(shortcut)).clicked();
    if clicked {
        ui.close_menu();
    }
    clicked
}

impl Viewer {
    pub(super) fn tool(&self) -> Tool {
        if self.tile_mode {
            Tool::Properties
        } else if self.ground_mode {
            Tool::Ground
        } else {
            Tool::Select
        }
    }

    /// The panel follows what is selected: an event (true) or an object (false).
    pub(super) fn set_events_focus(&mut self, on: bool, frame: &eframe::Frame) {
        if self.events_mode == on {
            return;
        }
        self.events_mode = on;
        if on {
            self.events_toggled(frame);
        } else {
            self.ev.selected = None;
            self.ev.put = None;
            self.ev.moving = false;
            self.refresh(frame);
        }
    }

    /// Changes the tool (the other tools stop).
    pub(super) fn set_tool(&mut self, tool: Tool, frame: &eframe::Frame) {
        if self.tool() == tool {
            return;
        }
        if self.tile_mode && tool != Tool::Properties {
            self.tile_mode = false;
            self.toggle_tiles(frame);
        }
        if self.ground_mode && tool != Tool::Ground {
            self.ground_mode = false;
            self.take_tile = false;
            self.stamp = None;
            self.refresh(frame);
        }
        if self.events_mode && tool != Tool::Select {
            self.events_mode = false;
            self.events_toggled(frame);
        }
        match tool {
            Tool::Select => {}
            Tool::Properties => {
                self.tile_mode = true;
                self.placing = false;
                self.stamp = None;
                self.toggle_tiles(frame);
            }
            Tool::Ground => {
                self.ground_mode = true;
                self.placing = false;
                self.selected.clear();
                self.catalog_open = true;
                self.tile_tab = true;
                self.npc_tab = false;
                self.refresh(frame);
            }
        }
        self.status = match tool {
            Tool::Select => "Objects: click an object or an event. Ctrl+click takes a whole building.".into(),
            Tool::Properties => "Ground properties: paint with the left button, turn the view with the right button.".into(),
            Tool::Ground => "Ground tiles: choose a tile in the catalogue, then click the ground. Or take a tile from the map.".into(),
        };
    }

    /// The keys that work in every tool.
    pub(super) fn global_keys(&mut self, ctx: &egui::Context, cmd: &mut Commands) {
        if ctx.wants_keyboard_input() {
            return;
        }
        let (ctrl, shift, alt) = ctx.input(|i| (i.modifiers.command, i.modifiers.shift, i.modifiers.alt));
        let pressed = |key: egui::Key| ctx.input(|i| i.key_pressed(key));
        if ctrl && pressed(egui::Key::S) {
            cmd.action = Some(Action::SaveProject);
        }
        if ctrl && pressed(egui::Key::O) {
            cmd.action = Some(Action::LoadProject);
        }
        if ctrl && pressed(egui::Key::E) {
            cmd.action = Some(Action::Export);
        }
        if pressed(egui::Key::F1) {
            self.help_open = !self.help_open;
        }
        if ctrl || alt {
            // Alt and the arrows move the camera.
            if alt {
                let (left, right, up, down) = (pressed(egui::Key::ArrowLeft), pressed(egui::Key::ArrowRight), pressed(egui::Key::ArrowUp), pressed(egui::Key::ArrowDown));
                if left || right || up || down {
                    let step = self.distance * 0.08;
                    self.pan_ground((right as i32 - left as i32) as f32 * step, (up as i32 - down as i32) as f32 * step);
                }
                if pressed(egui::Key::PageUp) {
                    self.zoom_by(0.8, None);
                }
                if pressed(egui::Key::PageDown) {
                    self.zoom_by(1.25, None);
                }
            }
            return;
        }
        if pressed(egui::Key::Home) {
            if shift { self.fit_world() } else { self.fit_block() }
        }
        if pressed(egui::Key::F) {
            self.focus_selection();
        }
        if pressed(egui::Key::Num1) {
            self.go_to_view(None, None, Some(TOP_PITCH));
        }
        if pressed(egui::Key::Num2) {
            self.go_to_view(None, Some(ISO_YAW), Some(ISO_PITCH));
        }
        if pressed(egui::Key::Plus) || pressed(egui::Key::Equals) {
            self.zoom_by(0.8, None);
        }
        if pressed(egui::Key::Minus) {
            self.zoom_by(1.25, None);
        }
        if pressed(egui::Key::C) {
            self.catalog_open = !self.catalog_open;
        }
        // Undo and redo work whatever is selected (an object or an event).
        if ctrl && pressed(egui::Key::Z) {
            cmd.action = Some(if shift { Action::Redo } else { Action::Undo });
        }
        if ctrl && pressed(egui::Key::Y) {
            cmd.action = Some(Action::Redo);
        }
        if pressed(egui::Key::S) && !ctrl {
            cmd.tool = Some(Tool::Select);
        }
        if pressed(egui::Key::T) {
            cmd.tool = Some(if self.tool() == Tool::Ground { Tool::Select } else { Tool::Ground });
        }
        if pressed(egui::Key::P) {
            cmd.tool = Some(if self.tool() == Tool::Properties { Tool::Select } else { Tool::Properties });
        }
    }

    // ------------------------------------------------------------ the menu

    pub(super) fn menu_bar(&mut self, ctx: &egui::Context, cmd: &mut Commands) {
        egui::TopBottomPanel::top("menu_bar").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("File", |ui| {
                    if menu_item(ui, "Save the project", "Ctrl+S", true) {
                        cmd.action = Some(Action::SaveProject);
                    }
                    if menu_item(ui, "Load the project", "Ctrl+O", true) {
                        cmd.action = Some(Action::LoadProject);
                    }
                    ui.separator();
                    if menu_item(ui, "Export the block", "Ctrl+E", true) {
                        cmd.action = Some(Action::Export);
                    }
                    if menu_item(ui, "Export and play", "F5", true) {
                        cmd.action = Some(Action::Launch);
                    }
                    ui.separator();
                    if menu_item(ui, "Quit", "", true) {
                        ctx.send_viewport_cmd(egui::ViewportCommand::Close);
                    }
                });
                ui.menu_button("Edit", |ui| {
                    let selected = !self.selected.is_empty();
                    if menu_item(ui, "Undo", "Ctrl+Z", self.can_undo()) {
                        cmd.action = Some(Action::Undo);
                    }
                    if menu_item(ui, "Redo", "Ctrl+Y", self.can_redo()) {
                        cmd.action = Some(Action::Redo);
                    }
                    ui.separator();
                    if menu_item(ui, "Copy (in the block)", "D", selected) {
                        cmd.action = Some(Action::Duplicate);
                    }
                    if menu_item(ui, "Copy out (to another block)", "Ctrl+C", selected) {
                        cmd.action = Some(Action::CopyOut);
                    }
                    if menu_item(ui, "Paste", "Ctrl+V", true) {
                        cmd.action = Some(Action::Paste);
                    }
                    if menu_item(ui, "Delete", "Del", selected) {
                        cmd.action = Some(Action::Delete);
                    }
                    ui.separator();
                    if menu_item(ui, "Flatten the terrain...", "", true) {
                        self.confirm = Some(Confirm::FlattenBlock);
                    }
                    if menu_item(ui, "Turn the block 90\u{b0} to the right", "", true) {
                        self.confirm = Some(Confirm::RotateRight);
                    }
                    if menu_item(ui, "Turn the block 90\u{b0} to the left", "", true) {
                        self.confirm = Some(Confirm::RotateLeft);
                    }
                    if menu_item(ui, "Clear the whole block...", "", true) {
                        self.confirm = Some(Confirm::ClearBlock);
                    }
                    if menu_item(ui, "Restore the original block...", "", true) {
                        self.confirm = Some(Confirm::ResetBlock);
                    }
                    ui.separator();
                    if menu_item(ui, "Grow the selection", "E", selected) {
                        cmd.action = Some(Action::Grow);
                    }
                    if menu_item(ui, "Select nothing", "Esc", selected) {
                        self.selected.clear();
                    }
                });
                ui.menu_button("View", |ui| {
                    if menu_item(ui, "Fit the block", "Home", true) {
                        self.fit_block();
                    }
                    if menu_item(ui, "Fit the whole world", "Shift+Home", true) {
                        self.fit_world();
                    }
                    if menu_item(ui, "Focus the selection", "F", true) {
                        self.focus_selection();
                    }
                    if menu_item(ui, "View from above", "1", true) {
                        self.go_to_view(None, None, Some(TOP_PITCH));
                    }
                    if menu_item(ui, "Isometric view", "2", true) {
                        self.go_to_view(None, Some(ISO_YAW), Some(ISO_PITCH));
                    }
                    ui.separator();
                    ui.checkbox(&mut self.show_inspector, "Panel of the properties");
                    ui.checkbox(&mut self.catalog_open, "Catalogue (C)");
                    if ui.checkbox(&mut self.show_objects, "Movable objects (marks)").changed() {
                        cmd.refresh = true;
                    }
                    ui.checkbox(&mut self.direct_v, "Pictures: direct V");
                    ui.separator();
                    ui.label("Blocks of the world drawn:");
                    ui.horizontal(|ui| {
                        if ui.button("All").clicked() {
                            self.ev.neigh_radius = 64;
                        }
                        if ui.button("None").clicked() {
                            self.ev.neigh_radius = 0;
                        }
                    });
                });
                ui.menu_button("Tools", |ui| {
                    if menu_item(ui, "Objects and events", "S", true) {
                        cmd.tool = Some(Tool::Select);
                    }
                    if menu_item(ui, "Ground tiles", "T", true) {
                        cmd.tool = Some(Tool::Ground);
                    }
                    if menu_item(ui, "Ground properties", "P", true) {
                        cmd.tool = Some(Tool::Properties);
                    }
                });
                ui.menu_button("Help", |ui| {
                    if menu_item(ui, "Keyboard and mouse", "F1", true) {
                        self.help_open = true;
                    }
                });
            });
        });
    }

    // ------------------------------------------------------------ the tool bar

    pub(super) fn toolbar(&mut self, ctx: &egui::Context, cmd: &mut Commands) {
        egui::TopBottomPanel::top("tool_bar").show(ctx, |ui| {
            ui.add_space(2.0);
            ui.horizontal(|ui| {
                let current = self.tool();
                if tool_button(ui, current, Tool::Select, "Objects", "The 3D objects and the events: select, move, copy and delete them (S)") {
                    cmd.tool = Some(Tool::Select);
                }
                if tool_button(ui, current, Tool::Ground, "Ground tiles", "The pieces of ground: put a tile of the catalogue, or copy one from the map (T)") {
                    cmd.tool = Some(Tool::Ground);
                }
                if tool_button(ui, current, Tool::Properties, "Ground properties", "What the ground allows: free to walk, blocked, tall grass, ledge, water (P)") {
                    cmd.tool = Some(Tool::Properties);
                }
                ui.separator();
                let catalog = egui::Button::new(egui::RichText::new("Catalogue").strong()).selected(self.catalog_open).min_size(egui::vec2(88.0, 28.0));
                if ui.add(catalog).on_hover_text("Objects, ground tiles and characters to put on the map (C)").clicked() {
                    self.catalog_open = !self.catalog_open;
                }
                ui.separator();
                if ui.add_enabled(self.can_undo(), egui::Button::new("Undo").min_size(egui::vec2(56.0, 28.0))).on_hover_text("Undo the last change (Ctrl+Z)").clicked() {
                    cmd.action = Some(Action::Undo);
                }
                if ui.add_enabled(self.can_redo(), egui::Button::new("Redo").min_size(egui::vec2(56.0, 28.0))).on_hover_text("Redo (Ctrl+Y)").clicked() {
                    cmd.action = Some(Action::Redo);
                }
                let reset = egui::Button::new(egui::RichText::new("Reset the world").color(egui::Color32::from_rgb(230, 120, 110))).min_size(egui::vec2(110.0, 28.0));
                if ui.add(reset).on_hover_text("All the maps, all the blocks and the grid of this world go back to the game (asks first)").clicked() {
                    self.confirm = Some(Confirm::ResetWorld);
                }
                ui.separator();
                if ui.add(egui::Button::new("Export").min_size(egui::vec2(66.0, 28.0))).on_hover_text("Write the block on the SD card (Ctrl+E)").clicked() {
                    cmd.action = Some(Action::Export);
                }
                let play = egui::Button::new(egui::RichText::new("Export and play").strong().color(egui::Color32::WHITE)).fill(ACCENT.linear_multiply(0.7)).min_size(egui::vec2(120.0, 28.0));
                if ui.add(play).on_hover_text("Write the block on the SD card, then start the game in the emulator (F5)").clicked() {
                    cmd.action = Some(Action::Launch);
                }
                ui.separator();
                self.world_ui(ui);
            });
            ui.add_space(2.0);
        });
    }

    // ------------------------------------------------------------ the status bar

    pub(super) fn status_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::bottom("status_bar").show(ctx, |ui| {
            ui.horizontal(|ui| {
                let bad = ["Cannot", "Export failed", "failed", "Warning"].iter().any(|w| self.status.contains(w));
                let colour = if bad { egui::Color32::from_rgb(220, 30, 30) } else { ui.visuals().text_color() };
                ui.add(egui::Label::new(egui::RichText::new(&self.status).color(colour)).truncate());
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(egui::RichText::new(format!("zoom {:.0}   view {:.0}\u{b0}/{:.0}\u{b0}", self.distance, self.yaw.to_degrees().rem_euclid(360.0), self.pitch.to_degrees())).weak());
                    ui.separator();
                    ui.label(self.hover_text());
                    ui.separator();
                    ui.label(egui::RichText::new(self.tool_hint()).weak());
                });
            });
        });
    }

    /// What the tool does with the mouse.
    fn tool_hint(&self) -> &'static str {
        match self.tool() {
            Tool::Select => "click: select an object or an event  \u{b7}  Ctrl+click: building  \u{b7}  Shift+click: add  \u{b7}  G: place",
            Tool::Properties => "left button: paint  \u{b7}  right button: turn the view",
            Tool::Ground => "click: put the armed tile  \u{b7}  Esc: stop  \u{b7}  right button: slide the view",
        }
    }

    /// The place under the mouse.
    fn hover_text(&self) -> String {
        let Some(p) = self.hover else { return "move the mouse over the map".to_string() };
        let tile = ((p.x + BLOCK_HALF) / TILE).floor() as i32;
        let tile_z = ((p.z + BLOCK_HALF) / TILE).floor() as i32;
        let mut text = format!("x {:.0}  z {:.0}  height {:.0}  tile ({tile}, {tile_z})", p.x, p.z, p.y);
        if let Some(at) = self.ev.at() {
            let zx = crate::events::local_to_tile(p.x, at.column).floor() as i32;
            let zz = crate::events::local_to_tile(p.z, at.row).floor() as i32;
            text += &format!("  map tile ({zx}, {zz})");
        }
        if let Some(name) = &self.hover_event {
            text += &format!("   -   {name}");
        }
        text
    }

    // ------------------------------------------------------------ the help and the questions

    pub(super) fn help_window(&mut self, ctx: &egui::Context) {
        if !self.help_open {
            return;
        }
        let mut open = true;
        egui::Window::new("Keyboard and mouse").open(&mut open).collapsible(false).default_width(520.0).show(ctx, |ui| {
            let rows: &[(&str, &[(&str, &str)])] = &[
                (
                    "Camera",
                    &[
                        ("Left button drag", "turn around the map (in the Ground properties tool: right button)"),
                        ("Right or middle button drag, Shift + left drag", "slide the view over the ground"),
                        ("Mouse wheel", "zoom toward the pointer"),
                        ("Home / Shift+Home", "fit the block / fit the whole world"),
                        ("F", "go to the selection"),
                        ("1 / 2", "view from above / isometric view"),
                        ("Alt + arrows, + and -", "slide and zoom with the keyboard"),
                    ],
                ),
                (
                    "Tools and panels",
                    &[("S / T / P", "Objects / Ground tiles / Ground properties"), ("C", "catalogue"), ("F1", "this help"), ("Esc", "stop what is armed and select nothing")],
                ),
                (
                    "Select tool",
                    &[
                        ("Click, Shift+click", "select an object, add or remove one"),
                        ("Ctrl+click", "take the whole building"),
                        ("Arrows, Page up/down", "move one tile (Shift: 2 units), change the height"),
                        ("D / G / R / Shift+R / E", "copy / place by click / turn / turn back / grow the selection"),
                        ("Delete or X", "delete"),
                        ("Ctrl+C, Ctrl+V", "copy to another block, paste"),
                    ],
                ),
                ("When an event is selected", &[("Click", "select an event (a character, a warp, a sign...)"), ("G then click", "move it to a tile"), ("Arrows", "move one tile"), ("Delete", "remove it")]),
                ("Project", &[("Ctrl+S / Ctrl+O", "save / load the project of the block"), ("Ctrl+E / F5", "export / export and play"), ("Ctrl+Z / Ctrl+Y", "undo / redo")]),
            ];
            for (title, list) in rows {
                ui.label(section_title(title));
                egui::Grid::new(*title).num_columns(2).spacing([16.0, 4.0]).show(ui, |ui| {
                    for (keys, what) in *list {
                        ui.label(egui::RichText::new(*keys).monospace());
                        ui.label(*what);
                        ui.end_row();
                    }
                });
                ui.add_space(6.0);
            }
        });
        self.help_open = open;
    }

    /// The question before a reset. Returns the answer when it is yes.
    pub(super) fn confirm_dialog(&mut self, ctx: &egui::Context) -> Option<Confirm> {
        let question = self.confirm?;
        // A turn of the block needs no question: turning it back undoes it.
        if matches!(question, Confirm::RotateRight | Confirm::RotateLeft) {
            self.confirm = None;
            return Some(question);
        }
        let (title, text, yes) = match question {
            Confirm::RotateRight | Confirm::RotateLeft => return None,
            Confirm::ResetMap => (
                "Reset the map to the game?",
                "The events of this map (file and lines of map_events.txt) go back to the game, and so do the blocks where the map has events: the objects that you moved or added there are lost.",
                "Reset the map",
            ),
            Confirm::ResetEvents => (
                "Give the events of the world back to the game?",
                "All the events (characters, warps, signs, triggers) of all the maps of this world go back to the ones of the game. The blocks (objects, tiles, grass) are not touched.",
                "Restore the events",
            ),
            Confirm::FlattenBlock => (
                "Flatten the terrain of the block?",
                "All the ground of the block gets the same height: the borders between the tiles that one can walk on and the others, and the slopes, go away. The objects, the characters and the warps keep their heights. You can undo it.",
                "Flatten the terrain",
            ),
            Confirm::ClearBlock => (
                "Clear the whole block?",
                "Every object (trees, buildings...), all the tall grass, all the ledges (the bumps of earth), all the door models and all the events (characters, signs, warps, triggers) of the maps in this block are taken away, and the events are written on the SD card. You can undo the objects and the grass, not the events.",
                "Clear the block",
            ),
            Confirm::ResetWorld => (
                "Reset the whole world to the game?",
                "Every map of this world, every block of this world and the grid of the blocks go back to the game. All your changes in this world are lost, in the window too.",
                "Reset the world",
            ),
            Confirm::ResetBlock => (
                "Restore the original block?",
                "The file of this block is removed from the SD card and the block is read again from the game. All its changes (objects, tiles, grass) are lost, in the window too.",
                "Restore the block",
            ),
            Confirm::ResetMatrix => (
                "Reset the world blocks?",
                "The matrix file is removed from the SD card: the blocks of the game come back.",
                "Reset the blocks",
            ),
        };
        let mut answer = None;
        egui::Window::new(title).collapsible(false).resizable(false).anchor(egui::Align2::CENTER_CENTER, [0.0, 0.0]).show(ctx, |ui| {
            ui.set_max_width(380.0);
            ui.label(text);
            if question == Confirm::ClearBlock {
                ui.checkbox(&mut self.clear_blocked, "Also free the blocked tiles (where one cannot walk)");
            }
            ui.add_space(8.0);
            ui.horizontal(|ui| {
                if ui.add(egui::Button::new(egui::RichText::new(yes).color(egui::Color32::WHITE)).fill(egui::Color32::from_rgb(170, 60, 60))).clicked() {
                    answer = Some(question);
                }
                if ui.button("Cancel").clicked() {
                    self.confirm = None;
                }
            });
        });
        if answer.is_some() {
            self.confirm = None;
        }
        answer
    }

    // ------------------------------------------------------------ the camera

    /// What a pixel is worth on the ground, at the distance of the target.
    fn units_per_pixel(&self, rect: egui::Rect) -> f32 {
        2.0 * self.distance * (FIELD_OF_VIEW.to_radians() / 2.0).tan() / rect.height().max(1.0)
    }

    /// The direction of the camera on the ground (to the horizon) and to the right.
    fn ground_axes(&self) -> (Vec3, Vec3) {
        let forward = Vec3::new(-self.yaw.cos(), 0.0, -self.yaw.sin()).normalize_or_zero();
        let right = Vec3::new(-forward.z, 0.0, forward.x);
        (forward, right)
    }

    /// Slides the target over the ground: `right` and `forward` in units.
    fn pan_ground(&mut self, right: f32, forward: f32) {
        let (f, r) = self.ground_axes();
        self.target += r * right + f * forward;
        self.cam_goal = None;
    }

    /// Zooms by a factor. With a point of the ground, this point stays under
    /// the pointer, so that the user can always reach a place.
    fn zoom_by(&mut self, factor: f32, anchor: Option<Vec3>) {
        let old = self.distance;
        let new = (old * factor).clamp(MIN_DISTANCE, MAX_DISTANCE);
        if let Some(a) = anchor {
            self.target = a + (self.target - a) * (new / old);
        }
        self.distance = new;
        self.cam_goal = None;
    }

    /// Goes to a view: a part of the angles can stay as they are.
    fn go_to_view(&mut self, target: Option<(Vec3, f32)>, yaw: Option<f32>, pitch: Option<f32>) {
        let goal = CamGoal {
            target: target.map_or(self.target, |t| t.0),
            distance: target.map_or(self.distance, |t| t.1),
            yaw: yaw.unwrap_or(self.yaw),
            pitch: pitch.unwrap_or(self.pitch),
        };
        self.cam_goal = Some(goal);
    }

    /// The distance from which a width fits in the view.
    fn distance_for(&self, width: f32, height: f32) -> f32 {
        let half = (FIELD_OF_VIEW.to_radians() / 2.0).tan();
        let aspect = 1.6;
        ((width / aspect).max(height) / 2.0 / half * 1.15 + 20.0).clamp(MIN_DISTANCE, MAX_DISTANCE)
    }

    pub(super) fn fit_block(&mut self) {
        let mut ground = 0.0f32;
        let mut count = 0;
        for (_, mesh) in self.editor.meshes() {
            for p in mesh.positions.iter().step_by(40) {
                ground += p[1];
                count += 1;
            }
        }
        let height = if count > 0 { ground / count as f32 } else { 0.0 };
        let distance = self.distance_for(BLOCK_HALF * 2.0, BLOCK_HALF * 2.0);
        self.go_to_view(Some((Vec3::new(0.0, height, 0.0), distance)), None, None);
    }

    pub(super) fn fit_world(&mut self) {
        let (Some(at), Some(matrix)) = (self.ev.at().cloned(), self.ev.matrix.clone()) else {
            self.fit_block();
            return;
        };
        let size = 2.0 * BLOCK_HALF;
        let (low_x, low_z) = (-(at.column as f32) * size - BLOCK_HALF, -(at.row as f32) * size - BLOCK_HALF);
        let (width, depth) = (matrix.width as f32 * size, matrix.height as f32 * size);
        let centre = Vec3::new(low_x + width / 2.0, 0.0, low_z + depth / 2.0);
        let distance = self.distance_for(width, depth);
        self.go_to_view(Some((centre, distance)), None, None);
    }

    pub(super) fn focus_selection(&mut self) {
        if let Some((low, high)) = self.selection_box() {
            let centre = Vec3::new((low[0] + high[0]) / 2.0, (low[1] + high[1]) / 2.0, (low[2] + high[2]) / 2.0);
            let size = (high[0] - low[0]).max(high[2] - low[2]).max(60.0);
            let distance = (size * 2.2).clamp(MIN_DISTANCE, self.distance.max(120.0));
            self.go_to_view(Some((centre, distance)), None, None);
        } else if let Some(p) = self.hover {
            self.go_to_view(Some((p, (self.distance * 0.5).clamp(MIN_DISTANCE, MAX_DISTANCE))), None, None);
        }
    }

    /// The camera moves a little each frame toward its goal.
    fn step_goal(&mut self, ctx: &egui::Context) {
        let Some(goal) = self.cam_goal else { return };
        let k = 0.28;
        self.target += (goal.target - self.target) * k;
        self.distance = (self.distance.ln() + (goal.distance.ln() - self.distance.ln()) * k).exp();
        let turn = |from: f32, to: f32| {
            let mut d = (to - from) % std::f32::consts::TAU;
            if d > std::f32::consts::PI {
                d -= std::f32::consts::TAU;
            }
            if d < -std::f32::consts::PI {
                d += std::f32::consts::TAU;
            }
            from + d * k
        };
        self.yaw = turn(self.yaw, goal.yaw);
        self.pitch += (goal.pitch - self.pitch) * k;
        let close = (goal.target - self.target).length() < 0.3 && (goal.distance - self.distance).abs() < 0.3 && (goal.pitch - self.pitch).abs() < 0.003;
        if close {
            self.target = goal.target;
            self.distance = goal.distance;
            self.pitch = goal.pitch;
            self.cam_goal = None;
        } else {
            ctx.request_repaint();
        }
    }

    /// The point of the map under a pixel: the ground of the block, or (outside of it) a plane at the height of the target.
    fn point_under(&self, rect: egui::Rect, pos: egui::Pos2) -> Option<Vec3> {
        let (origin, direction) = self.ray(rect, pos);
        if let Some(hit) = self.pick(origin, direction, true) {
            return Some(hit.point);
        }
        if direction.y.abs() > 1e-4 {
            let t = (self.target.y - origin.y) / direction.y;
            if t > 0.0 {
                return Some(origin + direction * t);
            }
        }
        None
    }

    /// Reads the mouse and moves the camera.
    pub(super) fn camera_input(&mut self, ctx: &egui::Context, rect: egui::Rect, response: &egui::Response) {
        let (shift, alt) = ctx.input(|i| (i.modifiers.shift, i.modifiers.alt));
        let orbit_button = if self.tile_mode { egui::PointerButton::Secondary } else { egui::PointerButton::Primary };

        // Turn around the map.
        let turning = (response.dragged_by(orbit_button) && !shift) || (alt && response.dragged_by(egui::PointerButton::Primary));
        if turning {
            let d = response.drag_delta();
            self.yaw -= d.x * 0.006;
            self.pitch = (self.pitch + d.y * 0.006).clamp(0.04, 1.52);
            self.cam_goal = None;
        }
        // Slide over the ground: the point that is grabbed follows the pointer.
        let sliding = response.dragged_by(egui::PointerButton::Middle)
            || (!self.tile_mode && response.dragged_by(egui::PointerButton::Secondary))
            || (shift && response.dragged_by(egui::PointerButton::Primary));
        if sliding {
            let d = response.drag_delta();
            let per_pixel = self.units_per_pixel(rect);
            self.pan_ground(-d.x * per_pixel, d.y * per_pixel / self.pitch.sin().max(0.2));
        }
        // Zoom toward the pointer.
        if response.hovered() {
            let scroll = ctx.input(|i| i.smooth_scroll_delta.y);
            if scroll.abs() > 0.01 {
                let anchor = response.hover_pos().and_then(|p| self.point_under(rect, p));
                self.zoom_by((-scroll * 0.0018).exp(), anchor);
            }
        }
        self.step_goal(ctx);
    }

    /// The matrix of the camera, from the target, the distance and the angles.
    pub(super) fn view_matrix(&self, rect: egui::Rect) -> Mat4 {
        let eye_offset = Vec3::new(self.pitch.cos() * self.yaw.cos(), self.pitch.sin(), self.pitch.cos() * self.yaw.sin());
        let eye = self.target + eye_offset * self.distance;
        let aspect = (rect.width() / rect.height().max(1.0)).max(0.01);
        let near = (self.distance * 0.01).clamp(0.3, 4.0);
        Mat4::perspective_rh(FIELD_OF_VIEW.to_radians(), aspect, near, 60000.0) * Mat4::look_at_rh(eye, self.target, Vec3::Y)
    }

    /// The point of the map under the pointer, for the status bar.
    pub(super) fn update_hover(&mut self, rect: egui::Rect, response: &egui::Response) {
        self.hover = match response.hover_pos() {
            Some(pos) if rect.contains(pos) => self.point_under(rect, pos),
            _ => None,
        };
        self.hover_event = match response.hover_pos() {
            Some(pos) if rect.contains(pos) && self.overlay_on() => {
                let (origin, direction) = self.ray(rect, pos);
                self.event_label_under(origin, direction)
            }
            _ => None,
        };
    }

    /// Small buttons over the map to move the camera with the mouse only.
    pub(super) fn camera_overlay(&mut self, ctx: &egui::Context, rect: egui::Rect) {
        egui::Area::new(egui::Id::new("camera_overlay")).fixed_pos(rect.right_top() + egui::vec2(-232.0, 10.0)).order(egui::Order::Foreground).show(ctx, |ui| {
            egui::Frame::popup(ui.style()).show(ui, |ui| {
                ui.horizontal(|ui| {
                    ui.spacing_mut().item_spacing.x = 4.0;
                    if ui.small_button("Fit").on_hover_text("Fit the block (Home). Shift+Home: the whole world").clicked() {
                        if ctx.input(|i| i.modifiers.shift) { self.fit_world() } else { self.fit_block() }
                    }
                    if ui.small_button("World").on_hover_text("Fit the whole world (Shift+Home)").clicked() {
                        self.fit_world();
                    }
                    if ui.small_button("Top").on_hover_text("View from above (1)").clicked() {
                        self.go_to_view(None, None, Some(TOP_PITCH));
                    }
                    if ui.small_button("Iso").on_hover_text("Isometric view (2)").clicked() {
                        self.go_to_view(None, Some(ISO_YAW), Some(ISO_PITCH));
                    }
                    ui.separator();
                    if ui.small_button(" - ").on_hover_text("Zoom out (-)").clicked() {
                        self.zoom_by(1.3, None);
                    }
                    if ui.small_button(" + ").on_hover_text("Zoom in (+)").clicked() {
                        self.zoom_by(0.77, None);
                    }
                });
            });
        });
    }
}
