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

#ifndef SANGO_PLUGIN_SAVEDATA_PSS_H
#define SANGO_PLUGIN_SAVEDATA_PSS_H
#include "savedata.h"

namespace savedata {
struct PssProfilePayload {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  c16 name[13];
  c16 shoutout_message[17];

  u32 principal_id;
  u64 local_friend_code;

  union {
    u32 flags;

    u32  : 8;
    u32 icon_for_passerby : 8;
    u32 gender : 4;
    u32 has_accepted_eula : 1;
    u32 has_promotion_video : 1;
    u32 has_pokemon_in_party : 1;
    u32 has_hall_of_frame_completed : 1;
    u32 rejects_friend_requests : 1;
    u32 rejects_acquaintance_requests : 1;
    u32 rejects_passerby_requests : 1;
    u32 rejects_voice_chat : 1;
    u32 parental_control_photo_sharing_disabled : 1;
    u32 parental_control_internet_disabled : 1;
    u32 parental_control_friend_registration_closed : 1;
    u32 parental_control_streetpass_disabled : 1;
  };

  u32 geographic_region_id;

  u8 birth_month;
  u8 birth_day;
  u8 game_version; // (X, Y, OR, AS)
  u8 console_region; // EUR, USA, etc.

  union {
    u32 flags2;

    u32 rejects_promotion_video_requests : 1;
    u32 has_shout_out_message : 1;
    u32 meets_trade_conditions : 1;
    u32 rejects_lower_version_battles : 1;
    u32 rejects_lower_version_pr_videos : 1;
    u32 : 8;
    u32 icon_for_passerby_2 : 8;
    u32  : 11;
  };

  u8 outfit_data[16];
};

struct PssBattleSendPayload {
  u64 guest_friend_ids[3];
  u32 ticket_id;

  u32 guest_user_count : 2;
  u32 is_wifi_connection : 1;
  u32 confirmed_users_bitmask : 3;
  u32 has_handicap : 1;
  u32 battle_format : 8; // Single, Double, Triple, Rotation, etc.
  u32 battle_ruleset : 8; // Normal, Flat, Strict, etc.
  u32 is_voice_chat_enabled : 1;
  u32 padding : 8;

  u16 handicap_score;
  u8 player_relations[3];
  u8 reserved[3];
};

struct PssChangeBattleHostPayload {
  u32 old_ticket_id;
  u32 new_ticket_id;
  u64 new_host_transfer_id;

  u64 new_host_principal_id : 32;
  u64 old_guest_count : 2;
  u64  : 30;

  u64 old_guest_transfer_ids[2];
};

struct PssBattleReceivePayload {
  u32 ticket_id;
  u32 host_principal_id;
  u64 host_local_friend_code;
  u8 meets_battle_regulations;
  u8 reserved[23];
};

struct PssTradeSendPayload {
  u32 ticket_id;
  u32 guest_principal_id;
  u64 guest_local_friend_code;
  u8 guest_user_count;
  u8 is_voice_chat_enabled;
  u8 player_relation;
  u8 reserved[21];
};

struct PssTradeReceivePayload {
  u32 ticket_id;
  u32 host_principal_id;
  u64 host_local_friend_code;
  u8 reserved[24];
};

struct PssVoiceSendPayload {
  u32 ticket_id;
  u32 guest_principal_id;
  u64 guest_local_friend_code;
  u8 player_relation;
  u8 reserved[23];
};

struct PssVoiceReceivePayload {
  u32 ticket_id;
  u32 host_principal_id;
  u64 host_local_friend_code;
  u8 reserved[24];
};

struct PssPromotionVideoSendPayload {
  u32 ticket_id;
  u32 guest_principal_id;
  u64 guest_local_friend_code;
  u8 player_relation;
  u8 reserved[23];
};

struct PssPromotionVideoReceivePayload {
  u32 ticket_id;
  u32 host_principal_id;
  u64 host_local_friend_code;
  u8 reserved[24];
};

struct PssOPowerPayload {
  u32 ticket_id;
  u32 o_power_id;
  u32 sender_principal_id;
  u32 receiver_principal_id;
  u64 sender_local_friend_code;
  u64 receiver_local_friend_code;
  u8 player_relation;
  u8 reserved[7];
};

struct PssMessagePayload {
  u32 sender_principal_id;
  u32 receiver_principal_id;
  u64 sender_local_friend_code;
  u64 receiver_local_friend_code;
  u32 ticket_id;
  u8 player_relation;
  u8 reserved[11];
};

struct PssGameDataPayload {
  u32 streetpass_encounter_count;

  u32 pokemon_amie_species_id : 11;
  u32 pokemon_amie_form_id : 5;
  u32 pokemon_amie_gender : 2;
  u32 pokemon_amie_affection_level : 8;
  u32 pokemon_amie_reserved : 6;

  u64 lifetime_battle_wins : 20;
  u64 lifetime_trades : 20;
  u64 lifetime_nice_received : 17;
  u64 stats_reserved : 7;

  u64 lifetime_birthday_wishes : 17;
  u64 profile_history_event_1 : 4;
  u64 profile_history_event_2 : 4;
  u64 profile_history_event_3 : 4;
  u64 profile_history_event_4 : 4;
  u64 profile_history_event_5 : 4;
  u64 profile_history_event_6 : 4;
  u64 total_playtime_hours : 17;
  u64 profile_dummy_2 : 6;

  u64 profile_question_1 : 5;
  u64 profile_question_2 : 5;
  u64 profile_question_3 : 5;
  u64 profile_question_4 : 5;
  u64 profile_question_5 : 5;
  u64 profile_question_6 : 5;
  u64 profile_answer_1 : 5;
  u64 profile_answer_2 : 5;
  u64 profile_answer_3 : 5;
  u64 profile_answer_4 : 5;
  u64 profile_answer_5 : 5;
  u64 profile_answer_6 : 5;
  u64 profile_dummy_1 : 4;

  u16 auto_generated_event_details;
  u8 auto_generated_event_id;
  u8 reserved[2];

  u8 pokemon_amie_type_1;
  u8 pokemon_amie_type_2;

  u8 is_initialized_flag;
};

// 40 Bytes
typedef union {
  PssBattleSendPayload battle_send;
  PssBattleReceivePayload battle_receive;
  PssChangeBattleHostPayload change_battle_host;
  PssTradeSendPayload trade_send;
  PssTradeReceivePayload trade_receive;
  PssVoiceSendPayload voice_send;
  PssVoiceReceivePayload voice_receive;
  PssPromotionVideoSendPayload promotion_video_send;
  PssPromotionVideoReceivePayload promotion_video_receive;
  PssGameDataPayload game_data;
  PssOPowerPayload opower;
  PssMessagePayload message;
} PssPayload;

struct PssDatagram {
  u32 beacon_version : 24;
  u32 request_command_id : 7;
  u32 is_currently_playing_multiplayer : 1;

  u32 timestamp;

  PssProfilePayload profile;
  PssPayload payload;
};

struct PssUserData {
  PssDatagram datagram;
  PssGameDataPayload game_data;
  s16 handicap_battle_points;
  u8 handicap_battle_count;
  u8 relation;
};

// Friends, Acquaintances, Passerby
struct PssGroup {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  void* vtable;
  PssUserData user_data[100];
  u32 count;
};
} // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_PSS_H