//! The commands of the program that work without a window.

use std::fmt::Write as _;
use std::path::Path;

use crate::edit::{mesh_groups, CopyRequest, EditableBlock};
use crate::{bch, data};

/// Prints the meshes of one block. With a path, also writes them as an OBJ file.
pub fn meshes(arc_dir: &str, block: usize, obj_path: Option<&str>) -> Result<(), String> {
    let parts = data::read_block(arc_dir, block)?;
    let meshes = data::block_meshes(&parts)?;
    let mut obj = String::new();
    let mut base = 1usize;
    println!("{} meshes", meshes.len());
    for (i, mesh) in meshes.iter().enumerate() {
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for p in &mesh.positions {
            for a in 0..3 {
                low[a] = low[a].min(p[a]);
                high[a] = high[a].max(p[a]);
            }
        }
        println!(
            "  {i:2} {:<22} tex {:<22} {:5} vertices {:5} triangles  x {:.0}..{:.0}  y {:.0}..{:.0}  z {:.0}..{:.0}",
            mesh.material, data::picture_of(mesh), mesh.positions.len(), mesh.indices.len() / 3,
            low[0], high[0], low[1], high[1], low[2], high[2]
        );
        if mesh.positions.is_empty() {
            continue;
        }
        let _ = writeln!(obj, "o {i}_{}", mesh.material);
        for p in &mesh.positions {
            let _ = writeln!(obj, "v {} {} {}", p[0], p[1], p[2]);
        }
        for t in &mesh.uvs {
            let _ = writeln!(obj, "vt {} {}", t[0], 1.0 - t[1]);
        }
        let has_uv = mesh.uvs.len() == mesh.positions.len();
        for t in mesh.indices.chunks_exact(3) {
            let v = |k: usize| {
                let j = t[k] as usize + base;
                if has_uv { format!("{j}/{j}") } else { j.to_string() }
            };
            let _ = writeln!(obj, "f {} {} {}", v(0), v(1), v(2));
        }
        base += mesh.positions.len();
    }
    if let Some(path) = obj_path {
        std::fs::write(path, obj).map_err(|e| format!("{path}: {e}"))?;
        println!("written {path}");
    }
    Ok(())
}

fn write_png(path: &str, width: usize, height: usize, rgba: &[u8]) -> Result<(), String> {
    let file = std::fs::File::create(path).map_err(|e| format!("{path}: {e}"))?;
    let mut encoder = png::Encoder::new(file, width as u32, height as u32);
    encoder.set_color(png::ColorType::Rgba);
    encoder.set_depth(png::BitDepth::Eight);
    encoder
        .write_header()
        .and_then(|mut writer| writer.write_image_data(rgba))
        .map_err(|e| e.to_string())
}

/// Writes the pictures that the meshes of a block use in one PNG file, to
/// check the decoding.
pub fn sheet(arc_dir: &str, block: usize, area: Option<usize>, path: &str) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, area)?;
    let cell = 128usize;
    let mut used: Vec<&bch::Texture> = Vec::new();
    for mesh in &scene.meshes {
        if let Some(t) = scene.textures.get(data::picture_of(mesh)) {
            if !used.iter().any(|u| u.name == t.name) {
                used.push(t);
            }
        }
    }
    let columns = 6usize;
    let rows = used.len().div_ceil(columns).max(1);
    let (w, h) = (columns * cell, rows * cell);
    let mut image = vec![40u8; w * h * 4];
    for (n, t) in used.iter().enumerate() {
        let (ox, oy) = ((n % columns) * cell, (n / columns) * cell);
        for y in 0..cell {
            for x in 0..cell {
                let (sx, sy) = (x * t.width / cell, y * t.height / cell);
                let src = (sy * t.width + sx) * 4;
                let dst = ((oy + y) * w + ox + x) * 4;
                image[dst..dst + 4].copy_from_slice(&t.rgba[src..src + 4]);
            }
        }
        println!("{n}: {} {}x{}", t.name, t.width, t.height);
    }
    write_png(path, w, h, &image)
}

