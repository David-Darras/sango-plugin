"""The Bulbapedia names of the music of Pokemon X/Y and Omega Ruby/Alpha Sapphire.

The names come from the track lists of the pages "Pokemon X & Pokemon Y: Super
Music Collection" and "Pokemon Omega Ruby & Pokemon Alpha Sapphire: Super Music
Collection" of Bulbapedia. The developer names of the music files (for example
"c_04") are linked to a title with the data of the game:

* the zone table of X/Y (which music a town, a route or a cave plays),
* the table of the trainer types (which "Trainers' Eyes Meet" a class plays),
* the calls of the battle and event code,
* the order of the tracks, when the files and the titles have the same
  number of items (the PR videos, the clothes, the Pokedex evaluations).

A music without a certain title keeps its developer name, written in words
(for example "Demo 05").
"""

import re

PR_VIDEO = ["Cute", "Elegant", "Cool", "Classical", "Lively", "Rhythmical",
            "Mature", "Thrilling"]
EYES = {
    "01": "Youngster", "02": "Lass", "03": "Twins", "04": "Swimmer M",
    "05": "Swimmer F", "06": "Poke Fan", "07": "Hiker",
    "08": "Roller Skater", "09": "Psychic", "10": "Punk Guy", "11": "Black Belt",
    "12": "Maid", "13": "Butler", "14": "Furisode Girl", "15": "Honeymooners",
    "16": "Tourist", "17": "Ace Trainer", "flare": "Team Flare",
}
CLOTHES = ["Basic Clothes (Boy)", "Basic Clothes (Girl)",
           "Colorful Clothes (Boy)", "Sporty Clothes (Girl)",
           "Exotic Clothes (Girl)", "Feminine Clothes (Girl)",
           "Elegant Clothes (Girl)", "Special Clothes (Boy)",
           "Special Clothes (Girl)", "Classic Clothes (Boy)",
           "Classic Clothes (Girl)"]
EVALUATIONS = ["You've Just Begun!", "You're on Your Way!", "Keep at It!",
               "Not Bad!", "Looking Good!", "It's Great!", "It's Amazing!",
               "It's Complete!"]

