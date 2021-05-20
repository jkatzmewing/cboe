#ifndef _INFODLGS_H
#define _INFODLGS_H

void put_spell_info();
Boolean display_spells_event_filter(short item_hit);
void display_spells(short mode, short force_spell, short parent_num);
void put_skill_info();
Boolean display_skills_event_filter(short item_hit);
void display_skills(short force_skill, short parent);
void put_pc_graphics();
Boolean display_pc_event_filter(short item_hit);
void display_pc(short pc_num, short mode, short parent_num);
void put_item_info();
Boolean display_pc_item_event_filter(short item_hit);
void display_pc_item(short pc_num, short item, item_record_type si,
                     short parent);
void put_monst_info();
Boolean display_monst_event_filter(short item_hit);
void display_monst(short array_pos, creature_data_type *which_m, short mode);
Boolean display_help_event_filter(short item_hit);
void display_help(short mode, short parent);
Boolean display_alchemy_event_filter(short item_hit);
void display_alchemy();
void pick_race_abil(pc_record_type *pc, short mode, short parent_num);
void pick_race_abil_event_filter(short item_hit);
void display_traits_graphics();
void give_pc_info(short pc_num);
void give_pc_info_event_filter(short item_hit);
void display_pc_info();
void adventure_notes_event_filter(short item_hit);
void adventure_notes();
void put_talk();
void talk_notes_event_filter(short item_hit);
void talk_notes();
void journal();
void journal_event_filter(short item_hit);
void add_to_journal(short event);
void anax_string(short val1, short val2);
void give_help(short help1, short help2, short parent_num);
void put_spec_item_info(short which_i);
void display_strings_event_filter(short item_hit);
void display_strings(char const *text1, char const *text2, short str_label_1,
                     short str_label_2, short str_label_1b, short str_label_2b,
                     char const *title, short sound_num, short graphic_num,
                     short parent_num);
void give_error(char const *text1, char const *text2, short parent_num);
void display_strings_with_nums(short a1, short a2, short b1, short b2,
                               char const *title, short sound_num,
                               short graphic_num, short parent_num);

#endif
