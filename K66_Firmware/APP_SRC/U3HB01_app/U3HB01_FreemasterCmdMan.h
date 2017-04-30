#ifndef U3HB01_FREEMASTERCMDMAN_H
#define U3HB01_FREEMASTERCMDMAN_H



typedef int32_t(*T_cmd_man)(uint16_t, uint8_t*);


uint8_t U3HB01_Freemaster_Command_Manager(uint16_t app_command);
void    U3HB01_Set_freemaster_cmd_man(T_cmd_man func);

#endif // MDC01_FREEMASTERCMDMAN_H