/// Writes one picture at its real size, on a checkerboard so that the
/// transparent parts are visible.
pub fn picture(arc_dir: &str, block: usize, name: &str, path: &str) -> Result<(), String> {
    let scene = data::load_scene(arc_dir, block, None)?;
    let t = scene.textures.get(name).ok_or("no such picture")?;
    let mut image = vec![0u8; t.width * t.height * 4];
    for y in 0..t.height {
        for x in 0..t.width {
            let i = (y * t.width + x) * 4;
            let back = if (x / 8 + y / 8) % 2 == 0 { 90 } else { 150 };
            let a = t.rgba[i + 3] as u32;
            for c in 0..3 {
                image[i + c] = ((t.rgba[i + c] as u32 * a + back * (255 - a)) / 255) as u8;
            }
            image[i + 3] = 255;
        }
    }
    write_png(path, t.width, t.height, &image)
}

/// Prints the groups of vertices of the meshes that have a name: the objects
/// of a block (a tree, a bush, a house) with their place.
pub fn groups(block: &EditableBlock, name: &str) {
    for (index, (_, mesh)) in block.meshes.iter().enumerate() {
        if mesh.material != name {
            continue;
        }
        println!("mesh {index} ({})", mesh.material);
        for (number, vertices) in mesh_groups(mesh).iter().enumerate() {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for &v in vertices {
                for a in 0..3 {
                    low[a] = low[a].min(mesh.positions[v][a]);
                    high[a] = high[a].max(mesh.positions[v][a]);
                }
            }
            println!(
                "  group {number:4}: {:4} vertices  x {:6.0}..{:6.0}  y {:4.0}..{:4.0}  z {:6.0}..{:6.0}",
                vertices.len(), low[0], high[0], low[1], high[1], low[2], high[2]
            );
        }
    }
}

/// Lists the groups of a block of the archive.
pub fn list_groups(arc_dir: &str, block: usize, name: &str) -> Result<(), String> {
    groups(&EditableBlock::load(arc_dir, block)?, name);
    Ok(())
}

/// Writes a block as it is, compressed again. This checks that the game
/// accepts a file that the tool wrote.
pub fn roundtrip(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    let edited = EditableBlock::load(arc_dir, block)?;
    let path = edited.save(out_dir)?;
    println!("written {}", path.display());
    Ok(())
}

/// The groups of vertices whose centre is near a point (seen from above), in
/// the meshes that have one of the materials. A tree is made of many groups
/// in several meshes, so a place selects all of them. Returns the number of
/// the mesh and the vertices of each group.
fn groups_near(block: &EditableBlock, materials: &str, place: [f32; 2], radius: f32) -> Vec<(usize, Vec<usize>)> {
    let wanted: Vec<&str> = materials.split(',').collect();
    let mut found = Vec::new();
    for (index, (_, mesh)) in block.meshes.iter().enumerate() {
        if !wanted.contains(&mesh.material.as_str()) {
            continue;
        }
        for vertices in mesh_groups(mesh) {
            let (mut low, mut high) = ([f32::MAX; 2], [f32::MIN; 2]);
            for &v in &vertices {
                low[0] = low[0].min(mesh.positions[v][0]);
                high[0] = high[0].max(mesh.positions[v][0]);
                low[1] = low[1].min(mesh.positions[v][2]);
                high[1] = high[1].max(mesh.positions[v][2]);
            }
            let centre = [(low[0] + high[0]) / 2.0, (low[1] + high[1]) / 2.0];
            let distance = ((centre[0] - place[0]).powi(2) + (centre[1] - place[1]).powi(2)).sqrt();
            // A group with no triangle is what a deleted object leaves.
            let has_triangle = mesh.indices.iter().any(|&i| vertices.contains(&(i as usize)));
            if distance <= radius && has_triangle {
                found.push((index, vertices));
            }
        }
    }
    found
}

/// Moves what is near a point and writes the block.
pub fn move_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    delta: [f32; 3],
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let mut moved = 0;
    for (mesh, vertices) in &found {
        edited.translate(*mesh, vertices, delta)?;
        moved += vertices.len();
    }
    let path = edited.save(out_dir)?;
    println!("moved {moved} vertices; written {}", path.display());
    Ok(())
}

/// Copies what is near a point to another place and writes the block.
pub fn copy_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    delta: [f32; 3],
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let groups = found.len();
    let added = edited.copy_groups(&[CopyRequest { source: found, foreign: None, centre: [0.0, 0.0], yaw: 0.0, delta }])?;
    let vertices: usize = added[0].iter().map(|(_, _, count)| count).sum();
    let path = edited.save(out_dir)?;
    println!("added {vertices} vertices in {groups} groups; written {}", path.display());
    Ok(())
}

