//! The window: a block in 3D that the user can change.
//!
//! Left click on an object selects it. The keys and the buttons move, copy
//! or delete it. The export writes the block in the folder of the SD card.

use std::collections::HashMap;

use eframe::egui;
use eframe::egui_wgpu::{self, wgpu};
use glam::{Mat4, Vec3, Vec4};
use wgpu::util::DeviceExt;

use crate::bch::{Mesh, Texture};
use crate::data::picture_of;
use crate::editor::{Editor, Instance};

/// The size of a tile of the game, in units.
const TILE: f32 = 18.0;

const SHADER: &str = r#"
struct Uniforms { matrix: mat4x4<f32>, params: vec4<f32> };
@group(0) @binding(0) var<uniform> u: Uniforms;
@group(1) @binding(0) var picture: texture_2d<f32>;
@group(1) @binding(1) var smp: sampler;

struct Out {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
    @location(1) colour: vec4<f32>,
    @location(2) world: vec3<f32>,
};

@vertex
fn vs(@location(0) position: vec3<f32>, @location(1) uv: vec2<f32>, @location(2) colour: vec4<f32>) -> Out {
    var out: Out;
    out.position = u.matrix * vec4<f32>(position, 1.0);
    out.uv = uv;
    out.colour = colour;
    out.world = position;
    return out;
}

@fragment
fn fs(in: Out) -> @location(0) vec4<f32> {
    let v = select(1.0 - in.uv.y, in.uv.y, u.params.x > 0.5);
    let texel = textureSample(picture, smp, vec2<f32>(in.uv.x, v));
    if (texel.a < 0.5) {
        discard;
    }
    let normal = normalize(cross(dpdx(in.world), dpdy(in.world)));
    let light = 0.6 + 0.4 * abs(dot(normal, normalize(vec3<f32>(0.4, 1.0, 0.3))));
    var rgb = texel.rgb * in.colour.rgb * light;
    if (u.params.y > 0.5) {
        rgb = mix(rgb, vec3<f32>(1.0, 0.85, 0.2), 0.55);
    }
    return vec4<f32>(rgb, 1.0);
}
"#;

#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
struct Vertex {
    position: [f32; 3],
    uv: [f32; 2],
    colour: [u8; 4],
}

struct GpuMesh {
    vertices: wgpu::Buffer,
    indices: wgpu::Buffer,
    index_count: u32,
    picture: wgpu::BindGroup,
}

/// The triangles of the selected object in one mesh.
struct Highlight {
    mesh: usize,
    indices: wgpu::Buffer,
    count: u32,
}

struct Gpu {
    pipeline: wgpu::RenderPipeline,
    overlay: wgpu::RenderPipeline,
    uniform: wgpu::Buffer,
    uniform_group: wgpu::BindGroup,
    highlight_uniform: wgpu::Buffer,
    highlight_group: wgpu::BindGroup,
    meshes: Vec<GpuMesh>,
    highlight: Vec<Highlight>,
}

fn buffers(device: &wgpu::Device, mesh: &Mesh) -> (wgpu::Buffer, wgpu::Buffer, u32) {
    let count = mesh.positions.len();
    let vertices: Vec<Vertex> = (0..count)
        .map(|i| Vertex {
            position: mesh.positions[i],
            uv: mesh.uvs.get(i).copied().unwrap_or([0.0, 0.0]),
            colour: mesh.colors.get(i).copied().unwrap_or([255; 4]),
        })
        .collect();
    let indices: Vec<u32> = mesh.indices.iter().copied().filter(|&i| (i as usize) < count).collect();
    let vertex_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: None,
        contents: bytemuck::cast_slice(&vertices),
        usage: wgpu::BufferUsages::VERTEX,
    });
    let index_buffer = index_buffer(device, &indices);
    (vertex_buffer, index_buffer, indices.len() as u32)
}

fn index_buffer(device: &wgpu::Device, indices: &[u32]) -> wgpu::Buffer {
    device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: None,
        contents: bytemuck::cast_slice(indices),
        usage: wgpu::BufferUsages::INDEX,
    })
}

