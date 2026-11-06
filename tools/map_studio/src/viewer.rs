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

/// The shader of the coloured squares that show the tiles.
const TILES_SHADER: &str = r#"
struct Uniforms { matrix: mat4x4<f32>, params: vec4<f32> };
@group(0) @binding(0) var<uniform> u: Uniforms;

struct Out {
    @builtin(position) position: vec4<f32>,
    @location(0) colour: vec4<f32>,
};

@vertex
fn vs(@location(0) position: vec3<f32>, @location(1) uv: vec2<f32>, @location(2) colour: vec4<f32>) -> Out {
    var out: Out;
    out.position = u.matrix * vec4<f32>(position, 1.0);
    out.colour = colour + vec4<f32>(uv.x * 0.0, 0.0, 0.0, 0.0);
    return out;
}

@fragment
fn fs(in: Out) -> @location(0) vec4<f32> {
    return in.colour;
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
    tiles_pipeline: wgpu::RenderPipeline,
    /// The squares of the tiles: the buffer and the number of vertices.
    tiles: Option<(wgpu::Buffer, u32)>,
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

    fn set_tiles(&mut self, device: &wgpu::Device, vertices: &[Vertex]) {
        self.tiles = if vertices.is_empty() {
            None
        } else {
            let buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: Some("tiles"),
                contents: bytemuck::cast_slice(vertices),
                usage: wgpu::BufferUsages::VERTEX,
            });
            Some((buffer, vertices.len() as u32))
        };
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
    show_tiles: bool,
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
        if self.show_tiles {
            if let Some((buffer, count)) = &gpu.tiles {
                pass.set_pipeline(&gpu.tiles_pipeline);
                pass.set_bind_group(0, &gpu.uniform_group, &[]);
                pass.set_vertex_buffer(0, buffer.slice(..));
                pass.draw(0..*count, 0..1);
            }
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
    let tiles_module = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("map studio tiles"),
        source: wgpu::ShaderSource::Wgsl(TILES_SHADER.into()),
    });
    let tiles_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: None,
        bind_group_layouts: &[&uniform_layout],
        push_constant_ranges: &[],
    });
    let tiles_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
        label: Some("map studio tiles"),
        layout: Some(&tiles_layout),
        vertex: wgpu::VertexState {
            module: &tiles_module,
            entry_point: "vs",
            compilation_options: Default::default(),
            buffers: &[wgpu::VertexBufferLayout {
                array_stride: std::mem::size_of::<Vertex>() as u64,
                step_mode: wgpu::VertexStepMode::Vertex,
                attributes: &attributes,
            }],
        },
        fragment: Some(wgpu::FragmentState {
            module: &tiles_module,
            entry_point: "fs",
            compilation_options: Default::default(),
            targets: &[Some(wgpu::ColorTargetState {
                format: state.target_format,
                blend: Some(wgpu::BlendState::ALPHA_BLENDING),
                write_mask: wgpu::ColorWrites::ALL,
            })],
        }),
        primitive: wgpu::PrimitiveState::default(),
        depth_stencil: Some(wgpu::DepthStencilState {
            format: wgpu::TextureFormat::Depth32Float,
            depth_write_enabled: false,
            depth_compare: wgpu::CompareFunction::LessEqual,
            stencil: Default::default(),
            bias: Default::default(),
        }),
        multisample: wgpu::MultisampleState::default(),
        multiview: None,
        cache: None,
    });
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
        tiles_pipeline,
        tiles: None,
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

/// The size of a tile, and the number of tiles of a side of a block.
const BLOCK_HALF: f32 = 360.0;
/// How high the squares of the tiles float above the ground.
const TILE_LIFT: f32 = 0.8;