/// Deletes what is near a point and writes the block.
pub fn delete_near(
    arc_dir: &str,
    block: usize,
    materials: &str,
    place: [f32; 2],
    radius: f32,
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let found = groups_near(&edited, materials, place, radius);
    if found.is_empty() {
        return Err("nothing near this place".into());
    }
    let mut removed = 0;
    for (mesh, vertices) in &found {
        removed += edited.delete_group(*mesh, vertices)?;
    }
    let path = edited.save(out_dir)?;
    println!("removed {removed} triangles in {} groups; written {}", found.len(), path.display());
    Ok(())
}

/// Reads a block file that the tool wrote and prints its groups, to check
/// that the file can be read again.
pub fn check(file: &str, block: usize, material: &str) -> Result<(), String> {
    let edited = EditableBlock::from_file(Path::new(file), block)?;
    println!("{} meshes", edited.meshes.len());
    for (_, mesh) in &edited.meshes {
        println!("  {:<22} {:5} triangles", mesh.material, mesh.indices.len() / 3);
    }
    if !material.is_empty() && material != "-" {
        groups(&edited, material);
    }
    Ok(())
}

/// A check of the editing session without a window: copies one object,
/// moves it, deletes another, undoes and does again, then writes the block.
pub fn editor_test(arc_dir: &str, block: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::{Editor, Instance};
    let mut editor = Editor::new(arc_dir, block)?;
    println!("{} objects in {} selectable meshes", editor.clusters.len(), editor.selectable.iter().filter(|s| **s).count());
    // The object that holds the pine at (-321, -25).
    let wanted = editor
        .clusters
        .iter()
        .position(|c| {
            let (x, z) = ((c.low[0] + c.high[0]) / 2.0, (c.low[2] + c.high[2]) / 2.0);
            (x + 321.0).abs() < 40.0 && (z + 25.0).abs() < 40.0
        })
        .ok_or("no object there")?;
    let c = &editor.clusters[wanted];
    println!("pine object {wanted}: {} parts, box x {:.0}..{:.0} z {:.0}..{:.0}", c.parts.len(), c.low[0], c.high[0], c.low[2], c.high[2]);
    let triangles = |e: &Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let before = triangles(&editor);
    editor.change(|v| v.push(Instance { cluster: wanted, offset: [54.0, 0.0, 0.0], yaw: 0.0, copy: true, deleted: false }))?;
    println!("after a copy: {} -> {} triangles", before, triangles(&editor));
    let copy = editor.instances.len() - 1;
    editor.change(|v| v[copy].offset[0] += 18.0)?;
    println!("after moving the copy: {} triangles", triangles(&editor));
    // Turn the copy by 90 degrees: its real vertices must turn, not only the box.
    let real_box = |e: &Editor| {
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for (m, (_, mesh)) in e.meshes().iter().enumerate() {
            for v in 0..mesh.positions.len() {
                if e.owner(m, v) == Some(copy) {
                    for a in 0..3 {
                        low[a] = low[a].min(mesh.positions[v][a]);
                        high[a] = high[a].max(mesh.positions[v][a]);
                    }
                }
            }
        }
        (high[0] - low[0], high[1] - low[1], high[2] - low[2])
    };
    let before_turn = real_box(&editor);
    editor.change(|v| v[copy].yaw = std::f32::consts::FRAC_PI_2)?;
    let after_turn = real_box(&editor);
    println!(
        "turned the copy by 90 degrees: real size x {:.1} y {:.1} z {:.1} -> x {:.1} y {:.1} z {:.1}; {} triangles",
        before_turn.0, before_turn.1, before_turn.2, after_turn.0, after_turn.1, after_turn.2, triangles(&editor)
    );
    editor.change(|v| v[wanted].deleted = true)?;
    println!("after deleting the original: {} triangles", triangles(&editor));
    editor.undo()?;
    println!("after undo: {} triangles", triangles(&editor));
    editor.redo()?;
    println!("after redo: {} triangles", triangles(&editor));
    // Paint a square of 2 x 2 tiles with the value of a wall.
    let (width, _) = editor.grid();
    let before = editor.tiles[20 * width + 20];
    editor.begin_stroke();
    editor.paint(20, 20, 2, 0x0100_0021);
    editor.end_stroke();
    println!("tile (20,20): {before:08x} -> {:08x}", editor.tiles[20 * width + 20]);
    println!("{}", editor.export(out_dir)?);
    let written = EditableBlock::from_file(&std::path::Path::new(out_dir).join("land").join(format!("{block}.bin")), block)?;
    println!("tile (20,20) in the written file: {:08x}", written.attributes()?.cells[20 * width + 20]);
    // Save the project, load it in a new session and write the block again.
    let text = editor.project_text()?;
    println!("project of {} lines:\n{}", text.lines().count(), text.lines().take(8).collect::<Vec<_>>().join("\n"));
    let mut fresh = Editor::new(arc_dir, block)?;
    fresh.load_project(&text)?;
    let second = format!("{out_dir}_reloaded");
    fresh.export(&second)?;
    let first_bytes = std::fs::read(std::path::Path::new(out_dir).join("land").join(format!("{block}.bin"))).map_err(|e| e.to_string())?;
    let second_bytes = std::fs::read(std::path::Path::new(&second).join("land").join(format!("{block}.bin"))).map_err(|e| e.to_string())?;
    println!("block written after loading the project is identical: {}", first_bytes == second_bytes);
    editor.undo()?;
    println!("after undo: tile (20,20) {:08x}, triangles {}", editor.tiles[20 * width + 20], triangles(&editor));
    Ok(())
}

