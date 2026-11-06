//! The state of an editing session, without any drawing.
//!
//! An object (a tree, a house) is a cluster of groups of vertices of the
//! original block. An edit is a list of instances: each instance shows a
//! cluster, at an offset from its original place. The block that the game
//! reads is built again from the original block after each change, so what
//! the window shows is what the export writes.

use std::path::Path;

use crate::bch::Mesh;
use crate::edit::{mesh_groups, CopyRequest, EditableBlock};

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

/// An object of the original block.
pub struct Cluster {
    /// The number of a mesh and the vertices of the object in this mesh.
    pub parts: Vec<(usize, Vec<usize>)>,
    pub low: [f32; 3],
    pub high: [f32; 3],
}

/// One object shown in the block. The first instances are the objects of the
/// original block, in the order of the clusters. A copy comes after them.
#[derive(Clone, PartialEq, Debug)]
pub struct Instance {
    pub cluster: usize,
    pub offset: [f32; 3],
    pub copy: bool,
    pub deleted: bool,
}

pub struct Editor {
    original: EditableBlock,
    pub clusters: Vec<Cluster>,
    /// True for each mesh that holds objects that the user can select.
    pub selectable: Vec<bool>,
    pub instances: Vec<Instance>,
    undo: Vec<Vec<Instance>>,
    redo: Vec<Vec<Instance>>,
    built: EditableBlock,
    /// For each vertex of each mesh of the built block: the instance that
    /// owns it, if any.
    owner: Vec<Vec<Option<usize>>>,
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
        Self::from_block(EditableBlock::load(arc_dir, block)?)
    }

    /// Starts from a block. The edits are measured from this block.
    pub fn from_block(original: EditableBlock) -> Result<Self, String> {
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
            clusters.push(Cluster { parts, low, high });
        }

        let instances = (0..clusters.len())
            .map(|cluster| Instance { cluster, offset: [0.0; 3], copy: false, deleted: false })
            .collect();
        let mut editor = Editor {
            built: original.clone(),
            original,
            clusters,
            selectable,
            instances,
            undo: Vec::new(),
            redo: Vec::new(),
            owner: Vec::new(),
        };
        editor.rebuild()?;
        Ok(editor)
    }

    /// The meshes of the block as the game will read them.
    pub fn meshes(&self) -> &[(usize, Mesh)] {
        &self.built.meshes
    }

    /// The instance that owns a vertex.
    pub fn owner(&self, mesh: usize, vertex: usize) -> Option<usize> {
        self.owner.get(mesh).and_then(|m| m.get(vertex)).copied().flatten()
    }

    /// The number of the mesh of a mesh in the original block, with its name.
    pub fn material(&self, mesh: usize) -> &str {
        &self.original.meshes[mesh].1.material
    }

    /// The box of an instance, with its offset.
    pub fn instance_bounds(&self, instance: usize) -> ([f32; 3], [f32; 3]) {
        let i = &self.instances[instance];
        let c = &self.clusters[i.cluster];
        let mut low = c.low;
        let mut high = c.high;
        for a in 0..3 {
            low[a] += i.offset[a];
            high[a] += i.offset[a];
        }
        (low, high)
    }

    /// Builds the block again from the original block and the instances.
    fn rebuild(&mut self) -> Result<(), String> {
        let mut block = self.original.clone();
        let copies: Vec<usize> =
            (0..self.instances.len()).filter(|&i| self.instances[i].copy && !self.instances[i].deleted).collect();
        let requests: Vec<CopyRequest> = copies
            .iter()
            .map(|&i| CopyRequest {
                source: self.clusters[self.instances[i].cluster].parts.clone(),
                delta: self.instances[i].offset,
            })
            .collect();
        let added = if requests.is_empty() { Vec::new() } else { block.copy_groups(&requests)? };

        // The objects of the original block: deleted or moved in place.
        for (index, instance) in self.instances.iter().enumerate() {
            if instance.copy {
                continue;
            }
            for (mesh, vertices) in &self.clusters[instance.cluster].parts {
                if instance.deleted {
                    block.delete_group(*mesh, vertices)?;
                } else if instance.offset != [0.0; 3] {
                    block.translate(*mesh, vertices, instance.offset)?;
                }
            }
            let _ = index;
        }

        let mut owner: Vec<Vec<Option<usize>>> = block.meshes.iter().map(|(_, m)| vec![None; m.positions.len()]).collect();
        for (index, instance) in self.instances.iter().enumerate() {
            if instance.copy {
                continue;
            }
            for (mesh, vertices) in &self.clusters[instance.cluster].parts {
                for &v in vertices {
                    owner[*mesh][v] = Some(index);
                }
            }
        }
        for (k, &index) in copies.iter().enumerate() {
            for &(mesh, first, count) in &added[k] {
                for v in first..first + count {
                    owner[mesh][v] = Some(index);
                }
            }
        }
        self.built = block;
        self.owner = owner;
        Ok(())
    }

    /// Changes the instances and builds the block again. When the block
    /// cannot be built, the change is cancelled.
    pub fn change(&mut self, change: impl FnOnce(&mut Vec<Instance>)) -> Result<(), String> {
        let before = self.instances.clone();
        change(&mut self.instances);
        if self.instances == before {
            return Ok(());
        }
        match self.rebuild() {
            Ok(()) => {
                self.undo.push(before);
                self.redo.clear();
                Ok(())
            }
            Err(message) => {
                self.instances = before;
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
            self.redo.push(std::mem::replace(&mut self.instances, before));
            self.rebuild()?;
        }
        Ok(())
    }

    pub fn redo(&mut self) -> Result<(), String> {
        if let Some(after) = self.redo.pop() {
            self.undo.push(std::mem::replace(&mut self.instances, after));
            self.rebuild()?;
        }
        Ok(())
    }

    /// Writes the block in the folder `land` of a folder of the SD card.
    pub fn export(&self, out_dir: &str) -> Result<String, String> {
        let path = self.built.save(Path::new(out_dir).to_str().unwrap_or(out_dir))?;
        let size = std::fs::metadata(&path).map(|m| m.len()).unwrap_or(0);
        Ok(format!("written {} ({} bytes)", path.display(), size))
    }
}