/// The name and the colour of a tile, from its attribute. The number of the
/// ground is in the high byte. The numbers come from the game: wall 1, plain
/// 0, tall grass 31 to 37, water 148 to 151.
fn tile_style(value: u32) -> (String, [u8; 4]) {
    match value >> 24 {
        0 => ("plain".into(), [255, 255, 255, 14]),
        1 => ("wall".into(), [235, 50, 50, 120]),
        31..=37 => ("tall grass".into(), [50, 205, 70, 120]),
        148..=151 => ("water".into(), [50, 120, 245, 130]),
        id => (format!("ground {id}"), [205, 80, 225, 120]),
    }
}

/// The file that holds the object that the user copied. It is outside the
/// project, so it works from one block to another (two runs of the tool).
fn clipboard_path() -> std::path::PathBuf {
    std::env::temp_dir().join("map_studio_clipboard.msobj")
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
    CopyOut,
    Paste,
    Rotate(f32),
    SaveProject,
    LoadProject,
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
    arc_dir: String,
    /// The file of the project.
    project_path: String,
    /// True while the user changes the tiles and not the objects.
    tile_mode: bool,
    /// The attribute that the brush gives to the tiles.
    brush: u32,
    brush_size: usize,
    /// True: a click takes the attribute of a tile and does not paint.
    pipette: bool,
    /// True while the button of the brush is down.
    stroking: bool,
    /// The height of the ground at each corner of the tiles.
    heights: Option<Vec<f32>>,
    /// The attributes of all the blocks, with how many tiles have each.
    palette: Vec<(u32, usize)>,
}

