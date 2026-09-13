/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

static const c8* CATEGORY_TYPES[] = {
    "Simple Damage", "Simple Status", "Simple Effect",
    "Simple Recover", "Damage + Status", "Effect + Status",
    "Damage + Effect", "Damage + Effect (User)", "Drain",
    "One-Hit KO", "Field Effect", "Side Effect"
};

static const c8* DAMAGE_CATEGORIES[] = {"None", "Physical", "Special"};

static const c8* EFFECTS[] = {
    "None",
    "Paralysis",
    "Sleep",
    "Ice",
    "Burn",
    "Poison",
    "Confusion",
    "Infatuation",
    "Bind",
    "Nightmare",
    "Curse",
    "Taunt",
    "Encore",
    "Rooted",
    "Yawn",
    "Perish Block",
    "Ability Null",
    "Scouted",
    "Leech Seed",
    "Throat Choke",
    "Perish Song",
    "Rooting",
    "Trapped",
    "Encore Repeat",
    "Roost Effect",
    "Move Lock",
    "Charge Lock",
    "Move Fixation",
    "Always Hit",
    "Always Hit Target",
    "Levitate",
    "Levitate Cancel",
    "Telekinesis",
    "Free Fall",
    "Accuracy Up",
    "Aqua Ring",
    "Force Move Type",
    "Smokescreen"
};

static const c8* TURN_TYPES[] = {
    "None", "Permanent", "Turn Based",
    "While User Present", "Turn Or User Leave"
};

static const c8* TARGETS[] = {
    "Other Select", "Friend User Select", "Friend Select", "Enemy Select",
    "Other All", "Enemy All", "Friend All", "User",
    "All", "Enemy Random", "Field", "Side Enemy",
    "Side Friend"
};

static const c8* STATS[] = {
    "None", "Attack", "Defense", "Sp. Atk",
    "Sp. Def", "Speed", "Accuracy", "Evasion"
};