# The music of Pokemon X/Y, by the name of the file without "bgm_xy_".
XY = {
    "ajito": "Lysandre Labs",
    "battle_house": "Battle Maison",
    "bicycle": "Bicycle",
    "boutique": "Boutique",
    "c_01": "Santalune City", "c_02": "Lumiose City", "c_03": "Cyllage City",
    "c_04": "Shalour City", "c_05": "Coumarine City", "c_06": "Laverre City",
    "c_07": "Anistar City", "c_08": "Snowbelle City", "c_09": "Kiloude City",
    "d_01": "Santalune Forest", "d_02": "Parfum Palace",
    "d_03": "Glittering Cave", "d_05": "The Kalos Power Plant",
    "d_06": "Pokemon Village", "d_07": "Team Flare Secret HQ",
    "d_champroad": "Victory Road",
    "demo_opening": "Opening Movie",
    "demo_weapon_x": "Firing the Ultimate Weapon (Pokemon X)",
    "demo_weapon_y": "Firing the Ultimate Weapon (Pokemon Y)",
    "dendouiri": "Congratulations on Entering the Hall of Fame!",
    "e_flare": "Team Flare Appears!",
    "e_tsureteke1": "Hurry Along 1", "e_tsureteke2": "Hurry Along 2",
    "epilogue": "FIN",
    "fs": "Shopping",
    "game_sync": "Game Sync",
    "gate": "Gate",
    "gts": "GTS",
    "gym": "Pokemon Gym",
    "hotel": "Hotel Richissime",
    "koukan": "Trade",
    "labo": "The Sycamore Pokemon Lab",
    "m_tower": "Tower of Mastery",
    "miracle_trade": "Wonder Trade",
    "naminori": "Surf",
    "pokecen": "Pokemon Center",
    "pokemon_league": "The Pokemon League",
    "pokemon_link": "Pokemon Link",
    "pokemon_theme": "Kalos Region Theme",
    "poketore": "A Poke Radar Hit!",
    "poketore_fever": "Poke Radar Chain!",
    "pvd_studio": "PR Video Studio",
    "r_a": "Route 1", "r_b": "Route 2", "r_c": "Route 4", "r_d": "Route 8",
    "r_e": "Route 15", "r_f": "Route 18",
    "random_match": "Battle Spot",
    "safari": "Friend Safari",
    "salon": "Coiffure Clips",
    "shinka": "Evolution",
    "silence": "Silence",
    "sp_clear": "Success!", "sp_gameover": "Try Again...",
    "sp_gamestart": "Start", "sp_menu": "Let's Super Train!",
    "sp_stage": "Super Training!", "sp_stage_02": "Secret Super Training!",
    "starting": "An Adventure Is Beginning",
    "sushi": "Sushi High Roller",
    "t_01": "Vaniville Town", "t_02": "Aquacorde Town",
    "t_03": "Camphrier Town", "t_04": "Geosenge Town",
    "t_05": "Dendemille Town",
    "title_01": "Title Screen",
    "vs_champ": "Battle! (Champion)",
    "vs_flare": "Battle! (Team Flare)",
    "vs_flare_boss": "Battle! (Lysandre)",
    "vs_gymleader": "Battle! (Gym Leader)",
    "vs_gymleader2": "Battle! (Successor Korrina)",
    "vs_legend": "Battle! (Xerneas/Yveltal/Zygarde)",
    "vs_norapoke": "Battle! (Wild Pokemon)",
    "vs_rival": "Battle! (Friend)",
    "vs_secret_01": "Battle! (Mewtwo)",
    "vs_shitenno": "Battle! (Elite Four)",
    "vs_trainer": "Battle! (Trainer Battle)",
    "wifi_present": "Mystery Gift",
    "win1": "Victory! (Wild Pokemon)",
    "win2": "Victory! (Trainer Battle)",
    "win3": "Victory! (Gym Leader)",
    "win4": "Victory! (Team Flare)",
    "win5": "Victory! (Champion)",
    "yagi": "Riding Skiddo",
    "yagi2": "Gogoat Shuttle",
}
for key, value in EYES.items():
  XY["eye_" + key] = "Trainers' Eyes Meet (%s)" % value