impl Gpu {
    /// Puts the geometry of the meshes in the buffers. The pictures stay.
    fn set_geometry(&mut self, device: &wgpu::Device, meshes: &[(usize, Mesh)]) {
        for (gpu, (_, mesh)) in self.meshes.iter_mut().zip(meshes) {
            let (vertices, indices, count) = buffers(device, mesh);
            gpu.vertices = vertices;
            gpu.indices = indices;
            gpu.index_count = count;
        }
    }

    fn set_highlight(&mut self, device: &wgpu::Device, lists: Vec<(usize, Vec<u32>)>) {
        self.highlight = lists
            .into_iter()
            .filter(|(_, indices)| !indices.is_empty())
            .map(|(mesh, indices)| Highlight { mesh, count: indices.len() as u32, indices: index_buffer(device, &indices) })
            .collect();
    }
}

struct SceneCallback {
    matrix: Mat4,
    visible: Vec<bool>,
    direct_v: bool,
}

impl egui_wgpu::CallbackTrait for SceneCallback {
    fn prepare(
        &self,
        _device: &wgpu::Device,
        queue: &wgpu::Queue,
        _screen: &egui_wgpu::ScreenDescriptor,
        _encoder: &mut wgpu::CommandEncoder,
        resources: &mut egui_wgpu::CallbackResources,
    ) -> Vec<wgpu::CommandBuffer> {
        if let Some(gpu) = resources.get::<Gpu>() {
            let mut values = [0f32; 20];
            values[..16].copy_from_slice(&self.matrix.to_cols_array());
            values[16] = if self.direct_v { 1.0 } else { 0.0 };
            queue.write_buffer(&gpu.uniform, 0, bytemuck::cast_slice(&values));
            values[17] = 1.0;
            queue.write_buffer(&gpu.highlight_uniform, 0, bytemuck::cast_slice(&values));
        }
        Vec::new()
    }

    fn paint(
        &self,
        _info: egui::PaintCallbackInfo,
        pass: &mut wgpu::RenderPass<'static>,
        resources: &egui_wgpu::CallbackResources,
    ) {
        let Some(gpu) = resources.get::<Gpu>() else { return };
        let shown = |i: usize| self.visible.get(i).copied().unwrap_or(true);
        pass.set_pipeline(&gpu.pipeline);
        pass.set_bind_group(0, &gpu.uniform_group, &[]);
        for (i, mesh) in gpu.meshes.iter().enumerate() {
            if !shown(i) || mesh.index_count == 0 {
                continue;
            }
            pass.set_bind_group(1, &mesh.picture, &[]);
            pass.set_vertex_buffer(0, mesh.vertices.slice(..));
            pass.set_index_buffer(mesh.indices.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..mesh.index_count, 0, 0..1);
        }
        if gpu.highlight.is_empty() {
            return;
        }
        pass.set_pipeline(&gpu.overlay);
        pass.set_bind_group(0, &gpu.highlight_group, &[]);
        for h in &gpu.highlight {
            let Some(mesh) = gpu.meshes.get(h.mesh) else { continue };
            if !shown(h.mesh) {
                continue;
            }
            pass.set_bind_group(1, &mesh.picture, &[]);
            pass.set_vertex_buffer(0, mesh.vertices.slice(..));
            pass.set_index_buffer(h.indices.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..h.count, 0, 0..1);
        }
    }
}

/// A colour from a name, for the meshes that have no picture.
fn name_colour(name: &str) -> [u8; 4] {
    let mut h: u32 = 2166136261;
    for b in name.bytes() {
        h = (h ^ b as u32).wrapping_mul(16777619);
    }
    [96 + (h & 0x7f) as u8, 96 + ((h >> 8) & 0x7f) as u8, 96 + ((h >> 16) & 0x7f) as u8, 255]
}

