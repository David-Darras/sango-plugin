//! Sango Editor: an editor for the maps of Omega Ruby and Alpha Sapphire.

mod icon;
mod bch;
mod bch_edit;
mod catalog;
mod events;
mod models;
mod names;
mod thumb;
mod commands;
mod config;
mod data;
mod dict;
mod edit;
mod editor;
mod garc;
mod lz11;
mod pack;
mod tex;
mod viewer;

const USAGE: &str = "usage:
  sango-editor view <arc dir> <block id> [tileset id] [--land <land file>]
  sango-editor meshes <arc dir> <block id> [out.obj]
  sango-editor sheet <arc dir> <block id> <out.png> [tileset id]
  sango-editor picture <arc dir> <block id> <name> <out.png>
  sango-editor groups <arc dir> <block id> <material>
  sango-editor roundtrip <arc dir> <block id> <out dir>
  sango-editor move-near <arc dir> <block id> <materials> <x> <z> <radius> <dx> <dy> <dz> <out dir>
  sango-editor copy-near <arc dir> <block id> <materials> <x> <z> <radius> <dx> <dy> <dz> <out dir>
  sango-editor delete-near <arc dir> <block id> <materials> <x> <z> <radius> <out dir>
  sango-editor editor-test <arc dir> <block id> <out dir>
  sango-editor add-mesh-test <arc dir> <block id> <material> <x> <z> <radius> <dx> <out dir>
  sango-editor validate <arc dir> <block id> [--land <land file>]
  sango-editor check <land file> <block id> <material>";

/// Opens a block in a window.
fn view(arc_dir: &str, block: usize, tileset: Option<usize>, land_file: Option<&str>) -> Result<(), String> {
    let scene = data::load_scene_from(arc_dir, block, tileset, land_file)?;
    let with_picture = scene
        .meshes
        .iter()
        .filter(|m| scene.textures.contains_key(data::picture_of(m)))
        .count();
    println!(
        "tileset {:?}: {} pictures, {} of {} meshes have a picture",
        scene.tileset, scene.textures.len(), with_picture, scene.meshes.len()
    );
    let mut editor = match land_file {
        Some(path) => editor::Editor::from_block(edit::EditableBlock::from_file(std::path::Path::new(path), block)?, arc_dir, false)?,
        None => editor::Editor::new(arc_dir, block)?,
    };
    editor.set_tileset(scene.tileset);
    viewer::run(scene.textures, editor, format!("Block {block}"), arc_dir.to_string())
}