for number in range(1, 33):
  XY["pvd_%02d" % number] = 'PR Video BGM "%s %d"' % (
      PR_VIDEO[(number - 1) // 4], (number - 1) % 4 + 1)

# The jingles, by the name of the file without "me_xy_".
XY_ME = {
    "badge": "Obtained a Gym Badge!",
    "bpget": "Received Battle Points!",
    "item": "Obtained an Item!",
    "kaifuku": "Pokemon Healed",
    "keyitem": "Obtained a Key Item!",
    "kinomi": "Obtained a Berry!",
    "lvup": "Level Up!",
    "mega_stone": "Obtained a Mega Stone!",
    "pokemon_flute": "Poke Flute",
    "shinkaome": "Congratulations! Your Pokemon Evolved!",
    "sp_stage_open": "Unlocked a New Super-Training Regimen!",
    "tamago_get": "Received a Pokemon Egg!",
    "wasure": "Move Deleted",
    "waza": "Obtained a TM!",
}
for number in range(1, 12):
  XY_ME["dressup_%02d" % number] = "Changed into " + CLOTHES[number - 1]
for number in range(1, 9):
  XY_ME["hyouka%02d" % number] = (
      "Pokedex Evaluation... " + EVALUATIONS[number - 1])

# The music of Omega Ruby/Alpha Sapphire, by the name of the value of
# BackgroundMusicId without "k". The other values keep their own name.
ORAS = {
    "Default": "Title Screen: Main Theme",
    "PokemonTheme": "Title Screen: Main Theme",
    "Bicycle": "Cycling",
    "PokemonCenter": "Pokemon Center",
    "Gym": "Pokemon Gym",
    "Laboratory": "Birch Pokemon Lab",
    "VsWildPokemon": "Battle! (Wild Pokemon)",
    "VsTrainer": "Battle! (Trainer Battle)",
    "VsGymLeader": "Battle! (Gym Leader)",
    "WinVictory": "Victory! (Trainer Battle)",
    "WinGymLeader": "Victory! (Gym Leader / Elite Four)",
    "WinWild": "Victory! (Wild Pokemon)",
    "Cinema": "Opening Movie",
    "BattleTowerLobby": "Battle Tower",
    "SootopolisCity": "Sootopolis City",
    "AbandonedShip": "Abandoned Ship",
    "Starting": "Opening Movie: Setting out on a Journey in the Hoenn Region",
    "PacifidlogTown": "Pacifidlog Town",
    "MossdeepCity": "Mossdeep City",
    "VerdanturfTown": "Verdanturf Town",
    "Ending": "Ending Theme",
    "VictoryRoad": "Victory Road",
    "BattleResort": "Battle Resort",
    "FrontierBrain": "Battle! (Frontier Brain)",
    "BattleTower": "Battle Tower",
    "CaveOfOrigin": "Cave of Origin",
    "SkyPillar": "Sky Pillar",
    "VsRival": "Battle! (Brendan / May)",
    "VsTeamMagmaAqua": "Battle! (Team Aqua / Team Magma)",
    "VsEliteFour": "Battle! (Elite Four)",
    "VsChampion": "Battle! (Steven)",
    "SealedChamber": "Sealed Chamber",
    "MtChimney": "Mt. Chimney",
    "AquaMagmaHideout": "Hideout", "AquaHideout": "Hideout",
    "MagmaHideout": "Hideout",
    "SafariZone": "Safari Zone",
    "WinEliteFour": "Victory! (Gym Leader / Elite Four)",
    "WinChampion": "Victory! (Steven)",
    "HallOfFame": "The Hall of Fame",
    "Underwater": "Dive", "Diving": "Dive",
    "DewfordTown": "Dewford Town",
    "RustboroCity": "Rustboro City",
    "FallarborTown": "Fallarbor Town",
    "FortreeCity": "Fortree City",
    "PokemonLink": "Pokemon Link",
    "LilycoveCity": "Lilycove City",
    "SlateportCity": "Slateport City",
    "PetalburgCity": "Petalburg City",
    "OldaleTown": "Oldale Town",
    "SecretBase": "Super-Secret Bases",
    "VsDeoxys": "Battle! (Deoxys)",
    "MiracleTrade": "Wonder Trade",
    "TrainerSchool": "Trainers' School",
    "Museum": "Museum",
    "ContestLobby": "Contest Lobby",
    "May": "May", "Brendan": "Brendan",
    "EliteFour": "The Elite Four Appear!",
    "ContestResult": "Results",
    "ContestChamp": "Contest Victor",
    "VsWildPokemonSg": "Battle! (Wild Pokemon)",
    "VsTrainerSg": "Battle! (Trainer Battle)",
    "VsGymLeaderSg": "Battle! (Gym Leader)",
    "VsRivalSg": "Battle! (Brendan / May)",
    "VsAqua": "Battle! (Team Aqua / Team Magma)",
    "VsMagma": "Battle! (Team Aqua / Team Magma)",
    "VsAquaLeader": "Battle! (Team Aqua / Team Magma Leaders)",
    "VsMagmaLeader": "Battle! (Team Aqua / Team Magma Leaders)",
    "VsEliteFourSg": "Battle! (Elite Four)",
    "VsChampionSg": "Battle! (Steven)",
    "PokemonCenter2": "Pokemon Center",
    "LittlerootTown": "Littleroot Town",
}
for number in range(101, 131):
  ORAS["Route%d" % number] = "Route %d" % number


def words(name):
  return " ".join(re.findall(r"[A-Z][a-z]+|[A-Z]+(?![a-z])|\d+|[a-z]+", name))


def xy_title(stem):
  """Returns the title of a file such as "bgm_xy_ajito" or "me_xy_lvup"."""
  if stem.startswith("bgm_xy_"):
    key = stem[len("bgm_xy_"):]
    return XY.get(key) or words(key.replace("_", " ")).title()
  if stem.startswith("me_xy_"):
    key = stem[len("me_xy_"):]
    return XY_ME.get(key) or words(key.replace("_", " ")).title()
  return stem
