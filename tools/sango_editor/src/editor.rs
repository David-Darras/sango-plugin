//! The state of an editing session, without any drawing.
//!
//! An object (a tree, a house) is a cluster of groups of vertices of the
//! original block. An edit is a list of instances: each instance shows a
//! cluster, at an offset from its original place. The block that the game
//! reads is built again from the original block after each change, so what
//! the window shows is what the export writes.

use std::path::Path;

use crate::bch::Mesh;
use crate::edit::{mesh_groups, Attributes, CopyRequest, EditableBlock, Portable, NO_ORIGIN};

/// A mesh is a mesh of objects, and not of ground, when no group of its
/// vertices is wider than this (in game units, seen from above).
const OBJECT_MAX_WIDTH: f32 = 160.0;
/// In a mesh, a group joins the object that is being built when its centre,
/// seen from above, is closer than this to the centre of the object.
const JOIN_DISTANCE: f32 = 40.0;
/// Pieces of two meshes (a crown and its trunk, a tree and its shadow) belong
/// to the same object when their centres, seen from above, are closer than
/// this.
const SAME_PLACE: f32 = 14.0;
/// The widest object that the tool builds from groups, seen from above.
const OBJECT_MAX_SIZE: f32 = 100.0;

use crate::catalog::PreviewPart;

/// An object that a catalogue shows (see `Editor::catalog_objects`).
pub struct CatalogObject {
    /// The number of a mesh and the vertices of the object in this mesh.
    pub parts: Vec<(usize, Vec<usize>)>,
    pub low: [f32; 3],
    pub high: [f32; 3],
}

/// An object of the original block.
pub struct Cluster {
    /// The number of a mesh and the vertices of the object in this mesh.
    pub parts: Vec<(usize, Vec<usize>)>,
    pub low: [f32; 3],
    pub high: [f32; 3],
    /// An object that was taken from another block (or from this one). Then
    /// `parts` is empty.
    pub foreign: Option<Portable>,
}

/// One object shown in the block. The first instances are the objects of the
/// original block, in the order of the clusters. A copy comes after them.
#[derive(Clone, PartialEq, Debug)]
pub struct Instance {
    pub cluster: usize,
    pub offset: [f32; 3],
    /// The turn around the vertical axis through the centre of the object
    /// (radians).
    pub yaw: f32,
    pub copy: bool,
    pub deleted: bool,
    /// Only the parts of the object that are in these meshes (numbers of the
    /// meshes of the original block). `None`: all the parts. A piece that has
    /// several meshes can be cut in two instances: one for some meshes, one
    /// for the others.
    pub only: Option<Vec<usize>>,
    /// A ground tile that was put in place of the tile (column, row): the
    /// triangles of the ground that were there are removed.
    pub replaces: Option<(usize, usize)>,
}

/// What an undo brings back: the instances and the tiles.
type Snapshot = (Vec<Instance>, Vec<u32>);

/// The part of a block that holds the model of the tall grass.
pub const GRASS_PART: usize = 5;

/// The mark of a cluster that lifts one corner of the tiles of the ground (it holds nothing).
const RAISE_MARK: (usize, usize) = (crate::edit::NO_ORIGIN, 3);

/// The mark of a cluster that makes the ground flat (it holds nothing).
const FLATTEN_MARK: (usize, usize) = (crate::edit::NO_ORIGIN, 2);

/// The mark of a cluster that takes ledge models away (it holds nothing).
const LEDGE_ERASER: (usize, usize) = (crate::edit::NO_ORIGIN, 1);

/// True when an object is made of tall grass only (or of nothing).
fn grass_only(object: &Portable) -> bool {
    object.parts.iter().all(|p| is_grass_material(&p.material))
}

/// True for the name of a mesh of the tall grass (`blc` and two digits).
pub fn is_grass_material(name: &str) -> bool {
    name.len() == 5 && name.starts_with("blc") && name[3..].chars().all(|c| c.is_ascii_digit())
}

pub struct Editor {
    original: EditableBlock,
    /// The folder of the archives, to read other blocks again.
    arc_dir: String,
    /// False when the block was a file that the tool wrote.
    from_archive: bool,
    /// The number of objects of the block itself. The objects that come
    /// from another block are after them.
    native: usize,
    pub clusters: Vec<Cluster>,
    /// True for each mesh that holds objects that the user can select.
    pub selectable: Vec<bool>,
    pub instances: Vec<Instance>,
    /// The tiles of the block (row after row), with the changes of the user.
    pub tiles: Vec<u32>,
    /// The tiles of the original block, to save only what changed.
    original_tiles: Vec<u32>,
    /// The door models of the block, as they are in the game and as the user changed them.
    original_props: Vec<crate::edit::Prop>,
    pub props: Vec<crate::edit::Prop>,
    grid: (usize, usize),
    undo: Vec<Snapshot>,
    redo: Vec<Snapshot>,
    /// The state before a stroke of the brush that is not finished.
    stroke: Option<Snapshot>,
    built: EditableBlock,
    /// For each vertex of each mesh of the built block: the instance that
    /// owns it, if any.
    owner: Vec<Vec<Option<usize>>>,
    /// For each mesh of the built block: the mesh of the original block that
    /// it comes from. A mesh that an import added has none.
    built_origin: Vec<Option<usize>>,
    /// The tileset pack of the block, and the pictures that imported objects
    /// bring (the tileset gets those that it does not have, when the block is exported).
    tileset: Option<usize>,
    pictures: Vec<crate::bch_edit::TextureBlob>,
}

/// What tells a mesh from the others when the meshes change places.
type MeshKey = (usize, String, usize, usize, [u32; 3]);

fn mesh_keys(meshes: &[(usize, Mesh)]) -> Vec<MeshKey> {
    meshes
        .iter()
        .map(|(part, m)| {
            (*part, m.material.clone(), m.positions.len(), 0, [0; 3])
        })
        .collect()
}

/// Finds the old meshes in the new list. Returns the new place of each old
/// mesh, and the new meshes that have no old one.
fn match_meshes(before: &[MeshKey], after: &[MeshKey]) -> Result<(Vec<usize>, Vec<usize>), String> {
    let mut used = vec![false; after.len()];
    let mut moved: Vec<Option<usize>> = vec![None; before.len()];
    // First the meshes that are the same. Then the meshes that lost all their
    // triangles: the file counts their vertices another way.
    for pass in 0..2 {
        for (i, key) in before.iter().enumerate() {
            if moved[i].is_some() {
                continue;
            }
            let found = (0..after.len()).find(|&k| {
                !used[k] && after[k].0 == key.0 && after[k].1 == key.1 && (pass == 1 || after[k].2 == key.2)
            });
            if let Some(k) = found {
                used[k] = true;
                moved[i] = Some(k);
            }
        }
    }
    let lost: Vec<String> = before.iter().zip(&moved).filter(|(_, m)| m.is_none()).map(|(k, _)| format!("{k:?}")).collect();
    let moved: Option<Vec<usize>> = moved.into_iter().collect();
    let moved = moved.ok_or_else(|| {
        format!("a mesh was lost when meshes were added (before {} after {}, lost {lost:?})", before.len(), after.len())
    })?;
    Ok((moved, (0..after.len()).filter(|&k| !used[k]).collect()))
}

fn bounds(mesh: &Mesh, vertices: &[usize]) -> ([f32; 3], [f32; 3]) {
    let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
    for &v in vertices {
        for a in 0..3 {
            low[a] = low[a].min(mesh.positions[v][a]);
            high[a] = high[a].max(mesh.positions[v][a]);
        }
    }
    (low, high)
}

impl Editor {
    pub fn new(arc_dir: &str, block: usize) -> Result<Self, String> {
        Self::from_block(EditableBlock::load(arc_dir, block)?, arc_dir, true)
    }

