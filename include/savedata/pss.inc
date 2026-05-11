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

static const char* RELATIONS[] =
{
    "Friend",
    "Acquaintance",
    "Passerby",
    "Self"
};

static const char* REQUESTS[] =
{
    "None",

    // Basic
    "Change Icon",
    "Send Happy Icon",
    "Send Screenshot",
    "Update Game Status",
    "Playing GTS",
    "Playing Battle Spot",
    "Playing Miracle Trade",
    "Waiting for IR Battle",

    // Battle
    "Battle Invite",
    "Cancel Battle",
    "Waiting for Battle Connection",
    "Accept Battle Invite",
    "Decline Battle Invite",
    "Battle Host Transfer",
    "IR Battle Invite",
    "Join IR Battle",

    // Trade
    "Trade Invite",
    "Cancel Trade",
    "Waiting for Trade Connection",
    "Accept Trade Invite",
    "Decline Trade Invite",
    "IR Trade Invite",
    "Join IR Trade",

    // O-Power
    "Use O-Power on Self",
    "Use O-Power on Other Player",

    // Messages
    "Birthday Message",
    "Nice Message",

    // Voice Chat
    "Voice Chat Invite",
    "Cancel Voice Chat",
    "Waiting for Voice Chat Connection",
    "Accept Voice Chat Invite",
    "Decline Voice Chat Invite",

    // Promo Video
    "Promo Video Invite",
    "Cancel Promo Video",
    "Waiting for Promo Video Connection",
    "Accept Promo Video Invite",
    "Decline Promo Video Invite",
    "Watching Promo Video",

    // Invalid / terminate
    "Terminate"
};

static const char* ICONS[130] =
{
    "Serena", "Calem", "Professor Sycamore", "Champion Diantha",
    "Elite Four Wikstrom (Steel)", "Elite Four Malva (Fire)",
    "Elite Four Drasna (Dragon)", "Elite Four Siebold (Water)",
    "Gym Leader Viola (Bug)", "Gym Leader Grant (Rock)",
    "Gym Leader Korrina (Fighting)", "Gym Leader Ramos (Grass)",
    "Gym Leader Clemont (Electric)", "Gym Leader Valerie (Fairy)",
    "Gym Leader Olympia (Psychic)", "Gym Leader Wulfric (Ice)",
    "Youngster", "Twins", "Lass", "Lady", "Schoolgirl", "Battle Girl",
    "Schoolboy", "Rich Boy",
    "Ace Trainer (Female)", "Veteran (Female)", "Pokemon Ranger (Female)",
    "Ace Trainer (Male)",
    "Pokemon Ranger (Male)", "Madame", "Monsieur", "Black Belt", "Bald Man",
    "Fairy Tale Girl",
    "Shauna", "Tierno", "Trevor", "Brendan", "May", "Unused", "Hiker",
    "Aroma Lady",
    "School Kid (Male)", "School Kid (Female)", "Black Belt (Alt)",
    "Battle Girl (Alt)",
    "Poke Maniac", "Fairy Tale Girl (Alt)", "Poke Fan (Male)",
    "Poke Fan (Female)",
    "Pokemon Ranger (Male)", "Pokemon Ranger (Female)", "Swimmer (Male)",
    "Hex Maniac",
    "Ace Trainer (Male)", "Ace Trainer (Female)", "Roughneck", "Delinquent",
    "Expert (Male)",
    "Expert (Female)", "Lady (Alt)", "Rich Boy (Alt)", "Ninja Boy", "Beauty",
    "Guitarist",
    "Lass (Alt)", "Pokemon Breeder (Male)", "Pokemon Breeder (Female)",
    "Camper", "Picnicker",
    "Wally (Full Power)", "Steven", "Maxie", "Archie",

    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr,

    "Poke Center", // 128
    "Gift" // 129
};