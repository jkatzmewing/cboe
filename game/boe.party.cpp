#include <windows.h>
#include <cstdio>
#include "global.h"
#include "boe.fileio.h"
#include "boe.graphics.h"
#include "boe.newgraph.h"
#include "boe.specials.h"
#include "boe.itemdata.h"
#include "boe.infodlg.h"
#include "boe.items.h"
#include <cstring>
#include "boe.party.h"
#include "boe.monster.h"
#include "tools/dlogtool.h"
#include "boe.town.h"
#include "boe.combat.h"
#include "boe.locutils.h"
#include "boe.fields.h"
#include "boe.text.h"
#include "tools/soundtool.h"
#include "tools/mathutil.h"
#include "boe.graphutil.h"
#include "globvar.h"

void draw_caster_buttons();
void draw_spell_info();
void draw_spell_pc_info();
void put_pc_caster_buttons();
void put_pc_target_buttons();
void put_spell_led_buttons();
void put_spell_list();

// mode; // 0 - prefab 1 - regular
void init_party(short mode) {
  short i, j, k, l;
  Boolean store_help;

  boat_record_type null_boat = {location(), location(), location(),
                                200,        false,      false};
  horse_record_type null_horse = {location(), location(), location(),
                                  200,        false,      false};

  party.age = 0;
  party.gold = 200;
  party.food = 100;

  store_help = PSD[SDF_NO_INSTANT_HELP];

  for (i = 0; i < 310; i++)
    for (j = 0; j < 10; j++)
      party.stuff_done[i][j] = 0;
  if (scenario.prog_make_ver[0] < 2)
    PSD[SDF_LEGACY_SCENARIO] = 1; // old scenario format ?
  else
    PSD[SDF_LEGACY_SCENARIO] = 0;

  PSD[SDF_NO_INSTANT_HELP] = store_help;

  party.light_level = 0;
  party.outdoor_corner.x = 7;
  party.outdoor_corner.y = 8;
  party.i_w_c.x = 1;
  party.i_w_c.y = 1;
  party.loc_in_sec.x = 36;
  party.loc_in_sec.y = 36;
  party.p_loc.x = 84;
  party.p_loc.y = 84;
  for (i = 0; i < 30; i++)
    party.boats[i] = null_boat;
  for (i = 0; i < 30; i++)
    party.horses[i] = null_horse;

  party.in_boat = -1;
  party.in_horse = -1;

  for (i = 0; i < 4; i++)
    party.creature_save[i].which_town = 200;
  for (i = 0; i < 10; i++)
    party.out_c[i].exists = false;
  for (i = 0; i < 5; i++)
    for (j = 0; j < 10; j++)
      party.magic_store_items[i][j].variety = ITEM_TYPE_NO_ITEM;
  for (i = 0; i < 4; i++)
    party.imprisoned_monst[i] = 0;
  for (i = 0; i < 256; i++)
    party.m_seen[i] = 0;
  for (i = 0; i < 50; i++)
    party.journal_str[i] = -1;
  for (i = 0; i < 140; i++)
    for (j = 0; j < 2; j++)
      party.special_notes_str[i][j] = 0;
  for (i = 0; i < 120; i++)
    party.talk_save[i].personality = -1;

  party.total_m_killed = 0;
  party.total_dam_done = 0;
  party.total_xp_gained = 0;
  party.total_dam_taken = 0;
  party.direction = 0;
  party.at_which_save_slot = 0;

  for (i = 0; i < 20; i++)
    party.alchemy[i] = 0;
  for (i = 0; i < 200; i++)
    party.can_find_town[i] = 0;
  for (i = 0; i < 100; i++)
    party.key_times[i] = 30000;
  for (i = 0; i < 30; i++)
    party.party_event_timers[i] = 0;
  for (i = 0; i < 50; i++)
    party.spec_items[i] = 0;
  for (i = 0; i < 120; i++)
    party.help_received[i] = 0;
  for (i = 0; i < 200; i++)
    party.m_killed[i] = 0;

  sprintf((char *)party.scen_name, "");

  for (i = 0; i < 200; i++)
    for (j = 0; j < 8; j++)
      party.item_taken[i][j] = 0;

  refresh_store_items();

  for (i = 0; i < 6; i++) {
    adven[i] = return_dummy_pc();
    if (mode != 1)
      adven[i] = create_prefab_pc(i);
  }

  for (i = 0; i < 96; i++)
    for (j = 0; j < 96; j++)
      out_e[i][j] = 0;

  for (i = 0; i < 3; i++)
    for (j = 0; j < NUM_TOWN_ITEMS; j++)
      stored_items[i].items[j] = return_dummy_item();

  for (i = 0; i < 200; i++)
    for (j = 0; j < 8; j++)
      for (k = 0; k < 64; k++) {
        town_maps.town_maps[i][j][k] = 0;
      }

  for (i = 0; i < 100; i++)
    for (k = 0; k < 6; k++)
      for (l = 0; l < 48; l++)
        o_maps.outdoor_maps[i][k][l] = 0;

  // Default is save maps
  party.stuff_done[SFD_NO_MAPS] = 0;
  save_maps = true;

  // NOT DEBUG
  build_outdoors();
  get_reg_data();
}

// This is only called after a scenario is loaded and the party is put into it.
// Until that time, the party scen vals are uninited
// Then, it inits the party properly for starting the scenario based
// on the loaded scenario
void init_party_scen_data() {
  short i, j, k, l;
  Boolean stored_item = false;
  short store_help;

  party.age = 0;
  store_help = PSD[SDF_NO_INSTANT_HELP];
  for (i = 0; i < 310; i++)
    for (j = 0; j < 10; j++)
      party.stuff_done[i][j] = 0;
  PSD[SDF_NO_INSTANT_HELP] = store_help;
  if (scenario.prog_make_ver[0] < 2)
    PSD[SDF_LEGACY_SCENARIO] = 1; // old scenario format ?
  else
    PSD[SDF_LEGACY_SCENARIO] = 0;

  party.light_level = 0;
  party.outdoor_corner.x = scenario.out_sec_start.x;
  party.outdoor_corner.y = scenario.out_sec_start.y;
  party.i_w_c.x = 0;
  party.i_w_c.y = 0;
  party.loc_in_sec.x = scenario.out_start.x;
  party.loc_in_sec.y = scenario.out_start.y;
  party.p_loc.x = scenario.out_start.x;
  party.p_loc.y = scenario.out_start.y;
  for (i = 0; i < NUM_OF_BOATS; i++)
    party.boats[i] = scenario.scen_boats[i];
  for (i = 0; i < NUM_OF_HORSES; i++)
    party.horses[i] = scenario.scen_horses[i];
  for (i = 0; i < 30; i++) {
    if ((scenario.scen_boats[i].which_town >= 0) &&
        (scenario.scen_boats[i].boat_loc.x >= 0)) {
      if (party.boats[i].exists == false) {
        party.boats[i] = scenario.scen_boats[i];
        party.boats[i].exists = true;
      }
    }
    if ((scenario.scen_horses[i].which_town >= 0) &&
        (scenario.scen_horses[i].horse_loc.x >= 0)) {
      if (party.horses[i].exists == false) {
        party.horses[i] = scenario.scen_horses[i];
        party.horses[i].exists = true;
      }
    }
  }

  party.in_boat = -1;
  party.in_horse = -1;

  for (i = 0; i < 4; i++)
    party.creature_save[i].which_town = INVALID_TOWN;
  for (i = 0; i < 10; i++)
    party.out_c[i].exists = false;
  for (i = 0; i < 5; i++)
    for (j = 0; j < 10; j++)
      party.magic_store_items[i][j].variety = ITEM_TYPE_NO_ITEM;
  for (i = 0; i < 4; i++)
    party.imprisoned_monst[i] = 0;
  for (i = 0; i < 256; i++)
    party.m_seen[i] = 0;
  for (i = 0; i < 50; i++)
    party.journal_str[i] = -1;
  for (i = 0; i < 140; i++)
    for (j = 0; j < 2; j++)
      party.special_notes_str[i][j] = 0;
  for (i = 0; i < 120; i++)
    party.talk_save[i].personality = -1;

  party.direction = 0;
  party.at_which_save_slot = 0;
  for (i = 0; i < 200; i++)
    party.can_find_town[i] = 1 - scenario.town_hidden[i];
  for (i = 0; i < 100; i++)
    party.key_times[i] = 30000;
  for (i = 0; i < 30; i++)
    party.party_event_timers[i] = 0;
  for (i = 0; i < 50; i++)
    party.spec_items[i] = (scenario.special_items[i] >= 10) ? 1 : 0;
  for (i = 0; i < 200; i++)
    party.m_killed[i] = 0;

  for (i = 0; i < 200; i++)
    for (j = 0; j < 8; j++)
      party.item_taken[i][j] = 0;

  refresh_store_items();

  for (i = 0; i < 96; i++)
    for (j = 0; j < 96; j++)
      out_e[i][j] = 0;

  for (i = 0; i < 3; i++)
    for (j = 0; j < NUM_TOWN_ITEMS; j++)
      if (stored_items[i].items[j].variety != ITEM_TYPE_NO_ITEM)
        stored_item = true;
  if (stored_item == true)
    if (FCD(911, 0) == 1) {
      for (i = 0; i < 3; i++)
        for (j = 0; j < NUM_TOWN_ITEMS; j++)
          if (stored_items[i].items[j].variety != ITEM_TYPE_NO_ITEM)
            if (give_to_party(stored_items[i].items[j], false) == false) {
              i = 20;
              j = NUM_TOWN_ITEMS + 1;
            }
    }
  for (i = 0; i < 3; i++)
    for (j = 0; j < NUM_TOWN_ITEMS; j++) {
      stored_items[i].items[j] = return_dummy_item();
    }

  for (i = 0; i < 200; i++)
    for (j = 0; j < 8; j++)
      for (k = 0; k < 64; k++) {
        town_maps.town_maps[i][j][k] = 0;
      }

  for (i = 0; i < 100; i++)
    for (k = 0; k < 6; k++)
      for (l = 0; l < 48; l++)
        o_maps.outdoor_maps[i][k][l] = 0;
  get_reg_data();
}

// When the party is placed into a scen from the startinbg screen, this is
// called to put the game into game mode and load in the scen and init the party
// info party record already contains scen name
void put_party_in_scen() {
  short i, j;
  char strs[6][256] = {"", "", "", "", "", ""};
  short buttons[3] = {-1, -1, -1};
  Boolean item_took = false;

  for (j = 0; j < 6; j++)
    for (i = 0; i < 15; i++)
      adven[j].status[i] = 0;

  for (j = 0; j < 6; j++) {
    // unite party
    if (adven[j].main_status >= MAIN_STATUS_SPLIT)
      adven[j].main_status -= MAIN_STATUS_SPLIT;
    // start with maximum health
    adven[j].cur_health = adven[j].max_health;
    // start with maximum amount of spell points
    adven[j].cur_sp = adven[j].max_sp;
  }

  for (j = 0; j < 6; j++)
    for (i = 23; i >= 0; i--) {
      adven[j].items[i].special_class = 0;
      if (adven[j].items[i].graphic_num >= 150) // custom item graphic?
      {
        adven[j].takeItem(i + 30); // strip away special items
        item_took = true;
      }
      if (adven[j].items[i].ability == ITEM_SPELL_SUMMONING) {
        adven[j].takeItem(i + 30); // strip away summoning items
        item_took = true;
      }
      if (adven[j].items[i].ability == ITEM_SPELL_MASS_SUMMONING) {
        adven[j].takeItem(i + 30); // strip away summoning items
        item_took = true;
      }
    }

  if (item_took == true)
    FCD(910, 0);

  if (load_scenario() == false)
    return;

  init_party_scen_data();

  // if at this point, startup must be over, so make this call to make sure
  // we're ready, graphics wise
  end_startup();
  in_startup_mode = false;

  set_up_ter_pics();

  load_outdoors(party.outdoor_corner.x + 1, party.outdoor_corner.y + 1, 1, 1, 0,
                0, NULL);
  load_outdoors(party.outdoor_corner.x, party.outdoor_corner.y + 1, 0, 1, 0, 0,
                NULL);
  load_outdoors(party.outdoor_corner.x + 1, party.outdoor_corner.y, 1, 0, 0, 0,
                NULL);
  load_outdoors(party.outdoor_corner.x, party.outdoor_corner.y, 0, 0, 0, 0,
                NULL);

  stat_screen_mode = 0;
  build_outdoors();
  erase_out_specials();

  current_pc = first_active_pc();
  force_town_enter(scenario.which_town_start, scenario.where_start);
  start_town_mode(scenario.which_town_start, 9);
  center = scenario.where_start;
  update_explored(scenario.where_start);
  overall_mode = MODE_TOWN;
  create_clip_region();
  redraw_screen(0);
  set_stat_window(0);
  adjust_spell_menus();
  adjust_monst_menu();

  // Throw up intro dialog
  buttons[0] = 1;
  for (j = 0; j < 6; j++)
    if (strlen(data_store5->scen_strs[4 + j]) > 0) {
      for (i = 0; i < 6; i++)
        strcpy((char *)strs[i], data_store5->scen_strs[4 + i]);
      if (scenario.intro_pic == 100)
        custom_choice_dialog((char *)strs, 2400, buttons);
      else
        custom_choice_dialog((char *)strs, -1 * (1600 + scenario.intro_pic),
                             buttons);
      j = 6;
    }
  give_help(1, 2, 0);
  // this is kludgy, put here to prevent problems
  for (i = 0; i < 50; i++)
    party.spec_items[i] = (scenario.special_items[i] >= 10) ? 1 : 0;
}

