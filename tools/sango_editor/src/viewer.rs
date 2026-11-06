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
use crate::events::MapEvents;
use crate::plugin_dir;

/// The size of a tile of the game, in units.
const TILE: f32 = 18.0;
/// "Grow" adds the pieces whose box is closer than this to the selection.
const GROW_GAP: f32 = 6.0;

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
pub(crate) struct Vertex {
    position: [f32; 3],
    uv: [f32; 2],
    colour: [u8; 4],
}

mod chrome;
mod events_view;

use chrome::{CamGoal, Commands, Confirm, Tool};

/// A mesh of the 3D model of a character, already put in its place.
pub struct PlacedMesh {
    /// What names the picture on the graphics card.
    pub key: String,
    pub vertices: Vec<Vertex>,
    pub indices: Vec<u32>,
    pub model: std::rc::Rc<crate::models::LoadedModel>,
    pub picture: String,
}

struct GpuPlaced {
    vertices: wgpu::Buffer,
    indices: wgpu::Buffer,
    index_count: u32,
    key: String,
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
    picture_layout: wgpu::BindGroupLayout,
    sampler: wgpu::Sampler,
    /// The squares and poles of the events: the buffer and the number of vertices.
    events: Option<(wgpu::Buffer, u32)>,
    /// The 3D models of the characters, and their pictures (kept by name).
    placed: Vec<GpuPlaced>,
    placed_pictures: HashMap<String, wgpu::BindGroup>,
    /// The blocks around the block (read only), already put in their place.
    neighbours: Vec<GpuMesh>,
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

    /// Puts the models of the characters in the buffers. A picture is sent to
    /// the graphics card once.
    fn set_placed_models(&mut self, device: &wgpu::Device, queue: &wgpu::Queue, placed: &[PlacedMesh]) {
        let mut out = Vec::new();
        for item in placed {
            if item.vertices.is_empty() || item.indices.is_empty() {
                continue;
            }
            if !self.placed_pictures.contains_key(&item.key) {
                let (width, height, pixels) = match item.model.textures.get(&item.picture) {
                    Some(t) => (t.width as u32, t.height as u32, t.rgba.clone()),
                    None => (1, 1, name_colour(&item.picture).to_vec()),
                };
                let picture = device.create_texture_with_data(
                    queue,
                    &wgpu::TextureDescriptor {
                        label: Some(&item.key),
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
                    layout: &self.picture_layout,
                    entries: &[
                        wgpu::BindGroupEntry { binding: 0, resource: wgpu::BindingResource::TextureView(&view) },
                        wgpu::BindGroupEntry { binding: 1, resource: wgpu::BindingResource::Sampler(&self.sampler) },
                    ],
                });
                self.placed_pictures.insert(item.key.clone(), group);
            }
            let vertices = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&item.vertices),
                usage: wgpu::BufferUsages::VERTEX,
            });
            out.push(GpuPlaced { vertices, indices: index_buffer(device, &item.indices), index_count: item.indices.len() as u32, key: item.key.clone() });
        }
        self.placed = out;
    }

    fn set_events(&mut self, device: &wgpu::Device, vertices: &[Vertex]) {
        self.events = if vertices.is_empty() {
            None
        } else {
            let buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: Some("events"),
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
    show_events: bool,
    show_neighbours: bool,
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
        if self.show_neighbours {
            pass.set_pipeline(&gpu.pipeline);
            pass.set_bind_group(0, &gpu.uniform_group, &[]);
            for mesh in &gpu.neighbours {
                if mesh.index_count == 0 {
                    continue;
                }
                pass.set_bind_group(1, &mesh.picture, &[]);
                pass.set_vertex_buffer(0, mesh.vertices.slice(..));
                pass.set_index_buffer(mesh.indices.slice(..), wgpu::IndexFormat::Uint32);
                pass.draw_indexed(0..mesh.index_count, 0, 0..1);
            }
        }
        if self.show_events {
            pass.set_pipeline(&gpu.pipeline);
            pass.set_bind_group(0, &gpu.uniform_group, &[]);
            for item in &gpu.placed {
                let Some(picture) = gpu.placed_pictures.get(&item.key) else { continue };
                pass.set_bind_group(1, picture, &[]);
                pass.set_vertex_buffer(0, item.vertices.slice(..));
                pass.set_index_buffer(item.indices.slice(..), wgpu::IndexFormat::Uint32);
                pass.draw_indexed(0..item.index_count, 0, 0..1);
            }
            if let Some((buffer, count)) = &gpu.events {
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
        label: Some("sango editor"),
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
            label: Some("sango editor"),
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
        label: Some("sango editor tiles"),
        source: wgpu::ShaderSource::Wgsl(TILES_SHADER.into()),
    });
    let tiles_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: None,
        bind_group_layouts: &[&uniform_layout],
        push_constant_ranges: &[],
    });
    let tiles_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
        label: Some("sango editor tiles"),
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

    let gpu_meshes = make_gpu_meshes(device, queue, &picture_layout, &sampler, meshes, textures);
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
        picture_layout,
        sampler,
        events: None,
        placed: Vec::new(),
        placed_pictures: HashMap::new(),
        neighbours: Vec::new(),
    }
}

/// The buffers and the picture of each mesh.
fn make_gpu_meshes(
    device: &wgpu::Device,
    queue: &wgpu::Queue,
    picture_layout: &wgpu::BindGroupLayout,
    sampler: &wgpu::Sampler,
    meshes: &[(usize, Mesh)],
    textures: &HashMap<String, Texture>,
) -> Vec<GpuMesh> {
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
            layout: picture_layout,
            entries: &[
                wgpu::BindGroupEntry { binding: 0, resource: wgpu::BindingResource::TextureView(&view) },
                wgpu::BindGroupEntry { binding: 1, resource: wgpu::BindingResource::Sampler(sampler) },
            ],
        });
        gpu_meshes.push(GpuMesh { vertices, indices, index_count, picture: group });
    }
    gpu_meshes
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

/// The coloured squares of the tiles of a block, moved by `offset`.
fn tile_squares(tiles: &[u32], (width, height): (usize, usize), heights: &[f32], offset: [f32; 2], out: &mut Vec<Vertex>) {
    let side = BLOCK_HALF * 2.0 / width as f32;
    for row in 0..height {
        for column in 0..width {
            let (_, colour) = tile_style(tiles[row * width + column]);
            if colour[3] == 0 {
                continue;
            }
            let corner = |i: usize, j: usize| Vertex {
                position: [
                    offset[0] - BLOCK_HALF + side * i as f32,
                    heights[j * (width + 1) + i] + TILE_LIFT,
                    offset[1] - BLOCK_HALF + side * j as f32,
                ],
                uv: [0.0, 0.0],
                colour,
            };
            let (a, b, c, d) = (corner(column, row), corner(column + 1, row), corner(column + 1, row + 1), corner(column, row + 1));
            out.extend_from_slice(&[a, b, c, a, c, d]);
        }
    }
}

/// The height of the ground at the corners of the tiles, read from the
/// vertices of the ground (fast: no ray). A corner without vertex gets the
/// mean height.
fn corner_heights(editor: &Editor) -> Vec<f32> {
    let (width, height) = editor.grid();
    let side = BLOCK_HALF * 2.0 / width as f32;
    let mut out = vec![f32::NAN; (width + 1) * (height + 1)];
    for (index, (part, mesh)) in editor.meshes().iter().enumerate() {
        if *part != 1 || editor.is_selectable(index) {
            continue;
        }
        for p in &mesh.positions {
            let (fx, fz) = ((p[0] + BLOCK_HALF) / side, (p[2] + BLOCK_HALF) / side);
            let (ix, iz) = (fx.round(), fz.round());
            if (fx - ix).abs() < 0.03 && (fz - iz).abs() < 0.03 && ix >= 0.0 && iz >= 0.0 && ix <= width as f32 && iz <= height as f32 {
                out[iz as usize * (width + 1) + ix as usize] = p[1];
            }
        }
    }
    let known: Vec<f32> = out.iter().copied().filter(|v| !v.is_nan()).collect();
    let mean = if known.is_empty() { 0.0 } else { known.iter().sum::<f32>() / known.len() as f32 };
    out.iter_mut().for_each(|v| if v.is_nan() { *v = mean });
    out
}

/// The name and the colour of a tile, from its attribute. The kind of the
/// tile is in the high byte: 0 free to walk, 1 blocked (a zone where one
/// cannot walk, it is not always a wall), 31 to 37 tall grass, 104 to 127 a
/// ledge (a bump of earth that one jumps over), 144 to 159 water. The other
/// kinds are not named yet.
fn tile_style(value: u32) -> (String, [u8; 4]) {
    match value >> 24 {
        0 => ("free to walk".into(), [120, 120, 120, 70]),
        1 => ("blocked (cannot walk)".into(), [235, 50, 50, 120]),
        31..=37 => ("tall grass".into(), [50, 205, 70, 120]),
        104..=127 => ("ledge (jump)".into(), [245, 160, 40, 140]),
        144..=159 => ("water".into(), [50, 120, 245, 130]),
        id => (format!("other kind {id}"), [205, 80, 225, 120]),
    }
}

/// The file that holds the object that the user copied. It is outside the
/// project, so it works from one block to another (two runs of the tool).
fn clipboard_path() -> std::path::PathBuf {
    std::env::temp_dir().join("sango_editor_clipboard.msobj")
}

/// What a click hits.
struct Hit {
    point: Vec3,
    owner: Option<usize>,
    /// The mesh of the triangle.
    mesh: usize,
}

struct Row {
    name: String,
    has_picture: bool,
    selectable: bool,
}

enum Action {
    Grow,
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
    /// Export the block, then start the game in the emulator.
    Launch,
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
    /// The instances that are selected.
    selected: Vec<usize>,
    placing: bool,
    snap: bool,
    /// The events under objects move with the objects.
    events_follow: bool,
    /// The blocked tiles (the collision) under the objects move with them.
    walls_follow: bool,
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
    /// The pictures that the viewer can show (those of the tileset of the block,
    /// and those that dropped objects bring).
    textures: HashMap<String, Texture>,
    /// The names of the pictures that the tileset of the block has.
    tileset_pictures: std::collections::HashSet<String>,
    catalog_open: bool,
    catalog: CatalogPanel,
    /// True while the user works on the events (characters, warps...).
    events_mode: bool,
    /// Draw a mark on each object that can be moved (in the Select tool).
    show_objects: bool,
    /// What the panel showed at the last frame: an event (true) or an object (false).
    last_focus: bool,
    /// The name of the event under the pointer (shown in the status bar, not in the 3D view).
    hover_event: Option<String>,
    /// The object that is being placed comes from outside (a paste, the catalogue, another block):
    /// the events and the blocked tiles at the place where it was taken are not its own.
    placing_imported: bool,
    /// The folder where the plugin reads its text files, as a text for the panel.
    plugin_folder_text: String,
    /// The scripts of Sango (the file scripts.txt), the one that is shown, and whether they were changed.
    scripts: crate::scripts::DataScripts,
    script_sel: usize,
    scripts_dirty: bool,
    /// The events changed since they were written: they are written when the mouse is up.
    events_dirty: bool,
    /// Fill: only the tiles that are free to walk.
    fill_free_only: bool,
    /// The event that was copied (Ctrl+C with an event selected).
    event_clip: Option<events_view::EventClip>,
    /// What goes with the object that was copied out (its door...), and with the one that is being placed.
    clipboard_extra: Option<events_view::Furniture>,
    pending_extra: Option<events_view::Furniture>,
    /// The tiles and the objects that the user starred: the keys of the entries.
    favorites: std::collections::HashSet<String>,
    /// True while the tool of the ground tiles works.
    ground_mode: bool,
    /// The next click on the map copies the ground tile under it into the catalogue.
    take_tile: bool,
    /// In the question before clearing a block: free the blocked tiles too.
    clear_blocked: bool,
    /// The tileset to use for the next block that opens (the one of the world).
    tileset_hint: Option<usize>,
    /// What can be undone: the changes of the objects and of the events, in order.
    history: Vec<HistoryEntry>,
    future: Vec<HistoryEntry>,
    /// The events and the props as they were at the end of the last frame (with the block and the map).
    history_base: Option<((usize, usize), MapEvents, Vec<crate::edit::Prop>)>,
    history_depth: usize,
    ev: events_view::EventsState,
    /// All the worlds of the game, for the list of the tilesets.
    worlds: Vec<crate::events::WorldInfo>,
    world_filter: String,
    /// The block or the world that the user asked for: the viewer changes at the end of the frame.
    pending_block: Option<usize>,
    pending_world: Option<usize>,
    /// The point of the map under the pointer.
    hover: Option<Vec3>,
    /// Where the camera goes (it moves over a few frames).
    cam_goal: Option<CamGoal>,
    help_open: bool,
    confirm: Option<Confirm>,
    show_inspector: bool,
    models: crate::models::ModelBank,
    npcs: events_view::NpcPanel,
    /// The emulator and the game that the Launch button starts.
    emulator_path: String,
    game_path: String,
    /// The catalogue of the ground tiles.
    tile_catalog: CatalogPanel,
    /// True when the panel shows the tiles.
    tile_tab: bool,
    /// True when the panel shows the characters (NPCs).
    npc_tab: bool,
    /// The tile of the catalogue that a click puts on the ground.
    stamp: Option<usize>,
    /// The other blocks of the world that were opened: they keep their changes
    /// until the user writes them (the block that is open is `editor`).
    world: HashMap<usize, Editor>,
    /// The height of the ground at the corners of the tiles of the blocks around.
    neigh_heights: HashMap<usize, Vec<f32>>,
    /// The height tool: Some(+1) lifts the ground, Some(-1) lowers it.
    height_tool: Option<f32>,
    /// How much a stroke lifts or lowers the ground.
    height_step: f32,
    /// The ground changed in this stroke: the heights are measured again when the button is up.
    height_dirty: bool,
    /// The tall grass tool: Some(true) puts grass, Some(false) takes it away.
    grass_tool: Option<bool>,
    /// The piece of tall grass that the tool puts on one tile.
    grass: Option<crate::edit::Portable>,
    /// The tiles that the grass tool already did in this stroke.
    grass_done: std::collections::HashSet<(usize, usize)>,
}

