#ifndef _TFILEIO_H
#define _TFILEIO_H

void Get_Path(char *path);
void file_initialize();
void save_scenario();
void load_scenario();
void load_spec_graphics();
void augment_terrain(location to_create);
void load_outdoors(location which_out, short mode);
void load_town(short which_town);
void import_town(short which_town);
void oops_error(short error);
void start_town_data_dump();
void start_data_dump();
void start_shopping_data_dump();
void start_monst_data_dump();
void start_spec_data_dump();
void scen_text_dump();
void port_talk_nodes();
void port_town();
void port_dummy_town();
void port_dummy_t_d(short size, char *buffer);
void port_dummy_talk_nodes();
void port_t_d();
void port_scenario();
void port_item_list();
void port_out(outdoor_record_type *out);
void flip_spec_node(special_node_type *spec);
void flip_short(short *s);
void alter_rect(RECT *r);
void flip_rect(RECT *s);
short FSRead(HFILE file, long *len, char *buffer);
short SetFPos(HFILE file, short mode, long len);
void make_new_scenario(char *file_name, short out_width, short out_height,
                       short making_warriors_grove, short use_grass,
                       char *title);
void flip_rect(RECT16 *s);
void alter_rect(RECT16 *r);
void object_scenario_data_dump();
void start_outdoor_data_dump();

// relics from the past--for temporary backwards compatibility with older
// scenario editor.
short init_data(short flag);
short town_s(short flag);
short out_s(short flag);
short str_size_1(short flag);
short str_size_2(short flag);
short str_size_3(short flag);
#endif
