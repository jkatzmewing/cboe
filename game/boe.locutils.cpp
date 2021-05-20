#include <windows.h>
#include <cmath>
#include "global.h"
#include "boe.locutils.h"
#include "boe.text.h"
#include "boe.monster.h"
#include "boe.fields.h"
#include "tools/mathutil.h"

#include "globvar.h"

short dist(location p1, location p2) {
  return (short)sqrt(
      (double)((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
}

short vdist(location p1, location p2) {
  short i, j;
  i = ex_abs((short)(p1.x - p2.x));
  j = ex_abs((short)(p1.y - p2.y));
  return max(i, j);
}

Boolean adjacent(location p1, location p2) {
  if ((ex_abs((short)(p1.x - p2.x)) <= 1) &&
      (ex_abs((short)(p1.y - p2.y)) <= 1))
    return true;
  else
    return false;
}

Boolean point_onscreen(location center, location check) {
  if ((ex_abs((short)(center.x - check.x)) <= 4) &&
      (ex_abs((short)(center.y - check.y)) <= 4))
    return true;
  else
    return false;
}

short set_direction(location old_pt, location new_pt) {
  if (old_pt.x == new_pt.x)
    if (old_pt.y > new_pt.y)
      return 0;
    else
      return 4;
  if (old_pt.x > new_pt.x) {
    if (old_pt.y > new_pt.y)
      return 7;
    if (old_pt.y < new_pt.y)
      return 5;
    return 6;
  }
  if (old_pt.y > new_pt.y)
    return 1;
  if (old_pt.y < new_pt.y)
    return 3;
  return 2;
}

Boolean loc_off_world(location p1) {
  if ((p1.x < 0) || (p1.x > town_size[town_type]) || (p1.y < 0) ||
      (p1.y > town_size[town_type]))
    return true;
  else
    return false;
}

Boolean loc_off_act_area(location p1) {
  if ((p1.x > c_town.town.in_town_rect.left) &&
      (p1.x < c_town.town.in_town_rect.right) &&
      (p1.y > c_town.town.in_town_rect.top) &&
      (p1.y < c_town.town.in_town_rect.bottom))
    return false;
  return true;
}

location get_cur_loc() {
  switch (overall_mode) {
  case MODE_OUTDOORS:
  case MODE_LOOK_OUTDOORS:
    return party.p_loc;
    break;

  case MODE_TOWN:
  case MODE_TALK_TOWN:
  case MODE_TOWN_TARGET:
  case MODE_USE:
  case MODE_LOOK_TOWN:
    return c_town.p_loc;
    break;

  default:
    return pc_pos[current_pc];
    break;
  }
}

short short_can_see(shortloc p1, shortloc p2) {
  location s1, s2;

  s1.x = (char)p1.x;
  s1.y = (char)p1.y;
  s2.x = (char)p2.x;
  s2.y = (char)p2.y;
  return (can_see(s1, s2, 0));
}

Boolean is_lava(short x, short y) ////
{
  unsigned char ter = coord_to_ter(x, y);
  if (scenario.ter_types[ter].picture == 404)
    return true;
  else
    return false;
}

short can_see(location p1, location p2, short mode)
// short mode; // 0 - normal  1 - counts 1 for blocked spaces or lava (used for
// party placement in 				   town combat)
// 2 - no light check
{
  short dx, dy, count, storage = 0;

  if (is_combat()) { // Light check
    if ((mode != 2) && (combat_pt_in_light(p2) == false)) {
      return 6;
    }
  } else {
    if ((mode != 2) && (pt_in_light(p1, p2) == false)) {
      return 6;
    }
  }

  if (p1.y == p2.y) {
    if (p1.x > p2.x) {
      for (count = p2.x + 1; count < p1.x; count++) {
        storage = storage + get_obscurity(count, p1.y);
        if (((terrain_blocked[coord_to_ter(count, p1.y)] > 2) ||
             (is_lava(count, p1.y) == true)) &&
            (mode == 1))
          return 5;
      }
    } else {
      for (count = p1.x + 1; count < p2.x; count++) {

        storage = storage + get_obscurity(count, p1.y);
        if (((terrain_blocked[coord_to_ter(count, p1.y)] > 2) ||
             (is_lava(count, p1.y) == true)) &&
            (mode == 1))
          return 5;
      }
    }
    return storage;
  }
  if (p1.x == p2.x) {
    if (p1.y > p2.y) {
      for (count = p1.y - 1; count > p2.y; count--) {
        storage = storage + get_obscurity(p1.x, count);
        if (((terrain_blocked[coord_to_ter(p1.x, count)] > 2) ||
             (is_lava(p1.x, count) == true)) &&
            (mode == 1))
          return 5;
      }
    } else {
      for (count = p1.y + 1; count < p2.y; count++) {
        storage = storage + get_obscurity(p1.x, count);
        if (((terrain_blocked[coord_to_ter(p1.x, count)] > 2) ||
             (is_lava(p1.x, count) == true)) &&
            (mode == 1))
          return 5;
      }
    }
    return storage;
  }
  dx = p2.x - p1.x;
  dy = p2.y - p1.y;

  if (ex_abs(dy) > ex_abs(dx)) {
    if (p2.y > p1.y) {
      for (count = 1; count < dy; count++) {
        storage =
            storage + get_obscurity(p1.x + (count * dx) / dy, p1.y + count);
        if (((terrain_blocked[coord_to_ter(p1.x + (count * dx) / dy,
                                           p1.y + count)] > 2) ||
             (is_lava(p1.x + (count * dx) / dy, p1.y + count) == true)) &&
            (mode == 1))
          return 5;
      }
    } else {
      for (count = -1; count > dy; count--) {
        storage =
            storage + get_obscurity(p1.x + (count * dx) / dy, p1.y + count);
        if (((terrain_blocked[coord_to_ter(p1.x + (count * dx) / dy,
                                           p1.y + count)] > 2) ||
             (is_lava(p1.x + (count * dx) / dy, p1.y + count) == true)) &&
            (mode == 1))
          return 5;
      }
    }
    return storage;
  }
  if (ex_abs(dy) <= ex_abs(dx)) {
    if (p2.x > p1.x) {
      for (count = 1; count < dx; count++) {
        storage =
            storage + get_obscurity(p1.x + count, p1.y + (count * dy) / dx);
        if (((terrain_blocked[coord_to_ter(p1.x + count,
                                           p1.y + (count * dy) / dx)] > 2) ||
             (is_lava(p1.x + count, p1.y + (count * dy) / dx) == true)) &&
            (mode == 1))
          return 5;
      }
    } else {
      for (count = -1; count > dx; count--) {
        storage =
            storage + get_obscurity(p1.x + count, p1.y + (count * dy) / dx);
        if (((terrain_blocked[coord_to_ter(p1.x + count,
                                           p1.y + (count * dy) / dx)] > 2) ||
             (is_lava(p1.x + count, p1.y + (count * dy) / dx) == true)) &&
            (mode == 1))
          return 5;
      }
    }
    return storage;
  }
  if (storage > 5)
    return 5;
  else
    return storage;
}

short get_obscurity(short x, short y) {
  unsigned char what_terrain;
  short store;

  what_terrain = coord_to_ter(x, y);

  if ((what_terrain >= 237) && (what_terrain <= 242))
    return 1;

  store = get_blockage(what_terrain);

  if (is_town())
    if (special(x, y))
      store++;

  if ((is_town()) || (is_combat())) {
    if (is_web(x, y))
      store += 2;
    if ((is_fire_barrier(x, y)) || (is_force_barrier(x, y)))
      return 5;
    if ((is_crate(x, y)) || (is_barrel(x, y)))
      store++;
  }
  return store;
}

unsigned char coord_to_ter(short x, short y) {
  char what_terrain;

  if ((overall_mode == MODE_OUTDOORS) || (overall_mode == MODE_LOOK_OUTDOORS))
    what_terrain = out[x][y];
  else if (((overall_mode > MODE_OUTDOORS) && (overall_mode < MODE_COMBAT)) ||
           (overall_mode == MODE_LOOK_TOWN))
    what_terrain = t_d.terrain[x][y];
  else
    what_terrain = combat_terrain[x][y];
  return what_terrain;
}

Boolean is_container(location loc) {
  unsigned char ter;

  if ((is_barrel(loc.x, loc.y)) || (is_crate(loc.x, loc.y)))
    return true;
  ter = coord_to_ter(loc.x, loc.y);
  if (scenario.ter_types[ter].special == TER_SPEC_IS_A_CONTAINER)
    return true;
  return false;
}

void update_explored(location dest) {
  shortloc shortdest, look;

  location look2;

  shortdest.x = (short)dest.x;
  shortdest.y = (short)dest.y;

  which_party_sec.x = party.outdoor_corner.x + party.i_w_c.x;
  which_party_sec.y = party.outdoor_corner.y + party.i_w_c.y;

  if (overall_mode == MODE_OUTDOORS) {
    out_e[dest.x][dest.y] = 2;
    for (look.x = shortdest.x - 4; look.x < shortdest.x + 5; look.x++)
      for (look.y = shortdest.y - 4; look.y < shortdest.y + 5; look.y++) {
        if ((look.x == minmax(0, 95, (int)look.x)) &&
            (look.y == minmax(0, 95, (int)look.y))) {
          if (out_e[look.x][look.y] == 0)
            if (short_can_see(shortdest, look) < 5)
              out_e[look.x][look.y] = 1;
        }
      }
  }

  if (overall_mode > MODE_OUTDOORS) {
    make_explored(dest.x, dest.y);
    for (look2.x = max(0, dest.x - 4);
         look2.x < min(town_size[town_type], dest.x + 5); look2.x++)
      for (look2.y = max(0, dest.y - 4);
           look2.y < min(town_size[town_type], dest.y + 5); look2.y++)
        if (is_explored(look2.x, look2.y) == false)
          if ((can_see(dest, look2, 0) < 5) &&
              (pt_in_light(dest, look2) == true))
            make_explored(look2.x, look2.y);
  }
}

// All purpose function to check is spot is free for travel into.
Boolean is_blocked(location to_check) {
  short i, gr;
  unsigned char ter;

  if (is_out()) {
    if (impassable(out[to_check.x][to_check.y]) == true) {
      return true;
    }
    if (same_point(to_check, party.p_loc) == true)
      return true;
    for (i = 0; i < 20; i++)
      if ((party.out_c[i].exists) == true)
        if (same_point(party.out_c[i].m_loc, to_check) == true)
          return true;
    return false;
  }

  if ((is_town()) || (is_combat())) {
    ter = (is_town()) ? t_d.terrain[to_check.x][to_check.y]
                      : combat_terrain[to_check.x][to_check.y];
    gr = scenario.ter_types[ter].picture;

    // Terrain blocking?
    if (impassable(ter) == true) {
      return true;
    }

    // Keep away from marked specials during combat
    if ((is_combat()) && (gr <= 212) && (gr >= 207))
      return true;
    if ((is_combat()) && (gr == 406)) // portal
      return true;

    // Party there?
    if (is_town())
      if (same_point(to_check, c_town.p_loc) == true)
        return true;
    if (is_combat())
      for (i = 0; i < 6; i++)
        if ((adven[i].isAlive()) && (same_point(to_check, pc_pos[i]) == true))
          return true;

    // Monster there?
    if (monst_there(to_check) < 90)
      return true;

    // Magic barrier?
    if (is_force_barrier(to_check.x, to_check.y))
      return true;

    return false;
  }
  return true;
}

Boolean monst_on_space(location loc, short m_num) {
  if (c_town.monst.dudes[m_num].active == 0)
    return false;
  if ((loc.x - c_town.monst.dudes[m_num].m_loc.x >= 0) &&
      (loc.x - c_town.monst.dudes[m_num].m_loc.x <=
       c_town.monst.dudes[m_num].m_d.x_width - 1) &&
      (loc.y - c_town.monst.dudes[m_num].m_loc.y >= 0) &&
      (loc.y - c_town.monst.dudes[m_num].m_loc.y <=
       c_town.monst.dudes[m_num].m_d.y_width - 1))
    return true;
  return false;
}
short monst_there(location where) // returns 90 if no
{
  short i;

  for (i = 0; i < T_M; i++)
    if ((c_town.monst.dudes[i].active != 0) &&
        (monst_on_space(where, i) == true))
      return i;
  return 90;
}
Boolean monst_can_be_there(location loc, short m_num) {
  short i, j;
  location destination;

  // First clear monst away so it doesn't block itself
  c_town.monst.dudes[m_num].m_loc.x += 100;

  for (i = 0; i < c_town.monst.dudes[m_num].m_d.x_width; i++)
    for (j = 0; j < c_town.monst.dudes[m_num].m_d.y_width; j++) {
      destination.x = loc.x + i;
      destination.y = loc.y + j;
      if ((is_blocked(destination) == true) ||
          (loc_off_act_area(destination) == true)) {
        c_town.monst.dudes[m_num].m_loc.x -= 100;
        return false;
      }
    }
  c_town.monst.dudes[m_num].m_loc.x -= 100;
  return true;
}

Boolean monst_adjacent(location loc, short m_num) {
  short i, j;
  location destination;

  for (i = 0; i < c_town.monst.dudes[m_num].m_d.x_width; i++)
    for (j = 0; j < c_town.monst.dudes[m_num].m_d.y_width; j++) {
      destination.x = c_town.monst.dudes[m_num].m_loc.x + i;
      destination.y = c_town.monst.dudes[m_num].m_loc.y + j;
      if (adjacent(destination, loc) == true)
        return true;
    }
  return false;
}

Boolean monst_can_see(short m_num, location l) {
  short i, j;
  location destination;

  for (i = 0; i < c_town.monst.dudes[m_num].m_d.x_width; i++)
    for (j = 0; j < c_town.monst.dudes[m_num].m_d.y_width; j++) {
      destination.x = c_town.monst.dudes[m_num].m_loc.x + i;
      destination.y = c_town.monst.dudes[m_num].m_loc.y + j;
      if (can_see(destination, l, 0) < 5)
        return true;
    }
  return false;
}

Boolean party_can_see_monst(short m_num) {
  short i, j;
  location destination;

  for (i = 0; i < c_town.monst.dudes[m_num].m_d.x_width; i++)
    for (j = 0; j < c_town.monst.dudes[m_num].m_d.y_width; j++) {
      destination.x = c_town.monst.dudes[m_num].m_loc.x + i;
      destination.y = c_town.monst.dudes[m_num].m_loc.y + j;
      if (party_can_see(destination) < 6)
        return true;
    }
  return false;
}

Boolean can_see_monst(location l, short m_num) {
  short i, j;
  location destination;

  for (i = 0; i < c_town.monst.dudes[m_num].m_d.x_width; i++)
    for (j = 0; j < c_town.monst.dudes[m_num].m_d.y_width; j++) {
      destination.x = c_town.monst.dudes[m_num].m_loc.x + i;
      destination.y = c_town.monst.dudes[m_num].m_loc.y + j;
      if (can_see(l, destination, 0) < 5)
        return true;
    }
  return false;
}

Boolean outd_is_blocked(location to_check) {
  short i;

  if (overall_mode == MODE_OUTDOORS) {
    if (impassable(out[to_check.x][to_check.y]) == true) {
      return true;
    }
    for (i = 0; i < 10; i++)
      if ((party.out_c[i].exists) == true)
        if (same_point(party.out_c[i].m_loc, to_check) == true)
          return true;
    return false;
  }
  return false;
}

// Checks if space is a special that prevents movement into or placement of a PC
// on
Boolean is_special(location to_check) {
  unsigned char which_ter;

  if (terrain_blocked[coord_to_ter(to_check.x, to_check.y)] == 2)
    return false;
  which_ter = coord_to_ter(to_check.x, to_check.y);
  if (terrain_blocked[which_ter] == 2)
    return true;
  else
    return false;
}

Boolean outd_is_special(location to_check) {
  if (overall_mode == MODE_OUTDOORS) {
    if (terrain_blocked[out[to_check.x][to_check.y]] == 2) {
      return true;
    } else
      return false;
  }
  return false;
}

Boolean impassable(unsigned char terrain_to_check) {
  if (terrain_blocked[terrain_to_check] > 2)
    return true;
  else
    return false;
}

short get_blockage(unsigned char terrain_type) {
  // little kludgy in here for pits
  if ((terrain_type == 90) && (is_combat()) && (which_combat_type == 0))
    return 5;
  if ((terrain_blocked[terrain_type] == 5) ||
      (terrain_blocked[terrain_type] == 1))
    return 5;
  else if (terrain_blocked[terrain_type] > 3)
    return 1;
  else {
    return 0;
  }
}

short light_radius() {
  short store = 1, i;
  short extra_levels[6] = {10, 20, 50, 75, 110, 140};

  if (((which_combat_type == 0) && (is_combat())) || (is_out()) ||
      (c_town.town.lighting == 0))
    return 200;
  for (i = 0; i < 6; i++)
    if (party.light_level > extra_levels[i])
      store++;
  return store;
}

Boolean pt_in_light(location from_where,
                    location to_where) // Assumes, of course, in town or combat
{
  if (c_town.town.lighting == 0)
    return true;
  if ((to_where.x < 0) || (to_where.x >= town_size[town_type]) ||
      (to_where.y < 0) || (to_where.y >= town_size[town_type]))
    return true;
  if (t_d.lighting[to_where.x / 8][to_where.y] &
      (char)(s_pow(2, to_where.x % 8)))
    return true;

  if (dist(from_where, to_where) <= light_radius())
    return true;

  return false;
}

Boolean combat_pt_in_light(location to_where) {
  short i, rad;

  if ((c_town.town.lighting == 0) || (which_combat_type == 0))
    return true;
  if ((to_where.x < 0) || (to_where.x >= town_size[town_type]) ||
      (to_where.y < 0) || (to_where.y >= town_size[town_type]))
    return true;
  if (t_d.lighting[to_where.x / 8][to_where.y] &
      (char)(s_pow(2, to_where.x % 8)))
    return true;

  rad = light_radius();
  for (i = 0; i < 6; i++)
    if (adven[i].isAlive()) {
      if (dist(pc_pos[i], to_where) <= rad)
        return true;
    }

  return false;
}

Boolean party_sees_a_monst() // Returns true is a hostile monster is in sight.
{
  short i;

  for (i = 0; i < T_M; i++) {
    if (c_town.monst.dudes[i].active > 0)
      if ((c_town.monst.dudes[i].attitude == 1) &&
          (party_can_see_monst(i) == true))
        return true;
  }

  return false;
}

// Returns 6 if can't see, O.W. returns the # of a PC that can see
short party_can_see(location where) {
  short i;

  if (is_out()) {
    if ((point_onscreen(party.p_loc, where) == true) &&
        (can_see(party.p_loc, where, 0) < 5))
      return 1;
    else
      return 6;
  }
  if (is_town()) {
    if (((point_onscreen(c_town.p_loc, where) == true) ||
         (overall_mode == MODE_LOOK_TOWN)) &&
        (pt_in_light(c_town.p_loc, where) == true) &&
        (can_see(c_town.p_loc, where, 0) < 5))
      return 1;
    else
      return 6;
  }

  // Now for combat checks. Doing separately for efficiency. Check first for
  // light. If
  //   dark, give up.
  if ((which_combat_type != 0) && (combat_pt_in_light(where) == false))
    return 6;

  for (i = 0; i < 6; i++)
    if (adven[i].isAlive()) {
      if (can_see(pc_pos[i], where, 0) < 5)
        return i;
    }

  return 6;
}
location push_loc(location from_where, location to_where) {
  location loc_to_try;

  loc_to_try = to_where;
  loc_to_try.x = loc_to_try.x + (to_where.x - from_where.x);
  loc_to_try.y = loc_to_try.y + (to_where.y - from_where.y);
  if ((t_d.terrain[loc_to_try.x][loc_to_try.y] == 90) ||
      ((t_d.terrain[loc_to_try.x][loc_to_try.y] >= 50) &&
       (t_d.terrain[loc_to_try.x][loc_to_try.y] <= 64)) ||
      (t_d.terrain[loc_to_try.x][loc_to_try.y] == 71) ||
      ((t_d.terrain[loc_to_try.x][loc_to_try.y] >= 74) &&
       (t_d.terrain[loc_to_try.x][loc_to_try.y] <= 78))) {
    // Destroy crate
    loc_to_try.x = 0;
    return loc_to_try;
  }
  if ((get_obscurity((short)loc_to_try.x, (short)loc_to_try.y) > 0) ||
      (terrain_blocked[t_d.terrain[loc_to_try.x][loc_to_try.y]] > 0) ||
      (loc_off_act_area(loc_to_try) == true) ||
      (monst_there(loc_to_try) < 90) || (pc_there(loc_to_try) < 6))
    return from_where;
  else
    return loc_to_try;
}

Boolean spot_impassable(short i, short j) {
  unsigned char ter;

  ter = coord_to_ter(i, j);
  if (terrain_blocked[ter] == 5)
    return true;
  else
    return false;
}

void alter_space(short i, short j, unsigned char ter) {
  location l;

  l.x = i;
  l.y = j;
  map_graphic_placed[i / 8][j] =
      map_graphic_placed[i / 8][j] & ~((unsigned char)(s_pow(2, i % 8)));

  if (is_out()) {
    l = l.toGlobal();
    out[l.x][l.y] = ter;
    outdoors[party.i_w_c.x][party.i_w_c.y].terrain[i][j] = ter;
  } else {
    t_d.terrain[i][j] = ter;
    combat_terrain[i][j] = ter;
    if ((scenario.ter_types[t_d.terrain[i][j]].special >=
         TER_SPEC_CONVEYOR_NORTH) &&
        (scenario.ter_types[t_d.terrain[i][j]].special <=
         TER_SPEC_CONVEYOR_WEST))
      belt_present = true;
  }
}
