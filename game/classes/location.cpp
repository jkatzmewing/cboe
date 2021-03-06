#include "location.h"		// location
#include "../tools/soundtool.h"		// play_sound
#include "../boe.main.h"			// out
#include "../boe.text.h"			// add_string_to_buf
#include "../boe.locutils.h"		// alter_space
#include "../boe.items.h"			// fancy_choice_dialog

#include "../global.h"
#include "../globvar.h"
#include "../tools/mathutil.h"

location location::toGlobal()
{
	//return local_to_global(*this);

	location global = *this;
	if (party.i_w_c.x == 1)	global.x = global.x + 48;
	if (party.i_w_c.y == 1)	global.y = global.y + 48;
	return global;
}

location location::toLocal()
{
//	return global_to_local(*this);

	location local = (*this);
	if (party.i_w_c.x == 1)	local.x = local.x - 48;
	if (party.i_w_c.y == 1)	local.y = local.y - 48;
	return local;
}

short location::countWalls() const
{
	unsigned char walls[31] = {5,6,7,8,9, 10,11,12,13,14, 15,16,17,18,19, 20,21,22,23,24,
							25,26,27,28,29, 30,31,32,33,34, 35};
	short answer = 0;
	short k = 0;

	for (k = 0; k < 31 ; k++)
	{
		if (out[x + 1][y] == walls[k]) answer++;
		if (out[x - 1][y] == walls[k]) answer++;
		if (out[x][y + 1] == walls[k]) answer++;
		if (out[x][y - 1] == walls[k]) answer++;
	}
	return answer;
}

// 0 if no pull.
// 1 if in right position now.
// 2 if in left position
// levers should always start to left.
short location::handleLever()
{
	if (FCD(1020,0) == 1) return 0;
	play_sound(94);
	alter_space(x,y,scenario.ter_types[t_d.terrain[x][y]].trans_to_what);
	return 1;
}

void location::crumbleWall()
{
	unsigned char ter;

	if (loc_off_act_area(*this) == true)
		return;
	ter = t_d.terrain[x][y];
	if (scenario.ter_types[ter].special == TER_SPEC_CRUMBLING_TERRAIN) {
			play_sound(60);
			t_d.terrain[x][y] = scenario.ter_types[ter].flag1;
			if(scenario.ter_types[scenario.ter_types[ter].flag1].special >= TER_SPEC_CONVEYOR_NORTH && scenario.ter_types[scenario.ter_types[ter].flag1].special <= TER_SPEC_CONVEYOR_WEST)
                belt_present = true;
			add_string_to_buf("  Barrier crumbles.");
		}
}

bool location::isDoor() const
{
	if ((scenario.ter_types[t_d.terrain[x][y]].special == TER_SPEC_UNLOCKABLE_TERRAIN) ||
		(scenario.ter_types[t_d.terrain[x][y]].special == TER_SPEC_CHANGE_WHEN_STEP_ON) ||
		(scenario.ter_types[t_d.terrain[x][y]].special == TER_SPEC_UNLOCKABLE_BASHABLE))
			return true;
	return false;
}

//void pick_lock(location where,short pc_num)
void location::pickLock(short pc_num)
{
	unsigned char terrain;
	short r1,which_item;
	Boolean will_break = false;
	short unlock_adjust;

	terrain = t_d.terrain[x][y];
	which_item = adven[pc_num].hasAbilEquip(ITEM_LOCKPICKS);
	if (which_item == 24) {
		add_string_to_buf("  Need lockpick equipped.        ");
		return;
		}

	r1 = get_ran(1,0,99) + adven[pc_num].items[which_item].ability_strength * 7;

	if (r1 < 75)
		will_break = true;

	r1 = get_ran(1,0,99) - 5 * adven[pc_num].statAdj(SKILL_DEXTERITY) + c_town.difficulty * 7
	 - 5 * adven[pc_num].skills[SKILL_LOCKPICKING] - adven[pc_num].items[which_item].ability_strength * 7;

	// Nimble?
	if (adven[pc_num].traits[TRAIT_NIMBLE] == true) r1 -= 8;

	if (adven[pc_num].hasAbilEquip(ITEM_THIEVING) < 24)
		r1 = r1 - 12;

	if ((scenario.ter_types[terrain].special < TER_SPEC_UNLOCKABLE_TERRAIN) || (scenario.ter_types[terrain].special > TER_SPEC_UNLOCKABLE_BASHABLE)) {
		add_string_to_buf("  Wrong terrain type.           ");
		return;
		}
	unlock_adjust = scenario.ter_types[terrain].flag2;
	if ((unlock_adjust >= 5) || (r1 > (90 - unlock_adjust * 15))) {
					add_string_to_buf("  Didn't work.                ");
					if (will_break == true)
					{
						add_string_to_buf("  Pick breaks.                ");
						adven[pc_num].removeCharge(which_item);
					}
				play_sound(41);
				}
				else {
						add_string_to_buf("  Door unlocked.                ");
						play_sound(9);
						t_d.terrain[x][y] = scenario.ter_types[terrain].flag1;
						if(scenario.ter_types[scenario.ter_types[terrain].flag1].special >= TER_SPEC_CONVEYOR_NORTH && scenario.ter_types[scenario.ter_types[terrain].flag1].special <= TER_SPEC_CONVEYOR_WEST)
                            belt_present = true;
					}
}

//void bash_door(location where,short pc_num)
void location::bashDoor(short pc_num)
{
	unsigned char terrain;
	short r1,unlock_adjust;

	terrain = t_d.terrain[x][y];
	r1 = get_ran(1,0,100) - 15 * adven[pc_num].statAdj(SKILL_STRENGTH) + c_town.difficulty * 4;

	if ((scenario.ter_types[terrain].special < TER_SPEC_UNLOCKABLE_TERRAIN) || (scenario.ter_types[terrain].special > TER_SPEC_UNLOCKABLE_BASHABLE))
	{
		add_string_to_buf("  Wrong terrain type.           ");
		return;
	}

	unlock_adjust = scenario.ter_types[terrain].flag2;

	if ((unlock_adjust >= 5) || (r1 > (100 - unlock_adjust * 15)) || (scenario.ter_types[terrain].special != TER_SPEC_UNLOCKABLE_BASHABLE))
	{
		add_string_to_buf("  Didn't work.                ");
		adven[pc_num].damage(get_ran(1,1,4),4,-1);
	}
	else
	{
		add_string_to_buf("  Lock breaks.                ");
		play_sound(9);
		t_d.terrain[x][y] = scenario.ter_types[terrain].flag1;
		if(scenario.ter_types[scenario.ter_types[terrain].flag1].special >= TER_SPEC_CONVEYOR_NORTH && scenario.ter_types[scenario.ter_types[terrain].flag1].special <= TER_SPEC_CONVEYOR_WEST)
            belt_present = true;
	}
}
