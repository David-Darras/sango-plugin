//! The dictionaries of the model files.
//!
//! A model keeps the names of its materials in a binary tree (a Patricia tree).
//! The game finds a name in it by looking at some bits of the name. A node is
//! 12 bytes: the number of the bit to look at, the numbers of the left and of
//! the right node (16 bits each), and the place of the name in the strings. The
//! first node is the root: its bit number is 0xFFFFFFFF.

/// A node of the tree.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Node {
    pub bit: u32,
    pub left: u16,
    pub right: u16,
    pub name: u32,
}

impl Node {
    pub fn to_bytes(self) -> [u8; 12] {
        let mut out = [0u8; 12];
        out[0..4].copy_from_slice(&self.bit.to_le_bytes());
        out[4..6].copy_from_slice(&self.left.to_le_bytes());
        out[6..8].copy_from_slice(&self.right.to_le_bytes());
        out[8..12].copy_from_slice(&self.name.to_le_bytes());
        out
    }

    pub fn from_bytes(b: &[u8]) -> Node {
        Node {
            bit: u32::from_le_bytes([b[0], b[1], b[2], b[3]]),
            left: u16::from_le_bytes([b[4], b[5]]),
            right: u16::from_le_bytes([b[6], b[7]]),
            name: u32::from_le_bytes([b[8], b[9], b[10], b[11]]),
        }
    }
}

/// The bit `index` of a name: the bits are counted from the first character,
/// and from the lowest bit of each character.
pub fn bit_of(name: &[u8], index: u32) -> bool {
    let byte = (index >> 3) as usize;
    if byte >= name.len() {
        return false;
    }
    (name[byte] >> (index & 7)) & 1 == 1
}

/// Finds the node that a name leads to. `name_of` gives the name of a node.
pub fn lookup(nodes: &[Node], name: &[u8]) -> usize {
    let mut parent = 0usize;
    let mut current = nodes[0].left as usize;
    while nodes[current].bit < nodes[parent].bit {
        parent = current;
        current = if bit_of(name, nodes[current].bit) { nodes[current].right } else { nodes[current].left } as usize;
    }
    current
}

/// The highest bit where two names differ, or None when they are the same.
fn first_difference(a: &[u8], b: &[u8]) -> Option<u32> {
    let length = a.len().max(b.len());
    for index in (0..length as u32 * 8).rev() {
        if bit_of(a, index) != bit_of(b, index) {
            return Some(index);
        }
    }
    None
}

/// Builds the tree of some names. The names are in the order of the table:
/// the node `k + 1` is for the name `k`. The place of the name in the strings
/// comes from `offsets`.
pub fn build(names: &[&[u8]], offsets: &[u32]) -> Vec<Node> {
    let mut nodes = vec![Node { bit: u32::MAX, left: 0, right: 0, name: 0 }];
    for (k, name) in names.iter().enumerate() {
        let new = nodes.len();
        // The name that is the nearest in the tree.
        let nearest = lookup(&nodes, name);
        let existing: &[u8] = if nearest == 0 { b"" } else { names[nearest - 1] };
        let Some(bit) = first_difference(name, existing) else {
            // The same name twice: the tree keeps only the first.
            continue;
        };
        // Where the new node goes: after the nodes whose bit is higher.
        let mut parent = 0usize;
        let mut current = nodes[0].left as usize;
        while nodes[current].bit < nodes[parent].bit && nodes[current].bit > bit {
            parent = current;
            current = if bit_of(name, nodes[current].bit) { nodes[current].right } else { nodes[current].left } as usize;
        }
        let node = if bit_of(name, bit) {
            Node { bit, left: current as u16, right: new as u16, name: offsets[k] }
        } else {
            Node { bit, left: new as u16, right: current as u16, name: offsets[k] }
        };
        nodes.push(node);
        if parent == 0 {
            nodes[0].left = new as u16;
        } else if bit_of(name, nodes[parent].bit) {
            nodes[parent].right = new as u16;
        } else {
            nodes[parent].left = new as u16;
        }
    }
    nodes
}