/// Prints the groups of a mesh in the order of their first vertex, to see how
/// the objects follow each other in the file.
pub fn order(arc_dir: &str, block: usize, material: &str, limit: usize) -> Result<(), String> {
    let edited = EditableBlock::load(arc_dir, block)?;
    for (index, (_, mesh)) in edited.meshes.iter().enumerate() {
        if mesh.material != material {
            continue;
        }
        let mut groups = mesh_groups(mesh);
        groups.sort_by_key(|g| g[0]);
        println!("mesh {index} {material}: {} groups", groups.len());
        for g in groups.iter().take(limit) {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for &v in g {
                for a in 0..3 {
                    low[a] = low[a].min(mesh.positions[v][a]);
                    high[a] = high[a].max(mesh.positions[v][a]);
                }
            }
            println!(
                "  first {:5} count {:3}  centre x {:6.0} z {:6.0}  y {:3.0}..{:3.0}",
                g[0], g.len(), (low[0] + high[0]) / 2.0, (low[2] + high[2]) / 2.0, low[1], high[1]
            );
        }
    }
    Ok(())
}

/// Prints how the objects of a block are made: how many objects have each
/// composition (the meshes that they use and how many vertices in each).
pub fn objects(arc_dir: &str, block: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let mut kinds: std::collections::BTreeMap<String, (usize, f32)> = Default::default();
    for cluster in &editor.clusters {
        let mut parts: Vec<String> =
            cluster.parts.iter().map(|(m, v)| format!("{}:{}", editor.material(*m), v.len())).collect();
        parts.sort();
        let width = (cluster.high[0] - cluster.low[0]).max(cluster.high[2] - cluster.low[2]);
        let entry = kinds.entry(parts.join(" + ")).or_insert((0, 0.0));
        entry.0 += 1;
        entry.1 = entry.1.max(width);
    }
    println!("{} objects, {} different compositions", editor.clusters.len(), kinds.len());
    let mut list: Vec<_> = kinds.into_iter().collect();
    list.sort_by_key(|(_, (count, _))| std::cmp::Reverse(*count));
    for (composition, (count, width)) in list.iter().take(25) {
        println!("{count:4} x (widest {width:3.0}) {composition}");
    }
    Ok(())
}