pc_record_type return_dummy_pc() {
  pc_record_type dummy_pc;
  short i;

  dummy_pc.main_status = MAIN_STATUS_ABSENT;
  sprintf((char *)dummy_pc.name, "\n");

  for (i = 0; i < 30; i++)
    dummy_pc.skills[i] = (i < 3) ? 1 : 0;
  dummy_pc.cur_health = 6;
  dummy_pc.max_health = 6;
  dummy_pc.cur_sp = 0;
  dummy_pc.max_sp = 0;
  dummy_pc.experience = 0;
  dummy_pc.skill_pts = 60;
  dummy_pc.level = 1;
  for (i = 0; i < 15; i++)
    dummy_pc.status[i] = 0;
  for (i = 0; i < 24; i++)
    dummy_pc.items[i] = return_dummy_item();
  for (i = 0; i < 24; i++)
    dummy_pc.equip[i] = false;

  for (i = 0; i < 62; i++) {
    dummy_pc.priest_spells[i] = (i < 30) ? true : false;
    dummy_pc.mage_spells[i] = (i < 30) ? true : false;
  }
  dummy_pc.which_graphic = 0;
  dummy_pc.weap_poisoned = 24;

  for (i = 0; i < 15; i++) {
    dummy_pc.advan[i] = false;
    dummy_pc.traits[i] = false;
  }
  dummy_pc.race = RACE_HUMAN;
  dummy_pc.exp_adj = 100;
  dummy_pc.direction = 0;

  return dummy_pc;
}

pc_record_type create_prefab_pc(short num) {
  pc_record_type dummy_pc;
  short i;
  short pc_stats[6][19] = {
      {8, 6, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0},
      {8, 7, 2, 0, 0, 6, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0},
      {8, 6, 2, 3, 3, 0, 0, 2, 0, 0, 0, 0, 0, 0, 4, 4, 0, 2, 1},
      {3, 2, 6, 2, 0, 0, 2, 0, 0, 3, 0, 3, 0, 1, 0, 0, 0, 0, 0},
      {2, 2, 6, 3, 0, 0, 2, 0, 0, 2, 1, 4, 0, 0, 0, 0, 0, 0, 1},
      {2, 2, 6, 0, 2, 0, 2, 0, 1, 0, 3, 3, 2, 0, 0, 0, 0, 0, 0}};
  short pc_health[6] = {22, 24, 24, 16, 16, 18};
  short pc_sp[6] = {0, 0, 0, 20, 20, 21};
  short pc_graphics[6] = {3, 32, 29, 16, 23, 14};
  short pc_race[6] = {0, 2, 1, 0, 0, 0};
  short pc_t[6][15] = {{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
                       {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
                       {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                       {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                       {0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
                       {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

  dummy_pc.main_status = MAIN_STATUS_ALIVE;

  switch (num) {
  case 0:
    strncpy((char *)dummy_pc.name, "Jenneke", (size_t)20);
    break;
  case 1:
    strncpy((char *)dummy_pc.name, "Thissa", (size_t)20);
    break;
  case 2:
    strncpy((char *)dummy_pc.name, "Frrrrrr", (size_t)20);
    break;
  case 3:
    strncpy((char *)dummy_pc.name, "Adrianna", (size_t)20);
    break;
  case 4:
    strncpy((char *)dummy_pc.name, "Feodoric", (size_t)20);
    break;
  case 5:
    strncpy((char *)dummy_pc.name, "Michael", (size_t)20);
    break;
  }

  for (i = 0; i < 19; i++)
    dummy_pc.skills[i] = pc_stats[num][i];

  dummy_pc.cur_health = pc_health[num];
  dummy_pc.max_health = pc_health[num];
  dummy_pc.experience = 0;
  dummy_pc.skill_pts = 0;
  dummy_pc.level = 1;

  for (i = 0; i < 15; i++)
    dummy_pc.status[i] = 0;
  for (i = 0; i < 24; i++)
    dummy_pc.items[i] = return_dummy_item();
  for (i = 0; i < 24; i++)
    dummy_pc.equip[i] = false;
  dummy_pc.cur_sp = pc_sp[num];
  dummy_pc.max_sp = pc_sp[num];

  for (i = 0; i < 62; i++) {
    dummy_pc.priest_spells[i] = (i < 30) ? true : false; ////
    dummy_pc.mage_spells[i] = (i < 30) ? true : false;
  }

  for (i = 0; i < 15; i++) {
    dummy_pc.traits[i] = pc_t[num][i];
    dummy_pc.advan[i] = false;
  }

  dummy_pc.race = pc_race[num];
  dummy_pc.exp_adj = 100;
  dummy_pc.direction = 0;

  dummy_pc.which_graphic = pc_graphics[num];

  return dummy_pc;
}

Boolean create_pc(short spot, short parent_num)
// spot; // if spot is 6, find one
{
  Boolean still_ok = true;

  if (spot == INVALID_PC) {
    for (spot = 0; spot < INVALID_PC; spot++)
      if (adven[spot].main_status == MAIN_STATUS_ABSENT)
        break;
  }
  if (spot == INVALID_PC)
    return false;

  adven[spot] = return_dummy_pc();

  pick_race_abil(&adven[spot], 0, parent_num);

  if (parent_num != 0)
    cd_initial_draw(989);

  still_ok = spend_xp(spot, 0, parent_num);
  if (still_ok == false)
    return false;
  adven[spot].cur_health = adven[spot].max_health;
  adven[spot].cur_sp = adven[spot].max_sp;
  if (parent_num != 0)
    cd_initial_draw(989);

  pick_pc_graphic(spot, 0, parent_num);

  if (parent_num != 0)
    cd_initial_draw(989);
  pick_pc_name(spot, parent_num);

  adven[spot].main_status = MAIN_STATUS_ALIVE;

  if (in_startup_mode == false) {
    adven[spot].items[0] = start_items[adven[spot].race * 2];
    adven[spot].equip[0] = true;
    adven[spot].items[1] = start_items[adven[spot].race * 2 + 1];
    adven[spot].equip[1] = true;

    // Do stat adjs for selected race.
    if (adven[spot].race == RACE_NEPHIL)
      adven[spot].skills[SKILL_DEXTERITY] += 2;
    if (adven[spot].race == RACE_SLITH) {
      adven[spot].skills[SKILL_STRENGTH] += 2;
      adven[spot].skills[SKILL_INTELLIGENCE] += 1;
    }
    adven[spot].max_sp += adven[spot].skills[SKILL_MAGE_SPELLS] * 3 +
                          adven[spot].skills[SKILL_PRIEST_SPELLS] * 3;
    adven[spot].cur_sp = adven[spot].max_sp;
  }

  return true;
}

void pc_array::heal(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].heal(amt);
}

void pc_array::cure(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].cure(amt);
}

void pc_array::dumbfound(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].dumbfound(amt);
}

void pc_array::disease(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].disease(amt);
}

void increase_light(short amt) {
  short i;
  location where;

  party.light_level += amt;
  if (is_combat()) {
    for (i = 0; i < 6; i++)
      if (adven[i].isAlive()) {
        update_explored(pc_pos[i]);
      }
  } else {
    where = get_cur_loc();
    update_explored(where);
  }
  put_pc_screen();
}

void pc_array::restoreSP(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].restoreSP(amt);
}

void pc_array::giveXP(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].giveXP(amt);
}

void pc_array::drainXP(short amt) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].drainXP(amt);
}

void do_xp_keep(short pc_num, short mode) {
  for (i = 0; i < 20; i++)
    adven[pc_num].skills[i] = store_skills[i];
  adven[pc_num].cur_health += store_h - adven[pc_num].max_health;
  adven[pc_num].max_health = store_h;
  adven[pc_num].cur_sp += store_sp - adven[pc_num].max_sp;
  adven[pc_num].max_sp = store_sp;
  if (mode == 1)
    party.gold = store_g;
  adven[pc_num].skill_pts = store_skp;
}

void draw_xp_skills() {
  short i;
  for (i = 0; i < 19; i++) {
    if ((store_skp >= skill_cost[i]) && (store_g >= skill_g_cost[i]))
      cd_text_frame(1010, 54 + i, 11);
    else
      cd_text_frame(1010, 54 + i, 1);
    cd_set_item_num(1010, 54 + i, store_skills[i]);
  }

  if ((store_skp >= 1) && (store_g >= 10))
    cd_text_frame(1010, 52, 11);
  else
    cd_text_frame(1010, 52, 1);
  cd_set_item_num(1010, 52, store_h);
  if ((store_skp >= 1) && (store_g >= 15))
    cd_text_frame(1010, 53, 11);
  else
    cd_text_frame(1010, 53, 1);
  cd_set_item_num(1010, 53, store_sp);
}

void do_xp_draw() {
  char get_text[256];
  short mode, pc_num;

  mode = store_train_mode;
  pc_num = store_train_pc;
  if (mode == 0) {
    if (adven[pc_num].main_status == MAIN_STATUS_ALIVE)
      sprintf((char *)get_text, "%s", (char *)adven[pc_num].name);
    else
      sprintf((char *)get_text, "New PC");
  } else
    sprintf((char *)get_text, "%s", (char *)adven[pc_num].name);

  cd_set_item_text(1010, 51, get_text);

  for (i = 0; i < 20; i++)
    store_skills[i] = adven[pc_num].skills[i];
  store_h = adven[pc_num].max_health;
  store_sp = adven[pc_num].max_sp;
  store_g = (mode == 0) ? 20000 : party.gold;
  store_skp = adven[pc_num].skill_pts;

  draw_xp_skills();

  update_gold_skills();
}

Boolean spend_xp_event_filter(short item_hit) {
  short mode, pc_num;

  Boolean talk_done = false;

  mode = store_train_mode;
  pc_num = store_train_pc;

  switch (item_hit) {
  case 73:
    if ((mode == 0) && (adven[pc_num].main_status < MAIN_STATUS_ABSENT))
      adven[pc_num].main_status = MAIN_STATUS_ABSENT;
    dialog_answer = 0;
    talk_done = true;
    break;

  case 82:
    party.help_received[10] = 0;
    give_help(210, 11, 1010);
    break;

  case 3:
  case 4:
    if (((store_h >= 250) && (item_hit == 4)) ||
        ((store_h == adven[pc_num].max_health) && (item_hit == 3) &&
         (mode == 1)) ||
        ((store_h == 6) && (item_hit == 3) && (mode == 0)))
      MessageBeep(MB_OK);
    else {
      if (item_hit == 3) {
        store_g += 10;
        store_h -= 2;
        store_skp += 1;
      } else {
        if ((store_g < 10) || (store_skp < 1)) {
          if (store_g < 10)
            give_help(24, 0, 1010);
          else
            give_help(25, 0, 1010);
          MessageBeep(MB_OK);
        } else {
          store_g -= 10;
          store_h += 2;
          store_skp -= 1;
        }
      }

      update_gold_skills();
      cd_set_item_num(1010, 52, store_h);
      draw_xp_skills();
    }
    break;

  case 5:
  case 6:
    if (((store_sp >= 150) && (item_hit == 6)) ||
        ((store_sp == adven[pc_num].max_sp) && (item_hit == 5) &&
         (mode == 1)) ||
        ((store_sp == 0) && (item_hit == 5) && (mode == 0)))
      MessageBeep(MB_OK);
    else {
      if (item_hit == 5) {
        store_g += 15;
        store_sp -= 1;
        store_skp += 1;
      } else {
        if ((store_g < 15) || (store_skp < 1)) {
          if (store_g < 15)
            give_help(24, 0, 1010);
          else
            give_help(25, 0, 1010);
          MessageBeep(MB_OK);
        } else {
          store_sp += 1;
          store_g -= 15;
          store_skp -= 1;
        }
      }

      update_gold_skills();
      cd_set_item_num(1010, 53, store_sp);
      draw_xp_skills();
    }
    break;

  case 48:
    do_xp_keep(pc_num, mode);
    dialog_answer = 1;
    talk_done = true;
    break;

  case 49:
    if (mode == 0) {
      MessageBeep(MB_OK);
      break;
    } else {
      do_xp_keep(pc_num, mode);
      do {
        pc_num = (pc_num == 0) ? 5 : pc_num - 1;
      } while (adven[pc_num].isAlive() == false);
      store_train_pc = pc_num;
      do_xp_draw();
    }
    break;

  case 50:
    if (mode == 0) {
      MessageBeep(MB_OK);
      break;
    } else {
      do_xp_keep(pc_num, mode);
      do {
        pc_num = (pc_num == 5) ? 0 : pc_num + 1;
      } while (adven[pc_num].isAlive() == false);
      store_train_pc = pc_num;
      do_xp_draw();
    }
    break;

  case 100:
    break;

  default:
    if (item_hit >= 100) {
      item_hit -= 100;
      if ((item_hit == 3) || (item_hit == 4)) {
        display_strings_with_nums(10, 63, 0, 0, "About Health", 57, 724, 1010);
      } else if ((item_hit == 5) || (item_hit == 6)) {
        display_strings_with_nums(10, 64, 0, 0, "About Spell Points", 57, 724,
                                  1010);
      } else {
        which_skill = (item_hit - 7) / 2;
        display_skills(which_skill, 1010);
      }
    } else {
      which_skill = (item_hit - 7) / 2;

      if (((store_skills[which_skill] >= skill_max[which_skill]) &&
           ((item_hit - 7) % 2 == 1)) ||
          ((store_skills[which_skill] == adven[pc_num].skills[which_skill]) &&
           ((item_hit - 7) % 2 == 0) && (mode == 1)) ||
          ((store_skills[which_skill] == 0) && ((item_hit - 7) % 2 == 0) &&
           (mode == 0) && (which_skill > 2)) ||
          ((store_skills[which_skill] == 1) && ((item_hit - 7) % 2 == 0) &&
           (mode == 0) && (which_skill <= 2)))
        MessageBeep(MB_OK);
      else {
        if ((item_hit - 7) % 2 == 0) {
          store_g += skill_g_cost[which_skill];
          store_skills[which_skill] -= 1;
          store_skp += skill_cost[which_skill];
        } else {
          if ((store_g < skill_g_cost[which_skill]) ||
              (store_skp < skill_cost[which_skill])) {
            if (store_g < skill_g_cost[which_skill])
              give_help(24, 0, 1010);
            else
              give_help(25, 0, 1010);
            MessageBeep(MB_OK);
          } else {
            store_skills[which_skill] += 1;
            store_g -= skill_g_cost[which_skill];
            store_skp -= skill_cost[which_skill];
          }
        }

        update_gold_skills();
        cd_set_item_num(1010, 54 + which_skill, store_skills[which_skill]);
        draw_xp_skills();
      }
    }
    break;
  }

  store_train_pc = pc_num;
  if (talk_done == true) {
    dialog_not_toast = false;
  }
  return false;
}