fn build_gpu(state: &egui_wgpu::RenderState, meshes: &[(usize, Mesh)], textures: &HashMap<String, Texture>) -> Gpu {
    let device = &state.device;
    let queue = &state.queue;
    let module = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("map studio"),
        source: wgpu::ShaderSource::Wgsl(SHADER.into()),
    });
    let uniform_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: None,
        entries: &[wgpu::BindGroupLayoutEntry {
            binding: 0,
            visibility: wgpu::ShaderStages::VERTEX | wgpu::ShaderStages::FRAGMENT,
            ty: wgpu::BindingType::Buffer { ty: wgpu::BufferBindingType::Uniform, has_dynamic_offset: false, min_binding_size: None },
            count: None,
        }],
    });
    let picture_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: None,
        entries: &[
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Texture {
                    sample_type: wgpu::TextureSampleType::Float { filterable: true },
                    view_dimension: wgpu::TextureViewDimension::D2,
                    multisampled: false,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                count: None,
            },
        ],
    });
    let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: None,
        bind_group_layouts: &[&uniform_layout, &picture_layout],
        push_constant_ranges: &[],
    });
    let attributes = wgpu::vertex_attr_array![0 => Float32x3, 1 => Float32x2, 2 => Unorm8x4];
    let make_pipeline = |compare: wgpu::CompareFunction| {
        device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("map studio"),
            layout: Some(&layout),
            vertex: wgpu::VertexState {
                module: &module,
                entry_point: "vs",
                compilation_options: Default::default(),
                buffers: &[wgpu::VertexBufferLayout {
                    array_stride: std::mem::size_of::<Vertex>() as u64,
                    step_mode: wgpu::VertexStepMode::Vertex,
                    attributes: &attributes,
                }],
            },
            fragment: Some(wgpu::FragmentState {
                module: &module,
                entry_point: "fs",
                compilation_options: Default::default(),
                targets: &[Some(state.target_format.into())],
            }),
            primitive: wgpu::PrimitiveState::default(),
            depth_stencil: Some(wgpu::DepthStencilState {
                format: wgpu::TextureFormat::Depth32Float,
                depth_write_enabled: true,
                depth_compare: compare,
                stencil: Default::default(),
                bias: Default::default(),
            }),
            multisample: wgpu::MultisampleState::default(),
            multiview: None,
            cache: None,
        })
    };
    let pipeline = make_pipeline(wgpu::CompareFunction::Less);
    // The selected object is drawn again on the same triangles.
    let overlay = make_pipeline(wgpu::CompareFunction::LessEqual);
    let make_uniform = || {
        let buffer = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("matrix"),
            size: 80,
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });
        let group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &uniform_layout,
            entries: &[wgpu::BindGroupEntry { binding: 0, resource: buffer.as_entire_binding() }],
        });
        (buffer, group)
    };
    let (uniform, uniform_group) = make_uniform();
    let (highlight_uniform, highlight_group) = make_uniform();
    let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
        address_mode_u: wgpu::AddressMode::Repeat,
        address_mode_v: wgpu::AddressMode::Repeat,
        mag_filter: wgpu::FilterMode::Linear,
        min_filter: wgpu::FilterMode::Linear,
        ..Default::default()
    });

    let mut gpu_meshes = Vec::new();
    for (_, mesh) in meshes {
        let (vertices, indices, index_count) = buffers(device, mesh);
        let (width, height, pixels) = match textures.get(picture_of(mesh)) {
            Some(t) => (t.width as u32, t.height as u32, t.rgba.clone()),
            None => (1, 1, name_colour(&mesh.material).to_vec()),
        };
        let picture = device.create_texture_with_data(
            queue,
            &wgpu::TextureDescriptor {
                label: Some(&mesh.material),
                size: wgpu::Extent3d { width, height, depth_or_array_layers: 1 },
                mip_level_count: 1,
                sample_count: 1,
                dimension: wgpu::TextureDimension::D2,
                format: wgpu::TextureFormat::Rgba8Unorm,
                usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
                view_formats: &[],
            },
            wgpu::util::TextureDataOrder::LayerMajor,
            &pixels,
        );
        let view = picture.create_view(&Default::default());
        let group = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: None,
            layout: &picture_layout,
            entries: &[
                wgpu::BindGroupEntry { binding: 0, resource: wgpu::BindingResource::TextureView(&view) },
                wgpu::BindGroupEntry { binding: 1, resource: wgpu::BindingResource::Sampler(&sampler) },
            ],
        });
        gpu_meshes.push(GpuMesh { vertices, indices, index_count, picture: group });
    }
    Gpu {
        pipeline,
        overlay,
        uniform,
        uniform_group,
        highlight_uniform,
        highlight_group,
        meshes: gpu_meshes,
        highlight: Vec::new(),
    }
}