/// Prints the grid of tiles of a block. The letters stand for the number of
/// the ground, and `*` marks the centre of each object of the block.
pub fn tiles(arc_dir: &str, block: usize) -> Result<(), String> {
    let editor = crate::editor::Editor::new(arc_dir, block)?;
    let attributes = EditableBlock::load(arc_dir, block)?.attributes()?;
    println!("grid {} x {}", attributes.width, attributes.height);
    let mut counts: std::collections::BTreeMap<u32, usize> = Default::default();
    for &c in &attributes.cells {
        *counts.entry(c).or_default() += 1;
    }
    println!("{} different values", counts.len());
    let mut by_count: Vec<_> = counts.iter().collect();
    by_count.sort_by_key(|(_, n)| std::cmp::Reverse(**n));
    for (value, n) in by_count.iter().take(12) {
        println!("  {value:08x}  ground {:3}  low {:06x}  x{n}", **value >> 24, **value & 0xffffff);
    }
    let symbol = |value: u32| match value >> 24 {
        0 => '.',
        1 => '#',
        31..=37 => ',',
        148..=151 => '~',
        _ => '?',
    };
    let mut marks = std::collections::HashSet::new();
    for c in &editor.clusters {
        let (x, z) = ((c.low[0] + c.high[0]) / 2.0, (c.low[2] + c.high[2]) / 2.0);
        marks.insert((((x + 360.0) / 18.0) as usize, ((z + 360.0) / 18.0) as usize));
    }
    for row in 0..attributes.height {
        let line: String = (0..attributes.width)
            .map(|col| if marks.contains(&(col, row)) { '*' } else { symbol(attributes.cells[row * attributes.width + col]) })
            .collect();
        println!("{line}");
    }
    Ok(())
}

/// Adds a new mesh to a block: a copy of what is near a point in the meshes
/// that have a material, moved along x. It checks that the block can be read
/// again, and writes it.
pub fn add_mesh_test(
    arc_dir: &str,
    block: usize,
    material: &str,
    place: [f32; 2],
    radius: f32,
    dx: f32,
    out_dir: &str,
) -> Result<(), String> {
    let mut edited = EditableBlock::load(arc_dir, block)?;
    let triangles = |e: &EditableBlock| e.meshes.iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let (meshes_before, triangles_before) = (edited.meshes.len(), triangles(&edited));
    // One new mesh for each material (a tree is in three meshes).
    for name in material.split(',') {
        let found = groups_near(&edited, name, place, radius);
        let like = found.first().map(|(mesh, _)| *mesh).ok_or("nothing near this place")?;
        let source: Vec<(usize, Vec<usize>)> = found.into_iter().filter(|(mesh, _)| *mesh == like).collect();
        let new_mesh = edited.add_mesh_from_groups(like, &source, [dx, 0.0, 0.0])?;
        println!("new mesh {new_mesh}: {} with {} triangles", edited.meshes[new_mesh].1.material, edited.meshes[new_mesh].1.indices.len() / 3);
    }
    println!(
        "meshes {meshes_before} -> {}, triangles {triangles_before} -> {}",
        edited.meshes.len(), triangles(&edited)
    );
    let path = edited.save(out_dir)?;
    println!("written {}", path.display());
    Ok(())
}

/// Looks for problems in the models of a block file (or of a block of the
/// archive) with the checks of the tool.
pub fn validate(arc_dir: &str, block: usize, land_file: Option<&str>) -> Result<(), String> {
    let parts = match land_file {
        Some(path) => data::read_block_from_bytes(&std::fs::read(path).map_err(|e| format!("{path}: {e}"))?)?,
        None => data::read_block(arc_dir, block)?,
    };
    let mut total = 0;
    for part in [1, 5] {
        let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) else { continue };
        let problems = crate::bch_edit::validate(model);
        println!("part {part}: {} problems", problems.len());
        for p in problems.iter().take(8) {
            println!("    {p}");
        }
        total += problems.len();
    }
    if total > 0 {
        return Err(format!("{total} problems"));
    }
    Ok(())
}

/// Checks, on all the blocks, that the width of the indices that the table of
/// relocation gives is the same as the one that the shape of the triangles
/// gives. A difference means that one of the two ways is wrong.
pub fn check_widths(arc_dir: &str) -> Result<(), String> {
    let files = crate::garc::read(&std::path::PathBuf::from(arc_dir).join("map_land_data.garc"))?;
    let (mut blocks, mut conflicts, mut meshes) = (0, 0, 0);
    for (id, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(parts) = data::read_block_from_bytes(raw) else { continue };
        for part in [1, 5] {
            let Some(model) = parts.get(part).filter(|p| p.starts_with(b"BCH\0")) else { continue };
            let Ok(bch) = bch::Bch::parse(model) else { continue };
            let Ok(found) = bch.meshes() else { continue };
            meshes += found.len();
            if bch.width_conflicts.get() > 0 {
                println!("block {id} part {part}: {} disagreements", bch.width_conflicts.get());
                for note in bch.conflict_notes.borrow().iter().take(2) {
                    println!("    {note}");
                }
                conflicts += bch.width_conflicts.get();
            }
        }
        blocks += 1;
    }
    println!("{blocks} blocks, {meshes} meshes, {conflicts} disagreements");
    Ok(())
}