void update_gold_skills() {
  cd_set_item_num(1010, 47, ((store_train_mode == 0) ? 0 : (short)store_g));
  cd_set_item_num(1010, 46, (short)store_skp);
}

Boolean spend_xp(short pc_num, short mode, short parent)
// short mode; // 0 - create  1 - train
// returns 1 if cancelled
{
  char get_text[256], text2[256];

  store_train_pc = pc_num;
  store_train_mode = mode;

  SetCursor(sword_curs);

  cd_create_dialog_parent_num(1010, parent);
  sprintf(get_text, "Health (%d/%d)", 1, 10);
  cd_add_label(1010, 52, (char *)get_text, 1075);
  sprintf(get_text, "Spell Pts. (%d/%d)", 1, 15);
  cd_add_label(1010, 53, (char *)get_text, 1075);
  for (i = 54; i < 73; i++) {
    GetIndString(text2, 9, 1 + 2 * (i - 54));
    sprintf(get_text, "%s (%d/%d)", text2, skill_cost[i - 54],
            skill_g_cost[i - 54]);
    cd_add_label(1010, i, get_text, (i < 63) ? 1075 : 1069);
  }
  do_xp_draw();

  dialog_answer = 0;

  if (party.help_received[10] == 0) {
    cd_initial_draw(1010);
    give_help(10, 11, 1010);
  }

  while (dialog_not_toast)
    ModalDialog();

  cd_kill_dialog(1010, 0);

  return dialog_answer;
}

short pc_array::getMageLore() {
  short total = 0;

  for (int i = 0; i < NUM_OF_PCS; i++)
    if (pc[i].isAlive())
      total += pc[i].skills[SKILL_MAGE_LORE];

  return total;
}

Boolean poison_weapon(short pc_num, short how_much, short safe)
// short safe; // 1 - always succeeds
{
  short i, weap = 24, p_level, r1;
  short p_chance[21] = {40, 72, 81, 85, 88, 89, 90,  91,  92,  93, 94,
                        94, 95, 95, 96, 97, 98, 100, 100, 100, 100};

  for (i = 0; i < 24; i++)
    if ((adven[pc_num].equip[i] == true) && (is_weapon(pc_num, i) == true)) {
      weap = i;
      i = 30;
    }
  if (weap == 24) {
    add_string_to_buf("  No weapon equipped.       ");
    return false;
  } else {
    p_level = how_much;
    add_string_to_buf("  You poison your weapon.       ");
    r1 = get_ran(1, 0, 100);
    // Nimble?
    if (adven[pc_num].traits[TRAIT_NIMBLE] == true)
      r1 -= 6;
    if ((r1 > p_chance[adven[pc_num].skills[SKILL_POISON]]) && (safe == 0)) {
      add_string_to_buf("  Poison put on badly.         ");
      p_level = p_level / 2;
      r1 = get_ran(1, 0, 100);
      if (r1 > p_chance[adven[pc_num].skills[SKILL_POISON]] + 10) {
        add_string_to_buf("  You nick yourself.          ");
        adven[pc_num].status[STATUS_POISON] += p_level;
      }
    }
    if (safe != 1)
      play_sound(55);
    adven[pc_num].weap_poisoned = weap;
    adven[pc_num].status[STATUS_POISONED_WEAPON] =
        max(adven[pc_num].status[STATUS_POISONED_WEAPON], p_level);

    return true;
  }
}

Boolean is_weapon(short pc_num, short item) {
  if ((adven[pc_num].items[item].variety == ITEM_TYPE_ONE_HANDED) ||
      (adven[pc_num].items[item].variety == ITEM_TYPE_TWO_HANDED) ||
      (adven[pc_num].items[item].variety == ITEM_TYPE_ARROW) ||
      (adven[pc_num].items[item].variety == ITEM_TYPE_BOLTS))
    return true;
  else
    return false;
}

void cast_spell(short type, short situation)
// short type; // 0 - mage  1 - priest
// short situation; // 0 - out  1 - town
{
  short spell;

  if ((is_town()) && (is_antimagic(c_town.p_loc.x, c_town.p_loc.y))) {
    add_string_to_buf("  Not in antimagic field.");
    return;
  }

  if (spell_forced == false)
    spell = pick_spell(6, type, situation);
  else {
    if (repeat_cast_ok(type) == false)
      return;
    spell = (type == 0) ? store_mage : store_priest;
  }

  if (spell < 70) {
    print_spell_cast(spell, type);

    if (type == 0)
      do_mage_spell(pc_casting, spell);
    else
      do_priest_spell(pc_casting, spell);
    put_pc_screen();
  }
}

Boolean repeat_cast_ok(short type) {
  short store_select, who_would_cast, what_spell;

  if (overall_mode == MODE_COMBAT)
    who_would_cast = current_pc;
  else if (overall_mode < MODE_TALK_TOWN)
    who_would_cast = pc_casting;
  else
    return false;

  if (is_combat())
    what_spell = pc_last_cast[type][who_would_cast];
  else
    what_spell = (type == 0) ? store_mage : store_priest;

  if (pc_can_cast_spell(who_would_cast, type, what_spell) == false) {
    add_string_to_buf("Repeat cast: Can't cast.");
    return false;
  }
  store_select = (type == 0) ? mage_need_select[what_spell]
                             : priest_need_select[what_spell];
  if ((store_select > 0) && (store_spell_target == 6)) {
    add_string_to_buf("Repeat cast: No target stored.");
    return false;
  }
  if ((store_select == 2) &&
      ((adven[store_spell_target].main_status == MAIN_STATUS_ABSENT) ||
       (adven[store_spell_target].main_status > MAIN_STATUS_STONE))) {
    add_string_to_buf("Repeat cast: No target stored.");
    return false;
  }
  if ((store_select == 1) && (adven[store_spell_target].isAlive() == false)) {
    add_string_to_buf("Repeat cast: No target stored.");
    return false;
  }

  return true;
}

void do_mage_spell(short pc_num, short spell_num) {
  short i, j, item, target, adj, store;
  location where;
  unsigned short r1;

  where = c_town.p_loc;
  play_sound(25);
  current_spell_range = 8;

  adj = adven[who_cast].statAdj(SKILL_INTELLIGENCE);

  switch (spell_num) {
  case SPELL_MAGE_LIGHT: // Light
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    increase_light(50);
    break;

  case SPELL_MAGE_IDENTIFY: // Identify
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    ASB("All of your items are identified.");
    for (i = 0; i < 6; i++)
      for (j = 0; j < 24; j++)
        adven[i].items[j].item_properties =
            adven[i].items[j].item_properties | 1;
    break;

  case SPELL_MAGE_TRUE_SIGHT: // true sight
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    for (where.x = 0; where.x < 64; where.x++)
      for (where.y = 0; where.y < 64; where.y++)
        if (dist(where, c_town.p_loc) <= 2)
          make_explored(where.x, where.y);
    clear_map();
    break;

  case SPELL_MAGE_SUMMON_BEAST: // summon beast ////
    r1 = get_summon_monster(1);
    if (r1 == 0)
      break;
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    store = get_ran(3, 1, 4) + adj;
    if (summon_monster(r1, where, store, 2) == false)
      add_string_to_buf("  Summon failed.");
    break;
  case SPELL_MAGE_WEAK_SUMMONING: // summon 1
    store = adven[who_cast].level / 5 +
            adven[who_cast].statAdj(SKILL_INTELLIGENCE) / 3 + get_ran(1, 0, 2);
    j = minmax(1, 7, (int)store);
    r1 = get_summon_monster(1); ////
    if (r1 == 0)
      break;
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    store = get_ran(4, 1, 4) + adj;
    for (i = 0; i < j; i++)
      if (summon_monster(r1, where, store, 2) == false)
        add_string_to_buf("  Summon failed.");
    break;
  case SPELL_MAGE_SUMMONING: // summon 2
    store = adven[who_cast].level / 7 +
            adven[who_cast].statAdj(SKILL_INTELLIGENCE) / 3 + get_ran(1, 0, 1);
    j = minmax(1, 6, (int)store);
    r1 = get_summon_monster(2); ////
    if (r1 == 0)
      break;
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    store = get_ran(5, 1, 4) + adj;
    for (i = 0; i < j; i++)
      if (summon_monster(r1, where, store, 2) == false)
        add_string_to_buf("  Summon failed.");
    break;
  case SPELL_MAGE_MAJOR_SUMMON: // summon 3
    store = adven[who_cast].level / 10 +
            adven[who_cast].statAdj(SKILL_INTELLIGENCE) / 3 + get_ran(1, 0, 1);
    j = minmax(1, 5, (int)store);
    r1 = get_summon_monster(3); ////
    if (r1 == 0)
      break;
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    store = get_ran(7, 1, 4) + adven[who_cast].statAdj(SKILL_INTELLIGENCE);
    for (i = 0; i < j; i++)
      if (summon_monster(r1, where, store, 2) == false)
        add_string_to_buf("  Summon failed.");
    break;
  case SPELL_MAGE_DAEMON:
    store = get_ran(5, 1, 4) + 2 * adven[who_cast].statAdj(SKILL_INTELLIGENCE);
    if (summon_monster(85, where, store, 2) == false)
      add_string_to_buf("  Summon failed.");
    else
      adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    break;

  case SPELL_MAGE_DISPEL_FIELDS: // dispel field
    add_string_to_buf("  Target spell.               ");
    current_pat = square;
    overall_mode = MODE_TOWN_TARGET;
    set_town_spell(spell_num, pc_num);
    break;

  case SPELL_MAGE_LONG_LIGHT: // Long light
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    increase_light(200);
    break;

  case SPELL_MAGE_MAGIC_MAP:                     // Magic map
    item = adven[pc_num].hasAbil(ITEM_SAPPHIRE); ////
    if (item == 24)
      add_string_to_buf("  You need a sapphire.        ");
    else if ((c_town.town.defy_scrying == 1) || (c_town.town.defy_mapping == 1))
      add_string_to_buf("  The spell fails.                ");
    else {
      adven[pc_num].removeCharge(item);
      adven[pc_num].cur_sp -= spell_cost[0][spell_num];
      add_string_to_buf("  As the sapphire dissolves,       ");
      add_string_to_buf("  you have a vision.               ");
      for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++)
          make_explored(i, j);
      clear_map();
    }
    break;

  case SPELL_MAGE_STEALTH: // Stealth
    adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    party.stuff_done[SDF_STEALTH] += max(6, adven[pc_num].level * 2);
    break;

  case SPELL_MAGE_SCRY_MONSTER:
  case SPELL_MAGE_UNLOCK:
  case SPELL_MAGE_CAPTURE_SOUL:
  case SPELL_MAGE_DISPEL_BARRIER: //  Scry monster, Unlock, Capture Soul, Disp.
                                  //  barrier
    add_string_to_buf("  Target spell.               ");
    current_pat = single;
    overall_mode = MODE_TOWN_TARGET;
    set_town_spell(spell_num, pc_num);
    break;

  case SPELL_MAGE_FIRE_BARRIER:
  case SPELL_MAGE_FORCE_BARRIER:
  case SPELL_MAGE_QUICKFIRE: // fire and force barriers, quickfire
    add_string_to_buf("  Target spell.               ");
    overall_mode = MODE_TOWN_TARGET;
    current_pat = single;
    set_town_spell(spell_num, pc_num);
    break;

  case SPELL_MAGE_ANTIMAGIC_CLOUD: // antimagic
    add_string_to_buf("  Target spell.               ");
    overall_mode = MODE_TOWN_TARGET;
    current_pat = radius2;
    set_town_spell(spell_num, pc_num);
    break;

  case SPELL_MAGE_FLIGHT: // fly
    if (party.stuff_done[SDF_FLYING] > 0) {
      add_string_to_buf("  Not while already flying.          ");
      return;
    }
    if (party.in_boat >= 0)
      add_string_to_buf("  Leave boat first.             ");
    else if (party.in_horse >= 0) ////
      add_string_to_buf("  Leave horse first.             ");
    else {
      adven[pc_num].cur_sp -= spell_cost[0][spell_num];
      add_string_to_buf("  You start flying!               ");
      party.stuff_done[SDF_FLYING] = 3;
    }
    break;

  case SPELL_MAGE_RESIST_MAGIC:
  case SPELL_MAGE_PROTECTION: //  resist magic, protection (!!! no cap ??? !!!)
    target = store_spell_target;
    if (target < 6)
      adven[pc_num].cur_sp -= spell_cost[0][spell_num];
    if ((spell_num == SPELL_MAGE_PROTECTION) && (target < 6)) {
      adven[target].status[STATUS_INVULNERABLE] +=
          2 + adven[pc_num].statAdj(SKILL_INTELLIGENCE) + get_ran(2, 1, 2);
      for (i = 0; i < 6; i++)
        if (adven[i].isAlive()) {
          adven[i].status[STATUS_MAGIC_RESISTANCE] +=
              4 + adven[pc_num].level / 3 +
              adven[pc_num].statAdj(SKILL_INTELLIGENCE);
        }
      sprintf(c_line, "  Party protected.                         ");
    }
    if ((spell_num == SPELL_MAGE_RESIST_MAGIC) && (target < 6)) {
      adven[target].status[STATUS_MAGIC_RESISTANCE] +=
          2 + adven[pc_num].statAdj(SKILL_INTELLIGENCE) + get_ran(2, 1, 2);
      sprintf(c_line, "  %s protected.", adven[target].name);
    }
    add_string_to_buf(c_line);
    break;
  }
}

