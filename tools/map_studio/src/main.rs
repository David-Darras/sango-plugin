//! Map Studio: an editor for the maps of Omega Ruby and Alpha Sapphire.

mod bch;
mod commands;
mod data;
mod edit;
mod editor;
mod garc;
mod lz11;
mod pack;
mod tex;
mod viewer;

const USAGE: &str = "usage:
  map-studio view <arc dir> <block id> [area id] [--land <land file>]
  map-studio meshes <arc dir> <block id> [out.obj]
  map-studio sheet <arc dir> <block id> <out.png> [area id]
  map-studio picture <arc dir> <block id> <name> <out.png>
  map-studio groups <arc dir> <block id> <material>
  map-studio roundtrip <arc dir> <block id> <out dir>
  map-studio move-near <arc dir> <block id> <materials> <x> <z> <radius> <dx> <dy> <dz> <out dir>
  map-studio copy-near <arc dir> <block id> <materials> <x> <z> <radius> <dx> <dy> <dz> <out dir>
  map-studio delete-near <arc dir> <block id> <materials> <x> <z> <radius> <out dir>
  map-studio editor-test <arc dir> <block id> <out dir>
  map-studio check <land file> <block id> <material>";

/// Opens a block in a window.
fn view(arc_dir: &str, block: usize, area: Option<usize>, land_file: Option<&str>) -> Result<(), String> {
    let scene = data::load_scene_from(arc_dir, block, area, land_file)?;
    let with_picture = scene
        .meshes
        .iter()
        .filter(|m| scene.textures.contains_key(data::picture_of(m)))
        .count();
    println!(
        "area {:?}: {} pictures, {} of {} meshes have a picture",
        scene.area, scene.textures.len(), with_picture, scene.meshes.len()
    );
    let editor = match land_file {
        Some(path) => editor::Editor::from_block(edit::EditableBlock::from_file(std::path::Path::new(path), block)?, arc_dir, false)?,
        None => editor::Editor::new(arc_dir, block)?,
    };
    viewer::run(scene.textures, editor, format!("Block {block}"), arc_dir.to_string())
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let number = |i: usize| args.get(i).and_then(|s| s.parse::<usize>().ok());
    let float = |i: usize| args.get(i).and_then(|s| s.parse::<f32>().ok()).unwrap_or(0.0);
    let block = number(3).unwrap_or(0);
    let result = match args.get(1).map(String::as_str) {
        Some("view") if args.len() >= 4 => {
            let land = args.iter().position(|a| a == "--land").and_then(|i| args.get(i + 1)).map(String::as_str);
            view(&args[2], block, number(4), land)
        }
        Some("meshes") if args.len() >= 4 => commands::meshes(&args[2], block, args.get(4).map(String::as_str)),
        Some("sheet") if args.len() >= 5 => commands::sheet(&args[2], block, number(5), &args[4]),
        Some("picture") if args.len() >= 6 => commands::picture(&args[2], block, &args[4], &args[5]),
        Some("groups") if args.len() >= 5 => commands::list_groups(&args[2], block, &args[4]),
        Some("roundtrip") if args.len() >= 5 => commands::roundtrip(&args[2], block, &args[4]),
        Some("move-near") if args.len() >= 12 => commands::move_near(
            &args[2],
            block,
            &args[4],
            [float(5), float(6)],
            float(7),
            [float(8), float(9), float(10)],
            &args[11],
        ),
        Some("copy-near") if args.len() >= 12 => commands::copy_near(
            &args[2],
            block,
            &args[4],
            [float(5), float(6)],
            float(7),
            [float(8), float(9), float(10)],
            &args[11],
        ),
        Some("delete-near") if args.len() >= 9 => {
            commands::delete_near(&args[2], block, &args[4], [float(5), float(6)], float(7), &args[8])
        }
        Some("editor-test") if args.len() >= 5 => commands::editor_test(&args[2], block, &args[4]),
        Some("order") if args.len() >= 6 => commands::order(&args[2], block, &args[4], number(5).unwrap_or(40)),
        Some("objects") if args.len() >= 4 => commands::objects(&args[2], block),
        Some("tiles") if args.len() >= 4 => commands::tiles(&args[2], block),
        Some("find") if args.len() >= 4 => commands::find(&args[2], &args[3]),
        Some("cross-test") if args.len() >= 6 => commands::cross_test(&args[2], block, number(4).unwrap_or(0), &args[5]),
        Some("check") if args.len() >= 5 => commands::check(&args[2], block, &args[4]),
        _ => Err(USAGE.to_string()),
    };
    if let Err(message) = result {
        eprintln!("{message}");
        std::process::exit(1);
    }
}
