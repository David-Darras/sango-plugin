//! A window that shows a block in 3D.

use eframe::egui;
use eframe::egui_wgpu::{self, wgpu};
use glam::{Mat4, Vec3};
use wgpu::util::DeviceExt;

use crate::data::{picture_of, Scene};

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
    return vec4<f32>(texel.rgb * in.colour.rgb * light, 1.0);
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

struct Gpu {
    pipeline: wgpu::RenderPipeline,
    uniform: wgpu::Buffer,
    uniform_group: wgpu::BindGroup,
    meshes: Vec<GpuMesh>,
}

struct SceneCallback {
    matrix: Mat4,
    visible: Vec<bool>,
    /// Use the second coordinate of the pictures as it is.
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
        pass.set_pipeline(&gpu.pipeline);
        pass.set_bind_group(0, &gpu.uniform_group, &[]);
        for (i, mesh) in gpu.meshes.iter().enumerate() {
            if !self.visible.get(i).copied().unwrap_or(true) {
                continue;
            }
            pass.set_bind_group(1, &mesh.picture, &[]);
            pass.set_vertex_buffer(0, mesh.vertices.slice(..));
            pass.set_index_buffer(mesh.indices.slice(..), wgpu::IndexFormat::Uint32);
            pass.draw_indexed(0..mesh.index_count, 0, 0..1);
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

fn build_gpu(state: &egui_wgpu::RenderState, scene: &Scene) -> Gpu {
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
    let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
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
            depth_compare: wgpu::CompareFunction::Less,
            stencil: Default::default(),
            bias: Default::default(),
        }),
        multisample: wgpu::MultisampleState::default(),
        multiview: None,
        cache: None,
    });
    let uniform = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("matrix"),
        size: 80,
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    });
    let uniform_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: None,
        layout: &uniform_layout,
        entries: &[wgpu::BindGroupEntry { binding: 0, resource: uniform.as_entire_binding() }],
    });
    let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
        address_mode_u: wgpu::AddressMode::Repeat,
        address_mode_v: wgpu::AddressMode::Repeat,
        mag_filter: wgpu::FilterMode::Linear,
        min_filter: wgpu::FilterMode::Linear,
        ..Default::default()
    });

    let mut meshes = Vec::new();
    for mesh in &scene.meshes {
        let count = mesh.positions.len();
        let vertices: Vec<Vertex> = (0..count)
            .map(|i| Vertex {
                position: mesh.positions[i],
                uv: mesh.uvs.get(i).copied().unwrap_or([0.0, 0.0]),
                colour: mesh.colors.get(i).copied().unwrap_or([255; 4]),
            })
            .collect();
        let indices: Vec<u32> = mesh.indices.iter().copied().filter(|&i| (i as usize) < count).collect();
        let (width, height, pixels) = match scene.textures.get(picture_of(mesh)) {
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
        meshes.push(GpuMesh {
            vertices: device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&vertices),
                usage: wgpu::BufferUsages::VERTEX,
            }),
            indices: device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
                label: None,
                contents: bytemuck::cast_slice(&indices),
                usage: wgpu::BufferUsages::INDEX,
            }),
            index_count: indices.len() as u32,
            picture: group,
        });
    }
    Gpu { pipeline, uniform, uniform_group, meshes }
}

struct Row {
    name: String,
    triangles: usize,
    has_picture: bool,
}

pub struct Viewer {
    rows: Vec<Row>,
    visible: Vec<bool>,
    title: String,
    yaw: f32,
    pitch: f32,
    distance: f32,
    target: Vec3,
    direct_v: bool,
}

impl Viewer {
    fn new(cc: &eframe::CreationContext<'_>, scene: Scene, title: String) -> Result<Self, String> {
        let state = cc.wgpu_render_state.as_ref().ok_or("the graphics card is not available")?;
        let gpu = build_gpu(state, &scene);
        state.renderer.write().callback_resources.insert(gpu);
        let rows: Vec<Row> = scene
            .meshes
            .iter()
            .map(|m| Row {
                name: m.material.clone(),
                triangles: m.indices.len() / 3,
                has_picture: scene.textures.contains_key(picture_of(m)),
            })
            .collect();
        let (mut low, mut high) = (Vec3::splat(f32::MAX), Vec3::splat(f32::MIN));
        for mesh in &scene.meshes {
            for p in &mesh.positions {
                low = low.min(Vec3::from(*p));
                high = high.max(Vec3::from(*p));
            }
        }
        let target = if low.x <= high.x { (low + high) * 0.5 } else { Vec3::ZERO };
        Ok(Viewer {
            visible: vec![true; rows.len()],
            rows,
            title,
            yaw: 0.8,
            pitch: 0.7,
            distance: 900.0,
            target,
            direct_v: true,
        })
    }
}

impl eframe::App for Viewer {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::SidePanel::left("meshes").default_width(300.0).show(ctx, |ui| {
            ui.heading(&self.title);
            ui.horizontal(|ui| {
                if ui.button("All").clicked() {
                    self.visible.iter_mut().for_each(|v| *v = true);
                }
                if ui.button("None").clicked() {
                    self.visible.iter_mut().for_each(|v| *v = false);
                }
            });
            ui.checkbox(&mut self.direct_v, "Direct V (untick to invert)");
            ui.separator();
            egui::ScrollArea::vertical().show(ui, |ui| {
                for (i, row) in self.rows.iter().enumerate() {
                    let text = format!("{}  ({})", row.name, row.triangles);
                    let text = if row.has_picture { egui::RichText::new(text) } else { egui::RichText::new(text).weak() };
                    ui.checkbox(&mut self.visible[i], text);
                }
            });
        });
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
            let matrix = Mat4::perspective_rh(45f32.to_radians(), aspect, 1.0, 20000.0)
                * Mat4::look_at_rh(eye, self.target, up);
            ui.painter().add(egui_wgpu::Callback::new_paint_callback(
                rect,
                SceneCallback { matrix, visible: self.visible.clone(), direct_v: self.direct_v },
            ));
        });
    }
}

/// Opens the window.
pub fn run(scene: Scene, title: String) -> Result<(), String> {
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
            Viewer::new(cc, scene, title).map(|v| Box::new(v) as Box<dyn eframe::App>).map_err(Into::into)
        }),
    )
    .map_err(|e| e.to_string())
}