void do_priest_spell(short pc_num, short spell_num) {
  short r1, r2, target, i, item, store, adj, x, y;
  location loc;
  location where;

  short store_victim_health, store_caster_health, targ_damaged; // for symbiosis

  where = c_town.p_loc;

  adj = adven[pc_num].statAdj(SKILL_INTELLIGENCE);

  play_sound(24);
  current_spell_range = 8;

  switch (spell_num) {
  case SPELL_PRIEST_LOCATION:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];

    if (is_town()) {
      loc = (overall_mode == MODE_OUTDOORS) ? party.p_loc : c_town.p_loc;
      sprintf(c_line, "  You're at: x %d  y %d.", (short)loc.x, (short)loc.y);
    }
    if (is_out()) {
      loc = (overall_mode == MODE_OUTDOORS) ? party.p_loc : c_town.p_loc;
      x = loc.x;
      y = loc.y;
      x += 48 * party.outdoor_corner.x;
      y += 48 * party.outdoor_corner.y;
      sprintf(c_line, "  You're outside at: x %d  y %d.", x, y);
    }
    add_string_to_buf(c_line);
    break;

  case SPELL_PRIEST_MINOR_MANNA:
  case SPELL_PRIEST_MANNA: // manna spells
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    store = adven[pc_num].level / 3 +
            2 * adven[who_cast].statAdj(SKILL_INTELLIGENCE) + get_ran(2, 1, 4);
    r1 = max(0, store);
    if (spell_num == SPELL_PRIEST_MINOR_MANNA)
      r1 = r1 / 3 + 1;
    sprintf(c_line, "  You gain %d food.   ", r1);
    add_string_to_buf(c_line);
    party.giveFood(r1, true);
    break;

  case SPELL_PRIEST_RITUAL_SANCTIFY: // Ritual - Sanctify
    add_string_to_buf("  Sanctify which space?               ");
    current_pat = single;
    overall_mode = MODE_TOWN_TARGET;
    set_town_spell(100 + spell_num, pc_num);
    break;

  case SPELL_PRIEST_LIGHT:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    // party.light_level += 210;
    increase_light(210);
    break;

  case SPELL_PRIEST_SUMMON_SPIRIT:
    store = adven[who_cast].statAdj(SKILL_INTELLIGENCE);
    if (summon_monster(125, where, get_ran(2, 1, 4) + store, 2) == false)
      add_string_to_buf("  Summon failed.");
    else
      adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    break;
  case SPELL_PRIEST_STICKS_TO_SNAKES:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    r1 = adven[who_cast].level / 6 +
         adven[who_cast].statAdj(SKILL_INTELLIGENCE) / 3 + get_ran(1, 0, 1);
    for (i = 0; i < r1; i++) {
      r2 = get_ran(1, 0, 7);
      store = get_ran(2, 1, 5) + adven[who_cast].statAdj(SKILL_INTELLIGENCE);
      if (summon_monster((r2 == 1) ? 100 : 99, where, store, 2) == false)
        add_string_to_buf("  Summon failed.");
    }
    break;
  case SPELL_PRIEST_SUMMON_HOST:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    store = get_ran(2, 1, 4) + adven[who_cast].statAdj(SKILL_INTELLIGENCE);
    if (summon_monster(126, where, store, 2) == false)
      add_string_to_buf("  Summon failed.");
    for (i = 0; i < 4; i++) {
      store = get_ran(2, 1, 4) + adven[who_cast].statAdj(SKILL_INTELLIGENCE);
      if (summon_monster(125, where, store, 2) == false)
        add_string_to_buf("  Summon failed.");
    }
    break;
  case SPELL_PRIEST_GUARDIAN:
    store = get_ran(6, 1, 4) + adven[who_cast].statAdj(SKILL_INTELLIGENCE);
    if (summon_monster(122, where, store, 2) == false)
      add_string_to_buf("  Summon failed.");
    else
      adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    break;

  case SPELL_PRIEST_MOVE_MOUNTAINS:
    add_string_to_buf("  Destroy what?               ");
    // current_pat = (spell_num == SPELL_PRIEST_MOVE_MOUNTAINS) ? single :
    // square; //apparently was planned (by Jeff) to do the shatter spell as well
    current_pat = single;
    overall_mode = MODE_TOWN_TARGET;
    set_town_spell(100 + spell_num, pc_num);
    break;

  case SPELL_PRIEST_DISPEL_FIELDS: // dispelling fields
    add_string_to_buf("  Target spell.               ");
    // current_pat = (spell_num == SPELL_PRIEST_AWAKEN) ? single : radius2;
    current_pat = radius2;
    overall_mode = MODE_TOWN_TARGET;
    set_town_spell(100 + spell_num, pc_num);
    break;

  case SPELL_PRIEST_DETECT_LIFE: // Detect life
    add_string_to_buf("  Monsters now on map.                ");
    party.stuff_done[SDF_DETECT_MONSTER] += 6 + get_ran(1, 0, 6);
    clear_map();
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    break;
  case SPELL_PRIEST_FIREWALK: // firewalk
    add_string_to_buf("  You are now firewalking.                ");
    party.stuff_done[SDF_LAVAWALK] += adven[pc_num].level / 12 + 2;
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    break;

  case SPELL_PRIEST_SHATTER: // shatter
    add_string_to_buf("  You send out a burst of energy. ");
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    for (loc.x = where.x - 1; loc.x < where.x + 2; loc.x++)
      for (loc.y = where.y - 1; loc.y < where.y + 2; loc.y++)
        loc.crumbleWall();
    update_explored(c_town.p_loc);
    break;

  case SPELL_PRIEST_WORD_OF_RECALL:
    if (overall_mode > MODE_OUTDOORS) {
      add_string_to_buf("  Can only cast outdoors. ");
      return;
    }
    if (party.in_boat >= 0) {
      add_string_to_buf("  Not while in boat. ");
      return;
    }
    if (party.in_horse >= 0) { ////
      add_string_to_buf("  Not while on horseback. ");
      return;
    }
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    add_string_to_buf("  You are moved... ");
    force_town_enter(scenario.which_town_start, scenario.where_start);
    start_town_mode(scenario.which_town_start, 9);
    position_party(scenario.out_sec_start.x, scenario.out_sec_start.y,
                   scenario.out_start.x, scenario.out_start.y);
    center = c_town.p_loc = scenario.where_start;
    //			overall_mode = MODE_OUTDOORS;
    //			center = party.p_loc;
    //			update_explored(party.p_loc);
    redraw_screen(0);
    break;

  case SPELL_PRIEST_MINOR_HEAL:
  case SPELL_PRIEST_HEAL:
  case SPELL_PRIEST_MAJOR_HEAL:
  case SPELL_PRIEST_WEAKEN_POISON:
  case SPELL_PRIEST_CURE_POISON:
  case SPELL_PRIEST_CURE_DISEASE:
  case SPELL_PRIEST_RESTORE_MIND:
  case SPELL_PRIEST_CLEANSE:
  case SPELL_PRIEST_AWAKEN:
  case SPELL_PRIEST_CURE_PARALYSIS:
    //			target = select_pc(11,0);
    target = store_spell_target;
    if (target < 6) {
      adven[pc_num].cur_sp -= spell_cost[1][spell_num];
      switch (spell_num) {
      case SPELL_PRIEST_MINOR_HEAL:
      case SPELL_PRIEST_HEAL:
      case SPELL_PRIEST_MAJOR_HEAL:
        r1 = get_ran(2 + 2 * (spell_num / 6), 1, 4);
        sprintf(c_line, "  %s healed %d.   ", adven[target].name, r1);
        adven[target].heal(r1);
        one_sound(52);
        break;

      case SPELL_PRIEST_WEAKEN_POISON:
      case SPELL_PRIEST_CURE_POISON:
        sprintf(c_line, "  %s cured.    ", adven[target].name);
        r1 = ((spell_num == SPELL_PRIEST_WEAKEN_POISON) ? 1 : 3) +
             get_ran(1, 0, 2) + adven[pc_num].statAdj(SKILL_INTELLIGENCE) / 2;
        adven[target].cure(r1);
        break;

      case SPELL_PRIEST_AWAKEN: // awaken
        if (adven[target].status[STATUS_ASLEEP] <= 0) {
          sprintf(c_line, "  %s is already awake!    ", adven[target].name);
          break;
        }
        sprintf(c_line, "  %s wakes up.    ", adven[target].name);
        adven[target].status[STATUS_ASLEEP] = 0;
        break;
      case SPELL_PRIEST_CURE_PARALYSIS: // cure paralysis
        if (adven[target].status[STATUS_PARALYZED] <= 0) {
          sprintf(c_line, "  %s isn't paralyzed!    ", adven[target].name);
          break;
        }
        sprintf(c_line, "  %s can move now.    ", adven[target].name);
        adven[target].status[STATUS_PARALYZED] = 0;
        break;

      case SPELL_PRIEST_CURE_DISEASE:
        sprintf(c_line, "  %s recovers.      ", adven[target].name);
        r1 = 2 + get_ran(1, 0, 2) +
             adven[pc_num].statAdj(SKILL_INTELLIGENCE) / 2;
        adven[target].status[STATUS_DISEASE] =
            max(0, adven[target].status[STATUS_DISEASE] - r1);
        break;

      case SPELL_PRIEST_RESTORE_MIND:
        sprintf(c_line, "  %s restored.      ", adven[target].name);
        r1 = 1 + get_ran(1, 0, 2) +
             adven[pc_num].statAdj(SKILL_INTELLIGENCE) / 2;
        adven[target].status[STATUS_DUMB] =
            max(0, adven[target].status[STATUS_DUMB] - r1);
        break;

      case SPELL_PRIEST_CLEANSE:
        sprintf(c_line, "  %s cleansed.      ", adven[target].name);
        adven[target].status[STATUS_DISEASE] = 0;
        adven[target].status[STATUS_WEBS] = 0;
        break;
      }
    }
    add_string_to_buf((char *)c_line);
    put_pc_screen();
    break;

  case SPELL_PRIEST_REVIVE:
  case SPELL_PRIEST_DESTONE:
  case SPELL_PRIEST_RAISE_DEAD:
  case SPELL_PRIEST_RESURRECT:
  case SPELL_PRIEST_REMOVE_CURSE:
  case SPELL_PRIEST_SANCTUARY:
  case SPELL_PRIEST_SYMBIOSIS:
  case SPELL_PRIEST_MARTYRS_SHIELD:
    target = store_spell_target;

    if (target < 6) {
      if ((spell_num == 6) && (target == pc_num)) { // check symbiosis
        add_string_to_buf("  Can't cast on self.");
        return;
      }

      adven[pc_num].cur_sp -= spell_cost[1][spell_num];
      if (spell_num == SPELL_PRIEST_MARTYRS_SHIELD) { // martyr's shield
        sprintf(c_line, "  %s shielded.         ", adven[target].name);
        r1 = max(1, get_ran((adven[pc_num].level + 5) / 5, 1, 3) + adj);
        adven[target].status[STATUS_MARTYRS_SHIELD] += r1;
      }
      if (spell_num == SPELL_PRIEST_SANCTUARY) { // sanctuary
        sprintf(c_line, "  %s hidden.         ", adven[target].name);
        r1 = max(0, get_ran(0, 1, 3) + adven[pc_num].level / 4 + adj);
        adven[target].status[STATUS_INVISIBLE] += r1;
      }
      if (spell_num == SPELL_PRIEST_SYMBIOSIS) { // symbiosis
        store_victim_health = adven[target].cur_health;
        store_caster_health = adven[pc_num].cur_health;
        targ_damaged = adven[target].max_health - adven[target].cur_health;
        while ((targ_damaged > 0) && (adven[pc_num].cur_health > 0)) {
          adven[target].cur_health++;
          r1 = get_ran(1, 0, 100) + adven[pc_num].level / 2 + 3 * adj;
          if (r1 < 100)
            adven[pc_num].cur_health--;
          if (r1 < 50)
            move_to_zero(adven[pc_num].cur_health);
          targ_damaged = adven[target].max_health - adven[target].cur_health;
        }
        add_string_to_buf("  You absorb damage.");
        sprintf(c_line, "  %s healed %d.         ", adven[target].name,
                adven[target].cur_health - store_victim_health);
        add_string_to_buf(c_line);
        sprintf(c_line, "  %s takes %d.         ", adven[pc_num].name,
                store_caster_health - adven[pc_num].cur_health);
      }
      if (spell_num == SPELL_PRIEST_REVIVE) {
        sprintf(c_line, "  %s healed.         ", adven[target].name);
        adven[target].heal(250);
        adven[target].status[STATUS_POISON] = 0;
        one_sound(-53);
        one_sound(52);
      }
      if (spell_num == SPELL_PRIEST_DESTONE) {
        if (adven[target].main_status == MAIN_STATUS_STONE) {
          adven[target].main_status = MAIN_STATUS_ALIVE;
          sprintf(c_line, "  %s destoned.                                  ",
                  adven[target].name);
          play_sound(53);
        } else
          sprintf(c_line, "  Wasn't stoned.              ");
      }
      if (spell_num == SPELL_PRIEST_REMOVE_CURSE) {
        for (i = 0; i < 24; i++)
          if (adven[target].items[i].isCursed()) {
            r1 = get_ran(1, 0, 200) -
                 10 * adven[pc_num].statAdj(SKILL_INTELLIGENCE);
            if (r1 < 60) {
              adven[target].items[i].item_properties =
                  adven[target].items[i].item_properties & 239;
            }
          }
        play_sound(52);
        sprintf(c_line, "  Your items glow.     ");
      }
      if (PSD[SDF_LEGACY_SCENARIO] == 0 &&
          ((spell_num == SPELL_PRIEST_RAISE_DEAD) ||
           (spell_num == SPELL_PRIEST_RESURRECT))) {
        if ((item = adven[pc_num].hasAbil(ITEM_RESSURECTION_BALM)) == 24) {
          sprintf(c_line, "  Need resurrection balm.        ");
          spell_num = 500;
        } else
          adven[pc_num].takeItem(item);
      }
      if (spell_num == SPELL_PRIEST_RAISE_DEAD) {
        if (adven[target].main_status == MAIN_STATUS_DEAD)
          if (get_ran(1, 1, adven[pc_num].level / 2) == 1) {
            sprintf(c_line, "  %s now dust.                          ",
                    adven[target].name);
            play_sound(5);
            adven[target].main_status = MAIN_STATUS_DUST;
          } else {
            adven[target].main_status = MAIN_STATUS_ALIVE;
            for (i = 0; i < 3; i++)
              if (get_ran(1, 0, 2) < 2)
                adven[target].skills[i] -=
                    (adven[target].skills[i] > 1) ? 1 : 0;
            adven[target].cur_health = 1;
            sprintf(c_line, "  %s raised.                          ",
                    adven[target].name);
            play_sound(52);
          }
        else
          sprintf(c_line, "  Didn't work.              ");
      }
      if (spell_num == SPELL_PRIEST_RESURRECT) {
        if (adven[target].isAlive() == false) {
          adven[target].main_status = MAIN_STATUS_ALIVE;
          for (i = 0; i < 3; i++)
            if (get_ran(1, 0, 2) < 1)
              adven[target].skills[i] -= (adven[target].skills[i] > 1) ? 1 : 0;
          adven[target].cur_health = 1;
          sprintf(c_line, "  %s raised.", adven[target].name);
          play_sound(52);
        } else
          sprintf(c_line, "  Was OK.              ");
      }
      add_string_to_buf(c_line);
      put_pc_screen();
    }
    break;

  case SPELL_PRIEST_LIGHT_HEAL_ALL:
  case SPELL_PRIEST_HEAL_ALL:
  case SPELL_PRIEST_REVIVE_ALL:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    r1 = get_ran(spell_num / 7 + adj, 1, 4);
    if (spell_num < SPELL_PRIEST_REVIVE_ALL) {
      sprintf(c_line, "  Party healed %d.       ", r1);
      add_string_to_buf(c_line);
      adven.heal(r1);
      play_sound(52);
    } else if (spell_num == SPELL_PRIEST_REVIVE_ALL) {
      sprintf(c_line, "  Party revived.     ");
      add_string_to_buf(c_line);
      r1 = r1 * 2;
      adven.heal(r1);
      play_sound(-53);
      play_sound(-52);
      adven.cure(3 + adj);
    }
    break;

  case SPELL_PRIEST_CURE_PARTY:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    sprintf(c_line, "  Party cured.  ");
    add_string_to_buf(c_line);
    adven.cure(3 + adven[pc_num].statAdj(SKILL_INTELLIGENCE));
    break;

  case SPELL_PRIEST_MASS_SANCTUARY:
  case SPELL_PRIEST_MAJOR_CLEANSING:
  case SPELL_PRIEST_HYPERACTIVITY:
    adven[pc_num].cur_sp -= spell_cost[1][spell_num];
    switch (spell_num) {
    case SPELL_PRIEST_MASS_SANCTUARY:
      add_string_to_buf("  Party hidden.");
      break;
    case SPELL_PRIEST_MAJOR_CLEANSING:
      add_string_to_buf("  Party cleansed.");
      break;
    case SPELL_PRIEST_HYPERACTIVITY:
      add_string_to_buf("  Party is now really, REALLY awake.");
      break;
    }

    for (i = 0; i < 6; i++)
      if (adven[i].isAlive()) {
        if (spell_num == SPELL_PRIEST_MASS_SANCTUARY) {
          store = get_ran(0, 1, 3) + adven[pc_num].level / 6 +
                  adven[pc_num].statAdj(SKILL_INTELLIGENCE);
          r1 = max(0, store);
          adven[i].status[STATUS_INVISIBLE] += r1;
        }
        if (spell_num == SPELL_PRIEST_MAJOR_CLEANSING) {
          adven[i].status[STATUS_WEBS] = 0;
          adven[i].status[STATUS_DISEASE] = 0;
        }
        if (spell_num == SPELL_PRIEST_HYPERACTIVITY) { // Hyperactivity
          adven[i].status[STATUS_ASLEEP] -=
              6 + 2 * adven[pc_num].statAdj(SKILL_INTELLIGENCE);
          adven[i].status[STATUS_HASTE_SLOW] =
              max(0, adven[i].status[STATUS_HASTE_SLOW]);
        }
      }
    break;
  }
}