    /// Starts from a block. The edits are measured from this block.
    pub fn from_block(original: EditableBlock, arc_dir: &str, from_archive: bool) -> Result<Self, String> {
        // Find the groups of the meshes of objects.
        struct Group {
            mesh: usize,
            vertices: Vec<usize>,
            bounds: ([f32; 3], [f32; 3]),
        }
        let mut selectable = vec![false; original.meshes.len()];
        let mut by_mesh: Vec<Vec<Group>> = Vec::new();
        for (index, (part, mesh)) in original.meshes.iter().enumerate() {
            if *part != 1 {
                continue; // The tall grass of the encounters is not an object.
            }
            let mut found: Vec<Group> = mesh_groups(mesh)
                .into_iter()
                .map(|vertices| Group { mesh: index, bounds: bounds(mesh, &vertices), vertices })
                .collect();
            let widest = found
                .iter()
                .map(|g| (g.bounds.1[0] - g.bounds.0[0]).max(g.bounds.1[2] - g.bounds.0[2]))
                .fold(0.0, f32::max);
            if !found.is_empty() && widest < OBJECT_MAX_WIDTH {
                selectable[index] = true;
                // The game writes the pieces of an object one after the
                // other, so the order of the vertices separates the objects.
                found.sort_by_key(|g| g.vertices[0]);
                by_mesh.push(found);
            }
        }

        fn centre(b: &([f32; 3], [f32; 3])) -> [f32; 2] {
            [(b.0[0] + b.1[0]) / 2.0, (b.0[2] + b.1[2]) / 2.0]
        }
        fn distance(a: [f32; 2], b: [f32; 2]) -> f32 {
            ((a[0] - b[0]).powi(2) + (a[1] - b[1]).powi(2)).sqrt()
        }
        fn merged(a: &([f32; 3], [f32; 3]), b: &([f32; 3], [f32; 3])) -> ([f32; 3], [f32; 3]) {
            let (mut low, mut high) = *a;
            for axis in 0..3 {
                low[axis] = low[axis].min(b.0[axis]);
                high[axis] = high[axis].max(b.1[axis]);
            }
            (low, high)
        }
        fn too_wide(b: &([f32; 3], [f32; 3])) -> bool {
            (b.1[0] - b.0[0]).max(b.1[2] - b.0[2]) > OBJECT_MAX_SIZE
        }

        // In each mesh, cut the list of groups into pieces: one piece for
        // each object of this mesh.
        struct Piece {
            mesh: usize,
            groups: Vec<usize>, // indexes in the list of the mesh
            bounds: ([f32; 3], [f32; 3]),
        }
        let mut pieces: Vec<Piece> = Vec::new();
        for (list, groups) in by_mesh.iter().enumerate() {
            let mut current: Option<Piece> = None;
            for (i, g) in groups.iter().enumerate() {
                current = match current.take() {
                    Some(mut piece)
                        if distance(centre(&piece.bounds), centre(&g.bounds)) <= JOIN_DISTANCE
                            && !too_wide(&merged(&piece.bounds, &g.bounds)) =>
                    {
                        piece.bounds = merged(&piece.bounds, &g.bounds);
                        piece.groups.push(i);
                        Some(piece)
                    }
                    other => {
                        if let Some(piece) = other {
                            pieces.push(piece);
                        }
                        Some(Piece { mesh: list, groups: vec![i], bounds: g.bounds })
                    }
                };
            }
            pieces.extend(current);
        }

        // Join the pieces of different meshes that are at the same place. The
        // closest pairs join first.
        let mut parent: Vec<usize> = (0..pieces.len()).collect();
        fn find(p: &mut [usize], mut x: usize) -> usize {
            while p[x] != x {
                p[x] = p[p[x]];
                x = p[x];
            }
            x
        }
        let mut pairs: Vec<(f32, usize, usize)> = Vec::new();
        for i in 0..pieces.len() {
            for j in i + 1..pieces.len() {
                if pieces[i].mesh == pieces[j].mesh {
                    continue;
                }
                let d = distance(centre(&pieces[i].bounds), centre(&pieces[j].bounds));
                if d <= SAME_PLACE {
                    pairs.push((d, i, j));
                }
            }
        }
        pairs.sort_by(|a, b| a.0.total_cmp(&b.0));
        let mut boxes: Vec<([f32; 3], [f32; 3])> = pieces.iter().map(|p| p.bounds).collect();
        let mut meshes_of: Vec<Vec<usize>> = pieces.iter().map(|p| vec![p.mesh]).collect();
        for (_, i, j) in pairs {
            let (a, b) = (find(&mut parent, i), find(&mut parent, j));
            // An object has at most one piece in a mesh.
            if a == b || meshes_of[a].iter().any(|m| meshes_of[b].contains(m)) {
                continue;
            }
            let both = merged(&boxes[a], &boxes[b]);
            if too_wide(&both) {
                continue;
            }
            parent[a] = b;
            boxes[b] = both;
            let moved = std::mem::take(&mut meshes_of[a]);
            meshes_of[b].extend(moved);
        }
        let mut by_root: std::collections::BTreeMap<usize, Vec<usize>> = Default::default();
        for i in 0..pieces.len() {
            let root = find(&mut parent, i);
            by_root.entry(root).or_default().push(i);
        }
        let mut clusters = Vec::new();
        for members in by_root.into_values() {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            let mut parts: Vec<(usize, Vec<usize>)> = Vec::new();
            for &m in &members {
                for &gi in &pieces[m].groups {
                    let g = &by_mesh[pieces[m].mesh][gi];
                    for a in 0..3 {
                        low[a] = low[a].min(g.bounds.0[a]);
                        high[a] = high[a].max(g.bounds.1[a]);
                    }
                    match parts.iter_mut().find(|(mesh, _)| *mesh == g.mesh) {
                        Some((_, vertices)) => vertices.extend(&g.vertices),
                        None => parts.push((g.mesh, g.vertices.clone())),
                    }
                }
            }
            clusters.push(Cluster { parts, low, high, foreign: None });
        }

        let instances = (0..clusters.len())
            .map(|cluster| Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: false, deleted: false, only: None, replaces: None })
            .collect();
        let attributes = original.attributes()?;
        let original_props = original.props();
        let native = clusters.len();
        let mut editor = Editor {
            built: original.clone(),
            original,
            arc_dir: arc_dir.to_string(),
            from_archive,
            native,
            clusters,
            selectable,
            instances,
            original_tiles: attributes.cells.clone(),
            original_props: original_props.clone(),
            props: original_props,
            tiles: attributes.cells,
            grid: (attributes.width, attributes.height),
            undo: Vec::new(),
            redo: Vec::new(),
            stroke: None,
            owner: Vec::new(),
            built_origin: Vec::new(),
            tileset: None,
            pictures: Vec::new(),
        };
        editor.rebuild()?;
        Ok(editor)
    }

    /// The meshes of the block as the game will read them.
    pub fn meshes(&self) -> &[(usize, Mesh)] {
        &self.built.meshes
    }

    /// The mesh of the original block that a mesh of the built block comes
    /// from. None for a mesh that an import added.
    pub fn original_mesh(&self, built: usize) -> Option<usize> {
        self.built_origin.get(built).copied().flatten()
    }

    /// Says whether the objects of a mesh of the built block can be selected.
    pub fn is_selectable(&self, built: usize) -> bool {
        self.original_mesh(built).map_or(true, |m| self.selectable[m])
    }

    /// The instance that owns a vertex.
    pub fn owner(&self, mesh: usize, vertex: usize) -> Option<usize> {
        self.owner.get(mesh).and_then(|m| m.get(vertex)).copied().flatten()
    }

    /// The number of the mesh of a mesh in the original block, with its name.
    pub fn material(&self, mesh: usize) -> &str {
        &self.original.meshes[mesh].1.material
    }

    /// The parts of an object that an instance moves.
    fn active_parts<'a>(&'a self, instance: &Instance) -> Vec<&'a (usize, Vec<usize>)> {
        self.clusters[instance.cluster]
            .parts
            .iter()
            .filter(|(mesh, _)| instance.only.as_ref().map_or(true, |meshes| meshes.contains(mesh)))
            .collect()
    }

    /// The box of an instance in the original block, before its turn and its
    /// offset: of its parts only, when it has only some of them.
    fn base_bounds(&self, instance: &Instance) -> ([f32; 3], [f32; 3]) {
        let c = &self.clusters[instance.cluster];
        if instance.only.is_none() || c.foreign.is_some() {
            return (c.low, c.high);
        }
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for (mesh, vertices) in self.active_parts(instance) {
            let (l, h) = bounds(&self.original.meshes[*mesh].1, vertices);
            for a in 0..3 {
                low[a] = low[a].min(l[a]);
                high[a] = high[a].max(h[a]);
            }
        }
        if low[0] > high[0] { (c.low, c.high) } else { (low, high) }
    }

    /// The horizontal centre of an instance in the original block.
    fn base_centre(&self, instance: &Instance) -> [f32; 2] {
        let (l, h) = self.base_bounds(instance);
        [(l[0] + h[0]) / 2.0, (l[2] + h[2]) / 2.0]
    }

    /// The box of an instance, with its turn and its offset.
    pub fn instance_bounds(&self, instance: usize) -> ([f32; 3], [f32; 3]) {
        let i = &self.instances[instance];
        let (base_low, base_high) = self.base_bounds(i);
        let centre = [(base_low[0] + base_high[0]) / 2.0, (base_low[2] + base_high[2]) / 2.0];
        let (sin, cos) = i.yaw.sin_cos();
        let mut low = [f32::MAX, base_low[1], f32::MAX];
        let mut high = [f32::MIN, base_high[1], f32::MIN];
        for (x, z) in [
            (base_low[0], base_low[2]),
            (base_low[0], base_high[2]),
            (base_high[0], base_low[2]),
            (base_high[0], base_high[2]),
        ] {
            let (dx, dz) = (x - centre[0], z - centre[1]);
            let (tx, tz) = (centre[0] + cos * dx - sin * dz, centre[1] + sin * dx + cos * dz);
            low[0] = low[0].min(tx);
            high[0] = high[0].max(tx);
            low[2] = low[2].min(tz);
            high[2] = high[2].max(tz);
        }
        for a in 0..3 {
            low[a] += i.offset[a];
            high[a] += i.offset[a];
        }
        (low, high)
    }

    /// The meshes that an instance holds and the number of vertices in each.
    pub fn instance_parts(&self, instance: usize) -> Vec<(String, usize)> {
        let i = &self.instances[instance];
        if let Some(object) = &self.clusters[i.cluster].foreign {
            return object.parts.iter().map(|p| (p.material.clone(), p.vertices.len() / p.stride)).collect();
        }
        self.active_parts(i).into_iter().map(|(m, v)| (self.original.meshes[*m].1.material.clone(), v.len())).collect()
    }

    /// The meshes that go together: the names that are the same but for the
    /// last word (`pokecen_out_pc_base`, `pokecen_out_pc_window`...). A name
    /// that starts with `chip` is too common to join anything.
    pub fn family_meshes(&self, mesh: usize) -> Vec<usize> {
        fn key(name: &str) -> &str {
            match name.rfind('_') {
                Some(at) if at >= 4 && !name.starts_with("chip_") => &name[..at],
                _ => name,
            }
        }
        let wanted = key(&self.original.meshes[mesh].1.material).to_string();
        (0..self.original.meshes.len())
            .filter(|&m| self.selectable[m] && key(&self.original.meshes[m].1.material) == wanted)
            .collect()
    }

    /// The box of the parts of an instance that are in some meshes, with the
    /// offset of the instance. None when it has no such part.
    fn box_in_meshes(&self, instance: usize, meshes: &[usize]) -> Option<([f32; 3], [f32; 3])> {
        let i = &self.instances[instance];
        let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
        for (mesh, vertices) in self.active_parts(i) {
            if meshes.contains(mesh) {
                let (l, h) = bounds(&self.original.meshes[*mesh].1, vertices);
                for a in 0..3 {
                    low[a] = low[a].min(l[a] + i.offset[a]);
                    high[a] = high[a].max(h[a] + i.offset[a]);
                }
            }
        }
        (low[0] <= high[0]).then_some((low, high))
    }

    /// The whole building that a piece belongs to: the pieces of the same
    /// family of meshes that touch each other, from this piece. The pieces
    /// that also have other meshes are cut, so that only the building is
    /// taken. Returns the instances of the building.
    pub fn building_of(&mut self, instance: usize, mesh: usize) -> Result<Vec<usize>, String> {
        // Two pieces of a building are one when their boxes are this close.
        let gap: f32 = 6.0;
        // The trees and the decorations (`chip_...`) are already whole objects.
        if self.original.meshes[mesh].1.material.starts_with("chip_") {
            return Ok(vec![instance]);
        }
        let family = self.family_meshes(mesh);
        let candidates: Vec<(usize, ([f32; 3], [f32; 3]))> = (0..self.instances.len())
            .filter(|&i| !self.instances[i].deleted && self.clusters[self.instances[i].cluster].foreign.is_none())
            .filter_map(|i| self.box_in_meshes(i, &family).map(|b| (i, b)))
            .collect();
        if !candidates.iter().any(|(i, _)| *i == instance) {
            return Ok(vec![instance]);
        }
        let touches = |a: &([f32; 3], [f32; 3]), b: &([f32; 3], [f32; 3])| {
            a.0[0] - gap <= b.1[0] && b.0[0] - gap <= a.1[0] && a.0[2] - gap <= b.1[2] && b.0[2] - gap <= a.1[2]
        };
        let mut found = vec![instance];
        let mut next = 0;
        while next < found.len() {
            let current = candidates.iter().find(|(i, _)| *i == found[next]).map(|(_, b)| *b);
            next += 1;
            let Some(current) = current else { continue };
            for (i, b) in &candidates {
                if !found.contains(i) && touches(&current, b) {
                    found.push(*i);
                }
            }
        }
        // The pieces that stand near the building without touching it (a door
        // in front of the entrance) are part of it when they are of the same
        // family and the box of the building, made a little larger, holds them.
        let near: f32 = 24.0;
        for _ in 0..4 {
            let boxes: Vec<([f32; 3], [f32; 3])> =
                found.iter().filter_map(|f| candidates.iter().find(|(i, _)| i == f).map(|(_, b)| *b)).collect();
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for b in &boxes {
                for a in 0..3 {
                    low[a] = low[a].min(b.0[a]);
                    high[a] = high[a].max(b.1[a]);
                }
            }
            let mut grew = false;
            for (i, b) in &candidates {
                let overlaps = low[0] - near <= b.1[0] && b.0[0] - near <= high[0] && low[2] - near <= b.1[2] && b.0[2] - near <= high[2];
                if !found.contains(i) && overlaps {
                    found.push(*i);
                    grew = true;
                }
            }
            if !grew {
                break;
            }
        }
        self.isolate_in(Some(&found), &family)
    }

    /// Makes the instances that have vertices in some meshes hold only these
    /// meshes: an instance that has other meshes too is cut in two, the part
    /// in these meshes and the rest. This is how to take a building that has
    /// its own meshes (a Pokemon Center) and nothing else. Returns the
    /// instances that hold these meshes.
    pub fn isolate_meshes(&mut self, meshes: &[usize]) -> Result<Vec<usize>, String> {
        self.isolate_in(None, meshes)
    }

    /// Like `isolate_meshes`, for some instances only (or all, with `None`).
    pub fn isolate_in(&mut self, only_these: Option<&[usize]>, meshes: &[usize]) -> Result<Vec<usize>, String> {
        let mut found: Vec<usize> = Vec::new();
        let mut splits: Vec<(usize, Vec<usize>, Vec<usize>)> = Vec::new();
        for (index, instance) in self.instances.iter().enumerate() {
            if instance.deleted || self.clusters[instance.cluster].foreign.is_some() {
                continue;
            }
            if only_these.is_some_and(|list| !list.contains(&index)) {
                continue;
            }
            let active: Vec<usize> = self.active_parts(instance).iter().map(|p| p.0).collect();
            let inside: Vec<usize> = active.iter().copied().filter(|m| meshes.contains(m)).collect();
            if inside.is_empty() {
                continue;
            }
            if inside.len() == active.len() {
                found.push(index);
            } else {
                splits.push((index, inside, active.iter().copied().filter(|m| !meshes.contains(m)).collect()));
            }
        }
        if !splits.is_empty() {
            let first = self.instances.len();
            let todo = splits.clone();
            self.change(|v| {
                for (i, inside, rest) in &todo {
                    v[*i].only = Some(rest.clone());
                    let mut part = v[*i].clone();
                    part.only = Some(inside.clone());
                    v.push(part);
                }
            })?;
            found.extend(first..first + splits.len());
        }
        Ok(found)
    }

    /// Builds the block again from the original block and the instances.
    fn rebuild(&mut self) -> Result<(), String> {
        let mut block = self.original.clone();
        // The tiles that were replaced lose their ground.
        let mut ground_cells: std::collections::BTreeSet<(usize, usize)> = Default::default();
        let mut grass_cells: std::collections::BTreeSet<(usize, usize)> = Default::default();
        let mut ledge_cells: std::collections::BTreeSet<(usize, usize)> = Default::default();
        let mut flatten = false;
        let mut lifts: Vec<((usize, usize), f32)> = Vec::new();
        for i in self.instances.iter().filter(|i| i.copy && !i.deleted) {
            if let Some(cell) = i.replaces {
                if self.cluster_is_flatten(i.cluster) {
                    flatten = true;
                } else if self.cluster_is_raise(i.cluster) {
                    lifts.push((cell, i.offset[1]));
                } else if self.cluster_is_ledge_eraser(i.cluster) {
                    ledge_cells.insert(cell);
                } else if self.cluster_is_grass(i.cluster) {
                    grass_cells.insert(cell);
                } else {
                    ground_cells.insert(cell);
                }
            }
        }
        for &(column, row) in ground_cells.union(&grass_cells) {
            for mesh in 0..block.meshes.len() {
                let part = block.meshes[mesh].0;
                let ground = part == 1 && !self.selectable[mesh] && ground_cells.contains(&(column, row));
                if ground || part == GRASS_PART {
                    block.delete_triangles_in_tile(mesh, column, row)?;
                }
            }
        }
        // The models of the ledges (the bumps of earth that one jumps over) go away from their tiles.
        for &(column, row) in &ledge_cells {
            for mesh in 0..block.meshes.len() {
                if block.meshes[mesh].0 == 1 && block.meshes[mesh].1.material.contains("jump") {
                    block.delete_triangles_in_tile(mesh, column, row)?;
                }
            }
        }
        // (A copy without any piece only takes the tile that it replaces: the eraser of the grass.)
        let all_copies: Vec<usize> = (0..self.instances.len())
            .filter(|&i| self.instances[i].copy && !self.instances[i].deleted)
            .filter(|&i| self.clusters[self.instances[i].cluster].foreign.as_ref().map_or(true, |o| !o.parts.is_empty()))
            .collect();
        // An object from another block goes in the meshes of this block when it
        // can. Else it gets new meshes, and its materials, after the other changes.
        // A mesh that has small indices (one byte) holds 256 vertices at most: a copy that would not
        // fit goes in new meshes.
        let (mut copies, mut fresh): (Vec<usize>, Vec<usize>) = (Vec::new(), Vec::new());
        let mut used: std::collections::HashMap<usize, usize> = Default::default();
        let limit = |mesh: usize| if block.meshes[mesh].1.places.index_width == 2 { 65536 } else { 256 };
        for i in all_copies {
            let instance = &self.instances[i];
            let foreign = self.clusters[instance.cluster].foreign.as_ref();
            if foreign.is_some_and(|o| self.original.can_host(o).is_err()) {
                fresh.push(i);
                continue;
            }
            // What this copy adds to each mesh.
            let mut adds: Vec<(usize, usize)> = Vec::new();
            match foreign {
                None => {
                    for (mesh, vertices) in self.active_parts(instance) {
                        adds.push((*mesh, vertices.len()));
                    }
                }
                Some(object) => {
                    for part in &object.parts {
                        if let Ok(mesh) = self.original.host_of(part) {
                            adds.push((mesh, part.vertices.len() / part.stride.max(1)));
                        }
                    }
                }
            }
            let mut fits = true;
            let mut total: std::collections::HashMap<usize, usize> = Default::default();
            for &(mesh, n) in &adds {
                *total.entry(mesh).or_default() += n;
            }
            for (&mesh, &n) in &total {
                if block.meshes[mesh].1.positions.len() + used.get(&mesh).copied().unwrap_or(0) + n > limit(mesh) {
                    fits = false;
                }
            }
            if fits {
                for (mesh, n) in total {
                    *used.entry(mesh).or_default() += n;
                }
                copies.push(i);
            } else {
                fresh.push(i);
            }
        }
        let requests: Vec<CopyRequest> = copies
            .iter()
            .map(|&i| CopyRequest {
                source: self.active_parts(&self.instances[i]).into_iter().cloned().collect(),
                foreign: self.clusters[self.instances[i].cluster].foreign.clone(),
                centre: self.base_centre(&self.instances[i]),
                yaw: self.instances[i].yaw,
                delta: self.instances[i].offset,
            })
            .collect();
        let added = if requests.is_empty() { Vec::new() } else { block.copy_groups(&requests)? };

        // The objects of the original block: deleted or moved in place.
        for instance in self.instances.iter() {
            if instance.copy {
                continue;
            }
            let centre = self.base_centre(instance);
            for (mesh, vertices) in self.active_parts(instance) {
                if instance.deleted {
                    block.delete_group(*mesh, vertices)?;
                } else if instance.offset != [0.0; 3] || instance.yaw != 0.0 {
                    block.transform(*mesh, vertices, centre, instance.yaw, instance.offset)?;
                }
            }
        }

        // New meshes for the objects that the block cannot take as they are.
        let mut map: Vec<usize> = (0..block.meshes.len()).collect();
        let mut fresh_meshes: Vec<(usize, Vec<usize>)> = Vec::new();
        // The copies of the same object from another block go in the same new meshes.
        let mut grouped: Vec<(usize, Vec<usize>)> = Vec::new();
        let mut singles: Vec<usize> = Vec::new();
        for &index in &fresh {
            let cluster = self.instances[index].cluster;
            if self.clusters[cluster].foreign.is_none() {
                singles.push(index);
            } else if let Some(group) = grouped.iter_mut().find(|(c, _)| *c == cluster) {
                group.1.push(index);
            } else {
                grouped.push((cluster, vec![index]));
            }
        }
        for (cluster, members) in &grouped {
            if members.len() < 2 {
                singles.push(members[0]);
                continue;
            }
            let Some(object) = self.clusters[*cluster].foreign.clone() else { continue };
            let places: Vec<([f32; 2], f32, [f32; 3])> = members
                .iter()
                .map(|&i| (self.base_centre(&self.instances[i]), self.instances[i].yaw, self.instances[i].offset))
                .collect();
            let before = mesh_keys(&block.meshes);
            block.add_foreign_meshes_many(&object, &places)?;
            let (moved, new) = match_meshes(&before, &mesh_keys(&block.meshes))?;
            for m in map.iter_mut() {
                *m = moved[*m];
            }
            for (_, list) in fresh_meshes.iter_mut() {
                for m in list.iter_mut() {
                    *m = moved[*m];
                }
            }
            fresh_meshes.push((members[0], new));
        }
        for &index in &singles {
            let instance = &self.instances[index];
            let Some(object) = self.clusters[instance.cluster].foreign.clone() else {
                // A copy of an object of this block that does not fit in the meshes that it uses: new meshes.
                let mut made: Vec<usize> = Vec::new();
                let (centre, yaw, delta) = (self.base_centre(instance), instance.yaw, instance.offset);
                for (mesh, vertices) in self.active_parts(instance) {
                    let like = map[*mesh];
                    let before = mesh_keys(&block.meshes);
                    block.add_mesh_from_groups_turned(like, &[(like, vertices.clone())], centre, yaw, delta)?;
                    let (moved, new) = match_meshes(&before, &mesh_keys(&block.meshes))?;
                    for m in map.iter_mut() {
                        *m = moved[*m];
                    }
                    for (_, list) in fresh_meshes.iter_mut() {
                        for m in list.iter_mut() {
                            *m = moved[*m];
                        }
                    }
                    for m in made.iter_mut() {
                        *m = moved[*m];
                    }
                    made.extend(new);
                }
                fresh_meshes.push((index, made));
                continue;
            };
            let before = mesh_keys(&block.meshes);
            block.add_foreign_meshes(&object, self.base_centre(instance), instance.yaw, instance.offset)?;
            let (moved, new) = match_meshes(&before, &mesh_keys(&block.meshes))?;
            for m in map.iter_mut() {
                *m = moved[*m];
            }
            for (_, list) in fresh_meshes.iter_mut() {
                for m in list.iter_mut() {
                    *m = moved[*m];
                }
            }
            fresh_meshes.push((index, new));
        }
        let mut built_origin: Vec<Option<usize>> = vec![None; block.meshes.len()];
        for (original, &now) in map.iter().enumerate() {
            built_origin[now] = Some(original);
        }

        let mut owner: Vec<Vec<Option<usize>>> = block.meshes.iter().map(|(_, m)| vec![None; m.positions.len()]).collect();
        for (index, instance) in self.instances.iter().enumerate() {
            if instance.copy {
                continue;
            }
            for (mesh, vertices) in self.active_parts(instance) {
                for &v in vertices {
                    if let Some(slot) = owner[map[*mesh]].get_mut(v) {
                        *slot = Some(index);
                    }
                }
            }
        }
        for (k, &index) in copies.iter().enumerate() {
            for &(mesh, first, count) in &added[k] {
                for v in first..first + count {
                    if let Some(slot) = owner[map[mesh]].get_mut(v) {
                    *slot = Some(index);
                }
                }
            }
        }
        for (index, list) in &fresh_meshes {
            for &mesh in list {
                for slot in owner[mesh].iter_mut() {
                    *slot = Some(*index);
                }
            }
        }
        if flatten {
            // The ground: the meshes of the ground of the block, and the tiles that were put on it.
            let is_ground = |mesh: usize, vertex: usize| {
                let tile = owner[mesh].get(vertex).copied().flatten().is_some_and(|i| self.instances[i].replaces.is_some());
                tile || built_origin[mesh].is_some_and(|o| !self.selectable[o])
            };
            let mut heights: std::collections::HashMap<i32, usize> = Default::default();
            let mut ground: Vec<(usize, Vec<usize>)> = Vec::new();
            for (mesh, (part, m)) in block.meshes.iter().enumerate() {
                if *part != 1 {
                    continue;
                }
                let vertices: Vec<usize> = (0..m.positions.len()).filter(|&v| is_ground(mesh, v)).collect();
                for &v in &vertices {
                    *heights.entry((m.positions[v][1] * 2.0).round() as i32).or_default() += 1;
                }
                if !vertices.is_empty() {
                    ground.push((mesh, vertices));
                }
            }
            // The height that most of the ground has.
            let level = heights.into_iter().max_by_key(|&(h, n)| (n, -h.abs())).map_or(0.0, |(h, _)| h as f32 / 2.0);
            for (mesh, vertices) in ground {
                block.set_height(mesh, &vertices, level)?;
            }
        }
        let lifts: Vec<((usize, usize), f32)> = lifts.into_iter().filter(|(_, d)| *d != 0.0).collect();
        if !lifts.is_empty() {
            let (width, height) = self.grid;
            let mut corners = vec![0.0f32; (width + 1) * (height + 1)];
            for &((c, r), d) in &lifts {
                if c <= width && r <= height {
                    corners[r * (width + 1) + c] += d;
                }
            }
            let side = 18.0;
            let offset_at = |x: f32, z: f32| {
                let (fx, fz) = (((x + 360.0) / side).clamp(0.0, width as f32), ((z + 360.0) / side).clamp(0.0, height as f32));
                let (i0, j0) = ((fx.floor() as usize).min(width - 1), (fz.floor() as usize).min(height - 1));
                let (tx, tz) = (fx - i0 as f32, fz - j0 as f32);
                let at = |i: usize, j: usize| corners[j * (width + 1) + i];
                at(i0, j0) * (1.0 - tx) * (1.0 - tz) + at(i0 + 1, j0) * tx * (1.0 - tz) + at(i0, j0 + 1) * (1.0 - tx) * tz + at(i0 + 1, j0 + 1) * tx * tz
            };
            for mesh in 0..block.meshes.len() {
                let part = block.meshes[mesh].0;
                let vertices: Vec<usize> = (0..block.meshes[mesh].1.positions.len())
                    .filter(|&v| {
                        part == GRASS_PART
                            || (part == 1
                                && (owner[mesh].get(v).copied().flatten().is_some_and(|i| self.instances[i].replaces.is_some())
                                    || built_origin[mesh].is_some_and(|o| !self.selectable[o])))
                    })
                    .collect();
                for v in vertices {
                    let p = block.meshes[mesh].1.positions[v];
                    let lifted = p[1] + offset_at(p[0], p[2]);
                    block.set_height(mesh, &[v], lifted)?;
                }
            }
        }
        if flatten || !lifts.is_empty() {
            // The game walks on the collision model: it follows the ground that was changed.
            let mut triangles: Vec<[[f32; 3]; 3]> = Vec::new();
            for (mesh, (part, m)) in block.meshes.iter().enumerate() {
                if *part != 1 {
                    continue;
                }
                let is_ground = |v: usize| {
                    owner[mesh].get(v).copied().flatten().is_some_and(|i| self.instances[i].replaces.is_some())
                        || built_origin[mesh].is_some_and(|o| !self.selectable[o])
                };
                for t in m.indices.chunks_exact(3) {
                    let (Some(a), Some(b), Some(c)) = (m.positions.get(t[0] as usize), m.positions.get(t[1] as usize), m.positions.get(t[2] as usize)) else { continue };
                    if !(is_ground(t[0] as usize) && is_ground(t[1] as usize) && is_ground(t[2] as usize)) {
                        continue;
                    }
                    // Only the faces that one can walk on (not the walls of the cliffs).
                    let (u, w) = ([b[0] - a[0], b[1] - a[1], b[2] - a[2]], [c[0] - a[0], c[1] - a[1], c[2] - a[2]]);
                    let normal = [u[1] * w[2] - u[2] * w[1], u[2] * w[0] - u[0] * w[2], u[0] * w[1] - u[1] * w[0]];
                    let length = (normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]).sqrt();
                    if length > 1.0e-6 && normal[1].abs() / length > 0.35 {
                        triangles.push([*a, *b, *c]);
                    }
                }
            }
            block.set_collision(&triangles)?;
        }
        block.set_attributes(&Attributes { width: self.grid.0, height: self.grid.1, cells: self.tiles.clone() })?;
        if self.props != self.original_props {
            block.set_props(&self.props)?;
        }
        self.built = block;
        self.owner = owner;
        self.built_origin = built_origin;
        Ok(())
    }

    /// Changes the instances and builds the block again. When the block
    /// cannot be built, the change is cancelled.
    pub fn change(&mut self, change: impl FnOnce(&mut Vec<Instance>)) -> Result<(), String> {
        let before: Snapshot = (self.instances.clone(), self.tiles.clone());
        change(&mut self.instances);
        if self.instances == before.0 {
            return Ok(());
        }
        match self.rebuild() {
            Ok(()) => {
                self.undo.push(before);
                self.redo.clear();
                Ok(())
            }
            Err(message) => {
                (self.instances, self.tiles) = before;
                let _ = self.rebuild();
                Err(message)
            }
        }
    }

    /// Changes the props and builds the block again. The change is cancelled
    /// when the block cannot be built. (Undo does not bring the props back.)
    pub fn set_props(&mut self, props: Vec<crate::edit::Prop>) -> Result<(), String> {
        let before = std::mem::replace(&mut self.props, props);
        match self.rebuild() {
            Ok(()) => Ok(()),
            Err(message) => {
                self.props = before;
                let _ = self.rebuild();
                Err(message)
            }
        }
    }

    /// Gives a door its place in the game again.
    pub fn restore_prop(&mut self, index: usize) -> Result<(), String> {
        let original = self.original_props.get(index).cloned().ok_or("no such door")?;
        let mut props = self.props.clone();
        *props.get_mut(index).ok_or("no such door")? = original;
        self.set_props(props)
    }

    /// How many changes can be undone.
    pub fn undo_depth(&self) -> usize {
        self.undo.len()
    }

    pub fn can_undo(&self) -> bool {
        !self.undo.is_empty()
    }

    pub fn can_redo(&self) -> bool {
        !self.redo.is_empty()
    }

    pub fn undo(&mut self) -> Result<(), String> {
        if let Some(before) = self.undo.pop() {
            let now = (std::mem::replace(&mut self.instances, before.0), std::mem::replace(&mut self.tiles, before.1));
            self.redo.push(now);
            self.rebuild()?;
        }
        Ok(())
    }

    pub fn redo(&mut self) -> Result<(), String> {
        if let Some(after) = self.redo.pop() {
            let now = (std::mem::replace(&mut self.instances, after.0), std::mem::replace(&mut self.tiles, after.1));
            self.undo.push(now);
            self.rebuild()?;
        }
        Ok(())
    }

    /// The size of the grid of tiles: columns and rows.
    pub fn grid(&self) -> (usize, usize) {
        self.grid
    }

    /// Starts a stroke of the brush. All the tiles that the brush changes
    /// until `end_stroke` make one step for the undo.
    pub fn begin_stroke(&mut self) {
        if self.stroke.is_none() {
            self.stroke = Some((self.instances.clone(), self.tiles.clone()));
        }
    }

    /// Gives a value to the tiles of a square. Returns true when a tile
    /// changed. The size is the side of the square, in tiles.
    pub fn paint(&mut self, column: usize, row: usize, size: usize, value: u32) -> bool {
        let (width, height) = self.grid;
        let mut changed = false;
        let first = size.saturating_sub(1) / 2;
        for r in row.saturating_sub(first)..(row + size - first).min(height) {
            for c in column.saturating_sub(first)..(column + size - first).min(width) {
                if self.tiles[r * width + c] != value {
                    self.tiles[r * width + c] = value;
                    changed = true;
                }
            }
        }
        if changed {
            let _ = self.built.set_attributes(&Attributes { width, height, cells: self.tiles.clone() });
        }
        changed
    }

    pub fn end_stroke(&mut self) {
        if let Some(before) = self.stroke.take() {
            if before.1 != self.tiles {
                self.undo.push(before);
                self.redo.clear();
            }
        }
    }

    /// The names of the meshes that an object uses.
    pub fn cluster_materials(&self, cluster: usize) -> Vec<String> {
        let c = &self.clusters[cluster];
        let mut names: Vec<String> = match &c.foreign {
            Some(object) => object.parts.iter().map(|p| p.material.clone()).collect(),
            None => c.parts.iter().map(|(m, _)| self.original.meshes[*m].1.material.clone()).collect(),
        };
        names.dedup();
        names
    }

    /// An object, with the data of its vertices, to put it in another block.
    fn portable_of_cluster(&self, cluster: usize) -> Result<Portable, String> {
        let c = &self.clusters[cluster];
        if let Some(object) = &c.foreign {
            return Ok(object.clone());
        }
        let block = if self.from_archive { self.original.id } else { NO_ORIGIN };
        self.original.portable((block, cluster), &c.parts, c.low, c.high)
    }

    /// Says whether the block has, for each piece of an object, a mesh with
    /// the same shape of vertices (needed to add the piece).
    pub fn has_layouts(&self, object: &Portable) -> bool {
        object.parts.iter().all(|p| self.original.meshes.iter().any(|(part, m)| *part == 1 && m.places.layout == p.layout))
    }

    /// Says whether this block can take an object from a block.
    pub fn can_import(&self, object: &Portable) -> Result<(), String> {
        self.original.can_host(object)
    }

    /// For the checks of the tool: the object with this number.
    pub fn portable_of_cluster_for_test(&self, cluster: usize) -> Result<Portable, String> {
        self.portable_of_cluster(cluster)
    }

    /// Several objects as one: the pieces that use the same mesh are joined.
    /// The object is as it is in the original block. It cannot be found again
    /// from a project, so its origin is not a block of the archive.
    pub fn portable_of_many(&self, instances: &[usize]) -> Result<Portable, String> {
        let mut joined: Option<Portable> = None;
        for &i in instances {
            let object = self.portable_of(i)?;
            let Some(all) = joined.as_mut() else {
                joined = Some(object);
                continue;
            };
            for a in 0..3 {
                all.low[a] = all.low[a].min(object.low[a]);
                all.high[a] = all.high[a].max(object.high[a]);
            }
            for piece in object.parts {
                match all.parts.iter_mut().find(|p| p.material == piece.material && p.layout == piece.layout) {
                    Some(target) => {
                        let first = (target.vertices.len() / target.stride) as u32;
                        target.vertices.extend_from_slice(&piece.vertices);
                        target.indices.extend(piece.indices.iter().map(|&i| first + i));
                    }
                    None => all.parts.push(piece),
                }
            }
            all.origin = (NO_ORIGIN, 0);
        }
        joined.ok_or_else(|| "nothing is selected".to_string())
    }

    /// The instances whose centre is in the box of the selection, seen from above (with a gap):
    /// the pieces of a building that the cutting into objects left apart.
    pub fn neighbours(&self, selection: &[usize], gap: f32) -> Vec<usize> {
        let Some((mut low, mut high)) = selection.iter().map(|&i| self.instance_bounds(i)).reduce(|a, b| {
            let (mut low, mut high) = a;
            for axis in 0..3 {
                low[axis] = low[axis].min(b.0[axis]);
                high[axis] = high[axis].max(b.1[axis]);
            }
            (low, high)
        }) else {
            return Vec::new();
        };
        low[0] -= gap;
        low[2] -= gap;
        high[0] += gap;
        high[2] += gap;
        (0..self.instances.len())
            .filter(|i| !selection.contains(i) && !self.instances[*i].deleted)
            .filter(|&i| {
                // The centre of the piece is in the box of the selection. A
                // large decoration of the ground (a border of grass) has a box
                // that touches many buildings, but its centre is elsewhere.
                let (l, h) = self.instance_bounds(i);
                let (cx, cz) = ((l[0] + h[0]) / 2.0, (l[2] + h[2]) / 2.0);
                cx >= low[0] && cx <= high[0] && cz >= low[2] && cz <= high[2]
            })
            .collect()
    }

    /// The object of an instance, as it is in the original block.
    pub fn portable_of(&self, instance: usize) -> Result<Portable, String> {
        let i = &self.instances[instance];
        if i.only.is_none() || self.clusters[i.cluster].foreign.is_some() {
            return self.portable_of_cluster(i.cluster);
        }
        // A part of a piece cannot be found again from a project.
        let parts: Vec<(usize, Vec<usize>)> = self.active_parts(i).into_iter().cloned().collect();
        let (low, high) = self.base_bounds(i);
        self.original.portable((NO_ORIGIN, i.cluster), &parts, low, high)
    }

    /// Puts an object that comes from a block in this block, as a copy at the
    /// place where it was. Returns the number of the new instance. The block
    /// must have the meshes that the object uses, with the same vertices.
    pub fn import(&mut self, object: Portable) -> Result<usize, String> {
        let cluster = self.clusters.len();
        self.clusters.push(Cluster { parts: Vec::new(), low: object.low, high: object.high, foreign: Some(object) });
        let result = self.change(|v| v.push(Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: true, deleted: false, only: None, replaces: None }));
        if let Err(message) = result {
            self.clusters.pop();
            return Err(message);
        }
        Ok(self.instances.len() - 1)
    }

    /// Puts a ground tile in the tile (column, row): the ground that was
    /// there is removed and the new piece takes its place. A tile that was
    /// put there before is taken away. `ground` is the height to give to the
    /// middle of the piece (None: the height it had).
    pub fn stamp_tile(&mut self, mut object: Portable, column: usize, row: usize, ground: Option<f32>) -> Result<usize, String> {
        // The tall grass is cut in bands of five tiles, one mesh for each band.
        // A band that the block does not have is replaced by the nearest band that it has.
        if object.parts.iter().any(|p| is_grass_material(&p.material)) {
            let want = row / 5;
            let bands: Vec<(usize, String)> = self
                .original
                .meshes
                .iter()
                .filter(|(part, mesh)| *part == GRASS_PART && is_grass_material(&mesh.material))
                .map(|(_, mesh)| (mesh.material[3..4].parse::<usize>().unwrap_or(0), mesh.material.clone()))
                .collect();
            let Some((_, name)) = bands.iter().min_by_key(|(band, _)| band.abs_diff(want)) else {
                return Err("this block has no tall grass model, and the tool cannot make one yet".into());
            };
            for part in object.parts.iter_mut().filter(|p| is_grass_material(&p.material)) {
                part.material = name.clone();
            }
        }
        let new_grass = grass_only(&object);
        // The grass has to go in a mesh of grass that the block has. A tile of ground that the
        // block cannot host gets its own meshes (as an object from another block does).
        if new_grass && !object.parts.is_empty() {
            self.original.can_host(&object)?;
        }
        let cluster = self.clusters.len();
        let (low, high) = (object.low, object.high);
        self.clusters.push(Cluster { parts: Vec::new(), low, high, foreign: Some(object) });
        let source = (
            ((low[0] + high[0]) / 2.0 + 360.0).div_euclid(18.0),
            ((low[2] + high[2]) / 2.0 + 360.0).div_euclid(18.0),
        );
        let delta = [
            18.0 * (column as f32 - source.0),
            ground.map_or(0.0, |g| g - if new_grass { low[1] } else { (low[1] + high[1]) / 2.0 }),
            18.0 * (row as f32 - source.1),
        ];
        // A tile of ground takes away what was put on the tile; the grass takes away only the grass.
        let grassy: Vec<bool> = (0..self.clusters.len()).map(|c| self.cluster_is_grass(c)).collect();
        let result = self.change(|v| {
            for i in v.iter_mut() {
                if i.copy && i.replaces == Some((column, row)) && (!new_grass || grassy[i.cluster]) {
                    i.deleted = true;
                }
            }
            v.push(Instance { cluster, offset: delta, yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((column, row)) });
        });
        match result {
            Ok(()) => Ok(self.instances.len() - 1),
            Err(message) => {
                self.clusters.pop();
                Err(message)
            }
        }
    }

    /// Takes all the tall grass of the block away (one step for the undo).
    pub fn clear_grass(&mut self) -> Result<(), String> {
        let (width, height) = self.grid;
        let cluster = self.clusters.len();
        let nothing = Portable { origin: (crate::edit::NO_ORIGIN, 0), low: [0.0; 3], high: [0.0; 3], parts: Vec::new() };
        self.clusters.push(Cluster { parts: Vec::new(), low: [0.0; 3], high: [0.0; 3], foreign: Some(nothing) });
        let result = self.change(|v| {
            for row in 0..height {
                for column in 0..width {
                    v.push(Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((column, row)) });
                }
            }
        });
        if result.is_err() {
            self.clusters.pop();
        }
        result
    }

    /// True when the cluster is made of tall grass only (or of nothing).
    fn cluster_is_grass(&self, cluster: usize) -> bool {
        self.clusters[cluster].foreign.as_ref().map_or(false, |o| grass_only(o) && o.origin != LEDGE_ERASER && o.origin != FLATTEN_MARK && o.origin != RAISE_MARK)
    }

    fn cluster_is_raise(&self, cluster: usize) -> bool {
        self.clusters[cluster].foreign.as_ref().map_or(false, |o| o.parts.is_empty() && o.origin == RAISE_MARK)
    }

    /// Lifts (or lowers, with a negative `delta`) corners (column, row) of the tiles of the
    /// ground, 0 to 40 on each side. The ground around follows (the tiles slope). The change adds
    /// up with the earlier ones.
    pub fn raise_corners(&mut self, corners: &[(usize, usize)], delta: f32) -> Result<(), String> {
        let corners: Vec<(usize, usize)> = corners.iter().copied().filter(|&(c, r)| c <= self.grid.0 && r <= self.grid.1).collect();
        if corners.is_empty() {
            return Err("these corners are outside of the block".into());
        }
        let cluster = match (0..self.clusters.len()).find(|&c| self.cluster_is_raise(c)) {
            Some(c) => c,
            None => {
                let nothing = Portable { origin: RAISE_MARK, low: [0.0; 3], high: [0.0; 3], parts: Vec::new() };
                self.clusters.push(Cluster { parts: Vec::new(), low: [0.0; 3], high: [0.0; 3], foreign: Some(nothing) });
                self.clusters.len() - 1
            }
        };
        self.change(|v| {
            for &(column, row) in &corners {
                if let Some(i) = v.iter_mut().find(|i| i.cluster == cluster && i.copy && !i.deleted && i.replaces == Some((column, row))) {
                    i.offset[1] += delta;
                } else {
                    v.push(Instance { cluster, offset: [0.0, delta, 0.0], yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((column, row)) });
                }
            }
        })
    }

    fn cluster_is_flatten(&self, cluster: usize) -> bool {
        self.clusters[cluster].foreign.as_ref().map_or(false, |o| o.parts.is_empty() && o.origin == FLATTEN_MARK)
    }

    /// Makes all the ground of the block flat (one step for the undo): the borders between the
    /// tiles that one can walk on and the others, and the slopes, go away. The objects stay where they are.
    pub fn flatten_ground(&mut self) -> Result<(), String> {
        if self.instances.iter().any(|i| i.copy && !i.deleted && self.cluster_is_flatten(i.cluster)) {
            return Ok(());
        }
        let cluster = self.clusters.len();
        let nothing = Portable { origin: FLATTEN_MARK, low: [0.0; 3], high: [0.0; 3], parts: Vec::new() };
        self.clusters.push(Cluster { parts: Vec::new(), low: [0.0; 3], high: [0.0; 3], foreign: Some(nothing) });
        let result = self.change(|v| v.push(Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((usize::MAX, 0)) }));
        if result.is_err() {
            self.clusters.pop();
        }
        result
    }

    /// True when the cluster only takes the ledge models away from the tile.
    fn cluster_is_ledge_eraser(&self, cluster: usize) -> bool {
        self.clusters[cluster].foreign.as_ref().map_or(false, |o| o.parts.is_empty() && o.origin == LEDGE_ERASER)
    }

    /// Takes the models of all the ledges of the block away (one step for the undo).
    pub fn clear_ledges(&mut self) -> Result<(), String> {
        let (width, height) = self.grid;
        let cluster = self.clusters.len();
        let nothing = Portable { origin: LEDGE_ERASER, low: [0.0; 3], high: [0.0; 3], parts: Vec::new() };
        self.clusters.push(Cluster { parts: Vec::new(), low: [0.0; 3], high: [0.0; 3], foreign: Some(nothing) });
        let result = self.change(|v| {
            for row in 0..height {
                for column in 0..width {
                    v.push(Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((column, row)) });
                }
            }
        });
        if result.is_err() {
            self.clusters.pop();
        }
        result
    }

    /// The tall grass of one tile, taken from this block: the piece that the
    /// grass tool puts on the ground. None when the block has no tall grass.
    pub fn grass_sample(&self) -> Option<Portable> {
        let mut found: Vec<(usize, CatalogObject)> = Vec::new();
        for object in self.tile_objects(true) {
            let parts: Vec<(usize, Vec<usize>)> =
                object.parts.iter().filter(|(m, _)| self.original.meshes[*m].0 == GRASS_PART).cloned().collect();
            if parts.is_empty() {
                continue;
            }
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            for (m, vertices) in &parts {
                let (l, h) = bounds(&self.original.meshes[*m].1, vertices);
                for a in 0..3 {
                    low[a] = low[a].min(l[a]);
                    high[a] = high[a].max(h[a]);
                }
            }
            let count = parts.iter().map(|(_, v)| v.len()).sum();
            found.push((count, CatalogObject { parts, low, high }));
        }
        // The tile that most tiles look like (the ones at the edge have less).
        let mut counts: std::collections::HashMap<usize, usize> = Default::default();
        for (n, _) in &found {
            *counts.entry(*n).or_default() += 1;
        }
        let best = counts.into_iter().max_by_key(|&(n, times)| (times, n))?.0;
        let index = found.iter().position(|(n, _)| *n == best)?;
        self.portable_of_catalog(&found[index].1, index).ok()
    }

    /// Says whether this block can take `count` copies of a tile: the tiles must go in meshes
    /// that the block has, and that can hold all of them (else each tile would need its own mesh).
    pub fn fill_check(&self, object: &Portable, count: usize) -> Result<(), String> {
        let _ = count;
        if !self.has_layouts(object) {
            return Err("this block has no mesh with the vertices of this tile: choose a tile that the catalogue shows with \"Only the models that this block can take\"".into());
        }
        Ok(())
    }

    /// Puts the same tile of ground on many tiles at once (one step for the undo).
    /// `cells` are the tiles (column, row) with the height of the ground in each.
    pub fn stamp_tiles(&mut self, object: Portable, cells: &[(usize, usize, f32)]) -> Result<usize, String> {
        if cells.is_empty() {
            return Err("no tile to fill".into());
        }
        if grass_only(&object) {
            return Err("the grass is put with the grass tool".into());
        }
        self.fill_check(&object, cells.len())?;
        let cluster = self.clusters.len();
        let (low, high) = (object.low, object.high);
        self.clusters.push(Cluster { parts: Vec::new(), low, high, foreign: Some(object) });
        let source = (((low[0] + high[0]) / 2.0 + 360.0).div_euclid(18.0), ((low[2] + high[2]) / 2.0 + 360.0).div_euclid(18.0));
        let middle = (low[1] + high[1]) / 2.0;
        let wanted: std::collections::HashSet<(usize, usize)> = cells.iter().map(|&(c, r, _)| (c, r)).collect();
        let result = self.change(|v| {
            for i in v.iter_mut() {
                if i.copy && i.replaces.is_some_and(|cell| wanted.contains(&cell)) {
                    i.deleted = true;
                }
            }
            for &(column, row, ground) in cells {
                let offset = [18.0 * (column as f32 - source.0), ground - middle, 18.0 * (row as f32 - source.1)];
                v.push(Instance { cluster, offset, yaw: 0.0, copy: true, deleted: false, only: None, replaces: Some((column, row)) });
            }
        });
        match result {
            Ok(()) => Ok(cells.len()),
            Err(message) => {
                self.clusters.pop();
                Err(message)
            }
        }
    }

    /// Puts tall grass on the tile (column, row), or takes the grass of the
    /// tile away (`sample` None). `ground` is the height of the ground.
    pub fn stamp_grass(&mut self, sample: Option<&Portable>, column: usize, row: usize, ground: f32) -> Result<usize, String> {
        let object = match sample {
            Some(sample) => sample.clone(),
            None => {
                let (x, z) = (-360.0 + 18.0 * column as f32 + 9.0, -360.0 + 18.0 * row as f32 + 9.0);
                Portable { origin: (crate::edit::NO_ORIGIN, 0), low: [x - 9.0, ground, z - 9.0], high: [x + 9.0, ground, z + 9.0], parts: Vec::new() }
            }
        };
        self.stamp_tile(object, column, row, Some(ground))
    }

    /// The ground of each tile: the triangles that are inside the tile, in
    /// all the meshes of the ground. A tile with pieces in several meshes
    /// (the grass and a path over it) is one object.
    pub fn tile_objects(&self, with_grass: bool) -> Vec<CatalogObject> {
        type Cell = (i32, i32);
        let mut per_tile: std::collections::BTreeMap<Cell, Vec<(usize, std::collections::BTreeSet<usize>)>> = Default::default();
        for (index, (part, mesh)) in self.original.meshes.iter().enumerate() {
            let grass = *part == GRASS_PART;
            if (*part != 1 && !(grass && with_grass)) || self.selectable[index] {
                continue;
            }
            let mut cells: std::collections::BTreeMap<Cell, std::collections::BTreeSet<usize>> = Default::default();
            for t in mesh.indices.chunks_exact(3) {
                let Some(p) = t.iter().map(|&k| mesh.positions.get(k as usize).copied()).collect::<Option<Vec<_>>>() else { continue };
                let cell = |v: f32| ((v + 360.0) / 18.0).floor() as i32;
                let home = (cell((p[0][0] + p[1][0] + p[2][0]) / 3.0), cell((p[0][2] + p[1][2] + p[2][2]) / 3.0));
                let (x0, z0) = (-360.0 + 18.0 * home.0 as f32, -360.0 + 18.0 * home.1 as f32);
                let fits = p.iter().all(|q| q[0] >= x0 - 0.05 && q[0] <= x0 + 18.05 && q[2] >= z0 - 0.05 && q[2] <= z0 + 18.05);
                // The tufts of the grass can go over the edge of their tile.
                if fits || grass {
                    cells.entry(home).or_default().extend(t.iter().map(|&k| k as usize));
                }
            }
            for (cell, vertices) in cells {
                per_tile.entry(cell).or_default().push((index, vertices));
            }
        }
        let mut found = Vec::new();
        for (_, pieces) in per_tile {
            let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
            let mut parts = Vec::new();
            for (index, vertices) in pieces {
                let vertices: Vec<usize> = vertices.into_iter().collect();
                let (l, h) = bounds(&self.original.meshes[index].1, &vertices);
                for a in 0..3 {
                    low[a] = low[a].min(l[a]);
                    high[a] = high[a].max(h[a]);
                }
                parts.push((index, vertices));
            }
            found.push(CatalogObject { parts, low, high });
        }
        found
    }

    /// The objects that a catalogue can show: the whole objects that are made of
    /// decoration meshes (trees, bushes: `chip_...`), and the buildings, that
    /// is the pieces of a family of meshes that touch each other. Each object
    /// is a list of meshes with the vertices of each, and a box.
    pub fn catalog_objects(&self) -> Vec<CatalogObject> {
        const GAP: f32 = 6.0;
        let mut found: Vec<CatalogObject> = Vec::new();
        let is_chip = |mesh: usize| self.original.meshes[mesh].1.material.starts_with("chip_");

        // The trees, the bushes: the objects of the first cutting.
        for (index, instance) in self.instances.iter().enumerate().take(self.native) {
            let parts = self.active_parts(instance);
            if parts.is_empty() || !parts.iter().all(|(m, _)| is_chip(*m)) {
                continue;
            }
            let _ = index;
            let (low, high) = self.base_bounds(instance);
            found.push(CatalogObject { parts: parts.into_iter().cloned().collect(), low, high });
        }

        // The buildings: for each family of meshes, the pieces that touch each other.
        let mut families: Vec<Vec<usize>> = Vec::new();
        for mesh in 0..self.original.meshes.len() {
            if !self.selectable[mesh] || is_chip(mesh) {
                continue;
            }
            if !families.iter().any(|f| f.contains(&mesh)) {
                families.push(self.family_meshes(mesh));
            }
        }
        for family in families {
            let candidates: Vec<(usize, ([f32; 3], [f32; 3]))> = (0..self.native)
                .filter_map(|i| self.box_in_meshes(i, &family).map(|b| (i, b)))
                .collect();
            let mut used = vec![false; candidates.len()];
            for start in 0..candidates.len() {
                if used[start] {
                    continue;
                }
                used[start] = true;
                let mut component = vec![start];
                let mut next = 0;
                while next < component.len() {
                    let current = candidates[component[next]].1;
                    next += 1;
                    for (k, (_, b)) in candidates.iter().enumerate() {
                        let touches = current.0[0] - GAP <= b.1[0]
                            && b.0[0] - GAP <= current.1[0]
                            && current.0[2] - GAP <= b.1[2]
                            && b.0[2] - GAP <= current.1[2];
                        if !used[k] && touches {
                            used[k] = true;
                            component.push(k);
                        }
                    }
                }
                // The vertices of the pieces, in the meshes of the family only.
                let mut parts: Vec<(usize, Vec<usize>)> = Vec::new();
                let (mut low, mut high) = ([f32::MAX; 3], [f32::MIN; 3]);
                for &k in &component {
                    let instance = &self.instances[candidates[k].0];
                    for (mesh, vertices) in self.active_parts(instance) {
                        if !family.contains(mesh) {
                            continue;
                        }
                        let (l, h) = bounds(&self.original.meshes[*mesh].1, vertices);
                        for a in 0..3 {
                            low[a] = low[a].min(l[a]);
                            high[a] = high[a].max(h[a]);
                        }
                        match parts.iter_mut().find(|(m, _)| m == mesh) {
                            Some((_, existing)) => existing.extend(vertices),
                            None => parts.push((*mesh, vertices.clone())),
                        }
                    }
                }
                found.push(CatalogObject { parts, low, high });
            }
        }
        found
    }

    /// The shape of a catalogue object, to draw it in a list. The vertices of
    /// each part are in the same order as in `portable_of_catalog`.
    pub fn preview_of(&self, object: &CatalogObject) -> Vec<PreviewPart> {
        let mut parts = Vec::new();
        for (index, vertices) in &object.parts {
            let mesh = &self.original.meshes[*index].1;
            let chosen: std::collections::BTreeSet<usize> = vertices.iter().copied().collect();
            let local: std::collections::HashMap<usize, u32> =
                chosen.iter().enumerate().map(|(k, &v)| (v, k as u32)).collect();
            let mut indices = Vec::new();
            for t in mesh.indices.chunks_exact(3) {
                if t.iter().all(|&i| local.contains_key(&(i as usize))) {
                    indices.extend(t.iter().map(|&i| local[&(i as usize)]));
                }
            }
            parts.push(PreviewPart {
                picture: crate::data::picture_of(mesh).to_string(),
                positions: chosen.iter().map(|&v| mesh.positions.get(v).copied().unwrap_or([0.0; 3])).collect(),
                uvs: chosen.iter().map(|&v| mesh.uvs.get(v).copied().unwrap_or([0.0; 2])).collect(),
                indices,
            });
        }
        parts
    }

    /// The picture of some objects of the block (None when one of them was
    /// imported from another block: it has no model in this block to draw).
    pub fn preview_of_instances(&self, instances: &[usize], low: [f32; 3], high: [f32; 3]) -> Option<Vec<PreviewPart>> {
        let mut parts: Vec<(usize, Vec<usize>)> = Vec::new();
        for &i in instances {
            let instance = self.instances.get(i)?;
            if self.clusters[instance.cluster].foreign.is_some() {
                return None;
            }
            parts.extend(self.active_parts(instance).into_iter().cloned());
        }
        Some(self.preview_of(&CatalogObject { parts, low, high }))
    }

    /// The data of a catalogue object, to put it in another block.
    pub fn portable_of_catalog(&self, object: &CatalogObject, index: usize) -> Result<Portable, String> {
        // A catalogue object is not an object of the block, so a project cannot find it again.
        let _ = index;
        self.original.portable((NO_ORIGIN, 0), &object.parts, object.low, object.high)
    }

    /// The number of the block.
    pub fn block_id(&self) -> usize {
        self.original.id
    }

    /// The edit as text: what moved, what was deleted, the copies and the
    /// tiles that changed. The file has no data of the game.
    pub fn project_text(&self) -> Result<String, String> {
        use std::fmt::Write as _;
        let mut out = String::new();
        let _ = writeln!(out, "sango-editor-project 1");
        let _ = writeln!(out, "block {}", self.original.id);
        let _ = writeln!(out, "objects {}", self.native);
        // The objects from other blocks: the project says where to find them.
        for cluster in &self.clusters[self.native..] {
            let Some(object) = &cluster.foreign else { continue };
            if object.origin.0 == NO_ORIGIN {
                return Err("an object comes from a block that was not in the archive, so the project cannot find it again".into());
            }
            let _ = writeln!(out, "import {} {}", object.origin.0, object.origin.1);
        }
        for instance in &self.instances {
            let [x, y, z] = instance.offset;
            let yaw = instance.yaw;
            if let Some(meshes) = &instance.only {
                // A part of a piece: the meshes that it holds, as a list.
                let list: Vec<String> = meshes.iter().map(|m| m.to_string()).collect();
                let list = if list.is_empty() { "-".to_string() } else { list.join(",") };
                if instance.copy {
                    let _ = writeln!(out, "mcopy {} {list} {x} {y} {z} {} {yaw}", instance.cluster, u8::from(instance.deleted));
                } else if instance.deleted {
                    let _ = writeln!(out, "mdelete {} {list}", instance.cluster);
                } else if instance.offset != [0.0; 3] || yaw != 0.0 {
                    let _ = writeln!(out, "mmove {} {list} {x} {y} {z} {yaw}", instance.cluster);
                }
                continue;
            }
            if instance.copy {
                let _ = writeln!(out, "copy {} {x} {y} {z} {} {yaw}", instance.cluster, u8::from(instance.deleted));
            } else if instance.deleted {
                let _ = writeln!(out, "delete {}", instance.cluster);
            } else if instance.offset != [0.0; 3] || yaw != 0.0 {
                let _ = writeln!(out, "move {} {x} {y} {z} {yaw}", instance.cluster);
            }
        }
        for (i, (now, before)) in self.props.iter().zip(&self.original_props).enumerate() {
            if now != before {
                let _ = writeln!(out, "door {i} {} {} {} {}", now.x, now.y, now.z, now.kind);
            }
        }
        for (i, (now, before)) in self.tiles.iter().zip(&self.original_tiles).enumerate() {
            if now != before {
                let _ = writeln!(out, "tile {i} {now:08x}");
            }
        }
        Ok(out)
    }

    /// Takes the edit from a text that `project_text` wrote. The undo list
    /// starts again. When the text does not fit this block, nothing changes.
    pub fn load_project(&mut self, text: &str) -> Result<(), String> {
        let mut lines = text.lines();
        if !matches!(lines.next(), Some("sango-editor-project 1") | Some("map-studio-project 1")) {
            return Err("this is not a project of Sango Editor".into());
        }
        let mut instances: Vec<Instance> = (0..self.native)
            .map(|cluster| Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: false, deleted: false, only: None, replaces: None })
            .collect();
        let mut tiles = self.original_tiles.clone();
        let mut props = self.original_props.clone();
        let mut imported: Vec<Cluster> = Vec::new();
        // The parts of pieces: (kind, cluster, meshes, offset, yaw, deleted).
        let mut masked: Vec<(String, usize, Vec<usize>, [f32; 3], f32, bool)> = Vec::new();
        let number = |word: Option<&str>| -> Result<f32, String> {
            word.and_then(|w| w.parse().ok()).ok_or_else(|| "a number is wrong in the project".to_string())
        };
        for line in lines {
            let mut words = line.split_whitespace();
            match words.next() {
                Some("block") => {
                    if number(words.next())? as usize != self.original.id {
                        return Err("this project is for another block".into());
                    }
                }
                Some("objects") => {
                    let count = number(words.next())? as usize;
                    if count != self.native {
                        return Err(format!(
                            "the project has {count} objects and this block has {}: the objects were cut another way",
                            self.native
                        ));
                    }
                }
                Some("import") => {
                    let (block, cluster) = (number(words.next())? as usize, number(words.next())? as usize);
                    let source = Editor::new(&self.arc_dir, block)?;
                    if cluster >= source.native {
                        return Err("an object to import does not exist".into());
                    }
                    let object = source.portable_of_cluster(cluster)?;
                    self.original.can_host(&object)?;
                    imported.push(Cluster { parts: Vec::new(), low: object.low, high: object.high, foreign: Some(object) });
                }
                Some(kind @ ("move" | "delete" | "copy")) => {
                    let cluster = number(words.next())? as usize;
                    if cluster >= self.native + imported.len() {
                        return Err("an object of the project does not exist".into());
                    }
                    if kind == "delete" {
                        instances[cluster].deleted = true;
                        continue;
                    }
                    let offset = [number(words.next())?, number(words.next())?, number(words.next())?];
                    if kind == "move" {
                        instances[cluster].offset = offset;
                        // A project of an older version has no turn.
                        instances[cluster].yaw = words.next().and_then(|w| w.parse().ok()).unwrap_or(0.0);
                    } else {
                        let deleted = number(words.next())? != 0.0;
                        let yaw = words.next().and_then(|w| w.parse().ok()).unwrap_or(0.0);
                        instances.push(Instance { cluster, offset, yaw, copy: true, deleted, only: None, replaces: None });
                    }
                }
                Some(kind @ ("mmove" | "mdelete" | "mcopy")) => {
                    let cluster = number(words.next())? as usize;
                    if cluster >= self.native + imported.len() {
                        return Err("an object of the project does not exist".into());
                    }
                    let list = words.next().ok_or("a list of meshes is missing in the project")?;
                    let meshes: Vec<usize> = if list == "-" {
                        Vec::new()
                    } else {
                        list.split(',').map(|m| m.parse().map_err(|_| "a mesh is wrong in the project".to_string())).collect::<Result<_, _>>()?
                    };
                    let (mut offset, mut yaw, mut deleted) = ([0.0; 3], 0.0, kind == "mdelete");
                    if kind != "mdelete" {
                        offset = [number(words.next())?, number(words.next())?, number(words.next())?];
                        if kind == "mcopy" {
                            deleted = number(words.next())? != 0.0;
                        }
                        yaw = words.next().and_then(|w| w.parse().ok()).unwrap_or(0.0);
                    }
                    masked.push((kind.to_string(), cluster, meshes, offset, yaw, deleted));
                }
                Some("door") => {
                    let index = number(words.next())? as usize;
                    let door = props.get_mut(index).ok_or("a door of the project does not exist")?;
                    door.x = number(words.next())?;
                    door.y = number(words.next())?;
                    door.z = number(words.next())?;
                    if let Some(kind) = words.next().and_then(|w| w.parse().ok()) {
                        door.kind = kind;
                    }
                }
                Some("tile") => {
                    let index = number(words.next())? as usize;
                    let value = words.next().and_then(|w| u32::from_str_radix(w, 16).ok()).ok_or("a tile is wrong")?;
                    *tiles.get_mut(index).ok_or("a tile of the project does not exist")? = value;
                }
                Some(_) | None => {}
            }
        }
        // The parts of pieces. The instance of a piece keeps the meshes that no
        // line takes, and each line makes a new instance.
        let mut taken: std::collections::BTreeMap<usize, Vec<usize>> = Default::default();
        for (kind, cluster, meshes, ..) in &masked {
            if kind != "mcopy" {
                taken.entry(*cluster).or_default().extend(meshes);
            }
        }
        for (cluster, meshes) in &taken {
            if *cluster >= self.native {
                return Err("a part of an object that does not come from this block".into());
            }
            let all: Vec<usize> = self.clusters[*cluster].parts.iter().map(|p| p.0).collect();
            instances[*cluster].only = Some(all.into_iter().filter(|m| !meshes.contains(m)).collect());
        }
        for (kind, cluster, meshes, offset, yaw, deleted) in masked {
            instances.push(Instance { cluster, offset, yaw, copy: kind == "mcopy", deleted, only: Some(meshes), replaces: None });
        }
        let before = (std::mem::replace(&mut self.instances, instances), std::mem::replace(&mut self.tiles, tiles));
        let old_doors = std::mem::replace(&mut self.props, props);
        let old_imported = self.clusters.split_off(self.native);
        self.clusters.extend(imported);
        if let Err(message) = self.rebuild() {
            (self.instances, self.tiles) = before;
            self.props = old_doors;
            self.clusters.truncate(self.native);
            self.clusters.extend(old_imported);
            let _ = self.rebuild();
            return Err(message);
        }
        self.undo.clear();
        self.redo.clear();
        self.stroke = None;
        Ok(())
    }

    /// Tells the editor which tileset pack the block uses.
    pub fn set_tileset(&mut self, tileset: Option<usize>) {
        self.tileset = tileset;
    }

    /// Keeps pictures that an imported object needs.
    pub fn provide_pictures(&mut self, pictures: Vec<crate::bch_edit::TextureBlob>) {
        for p in pictures {
            if !self.pictures.iter().any(|q| q.name == p.name) {
                self.pictures.push(p);
            }
        }
    }

    /// Writes the block in the folder `land` of a folder of the SD card. When
    /// imported objects need pictures that the tileset of the block does not
    /// have, the tileset is written too, in the folder `area`.
    pub fn export(&self, out_dir: &str) -> Result<String, String> {
        let path = self.built.save(Path::new(out_dir).to_str().unwrap_or(out_dir))?;
        let size = std::fs::metadata(&path).map(|m| m.len()).unwrap_or(0);
        let mut message = format!("written {} ({} bytes)", path.display(), size);
        if let (Some(tileset), false) = (self.tileset, self.pictures.is_empty()) {
            let (pack, compressed) = crate::data::tileset_pack(&self.arc_dir, tileset)?;
            let (_, parts) = crate::pack::split(&pack)?;
            let part = parts.get(crate::data::TILESET_PICTURES).ok_or("the tileset has no pictures")?;
            let wanted: Vec<crate::bch_edit::TextureBlob> = self.pictures.clone();
            let bigger = crate::bch_edit::add_textures(part, &wanted)?;
            if bigger.len() != part.len() {
                let mut padded = bigger;
                padded.resize(padded.len().div_ceil(crate::pack::ALIGNMENT) * crate::pack::ALIGNMENT, 0);
                let made = crate::pack::replace_part(&pack, crate::data::TILESET_PICTURES, &padded)?;
                let bytes = if compressed { crate::lz11::compress(&made)? } else { made };
                let folder = Path::new(out_dir).join("area");
                std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
                std::fs::write(folder.join(format!("{tileset}.bin")), &bytes).map_err(|e| e.to_string())?;
                crate::edit::write_index(&folder, tileset, bytes.len())?;
                message += &format!("; tileset {tileset} written with the new pictures ({} bytes{})", bytes.len(), if bytes.len() > 860_000 { ", maybe too big for the game" } else { "" });
            }
        }
        Ok(message)
    }
}