/// The distance along a ray to a triangle, if the ray hits it.
fn ray_triangle(origin: Vec3, direction: Vec3, a: Vec3, b: Vec3, c: Vec3) -> Option<f32> {
    let e1 = b - a;
    let e2 = c - a;
    let p = direction.cross(e2);
    let det = e1.dot(p);
    if det.abs() < 1e-9 {
        return None;
    }
    let inv = 1.0 / det;
    let from = origin - a;
    let u = from.dot(p) * inv;
    if !(0.0..=1.0).contains(&u) {
        return None;
    }
    let q = from.cross(e1);
    let v = direction.dot(q) * inv;
    if v < 0.0 || u + v > 1.0 {
        return None;
    }
    let t = e2.dot(q) * inv;
    (t > 0.0).then_some(t)
}

/// What a click hits.
struct Hit {
    point: Vec3,
    owner: Option<usize>,
}

struct Row {
    name: String,
    has_picture: bool,
    selectable: bool,
}

enum Action {
    Duplicate,
    Delete,
    Undo,
    Redo,
    Nudge([f32; 3]),
    Export,
}

pub struct Viewer {
    editor: Editor,
    rows: Vec<Row>,
    visible: Vec<bool>,
    title: String,
    yaw: f32,
    pitch: f32,
    distance: f32,
    target: Vec3,
    direct_v: bool,
    selected: Option<usize>,
    placing: bool,
    snap: bool,
    status: String,
    export_dir: String,
    matrix: Mat4,
}

impl Viewer {
    fn new(
        cc: &eframe::CreationContext<'_>,
        textures: HashMap<String, Texture>,
        editor: Editor,
        title: String,
    ) -> Result<Self, String> {
        let state = cc.wgpu_render_state.as_ref().ok_or("the graphics card is not available")?;
        let gpu = build_gpu(state, editor.meshes(), &textures);
        state.renderer.write().callback_resources.insert(gpu);
        let rows: Vec<Row> = editor
            .meshes()
            .iter()
            .enumerate()
            .map(|(i, (_, m))| Row {
                name: m.material.clone(),
                has_picture: textures.contains_key(picture_of(m)),
                selectable: editor.selectable[i],
            })
            .collect();
        let (mut low, mut high) = (Vec3::splat(f32::MAX), Vec3::splat(f32::MIN));
        for (_, mesh) in editor.meshes() {
            for p in &mesh.positions {
                low = low.min(Vec3::from(*p));
                high = high.max(Vec3::from(*p));
            }
        }
        let target = if low.x <= high.x { (low + high) * 0.5 } else { Vec3::ZERO };
        let default_dir = std::env::var("APPDATA")
            .map(|a| format!("{a}\\Azahar\\sdmc\\sango"))
            .ok()
            .filter(|d| std::path::Path::new(d).is_dir())
            .unwrap_or_else(|| "out".to_string());
        Ok(Viewer {
            visible: vec![true; rows.len()],
            rows,
            editor,
            title,
            yaw: 0.8,
            pitch: 0.7,
            distance: 900.0,
            target,
            direct_v: true,
            selected: None,
            placing: false,
            snap: true,
            status: "Click an object to select it.".into(),
            export_dir: default_dir,
            matrix: Mat4::IDENTITY,
        })
    }

    /// Sends the geometry and the selection to the graphics card.
    fn refresh(&self, frame: &eframe::Frame) {
        let Some(state) = frame.wgpu_render_state() else { return };
        let mut renderer = state.renderer.write();
        let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
        gpu.set_geometry(&state.device, self.editor.meshes());
        let lists = self
            .editor
            .meshes()
            .iter()
            .enumerate()
            .map(|(i, (_, mesh))| {
                let mut indices = Vec::new();
                if let Some(selected) = self.selected {
                    for t in mesh.indices.chunks_exact(3) {
                        if self.editor.owner(i, t[0] as usize) == Some(selected) {
                            indices.extend_from_slice(t);
                        }
                    }
                }
                (i, indices)
            })
            .collect();
        gpu.set_highlight(&state.device, lists);
    }