// priest spells : 100 + spell number
void cast_town_spell(location where) {
  short adjust, r1, targ;
  location loc;
  unsigned char ter;

  if ((where.x <= c_town.town.in_town_rect.left) ||
      (where.x >= c_town.town.in_town_rect.right) ||
      (where.y <= c_town.town.in_town_rect.top) ||
      (where.y >= c_town.town.in_town_rect.bottom)) {
    add_string_to_buf("  Can't target outside town.");
    return;
  }

  adjust = can_see(c_town.p_loc, where, 0);
  if (town_spell < 1000)
    adven[who_cast].cur_sp -= spell_cost[town_spell / 100][town_spell % 100];
  else
    town_spell -= 1000;
  ter = t_d.terrain[where.x][where.y];

  if (adjust > 4)
    add_string_to_buf("  Can't see target.       ");
  else
    switch (town_spell) {
    case SPELL_MAGE_SCRY_MONSTER:
    case SPELL_MAGE_CAPTURE_SOUL: // Scry, Capture Soul
      targ = monst_there(where);
      if (targ < T_M) {
        if (town_spell == SPELL_MAGE_SCRY_MONSTER) {
          party.m_seen[c_town.monst.dudes[targ].number] = true;
          adjust_monst_menu();
          display_monst(0, &c_town.monst.dudes[targ], 0);
        } else
          c_town.monst.dudes[targ].record();
      } else
        add_string_to_buf("  No monster there.");
      break;

    case 100 + SPELL_PRIEST_DISPEL_FIELDS:
    case SPELL_MAGE_DISPEL_FIELDS: // case 119: old (from previous Exile)
                                   // leftover case
      add_string_to_buf("  You attempt to dispel.              ");
      place_spell_pattern(current_pat, where, 11, false, 7);
      break;
    case 100 + SPELL_PRIEST_MOVE_MOUNTAINS: // Move M.
      add_string_to_buf("  You blast the area.              ");
      where.crumbleWall();
      update_explored(c_town.p_loc);
      break;
    case SPELL_MAGE_FIRE_BARRIER:
      if ((get_obscurity(where.x, where.y) == 5) || (monst_there(where) < 90)) {
        add_string_to_buf("  Target space obstructed.");
        break;
      }
      make_fire_barrier(where.x, where.y);
      if (is_fire_barrier(where.x, where.y))
        add_string_to_buf("  You create the barrier.              ");
      else
        add_string_to_buf("  Failed.");
      break;
    case SPELL_MAGE_FORCE_BARRIER:
      if ((get_obscurity(where.x, where.y) == 5) || (monst_there(where) < 90)) {
        add_string_to_buf("  Target space obstructed.");
        break;
      }
      make_force_barrier(where.x, where.y);
      if (is_force_barrier(where.x, where.y))
        add_string_to_buf("  You create the barrier.              ");
      else
        add_string_to_buf("  Failed.");
      break;
    case SPELL_MAGE_QUICKFIRE:
      make_quickfire(where.x, where.y);
      if (is_quickfire(where.x, where.y))
        add_string_to_buf("  You create quickfire.              ");
      else
        add_string_to_buf("  Failed.");
      break;

    case SPELL_MAGE_ANTIMAGIC_CLOUD: // Antimagic Cloud
      add_string_to_buf("  You create an antimagic cloud.              ");
      for (loc.x = 0; loc.x < town_size[town_type]; loc.x++)
        for (loc.y = 0; loc.y < town_size[town_type]; loc.y++)
          if ((dist(where, loc) <= 2) && (can_see(where, loc, 2) < 5) &&
              ((ex_abs(loc.x - where.x) < 2) || (ex_abs(loc.y - where.y) < 2)))
            make_antimagic(loc.x, loc.y);
      break;

    case 100 + SPELL_PRIEST_RITUAL_SANCTIFY: // Ritual - Sanctify
      check_spell_on_space(where, 108);
      break;

    case SPELL_MAGE_UNLOCK:
      switch (scenario.ter_types[ter].special) { ////
      case TER_SPEC_UNLOCKABLE_TERRAIN:
      case TER_SPEC_UNLOCKABLE_BASHABLE:
        r1 = get_ran(1, 0, 100) -
             5 * adven[who_cast].statAdj(SKILL_INTELLIGENCE) +
             5 * c_town.difficulty;
        r1 += scenario.ter_types[ter].flag2 *
              7; // unlock_adjust (door resistance)
        if (scenario.ter_types[ter].flag2 == 10)
          r1 = 10000;
        if (r1 < (135 - combat_percent[min(19, adven[who_cast].level)])) {
          add_string_to_buf("  Door unlocked.                 ");
          play_sound(9);
          t_d.terrain[where.x][where.y] = scenario.ter_types[ter].flag1;
          if (scenario.ter_types[scenario.ter_types[ter].flag1].special >=
                  TER_SPEC_CONVEYOR_NORTH &&
              scenario.ter_types[scenario.ter_types[ter].flag1].special <=
                  TER_SPEC_CONVEYOR_WEST)
            belt_present = true;
        } else {
          play_sound(41);
          add_string_to_buf("  Didn't work.                  ");
        }
        break;

      default:
        add_string_to_buf("  Wrong terrain type.               ");
        break;
      }
      break;

    case SPELL_MAGE_DISPEL_BARRIER:
      if ((is_fire_barrier(where.x, where.y)) ||
          (is_force_barrier(where.x, where.y))) {
        r1 = get_ran(1, 0, 100) -
             5 * adven[who_cast].statAdj(SKILL_INTELLIGENCE) +
             5 * (c_town.difficulty / 10);
        if (is_fire_barrier(where.x, where.y))
          r1 -= 8;
        if (r1 < (120 - combat_percent[min(19, adven[who_cast].level)])) {
          add_string_to_buf("  Barrier broken.                 ");
          take_fire_barrier(where.x, where.y);
          take_force_barrier(where.x, where.y);

          // Now, show party new things
          update_explored(c_town.p_loc);
        } else {
          play_sound(41);
          add_string_to_buf("  Didn't work.                  ");
        }
      }

      else
        add_string_to_buf("  No barrier there.");

      break;
    }
}

void check_spell_on_space(
    location where,
    unsigned char
        spell) // code is basically done for a "Check spell" node, remains to do
               // a check when targeting a space in all mode ...
{
  short i, s1, s2, s3;

  if (PSD[SDF_LEGACY_SCENARIO] == 1) {
    if (spell == 108) { // legacy behavior, only checks Ritual of Sanctify
      for (i = 0; i < 50; i++)
        if (same_point(where, c_town.town.special_locs[i]) == true) {
          if (c_town.town.specials[c_town.town.spec_id[i]].type ==
              SPEC_SANCTIFY)
            run_special(SPEC_TARGET, 2, c_town.town.spec_id[i], where, &s1, &s2,
                        &s3);
          return;
        }
      add_string_to_buf("  Nothing happens.");
    }
  } else {
    for (i = 0; i < 50; i++)
      if (same_point(where, c_town.town.special_locs[i]) == true) {
        if ((c_town.town.specials[c_town.town.spec_id[i]].type ==
             SPEC_SANCTIFY) &&
            (c_town.town.specials[c_town.town.spec_id[i]].ex1a == spell))
          run_special(SPEC_TARGET, 2, c_town.town.spec_id[i], where, &s1, &s2,
                      &s3);
        return;
      }
    if (spell == 108)
      add_string_to_buf("  Nothing happens.");
  }
}

void do_mindduel(short pc_num, creature_data_type *monst) {
  short i = 0, adjust, r1, r2, balance = 0;

  adjust =
      (adven[pc_num].level + adven[pc_num].skills[SKILL_INTELLIGENCE]) / 2 -
      monst->m_d.level * 2;

  if (adven[pc_num].hasAbilEquip(ITEM_WILL) < 24)
    adjust += 20;
  if (monst->attitude % 2 != 1)
    set_town_status(0);
  monst->attitude = 1;

  add_string_to_buf("Mindduel!");
  while ((adven[pc_num].isAlive()) && (monst->active > 0) && (i < 10)) {
    play_sound(1);
    r1 = get_ran(1, 0, 100) + adjust;
    r1 += 5 *
          (monst->m_d.status[STATUS_DUMB] - adven[pc_num].status[STATUS_DUMB]);
    r1 += 5 * balance;
    r2 = get_ran(1, 1, 6);
    if (r1 < 30) {
      sprintf(c_line, "  %s is drained %d.", adven[pc_num].name, r2);
      add_string_to_buf(c_line);
      monst->m_d.mp += r2;
      balance++;
      if (adven[pc_num].cur_sp == 0) {
        adven[pc_num].status[STATUS_DUMB] += 2;
        sprintf(c_line, "  %s is dumbfounded.", adven[pc_num].name);
        add_string_to_buf(c_line);
        if (adven[pc_num].status[STATUS_DUMB] > 7) {
          sprintf(c_line, "  %s is killed!", adven[pc_num].name);
          add_string_to_buf(c_line);
          adven[pc_num].kill(2);
        }
      } else
        adven[pc_num].cur_sp = max(0, adven[pc_num].cur_sp - r2);
    }
    if (r1 > 70) {
      sprintf(c_line, "  %s drains %d.", adven[pc_num].name, r2);
      add_string_to_buf(c_line);
      adven[pc_num].cur_sp += r2;
      balance--;
      if (monst->m_d.mp == 0) {
        monst->m_d.status[STATUS_DUMB] += 2;
        monst_spell_note(monst->number, 22);
        if (monst->m_d.status[STATUS_DUMB] > 7)
          kill_monst(monst, pc_num);
      } else
        monst->m_d.mp = max(0, monst->m_d.mp - r2);
    }
    print_buf();
    i++;
  }
}

