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
    /// The area pack of the block, and the pictures that imported objects
    /// bring (the area gets those that it does not have, when the block is exported).
    area: Option<usize>,
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
            tiles: attributes.cells,
            grid: (attributes.width, attributes.height),
            undo: Vec::new(),
            redo: Vec::new(),
            stroke: None,
            owner: Vec::new(),
            built_origin: Vec::new(),
            area: None,
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
        let cells: std::collections::BTreeSet<(usize, usize)> = self
            .instances
            .iter()
            .filter(|i| i.copy && !i.deleted)
            .filter_map(|i| i.replaces)
            .collect();
        for (column, row) in cells {
            for mesh in 0..block.meshes.len() {
                if block.meshes[mesh].0 == 1 && !self.selectable[mesh] {
                    block.delete_triangles_in_tile(mesh, column, row)?;
                }
            }
        }
        let all_copies: Vec<usize> =
            (0..self.instances.len()).filter(|&i| self.instances[i].copy && !self.instances[i].deleted).collect();
        // An object from another block goes in the meshes of this block when it
        // can. Else it gets new meshes, and its materials, after the other changes.
        let (copies, fresh): (Vec<usize>, Vec<usize>) = all_copies.into_iter().partition(|&i| {
            self.clusters[self.instances[i].cluster].foreign.as_ref().map_or(true, |o| self.original.can_host(o).is_ok())
        });
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
        for &index in &fresh {
            let instance = &self.instances[index];
            let Some(object) = self.clusters[instance.cluster].foreign.clone() else { continue };
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
        block.set_attributes(&Attributes { width: self.grid.0, height: self.grid.1, cells: self.tiles.clone() })?;
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

    /// The instances that have vertices in a mesh of the block, as the game
    /// reads it: all the pieces of a building that has its own meshes.
    pub fn instances_in_mesh(&self, mesh: usize) -> Vec<usize> {
        let Some((_, built)) = self.built.meshes.get(mesh) else { return Vec::new() };
        let mut found: Vec<usize> = Vec::new();
        for v in 0..built.positions.len() {
            if let Some(i) = self.owner(mesh, v) {
                if !self.instances[i].deleted && !found.contains(&i) {
                    found.push(i);
                }
            }
        }
        found.sort_unstable();
        found
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
    pub fn stamp_tile(&mut self, object: Portable, column: usize, row: usize, ground: Option<f32>) -> Result<usize, String> {
        let cluster = self.clusters.len();
        let (low, high) = (object.low, object.high);
        self.clusters.push(Cluster { parts: Vec::new(), low, high, foreign: Some(object) });
        let source = (
            ((low[0] + high[0]) / 2.0 + 360.0).div_euclid(18.0),
            ((low[2] + high[2]) / 2.0 + 360.0).div_euclid(18.0),
        );
        let delta = [
            18.0 * (column as f32 - source.0),
            ground.map_or(0.0, |g| g - (low[1] + high[1]) / 2.0),
            18.0 * (row as f32 - source.1),
        ];
        let result = self.change(|v| {
            for i in v.iter_mut() {
                if i.copy && i.replaces == Some((column, row)) {
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

    /// The ground of each tile: the triangles that are inside the tile, in
    /// all the meshes of the ground. A tile with pieces in several meshes
    /// (the grass and a path over it) is one object.
    pub fn tile_objects(&self) -> Vec<CatalogObject> {
        type Cell = (i32, i32);
        let mut per_tile: std::collections::BTreeMap<Cell, Vec<(usize, std::collections::BTreeSet<usize>)>> = Default::default();
        for (index, (part, mesh)) in self.original.meshes.iter().enumerate() {
            if *part != 1 || self.selectable[index] {
                continue;
            }
            let mut cells: std::collections::BTreeMap<Cell, std::collections::BTreeSet<usize>> = Default::default();
            for t in mesh.indices.chunks_exact(3) {
                let Some(p) = t.iter().map(|&k| mesh.positions.get(k as usize).copied()).collect::<Option<Vec<_>>>() else { continue };
                let cell = |v: f32| ((v + 360.0) / 18.0).floor() as i32;
                let home = (cell((p[0][0] + p[1][0] + p[2][0]) / 3.0), cell((p[0][2] + p[1][2] + p[2][2]) / 3.0));
                let (x0, z0) = (-360.0 + 18.0 * home.0 as f32, -360.0 + 18.0 * home.1 as f32);
                let fits = p.iter().all(|q| q[0] >= x0 - 0.05 && q[0] <= x0 + 18.05 && q[2] >= z0 - 0.05 && q[2] <= z0 + 18.05);
                if fits {
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
        let _ = writeln!(out, "map-studio-project 1");
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
        if lines.next() != Some("map-studio-project 1") {
            return Err("this is not a project of Map Studio".into());
        }
        let mut instances: Vec<Instance> = (0..self.native)
            .map(|cluster| Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: false, deleted: false, only: None, replaces: None })
            .collect();
        let mut tiles = self.original_tiles.clone();
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
        let old_imported = self.clusters.split_off(self.native);
        self.clusters.extend(imported);
        if let Err(message) = self.rebuild() {
            (self.instances, self.tiles) = before;
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

    /// Tells the editor which area pack the block uses.
    pub fn set_area(&mut self, area: Option<usize>) {
        self.area = area;
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
    /// imported objects need pictures that the area of the block does not
    /// have, the area is written too, in the folder `area`.
    pub fn export(&self, out_dir: &str) -> Result<String, String> {
        let path = self.built.save(Path::new(out_dir).to_str().unwrap_or(out_dir))?;
        let size = std::fs::metadata(&path).map(|m| m.len()).unwrap_or(0);
        let mut message = format!("written {} ({} bytes)", path.display(), size);
        if let (Some(area), false) = (self.area, self.pictures.is_empty()) {
            let (pack, compressed) = crate::data::area_pack(&self.arc_dir, area)?;
            let (_, parts) = crate::pack::split(&pack)?;
            let part = parts.get(crate::data::AREA_PICTURES).ok_or("the area has no pictures")?;
            let wanted: Vec<crate::bch_edit::TextureBlob> = self.pictures.clone();
            let bigger = crate::bch_edit::add_textures(part, &wanted)?;
            if bigger.len() != part.len() {
                let mut padded = bigger;
                padded.resize(padded.len().div_ceil(crate::pack::ALIGNMENT) * crate::pack::ALIGNMENT, 0);
                let made = crate::pack::replace_part(&pack, crate::data::AREA_PICTURES, &padded)?;
                let bytes = if compressed { crate::lz11::compress(&made)? } else { made };
                let folder = Path::new(out_dir).join("area");
                std::fs::create_dir_all(&folder).map_err(|e| format!("{}: {e}", folder.display()))?;
                std::fs::write(folder.join(format!("{area}.bin")), &bytes).map_err(|e| e.to_string())?;
                crate::edit::write_index(&folder, area, bytes.len())?;
                message += &format!("; area {area} written with the new pictures ({} bytes{})", bytes.len(), if bytes.len() > 860_000 { ", maybe too big for the game" } else { "" });
            }
        }
        Ok(message)
    }
}
