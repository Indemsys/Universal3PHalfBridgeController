#ifndef __PARAMETERS
  #define __PARAMETERS

#define  MAX_PARAMETER_STRING_LEN 256


void              Do_Params_editor(uint8_t keycode);
void              Display_params_menu(void);
void              Return_def_params(void);
int32_t           Restore_parameters_and_settings(void);
int               Restore_from_Flash(void);
int               Save_Params_to_Flash(void);
void              Edit_func(uint8_t b);


void              Display_params_func(uint8_t b);

void              Save_str_to_logfile(char* str);
void              Close_logfile(void);
void              Execute_param_func(uint16_t indx);

void              Param_to_str(uint8_t *buf,uint16_t maxlen, uint16_t indx);
void              Str_to_param(uint8_t *buf,uint16_t indx);

uint8_t*          Get_mn_shrtcapt(enum enm_parmnlev menu_lev);
uint8_t*          Get_mn_name(enum enm_parmnlev menu_lev);
int16_t           Get_menu_items_count(enum enm_parmnlev menu_item);
uint8_t*          Get_item_capt(enum enm_parmnlev curr_menu, uint8_t item, enum enm_parmnlev *submenu, uint16_t *parnum);
enum enm_parmnlev Get_parent_menu(enum enm_parmnlev curr_menu);
int32_t           Find_param_by_abbrev(char* abbrev);
enum  vartypes    Get_param_type(int n);


T_parmenu*        Get_parmenu(int16_t* sz);
T_work_params*    Get_params(int16_t* sz);

uint8_t           ascii_to_hex(uint8_t c);
uint8_t           hex_to_ascii(uint8_t c);

_mqx_int          Restore_from_INIfile(void);
_mqx_int          Save_Params_to_INIfile(void);
#endif