// mode 0 - dispel spell, 1 - always take  2 - always take and take fire and
// force too
void dispel_fields(short i, short j, short mode) {
  short r1;

  if (mode == 2) {
    take_fire_barrier(i, j);
    take_force_barrier(i, j);
    take_barrel(i, j);
    take_crate(i, j);
    take_web(i, j);
  }
  if (mode >= 1)
    mode = -10;
  take_fire_wall(i, j);
  take_force_wall(i, j);
  take_scloud(i, j);
  r1 = get_ran(1, 1, 6) + mode;
  if (r1 <= 1)
    take_web(i, j);
  r1 = get_ran(1, 1, 6) + mode;
  if (r1 < 6)
    take_ice_wall(i, j);
  r1 = get_ran(1, 1, 6) + mode;
  if (r1 < 5)
    take_sleep_cloud(i, j);
  r1 = get_ran(1, 1, 8) + mode;
  if (r1 <= 1)
    take_quickfire(i, j);
  r1 = get_ran(1, 1, 7) + mode;
  if (r1 < 5)
    take_blade_wall(i, j);
}
Boolean pc_can_cast_spell(short pc_num, short type, short spell_num)
// short type;  // 0 - mage  1 - priest
{
  short level, store_w_cast;

  level = spell_level[spell_num];

  if (overall_mode >= MODE_TALKING)
    return false;
  if ((spell_num < 0) || (spell_num > 61))
    return false;
  if (adven[pc_num].skills[SKILL_MAGE_SPELLS + type] < level)
    return false;
  if (adven[pc_num].isAlive() == false)
    return false;
  if (adven[pc_num].cur_sp < spell_cost[type][spell_num])
    return false;
  if ((type == 0) && (adven[pc_num].mage_spells[spell_num] == false))
    return false;
  if ((type == 1) && (adven[pc_num].priest_spells[spell_num] == false))
    return false;
  if (adven[pc_num].status[STATUS_DUMB] >= 8 - level)
    return false;
  if (adven[pc_num].status[STATUS_PARALYZED] != 0)
    return false;
  if (adven[pc_num].status[STATUS_ASLEEP] > 0)
    return false;

  // 0 - everywhere 1 - combat only 2 - town only 3 - town & outdoor only 4 -
  // town & combat only  5 - outdoor only
  store_w_cast = spell_w_cast[type][spell_num];

  if (is_out())
    if ((store_w_cast == 1) || (store_w_cast == 2) || (store_w_cast == 4))
      return false;

  if (is_town())
    if ((store_w_cast == 1) || (store_w_cast == 5))
      return false;

  if (is_combat())
    if ((store_w_cast == 2) || (store_w_cast == 3) || (store_w_cast == 5))
      return false;

  return true;
}

void draw_caster_buttons() {
  short i;

  if (can_choose_caster == false) {
    for (i = 0; i < 6; i++) {
      if (i == pc_casting)
        cd_activate_item(1098, 4 + i, 1);
      else
        cd_activate_item(1098, 4 + i, 0);
    }
  } else {
    for (i = 0; i < 6; i++) {
      if (pc_can_cast_spell(i, store_situation, store_situation) == true)
        cd_activate_item(1098, 4 + i, 1);
      else
        cd_activate_item(1098, 4 + i, 0);
    }
  }
}

void draw_spell_info() {
  if (((store_situation == 0) && (store_mage == 70)) ||
      ((store_situation == 1) && (store_priest == 70))) { // No spell selected
    for (i = 0; i < 6; i++)
      cd_activate_item(1098, 10 + i, 0);
  } else { // Spell selected
    for (i = 0; i < 6; i++) {
      switch (((store_situation == 0) ? mage_need_select[store_mage]
                                      : priest_need_select[store_priest])) {
      case 0:
        cd_activate_item(1098, 10 + i, 0);
        break;
      case 1:
        if (adven[i].isAlive() == false)
          cd_activate_item(1098, 10 + i, 0);
        else
          cd_activate_item(1098, 10 + i, 1);
        break;
      case 2:
        if (adven[i].main_status > MAIN_STATUS_ABSENT)
          cd_activate_item(1098, 10 + i, 1);
        else
          cd_activate_item(1098, 10 + i, 0);
        break;
      }
    }
  }
}

void draw_spell_pc_info() {
  for (int i = 0; i < 6; i++) {
    if (adven[i].main_status != MAIN_STATUS_ABSENT) {
      cd_set_item_text(1098, 18 + i, adven[i].name);

      if (adven[i].isAlive()) {
        cd_set_item_num(1098, 24 + i, adven[i].cur_health);
        cd_set_item_num(1098, 30 + i, adven[i].cur_sp);
      }
    }
  }
}

void put_pc_caster_buttons() {
  for (int i = 0; i < 6; i++)
    if (cd_get_active(1098, i + 4) > 0) {
      if (i == pc_casting)
        cd_text_frame(1098, i + 18, 11);
      else
        cd_text_frame(1098, i + 18, 1);
    }
}
void put_pc_target_buttons() {
  if (store_spell_target < 6) {
    cd_text_frame(1098, 24 + store_spell_target, 11);
    cd_text_frame(1098, 30 + store_spell_target, 11);
  }
  if ((store_last_target_darkened < 6) &&
      (store_last_target_darkened != store_spell_target)) {
    cd_text_frame(1098, 24 + store_last_target_darkened, 1);
    cd_text_frame(1098, 30 + store_last_target_darkened, 1);
  }
  store_last_target_darkened = store_spell_target;
}

void put_spell_led_buttons() {
  short i, spell_for_this_button;

  for (i = 0; i < 38; i++) {
    spell_for_this_button = (on_which_spell_page == 0) ? i : spell_index[i];

    if (spell_for_this_button < 90) {
      if (((store_situation == 0) && (store_mage == spell_for_this_button)) ||
          ((store_situation == 1) && (store_priest == spell_for_this_button))) {
        cd_set_led(1098, i + 37, 2);
      } else if (pc_can_cast_spell(pc_casting, store_situation,
                                   spell_for_this_button) == true)
        cd_set_led(1098, i + 37, 1);
      else
        cd_set_led(1098, i + 37, 0);
    }
  }
}

void put_spell_list() {
  short i;
  char add_text[256];

  if (on_which_spell_page == 0) {
    csit(1098, 80, "Level 1:");
    csit(1098, 81, "Level 2:");
    csit(1098, 82, "Level 3:");
    csit(1098, 83, "Level 4:");
    for (i = 0; i < 38; i++) {
      if (store_situation == 0) {
        if (i == 35)
          sprintf(add_text, "%s %c ?", mage_s_name[i],
                  (char)((97 + i > 122) ? 65 + (i - 26) : 97 + i));
        else
          sprintf(add_text, "%s %c %d", mage_s_name[i],
                  (char)((97 + i > 122) ? 65 + (i - 26) : 97 + i),
                  spell_cost[0][i]);
      } else
        sprintf(add_text, "%s %c %d", priest_s_name[i],
                (char)((97 + i > 122) ? 65 + (i - 26) : 97 + i),
                spell_cost[1][i]);
      // for (j = 0; j < 30; i++)
      //	if (add_text[j] == '&')
      //		add_text[j] = (char) ((97 + i > 122) ? 65 + (i - 26) :
      //97 + i);
      cd_add_label(1098, 37 + i, add_text, 53);
      if (spell_index[i] == 90)
        cd_activate_item(1098, 37 + i, 1);
    }
  } else {
    csit(1098, 80, "Level 5:");
    csit(1098, 81, "Level 6:");
    csit(1098, 82, "Level 7:");
    csit(1098, 83, "");
    for (i = 0; i < 38; i++)
      if (spell_index[i] < 90) {
        if (store_situation == 0)
          sprintf(add_text, "%s %c %d", mage_s_name[spell_index[i]],
                  (char)((97 + i > 122) ? 65 + (i - 26) : 97 + i),
                  spell_cost[0][spell_index[i]]);
        else
          sprintf(add_text, "%s %c %d", priest_s_name[spell_index[i]],
                  (char)((97 + i > 122) ? 65 + (i - 26) : 97 + i),
                  spell_cost[1][spell_index[i]]);
        cd_add_label(1098, 37 + i, add_text, 53);
      } else
        cd_activate_item(1098, 37 + i, 0);
  }
}

void pick_spell_event_filter(short item_hit) {
  char const *choose_target = " Now pick a target.";
  char const *no_target = " No target needed.";
  char const *bad_target = " Can't cast on him/her.";
  char const *got_target = " Target selected.";
  char const *bad_spell = " Spell not available.";
  Boolean spell_toast = false, dialog_done = false;

  switch (item_hit) {
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9: // pick caster
    if (cd_get_active(1098, item_hit) == 1) {
      pc_casting = item_hit - 4;
      if (pc_can_cast_spell(
              pc_casting, store_situation,
              ((store_situation == 0) ? store_mage : store_priest)) == false) {
        if (store_situation == 0)
          store_mage = 70;
        else
          store_priest = 70;
        store_spell_target = 6;
      }
      draw_spell_info();
      draw_spell_pc_info();
      put_spell_led_buttons();
      put_pc_caster_buttons();
      put_pc_target_buttons();
    }
    break;
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15: // pick target
    if (cd_get_active(1098, 10 + pc_casting) == false) {
      cd_set_item_text(1098, 36, no_target);
    } else if (cd_get_active(1098, item_hit) == false) {
      cd_set_item_text(1098, 36, bad_target);
    } else {

      cd_set_item_text(1098, 36, got_target);
      store_spell_target = item_hit - 10;
      draw_spell_info();
      put_pc_target_buttons();
    }
    break;

  case 16: // cancel
    spell_toast = true;
    dialog_done = true;
    break;
  case 1:
  case 17: // cast!
    dialog_done = true;
    break;

  case 75: // other spells
    on_which_spell_page = 1 - on_which_spell_page;
    put_spell_list();
    put_spell_led_buttons();
    break;

  case 79: // help
    party.help_received[7] = 0;
    give_help(207, 8, 1098);
    break;

  case 100:
    break;

  default:
    if (item_hit >= 100) {
      item_hit -= 100;
      i = (on_which_spell_page == 0) ? item_hit - 37
                                     : spell_index[item_hit - 37];
      display_spells(store_situation, i, 1098);
    } else if (cd_get_led(1098, item_hit) == 0) {
      cd_set_item_text(1098, 36, bad_spell);
    } else {

      if (store_situation == 0)
        store_mage = (on_which_spell_page == 0) ? item_hit - 37
                                                : spell_index[item_hit - 37];
      else
        store_priest = (on_which_spell_page == 0) ? item_hit - 37
                                                  : spell_index[item_hit - 37];
      draw_spell_info();
      put_spell_led_buttons();

      if (store_spell_target < 6) {
        if (cd_get_active(1098, 10 + store_spell_target) == false) {
          store_spell_target = 6;
          draw_spell_info();
          put_pc_target_buttons();
        }
      }
      // Cute trick now... if a target is needed, caster can always be picked
      if ((store_spell_target == 6) &&
          (cd_get_active(1098, 10 + pc_casting) == 1)) {
        cd_set_item_text(1098, 36, choose_target);
        draw_spell_info();
        //							force_play_sound(45);
        play_sound(45);
      } else if (cd_get_active(1098, 10 + pc_casting) == 0) {
        store_spell_target = 6;
        put_pc_target_buttons();
      }
    }
    break;
  }
  if (dialog_done == true) {
    if (spell_toast == true) {
      store_mage = store_mage_store;
      store_priest = store_priest_store;
      store_spell_target = store_store_target;
      if (store_situation == 0)
        store_last_cast_mage = pc_casting;
      else
        store_last_cast_priest = pc_casting;
      dialog_not_toast = false;
      dialog_answer = 70;
      return;
    }

    if (((store_situation == 0) && (store_mage == 70)) ||
        ((store_situation == 1) && (store_priest == 70))) {
      add_string_to_buf("Cast: No spell selected.");
      store_mage = store_mage_store;
      store_priest = store_priest_store;
      store_spell_target = store_store_target;
      dialog_not_toast = false;
      dialog_answer = 70;
      return;
    }
    if ((store_situation == 0) && (mage_need_select[store_mage] == 0)) {
      store_last_cast_mage = pc_casting;
      pc_last_cast[store_situation][pc_casting] = store_mage;
      dialog_not_toast = false;
      dialog_answer = store_mage;
      return;
    }
    if ((store_situation == 1) && (priest_need_select[store_priest] == 0)) {
      store_last_cast_priest = pc_casting;
      pc_last_cast[store_situation][pc_casting] = store_priest;
      dialog_not_toast = false;
      dialog_answer = store_priest;
      return;
    }
    if (store_spell_target == 6) {
      add_string_to_buf("Cast: Need to select target.");
      store_mage = store_mage_store;
      store_priest = store_priest_store;
      store_spell_target = store_store_target;
      dialog_not_toast = false;
      give_help(39, 0, 1098);
      dialog_answer = 70;
      return;
    }
    item_hit = ((store_situation == 0) ? store_mage : store_priest);
    if (store_situation == 0)
      store_last_cast_mage = pc_casting;
    else
      store_last_cast_priest = pc_casting;
    pc_last_cast[store_situation][pc_casting] =
        ((store_situation == 0) ? store_mage : store_priest);
    dialog_not_toast = false;
    dialog_answer = item_hit;
  }
}