/// Lists the blocks that have a mesh with this material: the name of the
/// block (from the names in its model), the vertices and the size of a vertex.
pub fn find(arc_dir: &str, material: &str) -> Result<(), String> {
    let files = crate::garc::read(&std::path::PathBuf::from(arc_dir).join("map_land_data.garc"))?;
    for (id, file) in files.iter().enumerate() {
        let Some(raw) = file.first() else { continue };
        let Ok(block) = data::read_block_from_bytes(raw) else { continue };
        let Some(model) = block.get(1).filter(|p| p.starts_with(b"BCH\0")) else { continue };
        let Ok(bch) = bch::Bch::parse(model) else { continue };
        let Ok(meshes) = bch.meshes() else { continue };
        let name = bch.names().into_iter().find(|n| n.contains('@') && n.contains("world")).unwrap_or_default();
        for mesh in meshes.iter().filter(|m| m.material == material) {
            println!(
                "block {id:3} {:<28} {material}: {:5} vertices, stride {}, layout {}",
                name.rsplit('@').next().unwrap_or(""), mesh.positions.len(), mesh.places.stride, mesh.places.layout
            );
        }
    }
    Ok(())
}

/// A check of the copy of an object from one block to another, without a
/// window: takes the biggest object of the source block (a tree), puts it in
/// the target block, moves it, writes the block and loads the project again.
pub fn cross_test(arc_dir: &str, source: usize, target: usize, out_dir: &str) -> Result<(), String> {
    use crate::editor::Editor;
    let from = Editor::new(arc_dir, source)?;
    let mut into = Editor::new(arc_dir, target)?;
    // The biggest object that the target block can take.
    let mut order: Vec<usize> = (0..from.clusters.len()).collect();
    order.sort_by_key(|&c| std::cmp::Reverse(from.clusters[c].parts.iter().map(|(_, v)| v.len()).sum::<usize>()));
    let mut chosen = None;
    for c in order {
        let object = from.portable_of_cluster_for_test(c)?;
        match into.can_import(&object) {
            Ok(()) => {
                chosen = Some((c, object));
                break;
            }
            Err(message) => println!("object {c} cannot go in block {target}: {message}"),
        }
        if chosen.is_none() && c % 1000 == 999 {
            break;
        }
    }
    let (biggest, object) = chosen.ok_or("no object of the source block fits in the target block")?;
    let bytes = object.to_bytes();
    let object = crate::edit::Portable::from_bytes(&bytes)?;
    println!(
        "object {biggest} of block {source}: {} pieces, {} bytes, materials {:?}",
        object.parts.len(), bytes.len(), object.parts.iter().map(|p| p.material.as_str()).collect::<Vec<_>>()
    );
    let triangles = |e: &Editor| e.meshes().iter().map(|(_, m)| m.indices.len() / 3).sum::<usize>();
    let before = triangles(&into);
    let index = into.import(object)?;
    println!("pasted in block {target}: {before} -> {} triangles", triangles(&into));
    into.change(|v| v[index].offset = [36.0, 0.0, 36.0])?;
    into.change(|v| v[index].yaw = std::f32::consts::FRAC_PI_2)?;
    println!("moved and turned: {} triangles", triangles(&into));
    println!("{}", into.export(out_dir)?);
    let text = into.project_text()?;
    println!("project:\n{}", text.lines().filter(|l| l.starts_with("import") || l.starts_with("copy")).collect::<Vec<_>>().join("\n"));
    let mut fresh = Editor::new(arc_dir, target)?;
    fresh.load_project(&text)?;
    let second = format!("{out_dir}_reloaded");
    fresh.export(&second)?;
    let name = format!("{target}.bin");
    let a = std::fs::read(std::path::Path::new(out_dir).join("land").join(&name)).map_err(|e| e.to_string())?;
    let b = std::fs::read(std::path::Path::new(&second).join("land").join(&name)).map_err(|e| e.to_string())?;
    println!("block written after loading the project is identical: {}", a == b);
    Ok(())
}