/// One step of what can be undone: how many changes of the objects it holds,
/// and the events and the props before and after it (when it changed them).
struct HistoryEntry {
    editor_steps: usize,
    before: Option<(MapEvents, Vec<crate::edit::Prop>)>,
    after: Option<(MapEvents, Vec<crate::edit::Prop>)>,
}

/// The state of the panel of the catalogue.
#[derive(Default)]
struct CatalogPanel {
    entries: Vec<crate::catalog::Entry>,
    /// The thread that builds the catalogue: the number of the block that it
    /// reads, and the result when it ends.
    building: Option<std::sync::Arc<(std::sync::atomic::AtomicUsize, std::sync::Mutex<Option<Result<Vec<crate::catalog::Entry>, String>>>)>>,
    search: String,
    hide_ground: bool,
    thumbs: HashMap<usize, egui::TextureHandle>,
    tilesets: HashMap<usize, HashMap<String, Texture>>,
    message: String,
    /// Tiles: show only those that this block can take.
    only_fit: bool,
    /// For each entry: can this block take it? (None until computed.)
    fits: Option<Vec<bool>>,
    /// Tiles: show only those that can fill the whole block.
    fill_only: bool,
    fill_fits: Option<Vec<bool>>,
    /// Show only the starred entries.
    fav_only: bool,
    /// The entries that the user made (from the map): their places in `entries`.
    mine: Vec<usize>,
    mine_loaded: bool,
}

/// The folder where the tool keeps its files.
fn config_dir() -> std::path::PathBuf {
    let base = std::env::var("LOCALAPPDATA").map(std::path::PathBuf::from).unwrap_or_else(|_| std::env::temp_dir());
    base.join("sango_editor")
}

/// The key of an entry of the catalogue in the list of the stars.
fn fav_key(tiles: bool, entry: &crate::catalog::Entry) -> String {
    format!("{}|{}|{}|{}", if tiles { 't' } else { 'o' }, entry.name, entry.donor, entry.occurrences)
}

fn load_favorites() -> std::collections::HashSet<String> {
    std::fs::read_to_string(config_dir().join("favorites.txt")).map(|text| text.lines().map(String::from).collect()).unwrap_or_default()
}

fn save_favorites(set: &std::collections::HashSet<String>) {
    let mut lines: Vec<&String> = set.iter().collect();
    lines.sort();
    let _ = std::fs::create_dir_all(config_dir());
    let _ = std::fs::write(config_dir().join("favorites.txt"), lines.iter().map(|l| l.as_str()).collect::<Vec<_>>().join("\n"));
}

/// The file of the entries that the user made.
fn mine_path(tiles: bool) -> std::path::PathBuf {
    config_dir().join(if tiles { "mine_tiles.bin" } else { "mine_objects.bin" })
}

fn save_mine(tiles: bool, panel: &CatalogPanel) {
    let list: Vec<crate::catalog::Entry> = panel.mine.iter().filter_map(|&i| panel.entries.get(i).cloned()).collect();
    let _ = std::fs::create_dir_all(config_dir());
    let _ = crate::catalog::save(&list, &mine_path(tiles));
}

/// The first of the files that exists (an empty text when none does).
fn first_existing(paths: &[&str]) -> String {
    paths.iter().find(|p| std::path::Path::new(p).is_file()).map(|p| p.to_string()).unwrap_or_default()
}

/// The file where the catalogue is kept, outside the project: it holds data
/// of the game.
fn catalog_path(tiles: bool) -> std::path::PathBuf {
    let base = std::env::var("LOCALAPPDATA").map(std::path::PathBuf::from).unwrap_or_else(|_| std::env::temp_dir());
    base.join("sango_editor").join(if tiles { "tiles.bin" } else { "catalog.bin" })
}

/// Says whether a model is a piece of the ground (a border, a shadow, a cliff)
/// and not an object that one puts in a map.
fn is_ground_piece(name: &str) -> bool {
    ["edge", "shadow", "gake", "wave", "kanazumi", "grass_decolate"].iter().any(|word| name.contains(word))
}