short pick_spell(short pc_num, short type, short) // 70 - no spell OW spell num
// short pc_num; // if 6, anyone
// short type; // 0 - mage   1 - priest
// short situation; // 0 - out  1 - town  2 - combat
{
  store_mage_store = store_mage;
  store_priest_store = store_priest;
  store_store_target = store_spell_target;
  store_situation = type;
  store_last_target_darkened = 6;
  can_choose_caster = (pc_num < 6) ? false : true;

  pc_casting = (type == 0) ? store_last_cast_mage : store_last_cast_priest;
  if (pc_casting == 6)
    pc_casting = current_pc;

  if (pc_num == 6) { // See if can keep same caster
    can_choose_caster = true;
    if (pc_can_cast_spell(pc_casting, type, type) == false) {
      for (i = 0; i < 6; i++)
        if (pc_can_cast_spell(i, type, type)) {
          pc_casting = i;
          i = 500;
        }
      if (i == 6) {
        add_string_to_buf("Cast: Nobody can.");
        return 70;
      }
    }
  } else {
    can_choose_caster = false;
    pc_casting = pc_num;
  }

  if (can_choose_caster == false) {
    if ((type == 0) && (adven[pc_num].skills[SKILL_MAGE_SPELLS] == 0)) {
      add_string_to_buf("Cast: No mage skill.");
      return 70;
    }
    if ((type == 1) && (adven[pc_num].skills[SKILL_PRIEST_SPELLS] == 0)) {
      add_string_to_buf("Cast: No priest skill.");
      return 70;
    }
    if (adven[pc_casting].cur_sp == 0) {
      add_string_to_buf("Cast: No spell points.");
      return 70;
    }
  }

  // If in combat, make the spell being cast this PCs most recent spell
  if (is_combat()) {
    if (type == 0)
      store_mage = pc_last_cast[0][pc_casting];
    else
      store_priest = pc_last_cast[1][pc_casting];
  }

  // Keep the stored spell, if it's still castable
  if (pc_can_cast_spell(pc_casting, type,
                        ((type == 0) ? store_mage : store_priest)) == false) {
    if (type == 0) {
      store_mage = 0;
      store_mage_lev = 1;
    } else {
      store_priest = 1;
      store_priest_lev = 1;
    }
  }

  // If a target is needed, keep the same target if that PC still targetable
  if (((type == 0) && (mage_need_select[store_mage] > 0)) ||
      ((type == 1) && (priest_need_select[store_priest] > 0))) {
    if (adven[store_spell_target].isAlive() == false)
      store_spell_target = 6;
  } else
    store_spell_target = 6;

  // Set the spell page, based on starting spell
  if (((type == 0) && (store_mage >= 38)) ||
      ((type == 1) && (store_priest >= 38)))
    on_which_spell_page = 1;
  else
    on_which_spell_page = 0;

  SetCursor(sword_curs);

  cd_create_dialog(1098, mainPtr);
  cd_set_pict(1098, 2, 714 + type);
  for (i = 37; i < 75; i++) {
    cd_add_label(1098, i, "", 55);
    if (i > 62)
      cd_attach_key(1098, i, (char)(65 + i - 63));
    else
      cd_attach_key(1098, i, (char)(97 + i - 37));
    cd_set_led(1098, i,
               (pc_can_cast_spell(pc_casting, type,
                                  (on_which_spell_page == 0)
                                      ? i - 37
                                      : spell_index[i - 37]) == true)
                   ? 1
                   : 0);
  }
  cd_attach_key(1098, 10, '!');
  cd_attach_key(1098, 11, '@');
  cd_attach_key(1098, 12, '#');
  cd_attach_key(1098, 13, '$');
  cd_attach_key(1098, 14, '%');
  cd_attach_key(1098, 15, '^');
  for (i = 0; i < 6; i++)
    cd_key_label(1098, 10 + i, 0);
  for (i = 24; i < 36; i++)
    cd_text_frame(1098, i, 1);

  cd_set_flag(1098, 78, 0);

  put_spell_list();
  draw_spell_info();
  put_pc_caster_buttons();
  draw_spell_pc_info();
  draw_caster_buttons();
  put_spell_led_buttons();

  if (party.help_received[7] == 0) {
    cd_initial_draw(1098);
    give_help(7, 8, 1098);
  }

  while (dialog_not_toast)
    ModalDialog();
  final_process_dialog(1098);

  return dialog_answer;
}

void print_spell_cast(short spell_num, short which)
// short which; // 0 - mage  1 - priest
{
  sprintf(c_line, "Spell: %s                  ",
          (which == 0) ? mage_s_name[spell_num] : priest_s_name[spell_num]);
  add_string_to_buf(c_line);
}

void set_town_spell(short s_num, short who_c) {
  town_spell = s_num;
  who_cast = who_c;
}

void do_alchemy() {
  short abil1_needed[20] = {150, 151, 150, 151, 153, 152, 152, 153, 156, 153,
                            156, 154, 156, 157, 155, 157, 157, 152, 156, 157};
  short abil2_needed[20] = {0,   0, 0,   153, 0, 0,   0,   152, 0,   154,
                            150, 0, 151, 0,   0, 154, 155, 155, 154, 155};
  short difficulty[20] = {1, 1,  1,  3,  3, 4,  5,  5,  7,  9,
                          9, 10, 12, 12, 9, 14, 19, 10, 16, 20};
  short fail_chance[20] = {50, 40, 30, 20, 10, 8, 6, 4, 2, 0,
                           0,  0,  0,  0,  0,  0, 0, 0, 0, 0};
  short which_p, which_item, which_item2, r1;
  short pc_num;
  item_record_type store_i = {7,  0,          0,        0,        0, 1, 0, 0,
                              50, 0,          0,        0,        0, 0, 0, 8,
                              0,  location(), "Potion", "Potion", 0, 5, 0, 0};

  short potion_abils[20] = {ITEM_AFFECT_POISON,
                            ITEM_AFFECT_HEALTH,
                            ITEM_POISON_WEAPON,
                            ITEM_HASTE_SLOW,
                            ITEM_POISON_WEAPON,
                            ITEM_AFFECT_HEALTH,
                            ITEM_AFFECT_POISON,
                            ITEM_HASTE_SLOW,
                            ITEM_AFFECT_DISEASE,
                            ITEM_AFFECT_SPELL_POINTS,
                            ITEM_AFFECT_DUMBFOUND,
                            ITEM_POISON_WEAPON,
                            ITEM_AFFECT_HEALTH,
                            ITEM_POISON_WEAPON,
                            ITEM_RESSURECTION_BALM,
                            ITEM_AFFECT_SPELL_POINTS,
                            ITEM_AFFECT_SKILL_POINTS,
                            ITEM_BLESS_CURSE,
                            ITEM_BLISS,
                            ITEM_AFFECT_SPELL_POINTS};
  short potion_strs[20] = {2, 2, 2, 2, 4, 5, 8, 5, 4, 2,
                           8, 6, 8, 8, 0, 5, 2, 8, 5, 8};
  short potion_val[20] = {40,  60,  15,  50,  50,  180, 200, 100, 150, 100,
                          200, 150, 300, 400, 100, 300, 500, 175, 250, 500};

  pc_num = select_pc(1, 0);
  if (pc_num == INVALID_PC)
    return;

  which_p = alch_choice(pc_num);
  if (which_p < 20) {
    if (adven[pc_num].hasSpace() == 24) {
      add_string_to_buf("Alchemy: Can't carry another item.");
      return;
    }
    if (((which_item = adven[pc_num].hasAbil(abil1_needed[which_p])) == 24) ||
        ((abil2_needed[which_p] > 0) && ((which_item2 = adven[pc_num].hasAbil(
                                              abil2_needed[which_p])) == 24))) {
      add_string_to_buf("Alchemy: Don't have ingredients.");
      return;
    }
    play_sound(8);
    adven[pc_num].removeCharge(which_item);
    if (abil2_needed[which_p] > 0)
      adven[pc_num].removeCharge(which_item2);

    r1 = get_ran(1, 0, 100);
    if (r1 < fail_chance[adven[pc_num].skills[SKILL_ALCHEMY] -
                         difficulty[which_p]]) {
      add_string_to_buf("Alchemy: Failed.               ");
      r1 = get_ran(1, 0, 1);
      play_sound(41);
    } else {
      store_i.value = potion_val[which_p];
      store_i.ability_strength = potion_strs[which_p];
      store_i.ability = potion_abils[which_p];
      if (which_p == 8)
        store_i.magic_use_type = 2;
      strcpy(store_i.full_name, alch_names_short[which_p]);
      if (adven[pc_num].skills[SKILL_ALCHEMY] - difficulty[which_p] >= 5)
        store_i.charges++;
      if (adven[pc_num].skills[SKILL_ALCHEMY] - difficulty[which_p] >= 11)
        store_i.charges++;
      if (store_i.variety == ITEM_TYPE_POTION)
        store_i.graphic_num += get_ran(1, 0, 2);
      if (adven[pc_num].giveToPC(store_i, 0) == false) {
        ASB("No room in inventory.");
        ASB("  Potion placed on floor.");
        place_item(store_i, c_town.p_loc, true);
      } else
        add_string_to_buf("Alchemy: Successful.             ");
    }
    put_item_screen(stat_window, 0);
  }
}

void alch_choice_event_filter(short item_hit) {
  if (item_hit == 49) { // OBoE changed
    party.help_received[20] = 0;
    give_help(220, 21, 1047);
    return;
  }
  if (item_hit == 1)
    dialog_answer = 20;
  else {
    item_hit = (item_hit - 9) / 2;
    dialog_answer = item_hit;
  }
  dialog_not_toast = false;
}

short alch_choice(short pc_num) {
  short difficulty[20] = {1, 1,  1,  3,  3, 4,  5,  5,  7,  9,
                          9, 10, 12, 12, 9, 14, 19, 10, 16, 20};
  char get_text[256];

  SetCursor(sword_curs);

  cd_create_dialog(1047, mainPtr);
  for (i = 0; i < 20; i++) {
    cd_set_item_text(1047, 10 + i * 2, alch_names[i]);
    if ((adven[pc_num].skills[SKILL_ALCHEMY] < difficulty[i]) ||
        (party.alchemy[i] == 0))
      cd_activate_item(1047, 9 + i * 2, 0);
  }
  sprintf(get_text, "%s (skill %d)", adven[pc_num].name,
          adven[pc_num].skills[SKILL_ALCHEMY]);
  cd_set_item_text(1047, 4, get_text);
  if (party.help_received[20] == 0) {
    cd_initial_draw(1047);
    give_help(20, 21, 1047);
  }
  while (dialog_not_toast)
    ModalDialog();

  final_process_dialog(1047);
  return dialog_answer;
}

void pc_graphic_event_filter(short item_hit) {
  switch (item_hit) {
  case 1:
    adven[store_graphic_pc_num].which_graphic = store_pc_graphic;
    if (store_graphic_mode == 0)
      dialog_not_toast = false;
    else {
      dialog_not_toast = false;
      dialog_answer = true;
    }
    break;

  case 4:
    if (store_graphic_mode == 0) {
      if (adven[store_graphic_pc_num].main_status < MAIN_STATUS_ABSENT)
        adven[store_graphic_pc_num].main_status = MAIN_STATUS_ABSENT;
      dialog_not_toast = false;
    } else {
      dialog_not_toast = false;
      dialog_answer = true;
    }
    break;

  default:
    cd_set_led(1050, store_pc_graphic + 5, 0);
    store_pc_graphic = item_hit - 5;
    cd_set_led(1050, item_hit, 1);
    break;
  }

  dialog_answer = false;
}

Boolean pick_pc_graphic(short pc_num, short mode, short parent_num)
// mode ... 0 - create  1 - created
{
  short i;

  store_graphic_pc_num = pc_num;
  store_graphic_mode = mode;
  store_pc_graphic = adven[pc_num].which_graphic;

  SetCursor(sword_curs);

  cd_create_dialog_parent_num(1050, parent_num);

  for (i = 41; i < 77; i++)
    csp(1050, i, 800 + i - 41);
  for (i = 5; i < 41; i++) {
    if (store_pc_graphic + 5 == i)
      cd_set_led(1050, i, 1);
    else
      cd_set_led(1050, i, 0);
  }
  while (dialog_not_toast)
    ModalDialog();
  cd_kill_dialog(1050, 0);

  return dialog_answer;
}

void pc_name_event_filter() {
  char get_text[256];

  cd_get_text_edit_str(1051, (char *)get_text);
  if ((get_text[0] < 33) || (get_text[0] > 126)) {
    csit(1051, 6, "Must begin with a letter.");
  } else {
    sprintf((char *)adven[store_train_pc].name, "%.18s", (char *)get_text);
    dialog_not_toast = false;
  }
}

Boolean pick_pc_name(short pc_num, short parent_num)
// town_num; // Will be 0 - 200 for town, 200 - 290 for outdoors
// short sign_type; // terrain type
{
  store_train_pc = pc_num;

  SetCursor(sword_curs);

  cd_create_dialog_parent_num(1051, parent_num);
  cd_set_edit_focus();

  while (dialog_not_toast)
    ModalDialog();
  cd_kill_dialog(1051, 0);

  return 1;
}

void pick_trapped_monst_event_filter(short item_hit) {
  dialog_not_toast = false;
  dialog_answer = item_hit;
}

unsigned char pick_trapped_monst()
// ignore parent in Mac version
{
  short i;
  char sp[256];
  monster_record_type get_monst;

  SetCursor(sword_curs);

  cd_create_dialog_parent_num(988, 0);

  for (i = 0; i < 4; i++)
    if (party.imprisoned_monst[i] == 0) {
      cd_activate_item(988, 2 + 3 * i, 0);
    } else {
      get_m_name(sp, (unsigned char)(party.imprisoned_monst[i]));
      csit(988, 3 + 3 * i, sp);
      get_monst =
          return_monster_template((unsigned char)(party.imprisoned_monst[i]));
      cdsin(988, 4 + 3 * i, get_monst.level);
    }

  while (dialog_not_toast)
    ModalDialog();
  cd_kill_dialog(988, 0);

  if (dialog_answer == 1)
    return 0;
  else
    return ((unsigned char)(party.imprisoned_monst[(dialog_answer - 2) / 3]));
}

void pc_record_type::poison(short how_much) {
  short tlevel = 0;

  if (isAlive()) {
    if ((tlevel = getProtLevel(ITEM_POISON_PROTECTION)) > 0) ////
      how_much -= tlevel / 2;
    if ((tlevel = getProtLevel(ITEM_FULL_PROTECTION)) > 0) ////
      how_much -= tlevel / 3;
    if ((traits[TRAIT_FRAIL] == true) && (how_much > 1))
      how_much++;
    if ((traits[TRAIT_FRAIL] == true) && (how_much == 1) &&
        (get_ran(1, 0, 1) == 0))
      how_much++;

    if (how_much > 0) {
      status[STATUS_POISON] = min(status[STATUS_POISON] + how_much, 8);
      sprintf(c_line, "  %s poisoned.", name);
      add_string_to_buf(c_line);
      one_sound(17);
      give_help(33, 0, 0);
    }
  }
  put_pc_screen();
}

void pc_array::poison(short how_much) {
  for (int i = 0; i < 6; i++)
    pc[i].poison(how_much);
}

void affect_pc(short which_pc, short type, short how_much) ////
// type; // which status to affect
{

  if (adven[which_pc].isAlive() == false)
    return;
  adven[which_pc].status[type] =
      minmax(-8, 8, adven[which_pc].status[type] + how_much);
  if (((type >= 4) && (type <= 10)) || (type == 12) || (type == 13))
    adven[which_pc].status[type] = max(adven[which_pc].status[type], 0);
  put_pc_screen();
}

void pc_array::affect(short type, short how_much)
// type; // which status to affect
{
  for (int i = 0; i < 6; i++)
    adven[i].status[type] = minmax(-8, 8, adven[i].status[type] + how_much);
  put_pc_screen();
}

void void_sanctuary(short pc_num) {
  if (adven[pc_num].status[STATUS_INVISIBLE] > 0) {
    add_string_to_buf("You become visible!");
    adven[pc_num].status[STATUS_INVISIBLE] = 0;
  }
}

