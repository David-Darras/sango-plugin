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
            .map(|cluster| Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: false, deleted: false })
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

    /// The horizontal centre of an object of the original block.
    fn cluster_centre(&self, cluster: usize) -> [f32; 2] {
        let c = &self.clusters[cluster];
        [(c.low[0] + c.high[0]) / 2.0, (c.low[2] + c.high[2]) / 2.0]
    }

    /// The box of an instance, with its turn and its offset.
    pub fn instance_bounds(&self, instance: usize) -> ([f32; 3], [f32; 3]) {
        let i = &self.instances[instance];
        let c = &self.clusters[i.cluster];
        let centre = self.cluster_centre(i.cluster);
        let (sin, cos) = i.yaw.sin_cos();
        let mut low = [f32::MAX, c.low[1], f32::MAX];
        let mut high = [f32::MIN, c.high[1], f32::MIN];
        for (x, z) in [(c.low[0], c.low[2]), (c.low[0], c.high[2]), (c.high[0], c.low[2]), (c.high[0], c.high[2])] {
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

    /// Builds the block again from the original block and the instances.
    fn rebuild(&mut self) -> Result<(), String> {
        let mut block = self.original.clone();
        let copies: Vec<usize> =
            (0..self.instances.len()).filter(|&i| self.instances[i].copy && !self.instances[i].deleted).collect();
        let requests: Vec<CopyRequest> = copies
            .iter()
            .map(|&i| CopyRequest {
                source: self.clusters[self.instances[i].cluster].parts.clone(),
                foreign: self.clusters[self.instances[i].cluster].foreign.clone(),
                centre: self.cluster_centre(self.instances[i].cluster),
                yaw: self.instances[i].yaw,
                delta: self.instances[i].offset,
            })
            .collect();
        let added = if requests.is_empty() { Vec::new() } else { block.copy_groups(&requests)? };

        // The objects of the original block: deleted or moved in place.
        for (index, instance) in self.instances.iter().enumerate() {
            if instance.copy {
                continue;
            }
            let centre = self.cluster_centre(instance.cluster);
            for (mesh, vertices) in &self.clusters[instance.cluster].parts {
                if instance.deleted {
                    block.delete_group(*mesh, vertices)?;
                } else if instance.offset != [0.0; 3] || instance.yaw != 0.0 {
                    block.transform(*mesh, vertices, centre, instance.yaw, instance.offset)?;
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
        block.set_attributes(&Attributes { width: self.grid.0, height: self.grid.1, cells: self.tiles.clone() })?;
        self.built = block;
        self.owner = owner;
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

    /// Says whether this block can take an object from a block.
    pub fn can_import(&self, object: &Portable) -> Result<(), String> {
        self.original.can_host(object)
    }

    /// For the checks of the tool: the object with this number.
    pub fn portable_of_cluster_for_test(&self, cluster: usize) -> Result<Portable, String> {
        self.portable_of_cluster(cluster)
    }

    /// The object of an instance, as it is in the original block.
    pub fn portable_of(&self, instance: usize) -> Result<Portable, String> {
        self.portable_of_cluster(self.instances[instance].cluster)
    }

    /// Puts an object that comes from a block in this block, as a copy at the
    /// place where it was. Returns the number of the new instance. The block
    /// must have the meshes that the object uses, with the same vertices.
    pub fn import(&mut self, object: Portable) -> Result<usize, String> {
        self.original.can_host(&object)?;
        let cluster = self.clusters.len();
        self.clusters.push(Cluster { parts: Vec::new(), low: object.low, high: object.high, foreign: Some(object) });
        let result = self.change(|v| v.push(Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: true, deleted: false }));
        if let Err(message) = result {
            self.clusters.pop();
            return Err(message);
        }
        Ok(self.instances.len() - 1)
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
            .map(|cluster| Instance { cluster, offset: [0.0; 3], yaw: 0.0, copy: false, deleted: false })
            .collect();
        let mut tiles = self.original_tiles.clone();
        let mut imported: Vec<Cluster> = Vec::new();
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
                        instances.push(Instance { cluster, offset, yaw, copy: true, deleted });
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

    /// Writes the block in the folder `land` of a folder of the SD card.
    pub fn export(&self, out_dir: &str) -> Result<String, String> {
        let path = self.built.save(Path::new(out_dir).to_str().unwrap_or(out_dir))?;
        let size = std::fs::metadata(&path).map(|m| m.len()).unwrap_or(0);
        Ok(format!("written {} ({} bytes)", path.display(), size))
    }
}