fn main() {
    config::migrate_old_files();
    let args: Vec<String> = std::env::args().collect();
    let number = |i: usize| args.get(i).and_then(|s| s.parse::<usize>().ok());
    let float = |i: usize| args.get(i).and_then(|s| s.parse::<f32>().ok()).unwrap_or(0.0);
    let block = number(3).unwrap_or(0);
    let result = match args.get(1).map(String::as_str) {
        // No block given: the tool opens the world that it remembers (or the first one).
        None => config::archives(None).and_then(|arc| view(&arc, config::load().block.unwrap_or(4), None, None)),
        Some("view") if args.len() < 4 => {
            config::archives(args.get(2).map(String::as_str)).and_then(|arc| view(&arc, config::load().block.unwrap_or(4), None, None))
        }
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
        Some("objects") if args.len() >= 4 => commands::objects(&args[2], block, args.get(4).map(String::as_str) == Some("all")),
        Some("tiles") if args.len() >= 4 => commands::tiles(&args[2], block),
        Some("find") if args.len() >= 4 => commands::find(&args[2], &args[3]),
        Some("cross-test") if args.len() >= 6 => commands::cross_test(&args[2], block, number(4).unwrap_or(0), &args[5]),
        Some("check-widths") if args.len() >= 3 => commands::check_widths(&args[2]),
        Some("add-mesh-test") if args.len() >= 10 => commands::add_mesh_test(
            &args[2],
            block,
            &args[4],
            [float(5), float(6)],
            float(7),
            float(8),
            &args[9],
        ),
        Some("validate") if args.len() >= 4 => {
            let land = args.iter().position(|a| a == "--land").and_then(|i| args.get(i + 1)).map(String::as_str);
            commands::validate(&args[2], block, land)
        }
        Some("grow-test") if args.len() >= 7 => commands::grow_test(&args[2], block, [float(4), float(5)], float(6) as usize),
        Some("check-trees") if args.len() >= 3 => commands::check_trees(&args[2]),
        Some("mesh-select-test") if args.len() >= 5 => commands::mesh_select_test(&args[2], block, &args[4]),
        Some("building-test") if args.len() >= 7 => commands::building_test(&args[2], block, [float(4), float(5)], &args[6]),
        Some("import-building-test") if args.len() >= 9 => commands::import_building_test(&args[2], block, [float(4), float(5)], &args[6], number(7).unwrap_or(0), &args[8]),
        Some("catalog-stats") if args.len() >= 3 => commands::catalog_stats(&args[2], number(3), number(4), args.get(5).map(|s| s.as_str())),
        Some("catalog-sheet") if args.len() >= 5 => commands::catalog_sheet(&args[2], &args[3], &args[4], number(5).unwrap_or(0)),
        Some("catalog-drop-test") if args.len() >= 7 => commands::catalog_drop_test(&args[2], &args[3], &args[4], number(5).unwrap_or(0), &args[6]),
        Some("delete-many-test") if args.len() >= 5 => commands::delete_many_test(&args[2], block, number(4).unwrap_or(10)),
        Some("delete-meshes-test") if args.len() >= 4 => commands::delete_meshes_test(&args[2], block),
        Some("drop-then-delete-test") if args.len() >= 6 => commands::drop_then_delete_test(&args[2], &args[3], &args[4], number(5).unwrap_or(0)),
        Some("ground-tiles") if args.len() >= 4 => commands::ground_tiles(&args[2], block),
        Some("tile-stats") if args.len() >= 5 => commands::tile_stats(&args[2], number(3).unwrap_or(0), number(4).unwrap_or(0), args.get(5).map(|s| s.as_str())),
        Some("attribute-values") if args.len() >= 3 => commands::attribute_values(&args[2]),
        Some("tiles-under") if args.len() >= 5 => commands::tiles_under(&args[2], block, &args[4]),
        Some("clear-test") if args.len() >= 5 => commands::clear_test(&args[2], block, &args[4]),
        Some("tile-many-test") if args.len() >= 7 => commands::tile_many_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(8), &args[6]),
        Some("dup-test") if args.len() >= 7 => commands::dup_test(&args[2], block, &args[4], number(5).unwrap_or(3), &args[6]),
        Some("icon") if args.len() >= 3 => icon::write_png(&args[2], 256),
        Some("prop-add-test") if args.len() >= 5 => commands::prop_add_test(&args[2], block, &args[4]),
        Some("characters") if args.len() >= 4 => commands::characters(&args[2], number(3).unwrap_or(0)),
        Some("fill-test") if args.len() >= 7 => commands::fill_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(0), &args[6]),
        Some("map-tilesets") if args.len() >= 4 => commands::map_tilesets(&args[2], number(3).unwrap_or(0)),
        Some("sd-pictures-check") if args.len() >= 5 => commands::sd_pictures_check(&args[2], &args[3], number(4).unwrap_or(0)),
        Some("flatten-test") if args.len() >= 5 => commands::flatten_test(&args[2], block, &args[4]),
        Some("grass-test") if args.len() >= 5 => commands::grass_test(&args[2], block, &args[4]),
        Some("tile-drop-test") if args.len() >= 7 => commands::tile_drop_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(0), &args[6]),
        Some("tile-holes-test") if args.len() >= 7 => commands::tile_holes_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(10), number(6).unwrap_or(0)),
        Some("tile-picture-test") if args.len() >= 7 => commands::tile_picture_test(&args[2], &args[3], &args[4].split(',').filter_map(|n| n.parse().ok()).collect::<Vec<_>>(), number(5).unwrap_or(0), &args[6]),
        Some("drop-all-test") if args.len() >= 7 => commands::drop_all_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(50), number(6).unwrap_or(0)),
        Some("layout-stats") if args.len() >= 5 => commands::layout_stats(&args[2], &args[3], number(4).unwrap_or(0)),
        Some("tileset-pictures-test") if args.len() >= 6 => commands::tileset_pictures_test(&args[2], number(3).unwrap_or(0), number(4).unwrap_or(0), &args[5..].to_vec()),
        Some("drop-tileset-test") if args.len() >= 7 => commands::drop_tileset_test(&args[2], &args[3], &args[4], number(5).unwrap_or(0), &args[6]),
        Some("model-strings") if args.len() >= 6 => commands::model_strings(&args[2], &args[3], &args[4], number(5).unwrap_or(0)),
        Some("tile-twice-test") if args.len() >= 7 => commands::tile_twice_test(&args[2], &args[3], number(4).unwrap_or(0), number(5).unwrap_or(10), number(6).unwrap_or(0)),
        Some("map-check") if args.len() >= 4 => commands::map_check(&args[2], block),
        Some("events-file-test") if args.len() >= 4 => commands::events_file_test(&args[2], number(3).unwrap_or(0)),
        Some("map-roundtrip") if args.len() >= 3 => commands::map_roundtrip(&args[2]),
        Some("tiles-around") if args.len() >= 7 => commands::tiles_around(&args[2], block, &args[4], number(5).unwrap_or(0), number(6).unwrap_or(0)),
        Some("warp-tiles") if args.len() >= 4 => commands::warp_tiles(&args[2], block),
        Some("objects-near") if args.len() >= 7 => commands::objects_near(&args[2], block, float(4), float(5), float(6)),
        Some("building-near") if args.len() >= 10 => commands::building_near(&args[2], block, [float(4), float(5)], &args[6], float(7), float(8), float(9)),
        Some("props-test") if args.len() >= 8 => commands::props_test(&args[2], block, float(4), float(5), float(6), &args[7]),
        Some("model-sheet") if args.len() >= 5 => commands::model_sheet(&args[2], &args[4].split(',').filter_map(|n| n.parse().ok()).collect::<Vec<_>>(), &args[3]),
        Some("models-check") if args.len() >= 3 => commands::models_check(&args[2]),
        Some("model-layout") if args.len() >= 4 => commands::model_layout(&args[2], number(3).unwrap_or(0)),
        Some("model-vertices") if args.len() >= 4 => commands::model_vertices(&args[2], number(3).unwrap_or(0)),
        Some("models-skin") if args.len() >= 3 => commands::models_skin(&args[2]),
        Some("xy-models-test") if args.len() >= 5 => commands::xy_models_test(&args[2], &args[3], &args[4]),
        Some("neighbours-test") if args.len() >= 4 => commands::neighbours_test(&args[2], block),
        Some("door-sheet") if args.len() >= 5 => commands::prop_sheet(&args[2], &args[4].split(',').filter_map(|n| n.parse().ok()).collect::<Vec<_>>(), &args[3]),
        Some("worlds-list") if args.len() >= 3 => commands::worlds_list(&args[2]),
        Some("check") if args.len() >= 5 => commands::check(&args[2], block, &args[4]),
        _ => Err(USAGE.to_string()),
    };
    if let Err(message) = result {
        eprintln!("{message}");
        std::process::exit(1);
    }
}