    /// The nearest triangle that a ray hits. With `ground`, the objects are
    /// ignored.
    fn pick(&self, origin: Vec3, direction: Vec3, ground: bool) -> Option<Hit> {
        let mut best: Option<(f32, Hit)> = None;
        for (i, (_, mesh)) in self.editor.meshes().iter().enumerate() {
            if !self.visible[i] {
                continue;
            }
            for t in mesh.indices.chunks_exact(3) {
                let owner = self.editor.owner(i, t[0] as usize);
                if ground && owner.is_some() {
                    continue;
                }
                let (Some(a), Some(b), Some(c)) =
                    (mesh.positions.get(t[0] as usize), mesh.positions.get(t[1] as usize), mesh.positions.get(t[2] as usize))
                else {
                    continue;
                };
                let hit = ray_triangle(origin, direction, Vec3::from(*a), Vec3::from(*b), Vec3::from(*c));
                if let Some(distance) = hit {
                    if best.as_ref().map_or(true, |(d, _)| distance < *d) {
                        best = Some((distance, Hit { point: origin + direction * distance, owner }));
                    }
                }
            }
        }
        best.map(|(_, hit)| hit)
    }

    fn ray(&self, rect: egui::Rect, pos: egui::Pos2) -> (Vec3, Vec3) {
        let x = (pos.x - rect.min.x) / rect.width() * 2.0 - 1.0;
        let y = 1.0 - (pos.y - rect.min.y) / rect.height() * 2.0;
        let inverse = self.matrix.inverse();
        let near = inverse * Vec4::new(x, y, 0.0, 1.0);
        let far = inverse * Vec4::new(x, y, 1.0, 1.0);
        let (near, far) = (near.truncate() / near.w, far.truncate() / far.w);
        (near, (far - near).normalize())
    }

    fn report<T>(&mut self, result: Result<T, String>, done: &str) {
        self.status = match result {
            Ok(_) => done.to_string(),
            Err(message) => format!("Cannot do it: {message}"),
        };
    }

    fn describe_selection(&self) -> String {
        let Some(selected) = self.selected else { return "Nothing selected.".into() };
        let instance = &self.editor.instances[selected];
        let cluster = &self.editor.clusters[instance.cluster];
        let mut names: Vec<&str> = cluster.parts.iter().map(|(m, _)| self.editor.material(*m)).collect();
        names.dedup();
        let (low, high) = self.editor.instance_bounds(selected);
        format!(
            "{}: {}\nat x {:.0} z {:.0}, height {:.0}..{:.0}",
            if instance.copy { "Copy" } else { "Object" },
            names.join(", "),
            (low[0] + high[0]) / 2.0,
            (low[2] + high[2]) / 2.0,
            low[1],
            high[1]
        )
    }

    fn apply(&mut self, action: Action, frame: &eframe::Frame) {
        match action {
            Action::Undo => {
                let r = self.editor.undo();
                self.report(r, "Undone.");
                self.selected = None;
            }
            Action::Redo => {
                let r = self.editor.redo();
                self.report(r, "Redone.");
                self.selected = None;
            }
            Action::Export => {
                let result = self.editor.export(&self.export_dir.clone());
                self.status = match result {
                    Ok(message) => message,
                    Err(message) => format!("Export failed: {message}"),
                };
                return;
            }
            Action::Duplicate => {
                let Some(selected) = self.selected else { return };
                let source = self.editor.instances[selected].clone();
                let result = self.editor.change(|v| {
                    v.push(Instance {
                        cluster: source.cluster,
                        offset: [source.offset[0] + TILE, source.offset[1], source.offset[2]],
                        copy: true,
                        deleted: false,
                    });
                });
                if result.is_ok() {
                    self.selected = Some(self.editor.instances.len() - 1);
                }
                self.report(result, "Copied one tile to the side.");
            }
            Action::Delete => {
                let Some(selected) = self.selected else { return };
                let result = self.editor.change(|v| v[selected].deleted = true);
                self.selected = None;
                self.report(result, "Deleted.");
            }
            Action::Nudge(delta) => {
                let Some(selected) = self.selected else { return };
                let result = self.editor.change(|v| {
                    for a in 0..3 {
                        v[selected].offset[a] += delta[a];
                    }
                });
                self.report(result, "Moved.");
            }
        }
        self.refresh(frame);
    }