impl Viewer {
    fn new(
        cc: &eframe::CreationContext<'_>,
        textures: HashMap<String, Texture>,
        editor: Editor,
        title: String,
        arc_dir: String,
    ) -> Result<Self, String> {
        chrome::apply_theme(&cc.egui_ctx);
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
                selectable: editor.is_selectable(i),
            })
            .collect();
        let block_id = editor.block_id();
        let arc_for_models = arc_dir.clone();
        // The scripts that the game uses, for the lists (found in the background).
        let arc_for_scripts = arc_dir.clone();
        std::thread::spawn(move || crate::scripts::find_game_scripts(&arc_for_scripts));
        let (mut low, mut high) = (Vec3::splat(f32::MAX), Vec3::splat(f32::MIN));
        for (_, mesh) in editor.meshes() {
            for p in &mesh.positions {
                low = low.min(Vec3::from(*p));
                high = high.max(Vec3::from(*p));
            }
        }
        let target = if low.x <= high.x { (low + high) * 0.5 } else { Vec3::ZERO };
        let early_dir = std::env::var("APPDATA").map(|a| format!("{a}/Azahar/sdmc/sango")).ok().filter(|d| std::path::Path::new(d).is_dir()).unwrap_or_else(|| "out".to_string());
        let plugin_folder = plugin_dir::detect(&early_dir);
        plugin_dir::migrate(std::path::Path::new(&early_dir), &plugin_folder);
        plugin_dir::set(&plugin_folder);
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
            selected: Vec::new(),
            placing: false,
            snap: true,
            events_follow: true,
            walls_follow: true,
            status: "Click an object to select it.".into(),
            export_dir: default_dir.clone(),
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
            tileset_pictures: textures.keys().cloned().collect(),
            textures,
            catalog_open: false,
            events_mode: false,
            show_objects: true,
            last_focus: false,
            tileset_hint: None,
            clear_blocked: true,
            favorites: load_favorites(),
            clipboard_extra: None,
            event_clip: None,
            events_dirty: false,
            plugin_folder_text: plugin_folder.to_string_lossy().to_string(),
            scripts: Default::default(),
            script_sel: 0,
            scripts_dirty: false,
            fill_free_only: true,
            placing_imported: false,
            pending_extra: None,
            ground_mode: false,
            take_tile: false,
            hover_event: None,
            history: Vec::new(),
            future: Vec::new(),
            history_base: None,
            history_depth: 0,
            ev: Default::default(),
            worlds: Vec::new(),
            world_filter: String::new(),
            pending_block: None,
            pending_world: None,
            hover: None,
            cam_goal: None,
            help_open: false,
            confirm: None,
            show_inspector: true,
            models: {
                let mut bank = crate::models::ModelBank::new(&arc_for_models);
                // The models of X and Y for ORAS are in `sango` (or in `sango_off`, when the folder is switched off).
                bank.set_overrides(vec![
                    std::path::PathBuf::from(&default_dir).join("overworld_model"),
                    std::path::PathBuf::from(format!("{default_dir}_off")).join("overworld_model"),
                ]);
                bank
            },
            npcs: Default::default(),
            emulator_path: first_existing(&["C:/Program Files/Azahar/azahar.exe"]),
            game_path: first_existing(&["C:/Users/David/Desktop/ctr/cia/sango.3ds"]),
            catalog: CatalogPanel { hide_ground: true, only_fit: true, ..Default::default() },
            tile_catalog: CatalogPanel { only_fit: true, ..Default::default() },
            tile_tab: false,
            npc_tab: false,
            stamp: None,
            world: HashMap::new(),
            neigh_heights: HashMap::new(),
            grass_tool: None,
            height_tool: None,
            height_step: 2.0,
            height_dirty: false,
            grass: None,
            grass_done: Default::default(),
        })
    }

    /// When an import added meshes (or an undo took them away), the list of
    /// the meshes and their buffers are made again.
    fn sync_meshes(&mut self, frame: &eframe::Frame) {
        let count = self.editor.meshes().len();
        if count == self.rows.len() {
            return;
        }
        let Some(state) = frame.wgpu_render_state() else { return };
        {
            let mut renderer = state.renderer.write();
            let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
            let made = make_gpu_meshes(&state.device, &state.queue, &gpu.picture_layout, &gpu.sampler, self.editor.meshes(), &self.textures);
            gpu.meshes = made;
        }
        self.rows = self
            .editor
            .meshes()
            .iter()
            .enumerate()
            .map(|(i, (_, m))| Row {
                name: m.material.clone(),
                has_picture: self.textures.contains_key(picture_of(m)),
                selectable: self.editor.is_selectable(i),
            })
            .collect();
        self.visible = vec![true; count];
    }

    /// The name of a world in the list: its first maps.
    fn world_label(world: &crate::events::WorldInfo) -> String {
        let names: Vec<String> = world
            .maps
            .iter()
            .filter_map(|&z| crate::names::MAP_NAMES.get(z))
            .filter(|n| !n.starts_with("Unknow"))
            .take(3)
            .map(|n| n.to_string())
            .collect();
        let more = if world.maps.len() > 3 { format!(" (+{})", world.maps.len() - 3) } else { String::new() };
        if names.is_empty() { format!("World {}", world.matrix) } else { format!("{}{more}", names.join(", ")) }
    }

    /// The list to choose the world to work on.
    fn world_ui(&mut self, ui: &mut egui::Ui) {
        if self.worlds.is_empty() {
            self.worlds = crate::events::list_worlds(&self.arc_dir).unwrap_or_default();
        }
        let current = self.ev.at().map(|z| z.matrix);
        let selected = current
            .and_then(|m| self.worlds.iter().find(|a| a.matrix == m))
            .map_or("(choose a world)".to_string(), |a| Self::world_label(a));
        let mut pick: Option<usize> = None;
        ui.horizontal(|ui| {
            ui.label("World:");
            // A list that opens under a button. A click in the search field must not close it:
            // only a click outside, or a choice, does.
            let button = ui.add(egui::Button::new(format!("{selected}   \u{25be}")).min_size(egui::vec2(250.0, 0.0)));
            let popup_id = ui.make_persistent_id("world_popup");
            if button.clicked() {
                ui.memory_mut(|m| m.toggle_popup(popup_id));
            }
            egui::popup_below_widget(ui, popup_id, &button, egui::PopupCloseBehavior::CloseOnClickOutside, |ui| {
                ui.set_min_width(250.0);
                ui.add(egui::TextEdit::singleline(&mut self.world_filter).hint_text("search a map"));
                let words: Vec<String> = self.world_filter.to_lowercase().split_whitespace().map(String::from).collect();
                egui::ScrollArea::vertical().max_height(360.0).show(ui, |ui| {
                    for a in &self.worlds {
                        let label = Self::world_label(a);
                        let all_names: String = a.maps.iter().filter_map(|&z| crate::names::MAP_NAMES.get(z)).map(|n| n.to_lowercase()).collect::<Vec<_>>().join(" ");
                        if !words.iter().all(|w| all_names.contains(w.as_str())) {
                            continue;
                        }
                        if ui.selectable_label(Some(a.matrix) == current, format!("{label}  [{}]", a.matrix)).clicked() {
                            pick = Some(a.matrix);
                            ui.memory_mut(|m| m.close_popup());
                        }
                    }
                });
            });
        });
        if pick.is_some() {
            self.pending_world = pick;
        }
    }

    /// The row that says how many blocks of the world are drawn.
    fn world_blocks_ui(&mut self, ui: &mut egui::Ui) {
        ui.horizontal(|ui| {
            ui.label("Blocks of the world drawn:");
            let mut radius = self.ev.neigh_radius.min(64);
            if chrome::spin_with(ui, &mut radius, 0.0, 64.0, |d| d.custom_formatter(|v, _| if v >= 64.0 { "all".to_string() } else { format!("{v}") })) {
                self.ev.neigh_radius = radius;
            }
            if ui.button("All").clicked() {
                self.ev.neigh_radius = 64;
            }
            if ui.button("None").clicked() {
                self.ev.neigh_radius = 0;
            }
        });
    }

    /// Does what the user confirmed.
    fn do_confirm(&mut self, what: Confirm, frame: &eframe::Frame) {
        match what {
            Confirm::ResetMap => {
                if let Some(at) = self.ev.at().cloned() {
                    let result = crate::events::reset_map(&self.export_dir, at.map);
                    let blocks = crate::events::blocks_of_map(&self.arc_dir, at.matrix, at.map);
                    let mut failed = None;
                    for &id in &blocks {
                        if let Err(e) = self.restore_block(id) {
                            failed = Some(format!("block {id}: {e}"));
                        }
                    }
                    self.reset_events_state();
                    self.finish_reset(frame);
                    self.status = match (result, failed) {
                        (Ok(message), None) => format!("Map {}: {message}; {} blocks come back to the game ({:?}). Start the game again.", at.map, blocks.len(), blocks),
                        (Ok(_), Some(e)) | (Err(e), _) => format!("Cannot reset the map: {e}"),
                    };
                }
            }
            Confirm::ResetWorld => {
                if let Some(at) = self.ev.at().cloned() {
                    let maps: Vec<usize> = self.ev.maps.iter().map(|m| m.map).collect();
                    let mut failed = None;
                    for &map in &maps {
                        if let Err(e) = crate::events::reset_map(&self.export_dir, map) {
                            failed = Some(format!("map {map}: {e}"));
                        }
                    }
                    if let Err(e) = crate::events::reset_matrix(&self.export_dir, at.matrix) {
                        failed = Some(format!("matrix: {e}"));
                    }
                    // The blocks of the game's grid (the matrix of the game, now that the file is gone).
                    let mut count = 0;
                    if let Ok(layout) = crate::events::read_matrix(&self.arc_dir, &self.export_dir, at.matrix) {
                        let mut ids: Vec<usize> = layout.ids.iter().map(|&b| b as usize).filter(|&b| b < 857).collect();
                        ids.sort_unstable();
                        ids.dedup();
                        for id in ids {
                            if let Err(e) = self.restore_block(id) {
                                failed = Some(format!("block {id}: {e}"));
                            }
                            count += 1;
                        }
                    }
                    self.world.clear();
                    self.neigh_heights.clear();
                    self.reset_events_state();
                    self.finish_reset(frame);
                    self.status = match failed {
                        None => format!("The world {} is the world of the game again ({} maps, {count} blocks). Start the game again.", at.matrix, maps.len()),
                        Some(e) => format!("Cannot reset all the world: {e}"),
                    };
                }
            }
            Confirm::ClearBlock => self.clear_block(frame),
            Confirm::ResetEvents => {
                let maps: Vec<usize> = self.ev.maps.iter().map(|m| m.map).collect();
                let mut failed = None;
                for &map in &maps {
                    if let Err(e) = crate::events::reset_map(&self.export_dir, map) {
                        failed = Some(format!("map {map}: {e}"));
                    }
                }
                self.history.clear();
                self.future.clear();
                self.reset_events_state();
                self.finish_reset(frame);
                self.status = match failed {
                    None => format!("The events of {} maps are the ones of the game again. Start the game again.", maps.len()),
                    Some(e) => format!("Cannot restore the events: {e}"),
                };
            }
            Confirm::FlattenBlock => {
                self.status = match self.editor.flatten_ground() {
                    Ok(()) => "The terrain is flat. (Undo with Ctrl+Z.) Export to write the block.".into(),
                    Err(message) => format!("Cannot flatten the terrain: {message}"),
                };
                self.heights = None;
                self.refresh(frame);
                if self.tile_mode {
                    self.heights = Some(self.ground_heights());
                    self.refresh(frame);
                }
            }
            Confirm::ResetBlock => {
                let id = self.editor.block_id();
                match self.restore_block(id) {
                    Ok(()) => {
                        self.finish_reset(frame);
                        self.status = format!("Block {id} is the block of the game again. Start the game again.");
                    }
                    Err(e) => self.status = format!("Cannot restore the block: {e}"),
                }
            }
            Confirm::ResetMatrix => {
                if let Some(at) = self.ev.at().cloned() {
                    self.status = match crate::events::reset_matrix(&self.export_dir, at.matrix) {
                        Ok(()) => {
                            self.ev.matrix = self.ev.original_matrix.clone();
                            self.ev.neigh_key = None;
                            format!("Matrix {} has the blocks of the game again.", at.matrix)
                        }
                        Err(e) => format!("Cannot reset the matrix: {e}"),
                    };
                    self.refresh_neighbours(frame);
                }
            }
        }
    }

    /// Opens another block in this window (the changes of the block that was
    /// open are kept in its project file).
    fn switch_block(&mut self, block: usize, frame: &eframe::Frame) {
        if self.editor.can_undo() {
            if let Ok(text) = self.editor.project_text() {
                let _ = std::fs::write(&self.project_path, text);
            }
        }
        if block == self.editor.block_id() {
            return;
        }
        if self.stroking {
            self.editor.end_stroke();
            self.stroking = false;
        }
        // The pictures are the ones of the tileset of the world (guessing from the models can pick another one).
        let hint = self.tileset_hint.take().or_else(|| {
            let in_world = self.ev.matrix.as_ref().is_some_and(|m| m.ids.contains(&(block as u16)));
            self.ev.at().filter(|_| in_world).and_then(|at| crate::events::map_tileset(&self.arc_dir, at.map))
        });
        let cached = self.world.remove(&block);
        let loaded = crate::data::load_scene(&self.arc_dir, block, hint).and_then(|scene| {
            let mut editor = match cached {
                Some(editor) => editor,
                None => Editor::new(&self.arc_dir, block)?,
            };
            editor.set_tileset(scene.tileset);
            Ok((scene, editor))
        });
        let (scene, editor) = match loaded {
            Ok(pair) => pair,
            Err(e) => {
                self.status = format!("Cannot open the block {block}: {e}");
                return;
            }
        };
        self.tileset_pictures = scene.textures.keys().cloned().collect();
        self.textures = scene.textures;
        // What the catalogues computed for the block that was open is not true for this one.
        self.catalog.fits = None;
        self.tile_catalog.fits = None;
        self.tile_catalog.fill_fits = None;
        // The block that was open waits with its changes.
        let left = std::mem::replace(&mut self.editor, editor);
        self.neigh_heights.remove(&left.block_id());
        self.world.insert(left.block_id(), left);
        self.rows.clear();
        self.visible.clear();
        self.selected.clear();
        self.placing = false;
        self.heights = None;
        self.title = format!("Block {block}");
        self.project_path = format!("block_{block}.msproj");
        let keep_show = self.ev.show;
        let keep_radius = self.ev.neigh_radius;
        self.ev = Default::default();
        self.ev.show = keep_show;
        self.ev.neigh_radius = keep_radius;
        // Look at the middle of the new block.
        let (mut low, mut high) = (Vec3::splat(f32::MAX), Vec3::splat(f32::MIN));
        for (_, mesh) in self.editor.meshes() {
            for p in &mesh.positions {
                low = low.min(Vec3::from(*p));
                high = high.max(Vec3::from(*p));
            }
        }
        if low.x <= high.x {
            self.target = (low + high) * 0.5;
        }
        crate::config::save(&self.arc_dir, block);
        self.events_enter();
        self.refresh(frame);
        self.refresh_neighbours(frame);
        if self.tile_mode {
            self.heights = Some(self.ground_heights());
            self.refresh(frame);
        }
        self.status = format!("Block {block} is open.");
    }

    pub(super) fn can_undo(&self) -> bool {
        !self.history.is_empty() || self.editor.can_undo()
    }

    pub(super) fn can_redo(&self) -> bool {
        !self.future.is_empty() || self.editor.can_redo()
    }

    /// Notes what the user changed in this frame (objects, events, props) as
    /// one step that Ctrl+Z can undo.
    fn track_history(&mut self) {
        let key = (self.editor.block_id(), self.ev.map);
        let depth = self.editor.undo_depth();
        let current = (self.ev.now.clone(), self.editor.props.clone());
        match self.history_base.take() {
            Some((k, events, props)) if k == key && self.ev.loaded => {
                let steps = depth.saturating_sub(self.history_depth);
                let events_changed = events != current.0 || props != current.1;
                if events_changed {
                    self.events_dirty = true;
                }
                if steps > 0 || events_changed {
                    self.history.push(HistoryEntry {
                        editor_steps: steps,
                        before: events_changed.then(|| (events, props)),
                        after: events_changed.then(|| current.clone()),
                    });
                    self.future.clear();
                    if self.history.len() > 200 {
                        self.history.remove(0);
                    }
                }
            }
            Some(_) => {
                // Another block or map: its history starts here.
                self.history.clear();
                self.future.clear();
            }
            None => {}
        }
        self.history_base = Some((key, current.0, current.1));
        self.history_depth = depth;
    }

    /// Adds the entries that the user made (kept in a file) to the catalogue, once.
    fn ensure_mine(&mut self, tiles: bool) {
        let panel = if tiles { &mut self.tile_catalog } else { &mut self.catalog };
        if panel.mine_loaded || panel.entries.is_empty() || panel.building.is_some() {
            return;
        }
        panel.mine_loaded = true;
        if let Ok(list) = crate::catalog::load(&mine_path(tiles)) {
            for entry in list {
                self.favorites.insert(fav_key(tiles, &entry));
                panel.mine.push(panel.entries.len());
                panel.entries.push(entry);
            }
            panel.fits = None;
        }
    }

    /// Puts an entry in the catalogue with a star, and keeps it for the next runs.
    fn add_mine(&mut self, tiles: bool, entry: crate::catalog::Entry) -> Result<usize, String> {
        let panel = if tiles { &mut self.tile_catalog } else { &mut self.catalog };
        if panel.entries.is_empty() || panel.building.is_some() {
            return Err("the catalogue is not ready: open it (C) and wait for it to load".into());
        }
        let key = fav_key(tiles, &entry);
        if let Some(index) = panel.entries.iter().position(|e| fav_key(tiles, e) == key) {
            self.favorites.insert(key);
            save_favorites(&self.favorites);
            return Ok(index);
        }
        let index = panel.entries.len();
        panel.entries.push(entry);
        panel.mine.push(index);
        panel.fits = None;
        self.favorites.insert(key);
        save_favorites(&self.favorites);
        save_mine(tiles, panel);
        Ok(index)
    }

    /// Puts the armed tile of the catalogue on the tiles of the block.
    fn fill_block(&mut self, frame: &eframe::Frame) {
        let Some(entry) = self.stamp.and_then(|i| self.tile_catalog.entries.get(i)) else {
            self.status = "Choose a tile in the catalogue first.".into();
            return;
        };
        let object = entry.portable.clone();
        let name = entry.name.clone();
        let (width, height) = self.editor.grid();
        let corners = self.heights.clone().unwrap_or_else(|| self.ground_heights());
        let mut cells: Vec<(usize, usize, f32)> = Vec::new();
        for row in 0..height {
            for column in 0..width {
                if self.fill_free_only && self.editor.tiles[row * width + column] >> 24 != 0 {
                    continue;
                }
                let at = |i: usize, j: usize| corners[j * (width + 1) + i];
                let ground = (at(column, row) + at(column + 1, row) + at(column, row + 1) + at(column + 1, row + 1)) / 4.0;
                cells.push((column, row, ground));
            }
        }
        self.status = match self.editor.stamp_tiles(object, &cells) {
            Ok(n) => format!("{n} tiles of the block are now {name}."),
            Err(message) => format!("Cannot fill: {message}"),
        };
        self.selected.clear();
        self.refresh(frame);
    }

    /// Keeps the selected objects in the catalogue, with a star.
    fn favorite_selection(&mut self) {
        let Some((low, high)) = self.selection_box() else {
            self.status = "Select an object first.".into();
            return;
        };
        let portable = match self.editor.portable_of_many(&self.selected) {
            Ok(portable) => portable,
            Err(message) => {
                self.status = format!("Cannot keep it: {message}");
                return;
            }
        };
        let Some(preview) = self.editor.preview_of_instances(&self.selected, low, high) else {
            self.status = "Cannot keep an object that comes from another block: keep the original.".into();
            return;
        };
        let mut names: Vec<String> = Vec::new();
        for &i in &self.selected {
            for (name, _) in self.editor.instance_parts(i) {
                if !names.contains(&name) {
                    names.push(name);
                }
            }
        }
        let block = self.editor.block_id();
        let tileset = self.ev.at().and_then(|at| crate::events::map_tileset(&self.arc_dir, at.map));
        let entry = crate::catalog::Entry { name: names.join(" + "), portable, preview, tileset, donor: block, occurrences: 1, variants: 1, blocks: vec![block] };
        self.status = match self.add_mine(false, entry) {
            Ok(_) => "The object is in the catalogue with a star (Catalogue > Objects, \u{2605} only).".into(),
            Err(message) => format!("Cannot keep it: {message}"),
        };
    }

    /// Copies the ground tile under a point into the catalogue (with a star) and arms it.
    fn take_tile_at(&mut self, point: Vec3) {
        let (column, row) = (((point.x + BLOCK_HALF) / TILE).floor() as i32, ((point.z + BLOCK_HALF) / TILE).floor() as i32);
        let objects = self.editor.tile_objects(false);
        let cell = |v: f32| ((v + BLOCK_HALF) / TILE).floor() as i32;
        let found = objects.iter().enumerate().find(|(_, o)| (cell((o.low[0] + o.high[0]) / 2.0), cell((o.low[2] + o.high[2]) / 2.0)) == (column, row));
        let Some((index, object)) = found else {
            self.status = "There is no ground tile here to copy.".into();
            return;
        };
        let preview = self.editor.preview_of(object);
        let portable = match self.editor.portable_of_catalog(object, index) {
            Ok(portable) if portable.parts.iter().all(|p| p.blob.is_some()) => portable,
            Ok(_) => {
                self.status = "Cannot copy this tile: its material is not known.".into();
                return;
            }
            Err(message) => {
                self.status = format!("Cannot copy this tile: {message}");
                return;
            }
        };
        let name = portable.parts.iter().map(|p| p.material.as_str()).collect::<Vec<_>>().join(" + ");
        let block = self.editor.block_id();
        let tileset = self.ev.at().and_then(|at| crate::events::map_tileset(&self.arc_dir, at.map));
        let entry = crate::catalog::Entry { name: name.clone(), portable, preview, tileset, donor: block, occurrences: 1, variants: 1, blocks: vec![block] };
        match self.add_mine(true, entry) {
            Ok(slot) => {
                self.stamp = Some(slot);
                self.take_tile = false;
                self.status = format!("Tile {name} copied (with a star in the catalogue). Click the ground to put it.");
            }
            Err(message) => self.status = format!("Cannot copy this tile: {message}"),
        }
    }

    /// Empties the open block: the objects, the tall grass and the events of
    /// the maps that are in it.
    fn clear_block(&mut self, frame: &eframe::Frame) {
        self.selected.clear();
        self.placing = false;
        self.stamp = None;
        let objects = self.editor.change(|v| {
            for instance in v.iter_mut() {
                if instance.replaces.is_none() {
                    instance.deleted = true;
                }
            }
        });
        let grass = objects.and_then(|_| self.editor.clear_grass()).and_then(|_| self.editor.clear_ledges());
        // The door models of the block go too.
        let doors = self.editor.props.len();
        if doors > 0 {
            if let Err(message) = self.editor.set_props(Vec::new()) {
                self.status = format!("Cannot take the door models away: {message}");
            }
        }
        if let Err(message) = grass {
            self.status = format!("Cannot clear the block: {message}");
            self.refresh(frame);
            return;
        }
        // The tiles: no more grass for the encounters, and (when asked) no more blocked tiles.
        let (width, height) = self.editor.grid();
        let plain = self.plain_value();
        let free_blocked = self.clear_blocked;
        self.editor.begin_stroke();
        for row in 0..height {
            for column in 0..width {
                let kind = self.editor.tiles[row * width + column] >> 24;
                if (31..=37).contains(&kind) || (104..=127).contains(&kind) || (free_blocked && kind == 1) {
                    self.editor.paint(column, row, 1, plain);
                }
            }
        }
        self.editor.end_stroke();
        // The events whose tile is in this block, in every map of the world.
        let mut removed = 0;
        if let Some(at) = self.ev.at().cloned() {
            let (x0, z0) = ((at.column * crate::events::BLOCK_TILES) as i32, (at.row * crate::events::BLOCK_TILES) as i32);
            let inside = |x: i32, z: i32| x >= x0 && x < x0 + crate::events::BLOCK_TILES as i32 && z >= z0 && z < z0 + crate::events::BLOCK_TILES as i32;
            let tile = crate::events::TILE as i32;
            let clean = |events: &mut MapEvents| {
                let before = events.signs.len() + events.triggers.len() + events.warps.len() + events.characters.len();
                events.signs.retain(|s| !inside(s.x as i32, s.z as i32));
                events.triggers.retain(|t| !inside(t.x as i32, t.z as i32));
                events.warps.retain(|w| !inside((w.x as i32).div_euclid(tile), (w.z as i32).div_euclid(tile)));
                events.characters.retain(|c| !inside(c.x as i32, c.z as i32));
                before - (events.signs.len() + events.triggers.len() + events.warps.len() + events.characters.len())
            };
            removed += clean(&mut self.ev.now);
            for other in self.ev.others.iter_mut().flatten() {
                removed += clean(&mut other.1);
            }
            self.ev.selected = None;
            self.events_save();
        }
        self.refresh(frame);
        self.status = format!("Block {} is clear: no object, no grass, no ledge, {doors} door models, {removed} events taken away. Export to write the block.", self.editor.block_id());
    }

    /// Gives a block back to the game: its file is taken off the SD card and
    /// the changes in the window are dropped (the open block is read again).
    fn restore_block(&mut self, id: usize) -> Result<(), String> {
        let folder = std::path::Path::new(&self.export_dir).join("land");
        let file = folder.join(format!("{id}.bin"));
        if file.exists() {
            std::fs::remove_file(&file).map_err(|e| format!("{}: {e}", file.display()))?;
            crate::edit::remove_index(&folder, id)?;
        }
        self.world.remove(&id);
        self.neigh_heights.remove(&id);
        if id == self.editor.block_id() {
            let hint = self.ev.at().and_then(|at| crate::events::map_tileset(&self.arc_dir, at.map));
            let scene = crate::data::load_scene(&self.arc_dir, id, hint)?;
            let mut editor = Editor::new(&self.arc_dir, id)?;
            editor.set_tileset(scene.tileset);
            self.editor = editor;
            self.rows.clear();
            self.visible.clear();
            self.selected.clear();
            self.placing = false;
            self.stamp = None;
            self.heights = None;
        }
        Ok(())
    }

    /// Reads the events and the grid of the blocks again (after a reset).
    fn reset_events_state(&mut self) {
        let (show, radius) = (self.ev.show, self.ev.neigh_radius);
        self.ev = Default::default();
        self.ev.show = show;
        self.ev.neigh_radius = radius;
        self.events_enter();
    }

    /// Sends the scene again after a reset.
    fn finish_reset(&mut self, frame: &eframe::Frame) {
        self.ev.neigh_key = None;
        self.refresh(frame);
        if self.tile_mode {
            self.heights = Some(self.ground_heights());
            self.refresh(frame);
        }
        self.refresh_neighbours(frame);
    }

    /// Writes the open block and the other blocks that have changes.
    fn export_all(&mut self) -> Result<String, String> {
        self.events_save_with(false);
        let dir = &self.export_dir;
        let mut message = self.editor.export(dir)?;
        let mut others = 0;
        for (id, editor) in &self.world {
            if editor.can_undo() || editor.can_redo() {
                editor.export(dir).map_err(|e| format!("block {id}: {e}"))?;
                others += 1;
            }
        }
        if others > 0 {
            message += &format!("; {others} other blocks written too");
        }
        Ok(message)
    }

    /// The block of the world under a ray, when it is not the open block: the
    /// block, and how far it is from the open block. Only the blocks that are
    /// drawn can be reached.
    fn enter_world_block(&mut self, origin: Vec3, direction: Vec3, frame: &eframe::Frame) -> Option<Vec3> {
        let (block, shift) = self.block_under_ray(origin, direction)?;
        if block == self.editor.block_id() {
            return None;
        }
        let old_target = self.target;
        self.switch_block(block, frame);
        self.target = old_target - shift;
        Some(shift)
    }

    /// The other block of the world that a ray goes through at the height of
    /// the target of the camera, with the shift of that block from this one.
    /// Only the blocks that are drawn can be reached.
    fn block_under_ray(&self, origin: Vec3, direction: Vec3) -> Option<(usize, Vec3)> {
        if self.ev.neigh_radius == 0 || direction.y.abs() < 1e-4 {
            return None;
        }
        let t = (self.target.y - origin.y) / direction.y;
        if t <= 0.0 {
            return None;
        }
        let point = origin + direction * t;
        let (dx, dz) = (((point.x + 360.0) / 720.0).floor() as i32, ((point.z + 360.0) / 720.0).floor() as i32);
        if (dx, dz) == (0, 0) || dx.abs().max(dz.abs()) > self.ev.neigh_radius as i32 {
            return None;
        }
        let at = self.ev.at()?;
        let matrix = self.ev.matrix.as_ref()?;
        let (column, row) = (at.column as i32 + dx, at.row as i32 + dz);
        if column < 0 || row < 0 || column as usize >= matrix.width || row as usize >= matrix.height {
            return None;
        }
        let id = *matrix.ids.get(row as usize * matrix.width + column as usize)? as usize;
        (id < 857).then(|| (id, Vec3::new(dx as f32 * 720.0, 0.0, dz as f32 * 720.0)))
    }

    /// Opens a world: its first block.
    fn switch_world(&mut self, matrix: usize, frame: &eframe::Frame) {
        match crate::events::read_matrix(&self.arc_dir, &self.export_dir, matrix) {
            Ok(layout) => match layout.ids.iter().find(|&&b| (b as usize) < 857) {
                Some(&block) => {
                    self.tileset_hint = crate::events::maps_of_matrix(&self.arc_dir, matrix).ok().and_then(|maps| maps.first().and_then(|&m| crate::events::map_tileset(&self.arc_dir, m)));
                    self.switch_block(block as usize, frame)
                }
                None => self.status = "This world has no block.".into(),
            },
            Err(e) => self.status = format!("Cannot read the matrix: {e}"),
        }
    }

    /// Sends only the squares of the events to the graphics card.
    fn refresh_events(&mut self, frame: &eframe::Frame) {
        let Some(state) = frame.wgpu_render_state() else { return };
        let vertices = self.event_vertices();
        let placed = self.placed_meshes();
        let mut renderer = state.renderer.write();
        let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
        gpu.set_events(&state.device, &vertices);
        gpu.set_placed_models(&state.device, &state.queue, &placed);
    }

    /// Starts or ends the mode of the events.
    fn events_toggled(&mut self, frame: &eframe::Frame) {
        // (the blocks around are made after the events are read)
        if self.events_mode {
            self.selected.clear();
            self.placing = false;
            self.stamp = None;
            if self.tile_mode {
                self.tile_mode = false;
                self.toggle_tiles(frame);
            }
            self.events_enter();
        }
        self.refresh(frame);
        self.refresh_neighbours(frame);
    }

    /// Sends the geometry and the selection to the graphics card.
    fn refresh(&mut self, frame: &eframe::Frame) {
        self.ensure_neigh_heights();
        self.sync_meshes(frame);
        let Some(state) = frame.wgpu_render_state() else { return };
        let placed = self.placed_meshes();
        let mut renderer = state.renderer.write();
        let Some(gpu) = renderer.callback_resources.get_mut::<Gpu>() else { return };
        gpu.set_geometry(&state.device, self.editor.meshes());
        gpu.set_tiles(&state.device, &self.tile_vertices());
        gpu.set_events(&state.device, &self.event_vertices());
        gpu.set_placed_models(&state.device, &state.queue, &placed);
        let lists = self
            .editor
            .meshes()
            .iter()
            .enumerate()
            .map(|(i, (_, mesh))| {
                let mut indices = Vec::new();
                if !self.selected.is_empty() {
                    for t in mesh.indices.chunks_exact(3) {
                        if self.editor.owner(i, t[0] as usize).is_some_and(|o| self.selected.contains(&o)) {
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
                // (A tile of ground or a piece of grass that was put on a tile is ground, not an object.)
                let owner = self.editor.owner(i, t[0] as usize).filter(|&o| self.editor.instances.get(o).is_some_and(|inst| inst.replaces.is_none()));
                if ground && (owner.is_some() || self.editor.is_selectable(i) || part != 1) {
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
                        best = Some((distance, Hit { point: origin + direction * distance, owner, mesh: i }));
                    }
                }
            }
        }
        best.map(|(_, hit)| hit)
    }

    /// The ground under a ray: the ground of the block, else the low flat pieces
    /// that the tool counts as objects but that look like ground (earth, paths).
    fn ground_hit(&self, origin: Vec3, direction: Vec3) -> Option<Hit> {
        if let Some(hit) = self.pick(origin, direction, true) {
            return Some(hit);
        }
        let mut best: Option<(f32, Hit)> = None;
        for (i, (part, mesh)) in self.editor.meshes().iter().enumerate() {
            if *part != 1 || !self.visible.get(i).copied().unwrap_or(false) {
                continue;
            }
            for t in mesh.indices.chunks_exact(3) {
                let (Some(a), Some(b), Some(c)) = (mesh.positions.get(t[0] as usize), mesh.positions.get(t[1] as usize), mesh.positions.get(t[2] as usize)) else { continue };
                if a[1].max(b[1]).max(c[1]) > 8.0 {
                    continue;
                }
                if let Some(distance) = ray_triangle(origin, direction, Vec3::from(*a), Vec3::from(*b), Vec3::from(*c)) {
                    if best.as_ref().map_or(true, |(d, _)| distance < *d) {
                        best = Some((distance, Hit { point: origin + direction * distance, owner: None, mesh: i }));
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
        let mut out = Vec::new();
        tile_squares(&self.editor.tiles, self.editor.grid(), heights, [0.0, 0.0], &mut out);
        // The blocks around, so that the whole world can be seen and painted.
        for (id, dx, dz) in self.world_cells() {
            if let (Some(editor), Some(corners)) = (self.world.get(&id), self.neigh_heights.get(&id)) {
                tile_squares(&editor.tiles, editor.grid(), corners, [dx as f32 * 720.0, dz as f32 * 720.0], &mut out);
            }
        }
        out
    }

    /// Finds the height of the ground of the blocks around (once for each block).
    fn ensure_neigh_heights(&mut self) {
        if !self.tile_mode {
            return;
        }
        for (id, _, _) in self.world_cells() {
            if !self.neigh_heights.contains_key(&id) {
                if let Some(editor) = self.world.get(&id) {
                    let corners = corner_heights(editor);
                    self.neigh_heights.insert(id, corners);
                }
            }
        }
    }

    /// Paints the tile under a ray, or takes its attribute with the pipette.
    fn paint_tile(&mut self, origin: Vec3, direction: Vec3, frame: &eframe::Frame) {
        let mut origin = origin;
        let mut found = self.ground_hit(origin, direction);
        if found.is_none() {
            if let Some(shift) = self.enter_world_block(origin, direction, frame) {
                origin -= shift;
                found = self.ground_hit(origin, direction);
            }
        }
        let Some(hit) = found else { return };
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
        if let Some(sign) = self.height_tool {
            self.paint_height(column, row, sign, frame);
            return;
        }
        if !self.stroking {
            self.editor.begin_stroke();
            self.stroking = true;
        }
        if let Some(add) = self.grass_tool {
            self.paint_grass(column, row, add, frame);
            return;
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

    /// The box of the selection, seen from above, and its centre.
    fn selection_box(&self) -> Option<([f32; 3], [f32; 3])> {
        self.selected.iter().map(|&i| self.editor.instance_bounds(i)).reduce(|a, b| {
            let (mut low, mut high) = a;
            for axis in 0..3 {
                low[axis] = low[axis].min(b.0[axis]);
                high[axis] = high[axis].max(b.1[axis]);
            }
            (low, high)
        })
    }

    fn describe_selection(&self) -> String {
        let Some((low, high)) = self.selection_box() else { return "Nothing selected.".into() };
        let mut names: Vec<String> = Vec::new();
        for &i in &self.selected {
            for (name, _) in self.editor.instance_parts(i) {
                if !names.contains(&name) {
                    names.push(name);
                }
            }
        }
        let what = if self.selected.len() == 1 {
            if self.editor.instances[self.selected[0]].copy { "Copy".to_string() } else { "Object".to_string() }
        } else {
            format!("{} pieces", self.selected.len())
        };
        format!(
            "{what}: {}\nat x {:.0} z {:.0}, size {:.0} x {:.0}, height {:.0}..{:.0}",
            names.join(", "),
            (low[0] + high[0]) / 2.0,
            (low[2] + high[2]) / 2.0,
            high[0] - low[0],
            high[2] - low[2],
            low[1],
            high[1]
        )
    }

    /// Called when the mode of the tiles starts or stops.
    fn toggle_tiles(&mut self, frame: &eframe::Frame) {
        if self.tile_mode {
            self.selected.clear();
            if self.heights.is_none() {
                self.heights = Some(self.ground_heights());
            }
            self.status = "Ground properties: paint with the left button, turn with the right button.".into();
        }
        self.refresh(frame);
    }

    fn apply(&mut self, action: Action, frame: &eframe::Frame) {
        match action {
            Action::Undo => {
                if let Some(entry) = self.history.pop() {
                    let mut result = Ok(());
                    for _ in 0..entry.editor_steps {
                        result = result.and(self.editor.undo());
                    }
                    if let Some((events, props)) = &entry.before {
                        self.ev.now = events.clone();
                        result = result.and(self.editor.set_props(props.clone()));
                    }
                    self.events_dirty |= entry.before.is_some();
                    self.future.push(entry);
                    self.history_base = None;
                    self.report(result, "Undone.");
                } else {
                    let r = self.editor.undo();
                    self.report(r, "Undone.");
                }
                self.selected.clear();
            }
            Action::Redo => {
                if let Some(entry) = self.future.pop() {
                    let mut result = Ok(());
                    for _ in 0..entry.editor_steps {
                        result = result.and(self.editor.redo());
                    }
                    if let Some((events, props)) = &entry.after {
                        self.ev.now = events.clone();
                        result = result.and(self.editor.set_props(props.clone()));
                    }
                    self.events_dirty |= entry.after.is_some();
                    self.history.push(entry);
                    self.history_base = None;
                    self.report(result, "Redone.");
                } else {
                    let r = self.editor.redo();
                    self.report(r, "Redone.");
                }
                self.selected.clear();
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
                self.selected.clear();
                self.report(result, "Project loaded.");
            }
            Action::Launch => {
                let exported = self.export_all();
                self.status = match exported {
                    Err(message) => format!("Export failed, the game is not started: {message}"),
                    Ok(message) => match std::process::Command::new(&self.emulator_path).arg(&self.game_path).spawn() {
                        Ok(_) => format!("{message}. The game is starting in the emulator."),
                        Err(e) => format!("{message}. Cannot start {}: {e}", self.emulator_path),
                    },
                };
                return;
            }
            Action::Export => {
                let result = self.export_all();
                self.status = match result {
                    Ok(message) => message,
                    Err(message) => format!("Export failed: {message}"),
                };
                return;
            }
            Action::Duplicate => {
                if self.selected.is_empty() {
                    return;
                }
                let sources: Vec<Instance> = self.selected.iter().map(|&i| self.editor.instances[i].clone()).collect();
                let before = self.selection_box();
                // The copy goes next to the original, not on it: a whole width and one tile to the side.
                let step = before.map_or(TILE, |(low, high)| (((high[0] - low[0]) / TILE).ceil() + 1.0) * TILE);
                let first = self.editor.instances.len();
                let result = self.editor.change(|v| {
                    for source in &sources {
                        v.push(Instance {
                            cluster: source.cluster,
                            offset: [source.offset[0] + step, source.offset[1], source.offset[2]],
                            yaw: source.yaw,
                            copy: true,
                            deleted: false,
                            only: source.only.clone(),
                            replaces: None,
                        });
                    }
                });
                let ok = result.is_ok();
                if ok {
                    self.selected = (first..first + sources.len()).collect();
                }
                self.report(result, "Copied next to the original.");
                // The door of a building: its warp, its door model and its blocked tiles are copied too.
                if let (true, Some((low, high))) = (ok, before) {
                    let delta = [step, 0.0, 0.0];
                    let n = self.events_copy_with_objects(low, high, delta);
                    let w = self.copy_walls(low, high, delta);
                    if n > 0 || w > 0 {
                        self.status = format!("Copied next to the original with {n} events (door...) and {w} blocked tiles (save the events to keep them).");
                    }
                }
            }
            Action::CopyOut => {
                if self.selected.is_empty() {
                    self.status = "Select an object first.".into();
                    return;
                }
                let path = clipboard_path();
                let extra = self.selection_box().and_then(|(low, high)| self.capture_furniture(low, high));
                let with = extra.as_ref().map(|f| f.signs.len() + f.triggers.len() + f.warps.len() + f.props.len()).unwrap_or(0);
                self.status = match self.editor.portable_of_many(&self.selected) {
                    Ok(object) => match std::fs::write(&path, object.to_bytes()) {
                        Ok(()) => {
                            self.clipboard_extra = extra;
                            format!("Copied to the clipboard ({} meshes, with {with} events and door models).", object.parts.len())
                        }
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
                    self.selected = vec![*index];
                    self.placing = true;
                    self.placing_imported = true;
                    self.pending_extra = self.clipboard_extra.clone();
                }
                self.report(result, "Pasted. Click on the ground to place it.");
            }
            Action::Grow => {
                let extra = self.editor.neighbours(&self.selected, GROW_GAP);
                self.status = if extra.is_empty() {
                    "No other piece touches the selection.".into()
                } else {
                    format!("{} pieces added to the selection.", extra.len())
                };
                self.selected.extend(extra);
            }
            Action::Rotate(angle) => {
                let Some((low, high)) = self.selection_box() else { return };
                let centre = [(low[0] + high[0]) / 2.0, (low[2] + high[2]) / 2.0];
                let (sin, cos) = angle.sin_cos();
                // The pieces turn around the centre of the selection.
                let places: Vec<(usize, [f32; 2])> = self
                    .selected
                    .iter()
                    .map(|&i| {
                        let (l, h) = self.editor.instance_bounds(i);
                        (i, [(l[0] + h[0]) / 2.0, (l[2] + h[2]) / 2.0])
                    })
                    .collect();
                let result = self.editor.change(|v| {
                    for (i, c) in &places {
                        let (dx, dz) = (c[0] - centre[0], c[1] - centre[1]);
                        v[*i].offset[0] += centre[0] + cos * dx - sin * dz - c[0];
                        v[*i].offset[2] += centre[1] + sin * dx + cos * dz - c[1];
                        let turned = v[*i].yaw + angle;
                        v[*i].yaw = (turned + std::f32::consts::PI).rem_euclid(std::f32::consts::TAU) - std::f32::consts::PI;
                    }
                });
                self.report(result, "Turned.");
            }
            Action::Delete => {
                let doomed = self.selected.clone();
                let result = self.editor.change(|v| {
                    for &i in &doomed {
                        v[i].deleted = true;
                    }
                });
                self.selected.clear();
                self.report(result, "Deleted.");
            }
            Action::Nudge(delta) => {
                let moved = self.selected.clone();
                let before = self.selection_box();
                let result = self.editor.change(|v| {
                    for &i in &moved {
                        for a in 0..3 {
                            v[i].offset[a] += delta[a];
                        }
                    }
                });
                let ok = result.is_ok();
                self.report(result, "Moved.");
                if let (true, Some((low, high))) = (ok, before) {
                    let n = self.events_follow_objects(low, high, delta);
                    let w = self.move_walls(low, high, delta);
                    if n > 0 || w > 0 {
                        self.status = format!("Moved. {n} events and {w} wall tiles moved with the objects (save the events to keep them).");
                    }
                }
            }
        }
        self.refresh(frame);
    }

    /// Takes an object of the catalogue into the block. The user then clicks
    /// on the ground to put it down.
    fn drop_entry(&mut self, tiles: bool, index: usize, frame: &eframe::Frame) {
        let panel = if tiles { &mut self.tile_catalog } else { &mut self.catalog };
        let Some(entry) = panel.entries.get(index) else { return };
        let object = entry.portable.clone();
        let name = entry.name.clone();
        let pictures: Vec<String> = entry.preview.iter().map(|p| p.picture.clone()).collect();
        let tileset = entry.tileset;
        // The viewer can show the pictures that this tileset does not have.
        if let Some(a) = tileset {
            let arc_dir = self.arc_dir.clone();
            let loaded = panel.tilesets.entry(a).or_insert_with(|| crate::data::tileset_textures(&arc_dir, a).unwrap_or_default());
            for picture in &pictures {
                if let Some(t) = loaded.get(picture) {
                    self.textures.entry(picture.clone()).or_insert_with(|| t.clone());
                }
            }
        }
        // The pictures that the tileset of the block does not have go in a new
        // tileset file when the block is exported.
        let missing: Vec<String> = pictures.iter().filter(|p| !self.tileset_pictures.contains(*p)).cloned().collect();
        let mut warning = String::new();
        if !missing.is_empty() {
            let blobs = tileset.map(|a| crate::data::tileset_texture_blobs(&self.arc_dir, a, &missing)).unwrap_or(Ok(Vec::new()));
            match blobs {
                Ok(blobs) if blobs.len() == missing.len() => {
                    self.editor.provide_pictures(blobs);
                    warning = format!(" The tileset gets the pictures {} when you export.", missing.join(", "));
                }
                Ok(_) => warning = format!(" Warning: pictures of {} were not found in the donor tileset: the game may show a black screen.", missing.join(", ")),
                Err(e) => warning = format!(" Warning: cannot read the pictures ({e}): the game may show a black screen."),
            }
        }
        if tiles {
            self.stamp = Some(index);
            self.placing = false;
            self.selected.clear();
            self.status = format!("Tile {name}: click a tile of the ground to replace it (Esc to stop).{warning}");
            self.refresh(frame);
            return;
        }
        let result = self.editor.import(object);
        if let Ok(i) = &result {
            self.selected = vec![*i];
            self.placing = true;
            self.placing_imported = true;
            self.pending_extra = None;
        }
        self.report(result, &format!("{name}: click on the ground to put it down.{warning}"));
        self.refresh(frame);
    }

    /// Puts the tile of the catalogue that is armed on the tile under the click.
    fn stamp_at(&mut self, point: Vec3, frame: &eframe::Frame) {
        let Some(entry) = self.stamp.and_then(|i| self.tile_catalog.entries.get(i)) else { return };
        let object = entry.portable.clone();
        let (column, row) = ((point.x + BLOCK_HALF).div_euclid(TILE), (point.z + BLOCK_HALF).div_euclid(TILE));
        let (width, height) = self.editor.grid();
        if column < 0.0 || row < 0.0 || column as usize >= width || row as usize >= height {
            self.status = "This is outside the block.".into();
            return;
        }
        let has_grass = object.parts.iter().any(|p| crate::editor::is_grass_material(&p.material));
        let result = self.editor.stamp_tile(object, column as usize, row as usize, Some(point.y));
        let ok = result.is_ok();
        self.report(result, &format!("Tile put in ({}, {}).", column as usize, row as usize));
        if ok {
            // The encounters follow the picture: a tile with tall grass is a tile of tall grass.
            let at = row as usize * width + column as usize;
            let old = self.editor.tiles[at];
            let is_grass = |v: u32| (31..=37).contains(&(v >> 24));
            let value = if has_grass {
                let common = self.editor.tiles.iter().copied().filter(|&v| is_grass(v)).max_by_key(|&v| self.editor.tiles.iter().filter(|&&w| w == v).count());
                Some(common.unwrap_or((31 << 24) | (self.plain_value() & 0x00FF_FFFF)))
            } else if is_grass(old) {
                Some(self.plain_value())
            } else {
                None
            };
            if let Some(value) = value.filter(|&v| v != old) {
                self.editor.begin_stroke();
                self.editor.paint(column as usize, row as usize, 1, value);
                self.editor.end_stroke();
                self.status = format!("Tile put in ({}, {}): {}.", column as usize, row as usize, tile_style(value).0);
            }
        }
        self.refresh(frame);
    }

    /// The piece of tall grass for one tile: from this block, else from the
    /// first block of the game that has some.
    fn grass_sample(&mut self) -> Option<crate::edit::Portable> {
        if self.grass.is_none() {
            self.grass = self.editor.grass_sample().or_else(|| self.world.values().find_map(|e| e.grass_sample())).or_else(|| {
                (0..120).find_map(|id| Editor::new(&self.arc_dir, id).ok().and_then(|e| e.grass_sample()))
            });
        }
        self.grass.clone()
    }

    /// Lifts or lowers the corners of the tiles under the brush, once for each corner in a stroke.
    fn paint_height(&mut self, column: usize, row: usize, sign: f32, frame: &eframe::Frame) {
        let first = self.brush_size.saturating_sub(1) / 2;
        let mut corners: Vec<(usize, usize)> = Vec::new();
        for r in row.saturating_sub(first)..=(row + self.brush_size - first) {
            for c in column.saturating_sub(first)..=(column + self.brush_size - first) {
                if self.grass_done.insert((c, r)) {
                    corners.push((c, r));
                }
            }
        }
        if corners.is_empty() {
            return;
        }
        self.status = match self.editor.raise_corners(&corners, sign * self.height_step) {
            Ok(()) => format!("The ground is {} by {:.0} on {} corners.", if sign > 0.0 { "lifted" } else { "lowered" }, self.height_step, corners.len()),
            Err(message) => format!("Cannot change the height: {message}"),
        };
        self.height_dirty = true;
        self.refresh(frame);
    }

    /// Puts tall grass on the tiles under the brush, or takes it away. The
    /// attribute of the tile follows: the encounters are where the grass is.
    fn paint_grass(&mut self, column: usize, row: usize, add: bool, frame: &eframe::Frame) {
        let sample = if add {
            match self.grass_sample() {
                Some(sample) => Some(sample),
                None => {
                    self.status = "No block of the game has tall grass to copy.".into();
                    return;
                }
            }
        } else {
            None
        };
        let (width, height) = self.editor.grid();
        let first = self.brush_size.saturating_sub(1) / 2;
        let is_grass = |v: u32| (31..=37).contains(&(v >> 24));
        let mut failed: Option<String> = None;
        let mut done = 0;
        for r in row.saturating_sub(first)..(row + self.brush_size - first).min(height) {
            for c in column.saturating_sub(first)..(column + self.brush_size - first).min(width) {
                if !self.grass_done.insert((c, r)) {
                    continue;
                }
                let corners = self.heights.as_ref().map_or(0.0, |h| {
                    let at = |i: usize, j: usize| h[j * (width + 1) + i];
                    (at(c, r) + at(c + 1, r) + at(c, r + 1) + at(c + 1, r + 1)) / 4.0
                });
                match self.editor.stamp_grass(sample.as_ref(), c, r, corners) {
                    Ok(_) => {
                        done += 1;
                        let old = self.editor.tiles[r * width + c];
                        let value = if add {
                            let common = self.editor.tiles.iter().copied().filter(|&v| is_grass(v)).max_by_key(|&v| self.editor.tiles.iter().filter(|&&w| w == v).count());
                            Some(common.unwrap_or((31 << 24) | (self.plain_value() & 0x00FF_FFFF)))
                        } else if is_grass(old) {
                            Some(self.plain_value())
                        } else {
                            None
                        };
                        if let Some(value) = value {
                            self.editor.paint(c, r, 1, value);
                        }
                    }
                    Err(message) => failed = Some(message),
                }
            }
        }
        self.status = match (failed, done) {
            (Some(message), 0) => format!("Cannot do it: {message}"),
            (Some(message), n) => format!("{n} tiles done; the others cannot take it: {message}"),
            (None, n) => format!("{} on {n} tiles.", if add { "Tall grass" } else { "Grass removed" }),
        };
        self.refresh(frame);
    }

    /// The value of a free tile (the most common one among the tiles that are
    /// not a wall, an encounter or water).
    fn plain_value(&self) -> u32 {
        let mut counts: std::collections::HashMap<u32, usize> = Default::default();
        for &v in &self.editor.tiles {
            if v >> 24 == 0 {
                *counts.entry(v).or_default() += 1;
            }
        }
        counts.into_iter().max_by_key(|(_, n)| *n).map_or(0x20, |(v, _)| v)
    }

    /// Copies the blocked tiles that are under a box by whole tiles (the
    /// original tiles stay). Returns how many tiles were copied.
    fn copy_walls(&mut self, low: [f32; 3], high: [f32; 3], delta: [f32; 3]) -> usize {
        if !self.walls_follow {
            return 0;
        }
        let (width, height) = self.editor.grid();
        let (dx, dz) = ((delta[0] / TILE).round() as i32, (delta[2] / TILE).round() as i32);
        if dx == 0 && dz == 0 {
            return 0;
        }
        let tile = |v: f32| ((v + BLOCK_HALF) / TILE).floor() as i32;
        let (c0, c1, r0, r1) = (tile(low[0] + 0.5), tile(high[0] - 0.5), tile(low[2] + 0.5), tile(high[2] - 0.5));
        let mut walls: Vec<(i32, i32, u32)> = Vec::new();
        for r in r0.max(0)..=r1.min(height as i32 - 1) {
            for c in c0.max(0)..=c1.min(width as i32 - 1) {
                let v = self.editor.tiles[r as usize * width + c as usize];
                if v >> 24 == 1 {
                    walls.push((c, r, v));
                }
            }
        }
        let mut copied = 0;
        self.editor.begin_stroke();
        for (c, r, v) in walls {
            let (nc, nr) = (c + dx, r + dz);
            if nc >= 0 && nr >= 0 && (nc as usize) < width && (nr as usize) < height {
                self.editor.paint(nc as usize, nr as usize, 1, v);
                copied += 1;
            }
        }
        self.editor.end_stroke();
        copied
    }

    /// Moves the wall tiles (a building, its door) that are under a box by
    /// whole tiles: the tiles that they leave become free. Returns how many
    /// tiles moved.
    fn move_walls(&mut self, low: [f32; 3], high: [f32; 3], delta: [f32; 3]) -> usize {
        if !self.walls_follow {
            return 0;
        }
        let (width, height) = self.editor.grid();
        let (dx, dz) = ((delta[0] / TILE).round() as i32, (delta[2] / TILE).round() as i32);
        if dx == 0 && dz == 0 {
            return 0;
        }
        let tile = |v: f32| ((v + BLOCK_HALF) / TILE).floor() as i32;
        let (c0, c1, r0, r1) = (tile(low[0] + 0.5), tile(high[0] - 0.5), tile(low[2] + 0.5), tile(high[2] - 0.5));
        let mut walls: Vec<(i32, i32, u32)> = Vec::new();
        for r in r0.max(0)..=r1.min(height as i32 - 1) {
            for c in c0.max(0)..=c1.min(width as i32 - 1) {
                let v = self.editor.tiles[r as usize * width + c as usize];
                if v >> 24 == 1 {
                    walls.push((c, r, v));
                }
            }
        }
        if walls.is_empty() {
            return 0;
        }
        let plain = self.plain_value();
        self.editor.begin_stroke();
        for &(c, r, _) in &walls {
            self.editor.paint(c as usize, r as usize, 1, plain);
        }
        for &(c, r, v) in &walls {
            let (nc, nr) = (c + dx, r + dz);
            if nc >= 0 && nr >= 0 && (nc as usize) < width && (nr as usize) < height {
                self.editor.paint(nc as usize, nr as usize, 1, v);
            }
        }
        self.editor.end_stroke();
        walls.len()
    }

    /// Puts the selection where the user clicked on the ground.
    fn place(&mut self, point: Vec3, frame: &eframe::Frame) {
        let Some((low, high)) = self.selection_box() else { return };
        let snap = |v: f32| if self.snap { ((v - TILE / 2.0) / TILE).round() * TILE + TILE / 2.0 } else { v };
        let (x, z) = (snap(point.x), snap(point.z));
        let delta = [x - (low[0] + high[0]) / 2.0, point.y - low[1], z - (low[2] + high[2]) / 2.0];
        let moved = self.selected.clone();
        let result = self.editor.change(|v| {
            for &i in &moved {
                for a in 0..3 {
                    v[i].offset[a] += delta[a];
                }
            }
        });
        self.placing = false;
        let ok = result.is_ok();
        self.report(result, "Placed.");
        if ok && self.placing_imported {
            // An object that comes from outside: its door and its blocked tiles come with it, if it had some.
            // (What is at the place where it was taken is not its own: it stays.)
            self.placing_imported = false;
            if let (Some(furniture), Some((new_low, new_high))) = (self.pending_extra.take(), self.selection_box()) {
                let (n, w) = self.paste_furniture(&furniture, new_low, new_high);
                if n > 0 || w > 0 {
                    self.status = format!("Placed with {n} events and door models and {w} blocked tiles (save the events to keep them).");
                }
            }
        } else if ok {
            let n = self.events_follow_objects(low, high, delta);
            let w = self.move_walls(low, high, delta);
            if n > 0 || w > 0 {
                self.status = format!("Placed. {n} events and {w} wall tiles moved with the objects (save the events to keep them).");
            }
        }
        self.refresh(frame);
    }
}

impl eframe::App for Viewer {
    fn update(&mut self, ctx: &egui::Context, frame: &mut eframe::Frame) {
        let mut action: Option<Action> = None;
        let tiles_toggled = false;
        let mut select_mesh: Option<usize> = None;

        if !self.ev.loaded {
            // The maps of the block: the list of the worlds shows the world that is open.
            self.events_enter();
            self.scripts = crate::scripts::read(&self.export_dir);
            crate::scripts::set_data_labels(&self.scripts);
            crate::config::save(&self.arc_dir, self.editor.block_id());
            // The marks of the events and of the objects, and the models of the characters.
            self.refresh(frame);
            self.refresh_neighbours(frame);
        }
        let mut cmd = Commands::default();
        self.global_keys(ctx, &mut cmd);
        let alt = ctx.input(|i| i.modifiers.alt);
        if !ctx.wants_keyboard_input() && !alt && self.events_mode {
            self.events_keys(ctx, frame);
            if ctx.input(|i| i.key_pressed(egui::Key::F5)) {
                action = Some(Action::Launch);
            }
        } else if !ctx.wants_keyboard_input() && !alt {
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
                } else if i.key_pressed(egui::Key::F5) {
                    action = Some(Action::Launch);
                } else if i.key_pressed(egui::Key::Delete) || i.key_pressed(egui::Key::Backspace) || i.key_pressed(egui::Key::X) {
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
            if ctx.input(|i| i.key_pressed(egui::Key::E)) && !self.selected.is_empty() {
                action = Some(Action::Grow);
            }
            if ctx.input(|i| i.key_pressed(egui::Key::E)) && !self.selected.is_empty() {
                action = Some(Action::Grow);
            }
            if ctx.input(|i| i.key_pressed(egui::Key::G)) && !self.selected.is_empty() {
                self.placing = !self.placing;
                self.status = if self.placing { "Click on the ground to place the object.".into() } else { "Cancelled.".into() };
            }
            // Enter: the object stays where it is (it is already there: the arrows move it at once).
            if ctx.input(|i| i.key_pressed(egui::Key::Enter)) && self.placing {
                self.placing = false;
                self.placing_imported = false;
                self.pending_extra = None;
                self.status = "Placed here.".into();
            }
            if ctx.input(|i| i.key_pressed(egui::Key::Escape)) {
                self.placing = false;
                self.placing_imported = false;
                self.stamp = None;
                self.pending_extra = None;
                self.take_tile = false;
                self.selected.clear();
                self.refresh(frame);
            }
        }

        self.menu_bar(ctx, &mut cmd);
        self.toolbar(ctx, &mut cmd);
        self.status_bar(ctx);

        let mut dropped: Option<(bool, usize)> = None;
        let mut npc_pick: Option<u16> = None;
        if self.catalog_open {
            let tiles = self.tile_tab && !self.npc_tab;
            if !self.npc_tab {
                self.catalog_start(tiles);
                self.ensure_mine(tiles);
            }
            egui::SidePanel::right("catalog").resizable(true).default_width(380.0).width_range(280.0..=640.0).show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.label(chrome::section_title("Catalogue"));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        if ui.small_button("Close").on_hover_text("Close the catalogue (C)").clicked() {
                            self.catalog_open = false;
                        }
                    });
                });
                ui.horizontal(|ui| {
                    if ui.selectable_label(!self.tile_tab && !self.npc_tab, "Objects").clicked() {
                        self.tile_tab = false;
                        self.npc_tab = false;
                    }
                    if ui.selectable_label(self.tile_tab && !self.npc_tab, "Ground tiles").clicked() {
                        self.tile_tab = true;
                        self.npc_tab = false;
                    }
                    if ui.selectable_label(self.npc_tab, "Characters").clicked() {
                        self.npc_tab = true;
                    }
                });
                ui.separator();
                if self.npc_tab {
                    npc_pick = self.npc_ui(ctx, ui);
                } else {
                    dropped = self.catalog_ui(ctx, ui, tiles).map(|i| (tiles, i));
                }
            });
        }

        // (The events are edited in the panel of the properties, in the place of the selection.)
        let mut events_changed = false;
        let mut fill_now = false;

        if self.show_inspector {
            egui::SidePanel::left("inspector").resizable(true).default_width(350.0).width_range(250.0..=600.0).show(ctx, |ui| {
                // (A row that is too wide scrolls: it must not make the panel wider than its frame.)
                egui::ScrollArea::both().auto_shrink([false, false]).show(ui, |ui| {
                    ui.label(egui::RichText::new(&self.title).heading());
                    ui.add_space(2.0);
                    let tool = self.tool();
                    egui::CollapsingHeader::new(chrome::section_title("World")).default_open(true).show(ui, |ui| {
                        self.world_blocks_ui(ui);
                    });
                    // The panel opens the part that fits what the user picked: an event or an object.
                    let focus_changed = self.events_mode != self.last_focus;
                    self.last_focus = self.events_mode;
                    let (force_selection, force_events) = match (focus_changed, self.events_mode) {
                        (true, true) => (None, Some(true)),
                        (true, false) => (Some(true), None),
                        _ => (None, None),
                    };
                    match tool {
                        Tool::Select => {
                            egui::CollapsingHeader::new(chrome::section_title("Selection")).default_open(true).open(force_selection).show(ui, |ui| {
                                let has_selection = !self.selected.is_empty();
                                ui.group(|ui| {
                                    ui.set_width(ui.available_width());
                                    ui.label(self.describe_selection());
                                });
                                ui.horizontal_wrapped(|ui| {
                                    if ui.add_enabled(has_selection, egui::Button::new("Copy")).on_hover_text("Copy the selection next to it (D)").clicked() {
                                        action = Some(Action::Duplicate);
                                    }
                                    if ui.add_enabled(has_selection, egui::Button::new("Place")).on_hover_text("Click the ground to put the selection there (G)").clicked() {
                                        self.placing = true;
                                        self.status = "Click on the ground to place the object.".into();
                                    }
                                    if ui.add_enabled(has_selection, egui::Button::new("Delete")).on_hover_text("Delete the selection (Del or X)").clicked() {
                                        action = Some(Action::Delete);
                                    }
                                });
                                ui.horizontal_wrapped(|ui| {
                                    let quarter = std::f32::consts::FRAC_PI_2;
                                    if ui.add_enabled(has_selection, egui::Button::new("Turn left")).on_hover_text("Turn a quarter to the left (Shift+R)").clicked() {
                                        action = Some(Action::Rotate(-quarter));
                                    }
                                    if ui.add_enabled(has_selection, egui::Button::new("Turn right")).on_hover_text("Turn a quarter to the right (R)").clicked() {
                                        action = Some(Action::Rotate(quarter));
                                    }
                                    if ui.add_enabled(has_selection, egui::Button::new("Grow")).on_hover_text("Add the pieces that touch the selection: for a house (E)").clicked() {
                                        action = Some(Action::Grow);
                                    }
                                });
                                ui.horizontal_wrapped(|ui| {
                                    if ui.add_enabled(has_selection, egui::Button::new("Copy out")).on_hover_text("Keep the selection to paste it in another block (Ctrl+C)").clicked() {
                                        action = Some(Action::CopyOut);
                                    }
                                    if ui.button("Paste").on_hover_text("Put the copied object in this block (Ctrl+V)").clicked() {
                                        action = Some(Action::Paste);
                                    }
                                });
                                if ui.add_enabled(has_selection, egui::Button::new("\u{2605} Add to the favorites")).on_hover_text("Keep the selection in the catalogue (Objects), with a star, to put it again anywhere").clicked() {
                                    self.favorite_selection();
                                }
                            });
                            egui::CollapsingHeader::new(chrome::section_title("Options")).default_open(true).show(ui, |ui| {
                                ui.checkbox(&mut self.snap, "Snap to the tiles").on_hover_text("An object that you place goes to the middle of a tile");
                                ui.checkbox(&mut self.walls_follow, "Blocked tiles follow the objects").on_hover_text("The blocked tiles (where one cannot walk) under an object move with it");
                                ui.checkbox(&mut self.events_follow, "Events follow the objects").on_hover_text("The warps, signs, props and characters under an object move with it");
                            });
                            egui::CollapsingHeader::new(chrome::section_title("Events")).default_open(true).open(force_events).show(ui, |ui| {
                                events_changed = self.events_ui(ui);
                            });
                            egui::CollapsingHeader::new(chrome::section_title("Scripts of Sango")).default_open(false).show(ui, |ui| {
                                if self.scripts_ui(ui) {
                                    self.scripts_dirty = true;
                                    crate::scripts::set_data_labels(&self.scripts);
                                }
                            });
                        }
                        Tool::Ground => {
                            egui::CollapsingHeader::new(chrome::section_title("Ground tiles")).default_open(true).show(ui, |ui| {
                                ui.label(egui::RichText::new("Choose a tile in the catalogue (right), then click the ground: the tile replaces the one that is there.").weak());
                                ui.horizontal(|ui| {
                                    if ui.selectable_label(self.take_tile, "Take a tile from the map").on_hover_text("Click a ground tile of the map: it goes in the catalogue with a star and is ready to be put somewhere else").clicked() {
                                        self.take_tile = !self.take_tile;
                                        if self.take_tile {
                                            self.stamp = None;
                                            self.status = "Click the ground tile to copy.".into();
                                        }
                                    }
                                    if ui.add_enabled(self.stamp.is_some(), egui::Button::new("Stop")).on_hover_text("Stop putting the armed tile (Esc)").clicked() {
                                        self.stamp = None;
                                    }
                                });
                                let armed = self.stamp.and_then(|i| self.tile_catalog.entries.get(i)).map(|e| e.name.clone());
                                ui.label(match armed {
                                    Some(name) => format!("Armed: {name}"),
                                    None => "No tile armed.".to_string(),
                                });
                                ui.checkbox(&mut self.catalog_open, "Catalogue of the tiles (C)");
                                ui.separator();
                                ui.checkbox(&mut self.fill_free_only, "Fill only the tiles that are free to walk").on_hover_text("The blocked tiles, the water and the ledges keep their ground");
                                if ui.add_enabled(self.stamp.is_some(), egui::Button::new("Fill the block with the armed tile")).on_hover_text("Puts the armed tile on all the tiles of this block (one step for Ctrl+Z)").clicked() {
                                    fill_now = true;
                                }
                            });
                        }
                        Tool::Properties => {
                            egui::CollapsingHeader::new(chrome::section_title("Ground properties")).default_open(true).show(ui, |ui| {
                                ui.horizontal(|ui| {
                                    ui.label("Brush");
                                    chrome::spin_with(ui, &mut self.brush_size, 1.0, 6.0, |d| d.suffix(" tiles"));
                                    if ui.selectable_label(self.pipette, "Pipette").on_hover_text("Click a tile to take its value").clicked() {
                                        self.pipette = !self.pipette;
                                        self.grass_tool = None;
                                        self.height_tool = None;
                                    }
                                });
                                ui.horizontal(|ui| {
                                    if ui.selectable_label(self.grass_tool == Some(true), "Tall grass").on_hover_text("Paint tall grass: the picture and the encounters (only in the bands of grass that the block has)").clicked() {
                                        self.grass_tool = if self.grass_tool == Some(true) { None } else { Some(true) };
                                        self.height_tool = None;
                                        self.pipette = false;
                                    }
                                    if ui.selectable_label(self.grass_tool == Some(false), "Remove grass").on_hover_text("Take the tall grass away: the picture and the encounters").clicked() {
                                        self.grass_tool = if self.grass_tool == Some(false) { None } else { Some(false) };
                                        self.height_tool = None;
                                        self.pipette = false;
                                    }
                                });
                                ui.horizontal(|ui| {
                                    if ui.selectable_label(self.height_tool == Some(1.0), "Raise the ground").on_hover_text("Click or drag: the corners of the tiles go up (the walking height too)").clicked() {
                                        self.height_tool = if self.height_tool == Some(1.0) { None } else { Some(1.0) };
                                        self.grass_tool = None;
                                        self.pipette = false;
                                    }
                                    if ui.selectable_label(self.height_tool == Some(-1.0), "Lower the ground").on_hover_text("Click or drag: the corners of the tiles go down (the walking height too)").clicked() {
                                        self.height_tool = if self.height_tool == Some(-1.0) { None } else { Some(-1.0) };
                                        self.grass_tool = None;
                                        self.pipette = false;
                                    }
                                    ui.label("by");
                                    chrome::spin_with(ui, &mut self.height_step, 1.0, 20.0, |d| d);
                                });
                                ui.label(match (self.height_tool, self.grass_tool) {
                                    (Some(sign), _) => format!("Giving: ground {} by {:.0}", if sign > 0.0 { "up" } else { "down" }, self.height_step),
                                    (None, Some(true)) => "Giving: tall grass".to_string(),
                                    (None, Some(false)) => "Giving: no grass".to_string(),
                                    (None, None) => format!("Giving: {} ({:08x})", tile_style(self.brush).0, self.brush),
                                });
                                ui.label(egui::RichText::new("Values in this block (click to take):").weak());
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
                                egui::ScrollArea::vertical().max_height(220.0).id_salt("tile_values").show(ui, |ui| {
                                    for (value, count) in list {
                                        let (name, colour) = tile_style(value);
                                        let swatch = egui::Color32::from_rgb(colour[0], colour[1], colour[2]);
                                        let text = egui::RichText::new(format!("{value:08x}  {name}  ({count})")).color(swatch);
                                        if ui.selectable_label(self.brush == value && self.grass_tool.is_none(), text).clicked() {
                                            self.brush = value;
                                            self.grass_tool = None;
                                            self.height_tool = None;
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
                            });
                        }
                    }
                    egui::CollapsingHeader::new(chrome::section_title("Project and SD card")).default_open(false).show(ui, |ui| {
                        ui.label("Project file");
                        ui.text_edit_singleline(&mut self.project_path);
                        ui.horizontal(|ui| {
                            if ui.button("Save").on_hover_text("Save the project (Ctrl+S)").clicked() {
                                action = Some(Action::SaveProject);
                            }
                            if ui.button("Load").on_hover_text("Load the project (Ctrl+O)").clicked() {
                                action = Some(Action::LoadProject);
                            }
                        });
                        ui.add_space(4.0);
                        ui.label("Folder of the SD card");
                        ui.text_edit_singleline(&mut self.export_dir);
                        ui.label("Folder where the plugin reads its text files (events, scripts)");
                        if ui.text_edit_singleline(&mut self.plugin_folder_text).on_hover_text("sango for the full plugin, sango/maker for Sango Maker: found from the plugin that is installed").changed() {
                            plugin_dir::set(std::path::Path::new(&self.plugin_folder_text));
                        }
                        ui.label("Emulator program");
                        ui.text_edit_singleline(&mut self.emulator_path);
                        ui.label("Game file");
                        ui.text_edit_singleline(&mut self.game_path);
                    });
                    egui::CollapsingHeader::new(chrome::section_title("Meshes")).default_open(false).show(ui, |ui| {
                        ui.horizontal(|ui| {
                            if ui.button("All").clicked() {
                                self.visible.iter_mut().for_each(|v| *v = true);
                            }
                            if ui.button("None").clicked() {
                                self.visible.iter_mut().for_each(|v| *v = false);
                            }
                        });
                        egui::ScrollArea::vertical().max_height(320.0).id_salt("meshes").show(ui, |ui| {
                            for (i, row) in self.rows.iter().enumerate() {
                                let mark = if row.selectable { "" } else { "  (ground)" };
                                let text = format!("{}  ({}){mark}", row.name, self.editor.meshes()[i].1.indices.len() / 3);
                                let text = if row.has_picture { egui::RichText::new(text) } else { egui::RichText::new(text).weak() };
                                ui.horizontal(|ui| {
                                    if row.selectable && ui.small_button("Select").on_hover_text("Select only this mesh: pieces that have other meshes are cut in two (Shift adds)").clicked() {
                                        select_mesh = Some(i);
                                    }
                                    if let Some(v) = self.visible.get_mut(i) {
                                        ui.checkbox(v, text);
                                    }
                                });
                            }
                        });
                    });
                });
            });
        }

        if events_changed {
            self.refresh_events(frame);
        }
        if fill_now {
            self.fill_block(frame);
        }
        if self.events_mode {
            self.refresh_neighbours(frame);
        }

        // What the menu, the tool bar and the keys asked for.
        if let Some(tool) = cmd.tool.take() {
            self.set_tool(tool, frame);
        }
        if cmd.refresh {
            self.refresh(frame);
        }
        if action.is_none() {
            action = cmd.action.take();
        }
        self.help_window(ctx);
        if let Some(what) = self.confirm_dialog(ctx) {
            self.do_confirm(what, frame);
        }

        let mut click: Option<(Vec3, Vec3)> = None;
        let mut paint: Option<(Vec3, Vec3)> = None;
        egui::CentralPanel::default().frame(egui::Frame::none()).show(ctx, |ui| {
            let (rect, response) = ui.allocate_exact_size(ui.available_size(), egui::Sense::click_and_drag());
            self.camera_input(ctx, rect, &response);
            self.matrix = self.view_matrix(rect);
            self.update_hover(rect, &response);
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
            self.camera_overlay(ctx, rect);
            ui.painter().add(egui_wgpu::Callback::new_paint_callback(
                rect,
                SceneCallback {
                    matrix: self.matrix,
                    visible: self.visible.clone(),
                    direct_v: self.direct_v,
                    show_tiles: self.tile_mode,
                    show_events: self.overlay_on(),
                    show_neighbours: self.ev.neigh_radius > 0,
                },
            ));
        });

        if let Some((tiles, index)) = dropped {
            self.drop_entry(tiles, index, frame);
        }
        if let Some(model) = npc_pick {
            self.arm_npc(model, frame);
        }
        if let Some(mesh) = select_mesh {
            let original = self.editor.original_mesh(mesh);
            let pieces = match original.ok_or_else(|| "this mesh was added by an import: click the object instead".to_string()).and_then(|m| self.editor.isolate_meshes(&[m])) {
                Ok(pieces) => pieces,
                Err(message) => {
                    self.status = format!("Cannot select: {message}");
                    Vec::new()
                }
            };
            if ctx.input(|i| i.modifiers.shift) {
                for p in pieces {
                    if !self.selected.contains(&p) {
                        self.selected.push(p);
                    }
                }
            } else {
                self.selected = pieces;
            }
            self.status = format!("{} pieces selected from the mesh.", self.selected.len());
            self.refresh(frame);
        }
        if tiles_toggled {
            self.toggle_tiles(frame);
        }
        if let Some((origin, direction)) = paint {
            self.paint_tile(origin, direction, frame);
        }
        if !ctx.input(|i| i.pointer.primary_down()) {
            self.grass_done.clear();
            if self.height_dirty {
                self.height_dirty = false;
                self.heights = Some(self.ground_heights());
                self.refresh(frame);
            }
        }
        if self.stroking && !ctx.input(|i| i.pointer.primary_down()) {
            self.editor.end_stroke();
            self.stroking = false;
        }
        if let Some((origin, direction)) = click {
            if self.ev.put.is_some() || self.ev.moving {
                self.events_click(origin, direction, frame);
            } else if self.take_tile {
                if let Some(hit) = self.ground_hit(origin, direction) {
                    self.take_tile_at(hit.point);
                }
            } else if self.stamp.is_some() {
                let mut origin = origin;
                let mut hit = self.ground_hit(origin, direction);
                if hit.is_none() {
                    if let Some(shift) = self.enter_world_block(origin, direction, frame) {
                        origin -= shift;
                        hit = self.ground_hit(origin, direction);
                    }
                }
                if let Some(hit) = hit {
                    self.stamp_at(hit.point, frame);
                }
            } else if self.placing {
                let mut origin = origin;
                let mut hit = self.ground_hit(origin, direction);
                if hit.is_none() && !self.selected.is_empty() {
                    // The object goes to another block of the world: it leaves this block and enters that one.
                    if let Some((block, _)) = self.block_under_ray(origin, direction).filter(|(b, _)| *b != self.editor.block_id()) {
                        let extra = self.selection_box().and_then(|(low, high)| self.capture_furniture(low, high));
                        match self.editor.portable_of_many(&self.selected) {
                            Ok(object) => {
                                let doomed = self.selected.clone();
                                let _ = self.editor.change(|v| {
                                    for &i in &doomed {
                                        v[i].deleted = true;
                                    }
                                });
                                if let Some(shift) = self.enter_world_block(origin, direction, frame) {
                                    origin -= shift;
                                    match self.editor.import(object) {
                                        Ok(i) => {
                                            self.selected = vec![i];
                                            self.placing_imported = true;
                                            self.pending_extra = extra;
                                            self.refresh(frame);
                                            hit = self.ground_hit(origin, direction);
                                        }
                                        Err(message) => self.status = format!("Block {block} cannot take the object: {message}"),
                                    }
                                }
                            }
                            Err(message) => self.status = format!("Cannot carry the object: {message}"),
                        }
                    }
                }
                if let Some(hit) = hit {
                    self.place(hit.point, frame);
                }
            } else if self.ground_mode {
                // (In the tool of the ground tiles a click does not select an object.)
            } else if self
                .event_hit(origin, direction)
                .is_some_and(|t| self.pick(origin, direction, false).and_then(|h| h.owner.map(|_| (h.point - origin).length())).map_or(true, |object| t < object))
            {
                // An event (a character, a warp, a sign...) is nearer than the objects: the panel shows the event.
                self.set_events_focus(true, frame);
                self.events_click(origin, direction, frame);
            } else {
                let shift = ctx.input(|i| i.modifiers.shift);
                let ctrl = ctx.input(|i| i.modifiers.command);
                let mut origin = origin;
                let mut hit = self.pick(origin, direction, false);
                if hit.is_none() {
                    // A click on another block of the world opens that block, then selects what is under the pointer.
                    if let Some(shift) = self.enter_world_block(origin, direction, frame) {
                        origin -= shift;
                        hit = self.pick(origin, direction, false);
                    }
                }
                // A click on the blue mark of an object (its square or its pole) selects the object too.
                let piece = hit.as_ref().and_then(|h| h.owner).or_else(|| self.object_mark_under(origin, direction));
                // The pieces that this click takes: the whole building with Ctrl,
                // else the piece.
                let taken: Vec<usize> = match (piece, &hit) {
                    (Some(i), Some(h)) if ctrl && h.owner.is_some() && self.editor.original_mesh(h.mesh).is_some() => match self.editor.building_of(i, self.editor.original_mesh(h.mesh).unwrap_or(0)) {
                        Ok(pieces) => pieces,
                        Err(message) => {
                            self.status = format!("Cannot select the building: {message}");
                            Vec::new()
                        }
                    },
                    (Some(i), _) => vec![i],
                    _ => Vec::new(),
                };
                if !taken.is_empty() && self.events_mode {
                    self.set_events_focus(false, frame);
                }
                if taken.is_empty() {
                    if !shift && piece.is_none() {
                        self.selected.clear();
                        self.status = "Nothing here.".into();
                    }
                } else if shift && !ctrl {
                    // Shift+click: add the piece, or take it out.
                    match self.selected.iter().position(|&s| s == taken[0]) {
                        Some(at) => {
                            self.selected.remove(at);
                        }
                        None => self.selected.push(taken[0]),
                    }
                    self.status = format!("{} pieces selected.", self.selected.len());
                } else if shift {
                    // Shift+Ctrl+click: add a whole building.
                    for p in taken {
                        if !self.selected.contains(&p) {
                            self.selected.push(p);
                        }
                    }
                    self.status = format!("{} pieces selected.", self.selected.len());
                } else {
                    self.status = if ctrl {
                        format!("The whole building: {} pieces.", taken.len())
                    } else {
                        "Selected. Ctrl+click takes the whole building.".into()
                    };
                    self.selected = taken;
                }
                self.refresh(frame);
            }
        }
        if let Some(action) = action {
            self.apply(action, frame);
        }
        if let Some(block) = self.pending_block.take() {
            self.switch_block(block, frame);
        } else if let Some(matrix) = self.pending_world.take() {
            self.switch_world(matrix, frame);
        }
        self.refresh_neighbours(frame);
        self.track_history();
        // The events are written as soon as the mouse is up: no button to press.
        if self.scripts_dirty && !ctx.input(|i| i.pointer.any_down()) {
            self.scripts_dirty = false;
            if let Err(message) = crate::scripts::write(&self.export_dir, &self.scripts) {
                self.status = format!("Cannot write the scripts: {message}");
            }
        }
        if self.events_dirty && !ctx.input(|i| i.pointer.any_down()) {
            self.events_dirty = false;
            self.events_save_with(false);
        }
        if self.ev.neigh_key.is_none() && self.ev.neigh_radius > 0 {
            // The blocks around are still being opened.
            ctx.request_repaint();
        }
    }
}

impl Viewer {
    /// Reads the catalogue from its file, or starts to build it.
    fn catalog_start(&mut self, tiles: bool) {
        let panel = if tiles { &mut self.tile_catalog } else { &mut self.catalog };
        if let Some(building) = &panel.building {
            let finished = building.1.lock().ok().and_then(|mut slot| slot.take());
            match finished {
                Some(Ok(entries)) => {
                    let path = catalog_path(tiles);
                    panel.message = match crate::catalog::save(&entries, &path) {
                        Ok(()) => format!("{} models, kept in {}", entries.len(), path.display()),
                        Err(e) => format!("{} models (cannot keep them: {e})", entries.len()),
                    };
                    panel.entries = entries;
                    panel.building = None;
                }
                Some(Err(message)) => {
                    panel.message = format!("Cannot build the catalogue: {message}");
                    panel.building = None;
                    self.catalog_open = false;
                }
                None => {}
            }
            return;
        }
        if !panel.entries.is_empty() || !panel.message.is_empty() {
            return;
        }
        match crate::catalog::load(&catalog_path(tiles)) {
            Ok(entries) => {
                panel.message = format!("{} models.", entries.len());
                panel.entries = entries;
            }
            Err(_) => {
                let shared = std::sync::Arc::new((std::sync::atomic::AtomicUsize::new(0), std::sync::Mutex::new(None)));
                let work = shared.clone();
                let arc_dir = self.arc_dir.clone();
                std::thread::spawn(move || {
                    let progress = |block: usize| work.0.store(block, std::sync::atomic::Ordering::Relaxed);
                    let result = if tiles {
                        crate::catalog::build_tiles(&arc_dir, None, progress)
                    } else {
                        crate::catalog::build(&arc_dir, None, progress)
                    };
                    if let Ok(mut slot) = work.1.lock() {
                        *slot = Some(result);
                    }
                });
                panel.building = Some(shared);
            }
        }
    }

    /// The panel of the catalogue. Returns the model that the user clicked.
    fn catalog_ui(&mut self, ctx: &egui::Context, ui: &mut egui::Ui, tiles: bool) -> Option<usize> {
        const THUMB: usize = 96;
        // The tiles that this block can take: its ground meshes have their picture.
        // A model is safe when this block has the shape of its vertices and
        // the tileset of the block has its pictures: else the game shows a black
        // screen or stops.
        let wanted = if tiles { &self.tile_catalog } else { &self.catalog };
        let need = wanted.only_fit && wanted.fits.as_ref().map_or(true, |f| f.len() != wanted.entries.len());
        let computed: Option<Vec<bool>> = need.then(|| {
            wanted
                .entries
                .iter()
                .map(|e| {
                    self.editor.has_layouts(&e.portable)
                })
                .collect()
        });
        let need_fill = tiles && wanted.fill_only && wanted.fill_fits.as_ref().map_or(true, |f| f.len() != wanted.entries.len());
        let (width, height) = self.editor.grid();
        let fill_computed: Option<Vec<bool>> = need_fill.then(|| wanted.entries.iter().map(|e| self.editor.fill_check(&e.portable, width * height).is_ok()).collect());
        let favorites = self.favorites.clone();
        let panel = if tiles { &mut self.tile_catalog } else { &mut self.catalog };
        if fill_computed.is_some() {
            panel.fill_fits = fill_computed;
        }
        if let Some(building) = &panel.building {
            let block = building.0.load(std::sync::atomic::Ordering::Relaxed);
            ui.label(format!("Building the catalogue from all the blocks (one time only): block {block} of 857..."));
            ui.add(egui::ProgressBar::new(block as f32 / 857.0));
            ctx.request_repaint_after(std::time::Duration::from_millis(300));
            return None;
        }
        ui.horizontal(|ui| {
            ui.label("Search:");
            ui.text_edit_singleline(&mut panel.search);
        });
        if !tiles {
            ui.checkbox(&mut panel.hide_ground, "Hide pieces of ground (borders, shadows, cliffs, paths)");
        }
        ui.checkbox(&mut panel.fav_only, "\u{2605} Favorites only");
        if tiles {
            ui.checkbox(&mut panel.fill_only, "Only the tiles that can fill this block").on_hover_text("The tiles that the button \"Fill the block\" can put (the block has their mesh)");
        }
        ui.checkbox(&mut panel.only_fit, "Only the models that this block can take (same shape of vertices)");
        if computed.is_some() {
            panel.fits = computed;
        }
        let words: Vec<String> = panel.search.to_lowercase().split_whitespace().map(String::from).collect();
        let is_starred = |entry: &crate::catalog::Entry| favorites.contains(&fav_key(tiles, entry));
        let mut shown: Vec<usize> = (0..panel.entries.len())
            .filter(|&i| {
                let name = panel.entries[i].name.to_lowercase();
                words.iter().all(|w| name.contains(w))
                    && !(panel.hide_ground && !tiles && is_ground_piece(&name))
                    && !(panel.only_fit && !panel.fits.as_ref().is_some_and(|f| f[i]))
                    && !(panel.fav_only && !is_starred(&panel.entries[i]))
                    && !(tiles && panel.fill_only && !panel.fill_fits.as_ref().is_some_and(|f| f.get(i).copied().unwrap_or(false)))
            })
            .collect();
        // The starred ones come first.
        shown.sort_by_key(|&i| !is_starred(&panel.entries[i]));
        ui.label(format!("{} of {} models. Click one to put it in the map.", shown.len(), panel.entries.len()));
        if !panel.message.is_empty() {
            ui.label(egui::RichText::new(&panel.message).weak());
        }
        let columns = ((ui.available_width() / (THUMB as f32 + 8.0)).floor() as usize).max(1);
        let rows = shown.len().div_ceil(columns);
        let row_height = THUMB as f32 + 8.0;
        let mut clicked = None;
        let mut toggled: Option<usize> = None;
        let mut made = 0;
        let arc_dir = self.arc_dir.clone();
        egui::ScrollArea::vertical().auto_shrink([false, false]).show_rows(ui, row_height, rows, |ui, range| {
            for row in range {
                ui.horizontal(|ui| {
                    for &index in shown.iter().skip(row * columns).take(columns) {
                        let handle = match panel.thumbs.get(&index) {
                            Some(handle) => Some(handle.clone()),
                            None if made < 6 => {
                                made += 1;
                                let entry = &panel.entries[index];
                                let textures = match entry.tileset {
                                    Some(a) => panel
                                        .tilesets
                                        .entry(a)
                                        .or_insert_with(|| crate::data::tileset_textures(&arc_dir, a).unwrap_or_default()),
                                    None => &mut HashMap::new(),
                                };
                                let pixels = crate::thumb::render(&entry.preview, textures, THUMB);
                                let image = egui::ColorImage::from_rgba_unmultiplied([THUMB, THUMB], &pixels);
                                let handle = ctx.load_texture(format!("thumb{index}"), image, egui::TextureOptions::LINEAR);
                                panel.thumbs.insert(index, handle.clone());
                                Some(handle)
                            }
                            None => None,
                        };
                        let entry = &panel.entries[index];
                        let tip = format!(
                            "{}\n{} seen in {} blocks, {} shapes\nTaken from block {}",
                            entry.name,
                            entry.occurrences,
                            entry.blocks.len(),
                            entry.variants,
                            entry.donor
                        );
                        let response = match handle {
                            Some(h) => ui.add(egui::ImageButton::new(egui::load::SizedTexture::new(h.id(), [THUMB as f32, THUMB as f32]))),
                            None => {
                                ctx.request_repaint();
                                ui.add_sized([THUMB as f32, THUMB as f32], egui::Button::new("..."))
                            }
                        };
                        let card = response.rect;
                        if response.on_hover_text(tip).clicked() {
                            clicked = Some(index);
                        }
                        // The star: it keeps the entry as a favorite.
                        let starred = is_starred(&panel.entries[index]);
                        let star_area = egui::Rect::from_min_size(card.right_top() + egui::vec2(-24.0, 2.0), egui::vec2(22.0, 22.0));
                        let star = ui.interact(star_area, ui.id().with(("star", tiles, index)), egui::Sense::click());
                        let backdrop = if star.hovered() { egui::Color32::from_black_alpha(190) } else { egui::Color32::from_black_alpha(110) };
                        ui.painter().rect_filled(star_area, 4.0, backdrop);
                        ui.painter().text(
                            star_area.center(),
                            egui::Align2::CENTER_CENTER,
                            if starred { "\u{2605}" } else { "\u{2606}" },
                            egui::FontId::proportional(16.0),
                            if starred { egui::Color32::from_rgb(255, 205, 60) } else { egui::Color32::WHITE },
                        );
                        if star.on_hover_text(if starred { "Take the star away" } else { "Keep as a favorite" }).clicked() {
                            toggled = Some(index);
                        }
                    }
                });
            }
        });
        if let Some(index) = toggled {
            let key = fav_key(tiles, &panel.entries[index]);
            let starred = if self.favorites.remove(&key) {
                false
            } else {
                self.favorites.insert(key);
                true
            };
            save_favorites(&self.favorites);
            // An entry that the user made and does not want any more goes away.
            if !starred {
                if let Some(at) = panel.mine.iter().position(|&m| m == index) {
                    panel.entries.remove(index);
                    panel.mine.remove(at);
                    for m in panel.mine.iter_mut() {
                        if *m > index {
                            *m -= 1;
                        }
                    }
                    panel.thumbs.clear();
                    panel.fits = None;
                    save_mine(tiles, panel);
                    self.stamp = None;
                    clicked = None;
                }
            }
        }
        clicked
    }
}

/// Opens the window.
pub fn run(textures: HashMap<String, Texture>, editor: Editor, title: String, arc_dir: String) -> Result<(), String> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size([1280.0, 800.0]).with_title("Sango Editor").with_icon(std::sync::Arc::new(egui::IconData { rgba: crate::icon::render(128), width: 128, height: 128 })),
        renderer: eframe::Renderer::Wgpu,
        depth_buffer: 32,
        ..Default::default()
    };
    eframe::run_native(
        "Sango Editor",
        options,
        Box::new(move |cc| {
            Viewer::new(cc, textures, editor, title, arc_dir)
                .map(|v| Box::new(v) as Box<dyn eframe::App>)
                .map_err(Into::into)
        }),
    )
    .map_err(|e| e.to_string())
}
