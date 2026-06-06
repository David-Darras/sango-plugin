switch (trainer_id) {
  case 7:
  {
    config.pokemon_teams[1]->count = 2;

    pkm1.species = SPECIES_ALAKAZAM;
    pkm1.SetNickname(u"ZettaD");
    pkm1.SetLevel(100);
    pkm1.item = ITEM_LIFE_ORB;
    pkm1.nature = NATURE_TIMID;
    pkm1.gender = GENDER_FEMALE;
    pkm1.form = FORM_ALAKAZAM_MEGA;
    pkm1.SetShiny(true);
    pkm1.ball = BALL_MASTER_BALL;

    pkm1.moves[0] = MOVE_LIGHT_OF_RUIN;
    pkm1.pp[0] = 99;
    pkm1.moves[1] = MOVE_DRAGON_ASCENT;
    pkm1.pp[1] = 99;
    pkm1.moves[2] = MOVE_PRECIPICE_BLADES;
    pkm1.pp[2] = 99;
    pkm1.moves[3] = MOVE_ORIGIN_PULSE;
    pkm1.pp[3] = 99;

    pkm1.ev_hp = 255;
    pkm1.ev_attack = 255;
    pkm1.ev_defense = 255;
    pkm1.ev_speed = 255;
    pkm1.ev_special_attack = 255;
    pkm1.ev_special_defense = 255;

    pkm1.iv_hp = 31;
    pkm1.iv_attack = 31;
    pkm1.iv_defense = 31;
    pkm1.iv_speed = 31;
    pkm1.iv_special_attack = 31;
    pkm1.iv_special_defense = 31;

    pkm2.species = SPECIES_RATTATA;
    pkm2.SetNickname(u"TopPercent");
    pkm2.SetLevel(1);
    pkm2.item = ITEM_FOCUS_SASH;
    pkm2.nature = NATURE_JOLLY;
    pkm2.gender = GENDER_MALE;
    pkm2.SetShiny(false);
    pkm2.ball = BALL_POKE_BALL;

    pkm2.moves[0] = MOVE_ENDEAVOR;
    pkm2.pp[0] = 5;
    pkm2.moves[1] = MOVE_QUICK_ATTACK;
    pkm2.pp[1] = 30;

    pkm2.ev_hp = 0;
    pkm2.ev_attack = 255;
    pkm2.ev_defense = 0;
    pkm2.ev_speed = 255;
    pkm2.ev_special_attack = 0;
    pkm2.ev_special_defense = 0;

    pkm2.iv_hp = 31;
    pkm2.iv_attack = 31;
    pkm2.iv_defense = 31;
    pkm2.iv_speed = 31;
    pkm2.iv_special_attack = 31;
    pkm2.iv_special_defense = 31;
  }
  break;
  case 8:
  {
    config.pokemon_teams[1]->count = 2;

    config.battle_format = BATTLE_FORMAT_DOUBLE;

    pkm1.species = SPECIES_CHARIZARD;
    pkm1.SetNickname(u"Infernus");
    pkm1.SetLevel(85);
    pkm1.nature = NATURE_MODEST;
    pkm1.gender = GENDER_MALE;
    pkm1.SetShiny(false);
    pkm1.ball = BALL_CHERISH_BALL;

    pkm1.moves[0] = MOVE_HEAT_WAVE;
    pkm1.pp[0] = 10;
    pkm1.moves[1] = MOVE_SOLAR_BEAM;
    pkm1.pp[1] = 10;
    pkm1.moves[2] = MOVE_AIR_SLASH;
    pkm1.pp[2] = 15;
    pkm1.moves[3] = MOVE_ROOST;
    pkm1.pp[3] = 10;

    pkm1.ev_hp = 4;
    pkm1.ev_attack = 0;
    pkm1.ev_defense = 0;
    pkm1.ev_speed = 252;
    pkm1.ev_special_attack = 252;
    pkm1.ev_special_defense = 0;

    pkm1.iv_hp = 31;
    pkm1.iv_attack = 0;
    pkm1.iv_defense = 31;
    pkm1.iv_speed = 31;
    pkm1.iv_special_attack = 31;
    pkm1.iv_special_defense = 31;

    pkm2.species = SPECIES_VENUSAUR;
    pkm2.SetNickname(u"Brindille");
    pkm2.SetLevel(85);
    pkm2.item = ITEM_LIFE_ORB;
    pkm2.nature = NATURE_MODEST;
    pkm2.gender = GENDER_FEMALE;
    pkm2.SetShiny(true);
    pkm2.ball = BALL_NEST_BALL;

    pkm2.moves[0] = MOVE_GIGA_DRAIN;
    pkm2.pp[0] = 10;
    pkm2.moves[1] = MOVE_SLUDGE_BOMB;
    pkm2.pp[1] = 10;
    pkm2.moves[2] = MOVE_WEATHER_BALL;
    pkm2.pp[2] = 10;
    pkm2.moves[3] = MOVE_GROWTH;
    pkm2.pp[3] = 20;

    pkm2.ev_hp = 0;
    pkm2.ev_attack = 0;
    pkm2.ev_defense = 4;
    pkm2.ev_speed = 252;
    pkm2.ev_special_attack = 252;
    pkm2.ev_special_defense = 0;

    pkm2.iv_hp = 31;
    pkm2.iv_attack = 31;
    pkm2.iv_defense = 31;
    pkm2.iv_speed = 31;
    pkm2.iv_special_attack = 31;
    pkm2.iv_special_defense = 31;
  }
  break;
}