    /// Puts the selected object where the user clicked on the ground.
    fn place(&mut self, point: Vec3, frame: &eframe::Frame) {
        let Some(selected) = self.selected else { return };
        let (low, high) = self.editor.instance_bounds(selected);
        let snap = |v: f32| if self.snap { ((v - TILE / 2.0) / TILE).round() * TILE + TILE / 2.0 } else { v };
        let (x, z) = (snap(point.x), snap(point.z));
        let delta = [x - (low[0] + high[0]) / 2.0, point.y - low[1], z - (low[2] + high[2]) / 2.0];
        let result = self.editor.change(|v| {
            for a in 0..3 {
                v[selected].offset[a] += delta[a];
            }
        });
        self.placing = false;
        self.report(result, "Placed.");
        self.refresh(frame);
    }
}

impl eframe::App for Viewer {
    fn update(&mut self, ctx: &egui::Context, frame: &mut eframe::Frame) {
        let mut action: Option<Action> = None;

        if !ctx.wants_keyboard_input() {
            ctx.input(|i| {
                let step = if i.modifiers.shift { 2.0 } else { TILE };
                if i.modifiers.command && i.key_pressed(egui::Key::Z) {
                    action = Some(if i.modifiers.shift { Action::Redo } else { Action::Undo });
                } else if i.modifiers.command && i.key_pressed(egui::Key::Y) {
                    action = Some(Action::Redo);
                } else if i.key_pressed(egui::Key::Delete) {
                    action = Some(Action::Delete);
                } else if i.key_pressed(egui::Key::D) {
                    action = Some(Action::Duplicate);
                } else if i.key_pressed(egui::Key::ArrowLeft) {
                    action = Some(Action::Nudge([-step, 0.0, 0.0]));
                } else if i.key_pressed(egui::Key::ArrowRight) {
                    action = Some(Action::Nudge([step, 0.0, 0.0]));
                } else if i.key_pressed(egui::Key::ArrowUp) {
                    action = Some(Action::Nudge([0.0, 0.0, -step]));
                } else if i.key_pressed(egui::Key::ArrowDown) {
                    action = Some(Action::Nudge([0.0, 0.0, step]));
                } else if i.key_pressed(egui::Key::PageUp) {
                    action = Some(Action::Nudge([0.0, 2.0, 0.0]));
                } else if i.key_pressed(egui::Key::PageDown) {
                    action = Some(Action::Nudge([0.0, -2.0, 0.0]));
                }
            });
            if ctx.input(|i| i.key_pressed(egui::Key::G)) && self.selected.is_some() {
                self.placing = !self.placing;
                self.status = if self.placing { "Click on the ground to place the object.".into() } else { "Cancelled.".into() };
            }
            if ctx.input(|i| i.key_pressed(egui::Key::Escape)) {
                self.placing = false;
                self.selected = None;
                self.refresh(frame);
            }
        }

        egui::SidePanel::left("panel").default_width(320.0).show(ctx, |ui| {
            ui.heading(&self.title);
            ui.separator();
            ui.horizontal(|ui| {
                if ui.add_enabled(self.editor.can_undo(), egui::Button::new("Undo")).clicked() {
                    action = Some(Action::Undo);
                }
                if ui.add_enabled(self.editor.can_redo(), egui::Button::new("Redo")).clicked() {
                    action = Some(Action::Redo);
                }
            });
            ui.label(self.describe_selection());
            let has_selection = self.selected.is_some();
            ui.horizontal(|ui| {
                if ui.add_enabled(has_selection, egui::Button::new("Copy (D)")).clicked() {
                    action = Some(Action::Duplicate);
                }
                if ui.add_enabled(has_selection, egui::Button::new("Place (G)")).clicked() {
                    self.placing = true;
                    self.status = "Click on the ground to place the object.".into();
                }
                if ui.add_enabled(has_selection, egui::Button::new("Delete")).clicked() {
                    action = Some(Action::Delete);
                }
            });
            ui.checkbox(&mut self.snap, "Snap to the tiles");
            ui.label("Arrows: move one tile (Shift: 2 units). Page up/down: height. Esc: select nothing.");
            ui.separator();
            ui.label("Folder of the SD card:");
            ui.text_edit_singleline(&mut self.export_dir);
            if ui.button("Export the block").clicked() {
                action = Some(Action::Export);
            }
            ui.add(egui::Label::new(egui::RichText::new(&self.status).weak()).wrap());
            ui.separator();
            ui.checkbox(&mut self.direct_v, "Direct V (untick to invert)");
            ui.collapsing("Meshes", |ui| {
                ui.horizontal(|ui| {
                    if ui.button("All").clicked() {
                        self.visible.iter_mut().for_each(|v| *v = true);
                    }
                    if ui.button("None").clicked() {
                        self.visible.iter_mut().for_each(|v| *v = false);
                    }
                });
                egui::ScrollArea::vertical().show(ui, |ui| {
                    for (i, row) in self.rows.iter().enumerate() {
                        let mark = if row.selectable { "" } else { "  (ground)" };
                        let text = format!("{}  ({}){mark}", row.name, self.editor.meshes()[i].1.indices.len() / 3);
                        let text = if row.has_picture { egui::RichText::new(text) } else { egui::RichText::new(text).weak() };
                        ui.checkbox(&mut self.visible[i], text);
                    }
                });
            });
        });

        let mut click: Option<(Vec3, Vec3)> = None;
        egui::CentralPanel::default().frame(egui::Frame::none()).show(ctx, |ui| {
            let (rect, response) = ui.allocate_exact_size(ui.available_size(), egui::Sense::click_and_drag());
            let up = Vec3::Y;
            let eye_offset = Vec3::new(
                self.pitch.cos() * self.yaw.cos(),
                self.pitch.sin(),
                self.pitch.cos() * self.yaw.sin(),
            );
            if response.dragged_by(egui::PointerButton::Primary) {
                let d = response.drag_delta();
                self.yaw -= d.x * 0.005;
                self.pitch = (self.pitch + d.y * 0.005).clamp(-1.5, 1.5);
            }
            if response.dragged_by(egui::PointerButton::Secondary) || response.dragged_by(egui::PointerButton::Middle) {
                let d = response.drag_delta();
                let right = up.cross(eye_offset).normalize_or_zero();
                let screen_up = eye_offset.cross(right).normalize_or_zero();
                let scale = self.distance * 0.0016;
                self.target += (right * d.x + screen_up * d.y) * scale;
            }
            if response.hovered() {
                let scroll = ctx.input(|i| i.smooth_scroll_delta.y);
                self.distance = (self.distance * (-scroll * 0.002).exp()).clamp(5.0, 8000.0);
            }
            let eye = self.target + eye_offset * self.distance;
            let aspect = (rect.width() / rect.height().max(1.0)).max(0.01);
            self.matrix = Mat4::perspective_rh(45f32.to_radians(), aspect, 1.0, 20000.0)
                * Mat4::look_at_rh(eye, self.target, up);
            if response.clicked_by(egui::PointerButton::Primary) {
                if let Some(pos) = response.interact_pointer_pos() {
                    click = Some(self.ray(rect, pos));
                }
            }
            ui.painter().add(egui_wgpu::Callback::new_paint_callback(
                rect,
                SceneCallback { matrix: self.matrix, visible: self.visible.clone(), direct_v: self.direct_v },
            ));
        });

        if let Some((origin, direction)) = click {
            if self.placing {
                if let Some(hit) = self.pick(origin, direction, true) {
                    self.place(hit.point, frame);
                }
            } else {
                let hit = self.pick(origin, direction, false);
                self.selected = hit.and_then(|h| h.owner);
                self.status = if self.selected.is_some() { "Selected.".into() } else { "Nothing here.".into() };
                self.refresh(frame);
            }
        }
        if let Some(action) = action {
            self.apply(action, frame);
        }
    }
}

/// Opens the window.
pub fn run(textures: HashMap<String, Texture>, editor: Editor, title: String) -> Result<(), String> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([1280.0, 800.0]).with_title("Map Studio"),
        renderer: eframe::Renderer::Wgpu,
        depth_buffer: 32,
        ..Default::default()
    };
    eframe::run_native(
        "Map Studio",
        options,
        Box::new(move |cc| {
            Viewer::new(cc, textures, editor, title).map(|v| Box::new(v) as Box<dyn eframe::App>).map_err(Into::into)
        }),
    )
    .map_err(|e| e.to_string())
}