void pc_array::damage(short how_much, short damage_type) {
  for (int i = 0; i < NUM_OF_PCS; i++)
    pc[i].damage(how_much, damage_type, -1);

  put_pc_screen();
}

void pc_array::kill(short mode) {
  short i;

  boom_anim_active = false;
  for (i = 0; i < 6; i++)
    if (pc[i].isAlive())
      pc[i].main_status = mode;

  put_pc_screen();
}

bool pc_record_type::damage(short how_much, short damage_type,
                            short type_of_attacker)
// short damage_type; // 0 - weapon   1 - fire   2 - poison   3 - general magic
// 4 - unblockable
// 5 - cold  6 - undead attack  7 - demon attack
// 10 - marked damage, from during anim mode ... no boom, and totally
// unblockable 30 + *   same as *, but no print 100s digit - sound data
{
  short i, r1, sound_type, tlevel;
  bool do_print = true;

  if (!isAlive())
    return false;

  sound_type = damage_type / 100;
  damage_type = damage_type % 100;

  if (damage_type >= DAMAGE_NO_PRINT) {
    do_print = false;
    damage_type -= DAMAGE_NO_PRINT;
  }

  if (sound_type == 0) {
    if ((damage_type == DAMAGE_FIRE) || (damage_type == DAMAGE_UNBLOCKABLE))
      sound_type = 5;
    if (damage_type == DAMAGE_MAGIC)
      sound_type = 12;
    if (damage_type == DAMAGE_COLD)
      sound_type = 7;
    if (damage_type == DAMAGE_POISON)
      sound_type = 11;
  }

  // armor
  if ((damage_type == DAMAGE_WEAPON) || (damage_type == DAMAGE_UNDEAD) ||
      (damage_type == DAMAGE_DEMON)) {
    how_much -= minmax(-5, 5, (int)status[STATUS_BLESS_CURSE]);
    for (i = 0; i < 24; i++)
      if ((items[i].variety != ITEM_TYPE_NO_ITEM) && (equip[i] == true)) {
        if ((items[i].variety >= ITEM_TYPE_SHIELD) &&
            (items[i].variety <= ITEM_TYPE_BOOTS)) { // armor
          r1 = get_ran(1, 1, items[i].item_level);
          how_much -= r1;

          // bonus for magical items
          if (items[i].bonus > 0) {
            r1 = get_ran(1, 1, items[i].bonus);
            how_much -= r1;
            how_much -= items[i].bonus / 2;
          }

          if (items[i].bonus < 0)
            how_much -= items[i].bonus;

          r1 = get_ran(1, 0, 100);
          if (r1 < hit_chance[skills[SKILL_DEFENSE]] - 20)
            how_much -= 1;
        }
        if (items[i].protection > 0) {
          r1 = get_ran(1, 1, items[i].protection);
          how_much -= r1;
        }
        if (items[i].protection < 0) {
          r1 = get_ran(1, 1, -1 * items[i].protection);
          how_much += r1;
        }
      }
  }

  // parry

  // ugly hack
  short which_pc = getNum();

  // temporarily disabled
  if ((damage_type < DAMAGE_POISON) && (pc_parry[which_pc] < 100))
    how_much -= pc_parry[which_pc] / 4;

  if (damage_type != DAMAGE_MARKED) {
    if (PSD[SDF_EASY_MODE] > 0)
      how_much -= 3;

    // toughness
    if (traits[TRAIT_TOUGHNESS] == true)
      how_much--;

    // luck
    if (get_ran(1, 0, 100) < 2 * (hit_chance[skills[SKILL_LUCK]] - 20))
      how_much -= 1;
  }

  if ((damage_type == DAMAGE_WEAPON) &&
      ((tlevel = getProtLevel(ITEM_PROTECTION)) > 0))
    how_much -= tlevel;
  if ((damage_type == DAMAGE_UNDEAD) &&
      ((tlevel = getProtLevel(ITEM_PROTECT_FROM_UNDEAD)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);
  if ((damage_type == DAMAGE_DEMON) &&
      ((tlevel = getProtLevel(ITEM_PROTECT_FROM_DEMONS)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);

  if ((type_of_attacker == MONSTER_TYPE_HUMANOID) &&
      ((tlevel = getProtLevel(ITEM_PROTECT_FROM_HUMANOIDS)) >
       0)) // protection from humanoids
    how_much /= ((tlevel >= 7) ? 4 : 2);
  if ((type_of_attacker == MONSTER_TYPE_REPTILE) &&
      ((tlevel = getProtLevel(ITEM_PROTECT_FROM_REPTILES)) >
       0)) // protection from reptiles
    how_much /= ((tlevel >= 7) ? 4 : 2);
  if ((type_of_attacker == MONSTER_TYPE_GIANT) &&
      ((tlevel = getProtLevel(ITEM_PROTECT_FROM_GIANTS)) >
       0)) // protection from giants
    how_much /= ((tlevel >= 7) ? 4 : 2);

  // invuln
  if (status[STATUS_INVULNERABLE] > 0)
    how_much = 0;

  // magic resistance
  if ((damage_type == DAMAGE_MAGIC) &&
      ((tlevel = getProtLevel(ITEM_MAGIC_PROTECTION)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);

  // Mag. res helps w. fire and cold
  if (((damage_type == DAMAGE_FIRE) || (damage_type == DAMAGE_COLD)) &&
      (status[STATUS_MAGIC_RESISTANCE] > 0))
    how_much /= 2;

  // fire res.
  if ((damage_type == DAMAGE_FIRE) &&
      ((tlevel = getProtLevel(ITEM_FIRE_PROTECTION)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);

  // cold res.
  if ((damage_type == DAMAGE_COLD) &&
      ((tlevel = getProtLevel(ITEM_COLD_PROTECTION)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);

  // major resistance
  if (((damage_type == DAMAGE_FIRE) || (damage_type == DAMAGE_POISON) ||
       (damage_type == DAMAGE_MAGIC) || (damage_type == DAMAGE_COLD)) &&
      ((tlevel = getProtLevel(ITEM_FULL_PROTECTION)) > 0))
    how_much /= ((tlevel >= 7) ? 4 : 2);

  // ugly hack
  //	short which_pc = getNum();

  // temporarily disabled
  if (boom_anim_active == true) {
    if (how_much < 0)
      how_much = 0;
    pc_marked_damage[which_pc] += how_much;

    if (is_town())
      add_explosion(c_town.p_loc, how_much, 0,
                    (damage_type > DAMAGE_POISON) ? 2 : 0, 0, 0);
    else
      add_explosion(pc_pos[which_pc], how_much, 0,
                    (damage_type > DAMAGE_POISON) ? 2 : 0, 0, 0);

    if (how_much == 0)
      return false;
    else
      return true;
  }

  if (how_much <= 0) {
    if ((damage_type == DAMAGE_WEAPON) || (damage_type == DAMAGE_UNDEAD) ||
        (damage_type == DAMAGE_DEMON))
      play_sound(2);
    add_string_to_buf("  No damage.");
    return false;
  } else {
    // if asleep, get bonus
    if (status[11] > 0)
      status[11]--;

    sprintf(c_line, "  %s takes %d. ", name, how_much);
    if (do_print)
      add_string_to_buf(c_line);
    if (damage_type != DAMAGE_MARKED) {
      if (is_combat())
        boom_space(pc_pos[which_pc], overall_mode, boom_gr[damage_type],
                   how_much, sound_type);
      else if (is_town())
        boom_space(c_town.p_loc, overall_mode, boom_gr[damage_type], how_much,
                   sound_type);
      else
        boom_space(party.p_loc, 100, boom_gr[damage_type], how_much,
                   sound_type);
    }
    if (overall_mode != MODE_OUTDOORS)
      FlushEvents(1);
    FlushEvents(0);
  }

  party.total_dam_taken += how_much;

  if (cur_health >= how_much)
    cur_health -= how_much;
  else if (cur_health > 0)
    cur_health = 0;
  else // Check if PC can die
      if (how_much > 25) {
    sprintf(c_line, "  %s is obliterated.  ", name);
    add_string_to_buf(c_line);
    kill(3);
  } else {
    sprintf(c_line, "  %s is killed.", name);
    add_string_to_buf(c_line);
    kill(2);
  }
  if ((cur_health == 0) && isAlive())
    play_sound(3);
  put_pc_screen();
  return true;
}

void set_pc_moves() {
  short i, r, i_level;

  for (i = 0; i < 6; i++)
    if (adven[i].isAlive() == false)
      pc_moves[i] = 0;
    else {
      pc_moves[i] = (adven[i].traits[TRAIT_SLUGGISH] == true) ? 3 : 4;
      r = get_encumberance(i);
      pc_moves[i] = minmax(1, 8, pc_moves[i] - (r / 3));

      if ((i_level = adven[i].getProtLevel(ITEM_SPEED)) > 0)
        pc_moves[i] += i_level / 7 + 1;
      if ((i_level = adven[i].getProtLevel(ITEM_SLOW_WEARER)) > 0)
        pc_moves[i] -= i_level / 5;

      if ((adven[i].status[STATUS_HASTE_SLOW] < 0) &&
          (party.age % 2 == 1)) // slowed?
        pc_moves[i] = 0;
      else { // do webs
        pc_moves[i] = max(0, pc_moves[i] - adven[i].status[STATUS_WEBS] / 2);
        if (pc_moves[i] == 0) {
          sprintf(c_line, "%s must clean webs.", adven[i].name);
          add_string_to_buf(c_line);
          adven[i].status[STATUS_WEBS] =
              max(0, adven[i].status[STATUS_WEBS] - 3);
        }
      }
      if (adven[i].status[STATUS_HASTE_SLOW] > 7)
        pc_moves[i] = pc_moves[i] * 3;
      else if (adven[i].status[STATUS_HASTE_SLOW] > 0)
        pc_moves[i] = pc_moves[i] * 2;
      if ((adven[i].status[STATUS_ASLEEP] > 0) ||
          (adven[i].status[STATUS_PARALYZED] > 0))
        pc_moves[i] = 0;
    }
}

void take_ap(short num) {
  pc_moves[current_pc] = max(0, pc_moves[current_pc] - num);
}

short cave_lore_present() {
  short i, ret = 0;
  for (i = 0; i < 6; i++)
    if ((adven[i].isAlive()) && (adven[i].traits[TRAIT_CAVE_LORE] > 0))
      ret += 1;
  return ret;
}

// Time for some chicanery
// The how to item for mage will be 399, for priest will be 499
// item 400 + i will mean cast mage spell i.
// item 500 + i will mean cast priest spell i.
void adjust_spell_menus() {
  short i, j, spell_pos = 0;
  short total_added = 0;
  char spell_name[256];
  short old_on_spell_menu[2][62];
  Boolean need_menu_change = false;
  HMENU menu, big_menu;

  if (in_startup_mode == true)
    return;

  big_menu = GetMenu(mainPtr);
  menu = GetSubMenu(big_menu, 6);
  if (menu == NULL)
    return;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 62; j++)
      old_on_spell_menu[i][j] = on_spell_menu[i][j];

  for (i = 0; i < 62; i++) {
    on_spell_menu[0][i] = -1;
  }
  for (i = 0; i < 62; i++)
    if (pc_can_cast_spell(current_pc, 0, i)) {
      on_spell_menu[0][spell_pos] = i;
      spell_pos++;
    }

  for (i = 0; i < 62; i++)
    if (on_spell_menu[0][i] != old_on_spell_menu[0][i])
      need_menu_change = true;

  if (need_menu_change) {
    for (i = 0; i < 62; i++) {
      DeleteMenu(menu, 400 + i, MF_BYCOMMAND);
    }

    for (i = 0; i < 62; i++)
      if (pc_can_cast_spell(current_pc, 0, i) == true) {
        if (spell_cost[0][i] > 0)
          sprintf(spell_name, "L%d - %s, C %d", spell_level[i], mage_s_name[i],
                  spell_cost[0][i]);
        else
          sprintf(spell_name, "L%d - %s, C ?", spell_level[i], mage_s_name[i]);
        total_added++;
        if (total_added % 24 == 0)
          InsertMenu(menu, 399,
                     MF_MENUBREAK | MF_BYCOMMAND | MF_ENABLED | MF_STRING,
                     400 + i, spell_name);
        else
          InsertMenu(menu, 399, MF_BYCOMMAND | MF_ENABLED | MF_STRING, 400 + i,
                     spell_name);

        // InsertMenu(menu,399,MF_BYCOMMAND | MF_ENABLED | MF_STRING,
        //	 400 + i, spell_name);
        // beep();
      }
  }

  need_menu_change = false;
  spell_pos = 0;
  total_added = 0;
  menu = GetSubMenu(big_menu, 7);
  if (menu == NULL)
    return;

  for (i = 0; i < 62; i++) {
    on_spell_menu[1][i] = -1;
  }
  for (i = 0; i < 62; i++)
    if (pc_can_cast_spell(current_pc, 1, i)) {
      on_spell_menu[1][spell_pos] = i;
      spell_pos++;
    }
  for (i = 0; i < 62; i++)
    if (on_spell_menu[1][i] != old_on_spell_menu[1][i])
      need_menu_change = true;
  if (need_menu_change) {
    for (i = 0; i < 62; i++) {
      DeleteMenu(menu, 500 + i, MF_BYCOMMAND);
    }
    for (i = 0; i < 62; i++)
      if (pc_can_cast_spell(current_pc, 1, i) == true) {
        // spell_name[0] = strlen((char *) priest_s_name[on_spell_menu[1][i]]);
        // strcpy((char *) (spell_name + 1),priest_s_name[on_spell_menu[1][i]]);
        if (spell_cost[1][i] > 0)
          sprintf(spell_name, " L%d - %s, C %d", spell_level[i],
                  priest_s_name[i], spell_cost[1][i]);
        else
          sprintf(spell_name, " L%d - %s, C ?", spell_level[i],
                  priest_s_name[i]);
        total_added++;
        if (total_added % 24 == 0)
          InsertMenu(menu, 499,
                     MF_MENUBREAK | MF_BYCOMMAND | MF_ENABLED | MF_STRING,
                     500 + i, spell_name);
        else
          InsertMenu(menu, 499, MF_BYCOMMAND | MF_ENABLED | MF_STRING, 500 + i,
                     spell_name);
      }
  }
}
