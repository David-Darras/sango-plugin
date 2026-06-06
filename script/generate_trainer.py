import json
import os


def generate_trainer_inc(json_path="./config/trainer.json", output_path="../include/feature/trainer.inc"):
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if isinstance(data, dict):
        trainers = [data]
    else:
        trainers = data

    lines = []
    lines.append("switch (trainer_id) {")

    for trainer in trainers:
        t_id = trainer.get("trainer_id")
        config = trainer.get("config", {})
        party = trainer.get("party", [])
        pokemon_count = len(party)

        lines.append(f"  case {t_id}:")
        lines.append("  {")
        lines.append(f"    config.pokemon_teams[1]->count = {pokemon_count};")
        lines.append("")

        if config:
            if "battle_format" in config:
                lines.append(f"    config.battle_format = {config['battle_format']};")
            if "ground" in config:
                lines.append(f"    config.ground = {config['ground']};")
            if "background" in config:
                lines.append(f"    config.background = {config['background']};")
            if "platform" in config:
                lines.append(f"    config.platform = {config['platform']};")
            if "encounter_animation" in config:
                lines.append(f"    config.encounter_animation = {config['encounter_animation']};")
            if "background_music" in config:
                lines.append(f"    config.background_music = (1 << 16) + {config['background_music']};")
            if "weather" in config:
                lines.append(f"    config.weather = {config['weather']};")
            lines.append("")

        for idx, pkm in enumerate(party):
            p_num = idx + 1
            p_prefix = f"    pkm{p_num}"

            if "species" in pkm: lines.append(f"{p_prefix}.species = {pkm['species']};")
            if "nickname" in pkm: lines.append(f"{p_prefix}.SetNickname(u\"{pkm['nickname']}\");")
            if "level" in pkm: lines.append(f"{p_prefix}.SetLevel({pkm['level']});")
            if "item" in pkm: lines.append(f"{p_prefix}.item = {pkm['item']};")
            if "nature" in pkm: lines.append(f"{p_prefix}.nature = {pkm['nature']};")
            if "gender" in pkm: lines.append(f"{p_prefix}.gender = {pkm['gender']};")
            if "form" in pkm: lines.append(f"{p_prefix}.form = {pkm['form']};")
            if "is_shiny" in pkm:
                shiny_val = "true" if pkm['is_shiny'] else "false"
                lines.append(f"{p_prefix}.SetShiny({shiny_val});")
            if "ball" in pkm: lines.append(f"{p_prefix}.ball = {pkm['ball']};")

            moves = pkm.get("moves", [])
            if moves:
                lines.append("")
                for m_idx, move_data in enumerate(moves):
                    if "move" in move_data:
                        lines.append(f"{p_prefix}.moves[{m_idx}] = {move_data['move']};")
                    if "pp" in move_data:
                        lines.append(f"{p_prefix}.pp[{m_idx}] = {move_data['pp']};")

            evs = pkm.get("evs", {})
            if evs:
                lines.append("")
                if "hp" in evs: lines.append(f"{p_prefix}.ev_hp = {evs['hp']};")
                if "attack" in evs: lines.append(f"{p_prefix}.ev_attack = {evs['attack']};")
                if "defense" in evs: lines.append(f"{p_prefix}.ev_defense = {evs['defense']};")
                if "speed" in evs: lines.append(f"{p_prefix}.ev_speed = {evs['speed']};")
                if "special_attack" in evs: lines.append(f"{p_prefix}.ev_special_attack = {evs['special_attack']};")
                if "special_defense" in evs: lines.append(f"{p_prefix}.ev_special_defense = {evs['special_defense']};")

            ivs = pkm.get("ivs", {})
            if ivs:
                lines.append("")
                if "hp" in ivs: lines.append(f"{p_prefix}.iv_hp = {ivs['hp']};")
                if "attack" in ivs: lines.append(f"{p_prefix}.iv_attack = {ivs['attack']};")
                if "defense" in ivs: lines.append(f"{p_prefix}.iv_defense = {ivs['defense']};")
                if "speed" in ivs: lines.append(f"{p_prefix}.iv_speed = {ivs['speed']};")
                if "special_attack" in ivs: lines.append(f"{p_prefix}.iv_special_attack = {ivs['special_attack']};")
                if "special_defense" in ivs: lines.append(f"{p_prefix}.iv_special_defense = {ivs['special_defense']};")

            if idx < len(party) - 1:
                lines.append("")

        lines.append("  }")
        lines.append("  break;")

    lines.append("}")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("\n".join(lines))


if __name__ == '__main__':
    generate_trainer_inc()