impl Viewer {
    fn new(
        cc: &eframe::CreationContext<'_>,
        textures: HashMap<String, Texture>,
        editor: Editor,
        title: String,
        arc_dir: String,
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
        let block_id = editor.block_id();
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
            arc_dir,
            project_path: format!("block_{}.msproj", block_id),
            tile_mode: false,
            brush: 0x0100_0021,
            brush_size: 1,
            pipette: false,
            stroking: false,
            heights: None,
            palette: Vec::new(),
        })
    }

    /// Sends the geometry and the selection to the graphics card.
    fn refresh(&self, frame: &eframe::Frame) {
        let Some(state) = frame.wgpu_render_state() else { return };
        let mut renderer = state.renderer.write();
        let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
        gpu.set_geometry(&state.device, self.editor.meshes());
        gpu.set_tiles(&state.device, &self.tile_vertices());
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
        for (i, (part, mesh)) in self.editor.meshes().iter().enumerate() {
            if !self.visible[i] {
                continue;
            }
            let part = *part;
            for t in mesh.indices.chunks_exact(3) {
                let owner = self.editor.owner(i, t[0] as usize);
                if ground && (owner.is_some() || self.editor.selectable[i] || part != 1) {
                    break;
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

    /// The height of the ground at the corners of the tiles: a ray goes down
    /// on each corner.
    fn ground_heights(&self) -> Vec<f32> {
        let (width, height) = self.editor.grid();
        let side = BLOCK_HALF * 2.0 / width as f32;
        let mut heights = vec![0.0; (width + 1) * (height + 1)];
        for j in 0..=height {
            for i in 0..=width {
                let origin = Vec3::new(-BLOCK_HALF + side * i as f32, 1000.0, -BLOCK_HALF + side * j as f32);
                if let Some(hit) = self.pick(origin, Vec3::NEG_Y, true) {
                    heights[j * (width + 1) + i] = hit.point.y;
                }
            }
        }
        heights
    }

    /// The coloured squares that show the tiles on the ground.
    fn tile_vertices(&self) -> Vec<Vertex> {
        let Some(heights) = &self.heights else { return Vec::new() };
        if !self.tile_mode {
            return Vec::new();
        }
        let (width, height) = self.editor.grid();
        let side = BLOCK_HALF * 2.0 / width as f32;
        let mut out = Vec::new();
        for row in 0..height {
            for column in 0..width {
                let (_, colour) = tile_style(self.editor.tiles[row * width + column]);
                if colour[3] == 0 {
                    continue;
                }
                let corner = |i: usize, j: usize| Vertex {
                    position: [
                        -BLOCK_HALF + side * i as f32,
                        heights[j * (width + 1) + i] + TILE_LIFT,
                        -BLOCK_HALF + side * j as f32,
                    ],
                    uv: [0.0, 0.0],
                    colour,
                };
                let (a, b, c, d) = (corner(column, row), corner(column + 1, row), corner(column + 1, row + 1), corner(column, row + 1));
                out.extend_from_slice(&[a, b, c, a, c, d]);
            }
        }
        out
    }

    /// Paints the tile under a ray, or takes its attribute with the pipette.
    fn paint_tile(&mut self, origin: Vec3, direction: Vec3, frame: &eframe::Frame) {
        let Some(hit) = self.pick(origin, direction, true) else { return };
        let (width, height) = self.editor.grid();
        let side = BLOCK_HALF * 2.0 / width as f32;
        let (column, row) = (((hit.point.x + BLOCK_HALF) / side).floor(), ((hit.point.z + BLOCK_HALF) / side).floor());
        if column < 0.0 || row < 0.0 || column >= width as f32 || row >= height as f32 {
            return;
        }
        let (column, row) = (column as usize, row as usize);
        if self.pipette {
            self.brush = self.editor.tiles[row * width + column];
            self.pipette = false;
            self.status = format!("Brush: {}.", tile_style(self.brush).0);
            return;
        }
        if !self.stroking {
            self.editor.begin_stroke();
            self.stroking = true;
        }
        if self.editor.paint(column, row, self.brush_size, self.brush) {
            self.refresh(frame);
        }
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
        let names = self.editor.cluster_materials(instance.cluster);
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

    /// Called when the mode of the tiles starts or stops.
    fn toggle_tiles(&mut self, frame: &eframe::Frame) {
        if self.tile_mode {
            self.selected = None;
            if self.heights.is_none() {
                self.heights = Some(self.ground_heights());
            }
            self.status = "Tiles: paint with the left button, turn with the right button.".into();
        }
        self.refresh(frame);
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
            Action::SaveProject => {
                self.status = match self.editor.project_text().and_then(|text| std::fs::write(&self.project_path, text).map_err(|e| e.to_string())) {
                    Ok(()) => format!("Project saved in {}.", self.project_path),
                    Err(e) => format!("Cannot save the project: {e}"),
                };
                return;
            }
            Action::LoadProject => {
                let result = std::fs::read_to_string(&self.project_path)
                    .map_err(|e| format!("{}: {e}", self.project_path))
                    .and_then(|text| self.editor.load_project(&text));
                self.selected = None;
                self.report(result, "Project loaded.");
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
                        yaw: source.yaw,
                        copy: true,
                        deleted: false,
                    });
                });
                if result.is_ok() {
                    self.selected = Some(self.editor.instances.len() - 1);
                }
                self.report(result, "Copied one tile to the side.");
            }
            Action::CopyOut => {
                let Some(selected) = self.selected else {
                    self.status = "Select an object first.".into();
                    return;
                };
                let path = clipboard_path();
                self.status = match self.editor.portable_of(selected) {
                    Ok(object) => match std::fs::write(&path, object.to_bytes()) {
                        Ok(()) => format!("Object copied to the clipboard ({} meshes).", object.parts.len()),
                        Err(e) => format!("Cannot write the clipboard: {e}"),
                    },
                    Err(message) => format!("Cannot copy: {message}"),
                };
                return;
            }
            Action::Paste => {
                let path = clipboard_path();
                let result = std::fs::read(&path)
                    .map_err(|_| "the clipboard is empty: copy an object (Ctrl+C) in a block first".to_string())
                    .and_then(|bytes| crate::edit::Portable::from_bytes(&bytes))
                    .and_then(|object| self.editor.import(object));
                if let Ok(index) = &result {
                    self.selected = Some(*index);
                    self.placing = true;
                }
                self.report(result, "Pasted. Click on the ground to place it.");
            }
            Action::Rotate(angle) => {
                let Some(selected) = self.selected else { return };
                let result = self.editor.change(|v| {
                    let turned = v[selected].yaw + angle;
                    v[selected].yaw = (turned + std::f32::consts::PI).rem_euclid(std::f32::consts::TAU) - std::f32::consts::PI;
                });
                self.report(result, "Turned.");
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
        let mut tiles_toggled = false;

        if !ctx.wants_keyboard_input() {
            ctx.input(|i| {
                let step = if i.modifiers.shift { 2.0 } else { TILE };
                let copy_event = i.events.iter().any(|e| matches!(e, egui::Event::Copy));
                let paste_event = i.events.iter().any(|e| matches!(e, egui::Event::Paste(_)));
                if copy_event || (i.modifiers.command && i.key_pressed(egui::Key::C)) {
                    action = Some(Action::CopyOut);
                } else if paste_event || (i.modifiers.command && i.key_pressed(egui::Key::V)) {
                    action = Some(Action::Paste);
                } else if i.modifiers.command && i.key_pressed(egui::Key::Z) {
                    action = Some(if i.modifiers.shift { Action::Redo } else { Action::Undo });
                } else if i.modifiers.command && i.key_pressed(egui::Key::Y) {
                    action = Some(Action::Redo);
                } else if i.key_pressed(egui::Key::Delete) {
                    action = Some(Action::Delete);
                } else if i.key_pressed(egui::Key::D) {
                    action = Some(Action::Duplicate);
                } else if i.key_pressed(egui::Key::R) {
                    let quarter = std::f32::consts::FRAC_PI_2;
                    action = Some(Action::Rotate(if i.modifiers.shift { -quarter } else { quarter }));
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
            if ctx.input(|i| i.key_pressed(egui::Key::T)) {
                self.tile_mode = !self.tile_mode;
                self.placing = false;
                self.toggle_tiles(frame);
            }
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
            ui.horizontal(|ui| {
                if ui.add_enabled(has_selection, egui::Button::new("Copy out (Ctrl+C)")).clicked() {
                    action = Some(Action::CopyOut);
                }
                if ui.button("Paste (Ctrl+V)").clicked() {
                    action = Some(Action::Paste);
                }
            });
            ui.horizontal(|ui| {
                let quarter = std::f32::consts::FRAC_PI_2;
                if ui.add_enabled(has_selection, egui::Button::new("Turn left (Shift+R)")).clicked() {
                    action = Some(Action::Rotate(-quarter));
                }
                if ui.add_enabled(has_selection, egui::Button::new("Turn right (R)")).clicked() {
                    action = Some(Action::Rotate(quarter));
                }
            });
            ui.checkbox(&mut self.snap, "Snap to the tiles");
            ui.label("Arrows: move one tile (Shift: 2 units). Page up/down: height. Esc: select nothing.");
            ui.separator();
            let was_tile_mode = self.tile_mode;
            ui.checkbox(&mut self.tile_mode, "Edit the tiles (T)");
            if self.tile_mode != was_tile_mode {
                self.placing = false;
                tiles_toggled = true;
            }
            if self.tile_mode {
                ui.horizontal(|ui| {
                    ui.label("Brush:");
                    ui.add(egui::DragValue::new(&mut self.brush_size).range(1..=6).suffix(" tiles"));
                    if ui.selectable_label(self.pipette, "Pipette").clicked() {
                        self.pipette = !self.pipette;
                    }
                });
                ui.label(format!("Giving: {} ({:08x})", tile_style(self.brush).0, self.brush));
                ui.label("Values in this block (click to take):");
                let mut counts: std::collections::BTreeMap<u32, usize> = Default::default();
                for &value in &self.editor.tiles {
                    *counts.entry(value).or_default() += 1;
                }
                let mut list: Vec<(u32, usize)> = counts.into_iter().collect();
                for &(value, count) in &self.palette {
                    if !list.iter().any(|(v, _)| *v == value) {
                        list.push((value, count));
                    }
                }
                list.sort_by_key(|(_, n)| std::cmp::Reverse(*n));
                egui::ScrollArea::vertical().max_height(180.0).id_salt("tile_values").show(ui, |ui| {
                    for (value, count) in list {
                        let (name, colour) = tile_style(value);
                        let swatch = egui::Color32::from_rgb(colour[0], colour[1], colour[2]);
                        let text = egui::RichText::new(format!("{value:08x}  {name}  ({count})")).color(swatch);
                        if ui.selectable_label(self.brush == value, text).clicked() {
                            self.brush = value;
                        }
                    }
                });
                if ui.button("Add the values of all the blocks").clicked() {
                    match crate::data::attribute_values(&self.arc_dir) {
                        Ok(values) => {
                            self.palette = values.into_iter().take(60).collect();
                            self.status = format!("{} values from all the blocks.", self.palette.len());
                        }
                        Err(message) => self.status = format!("Cannot read the blocks: {message}"),
                    }
                }
            }
            ui.separator();
            ui.label("File of the project:");
            ui.text_edit_singleline(&mut self.project_path);
            ui.horizontal(|ui| {
                if ui.button("Save the project").clicked() {
                    action = Some(Action::SaveProject);
                }
                if ui.button("Load the project").clicked() {
                    action = Some(Action::LoadProject);
                }
            });
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
        let mut paint: Option<(Vec3, Vec3)> = None;
        egui::CentralPanel::default().frame(egui::Frame::none()).show(ctx, |ui| {
            let (rect, response) = ui.allocate_exact_size(ui.available_size(), egui::Sense::click_and_drag());
            let up = Vec3::Y;
            let eye_offset = Vec3::new(
                self.pitch.cos() * self.yaw.cos(),
                self.pitch.sin(),
                self.pitch.cos() * self.yaw.sin(),
            );
            let orbit_button = if self.tile_mode { egui::PointerButton::Secondary } else { egui::PointerButton::Primary };
            if response.dragged_by(orbit_button) {
                let d = response.drag_delta();
                self.yaw -= d.x * 0.005;
                self.pitch = (self.pitch + d.y * 0.005).clamp(-1.5, 1.5);
            }
            if response.dragged_by(egui::PointerButton::Middle)
                || (!self.tile_mode && response.dragged_by(egui::PointerButton::Secondary))
            {
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
            let primary_down = ctx.input(|i| i.pointer.primary_down());
            if self.tile_mode {
                if primary_down && response.hovered() {
                    if let Some(pos) = response.hover_pos() {
                        paint = Some(self.ray(rect, pos));
                    }
                }
            } else if response.clicked_by(egui::PointerButton::Primary) {
                if let Some(pos) = response.interact_pointer_pos() {
                    click = Some(self.ray(rect, pos));
                }
            }
            ui.painter().add(egui_wgpu::Callback::new_paint_callback(
                rect,
                SceneCallback {
                    matrix: self.matrix,
                    visible: self.visible.clone(),
                    direct_v: self.direct_v,
                    show_tiles: self.tile_mode,
                },
            ));
        });

        if tiles_toggled {
            self.toggle_tiles(frame);
        }
        if let Some((origin, direction)) = paint {
            self.paint_tile(origin, direction, frame);
        }
        if self.stroking && !ctx.input(|i| i.pointer.primary_down()) {
            self.editor.end_stroke();
            self.stroking = false;
        }
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
pub fn run(textures: HashMap<String, Texture>, editor: Editor, title: String, arc_dir: String) -> Result<(), String> {
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
            Viewer::new(cc, textures, editor, title, arc_dir)
                .map(|v| Box::new(v) as Box<dyn eframe::App>)
                .map_err(Into::into)
        }),
    )
    .map_err(|e| e.to_string())
}
