// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-13
// 08:45:06
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "FreeMaster.h"


#define FMCMD_RESET_DEVICE                      100
#define FMCMD_CHANGE_RECORDER_SMPS_SRC          20 // Сменить источник сэмплирования рекордера FreeMaster

static T_cmd_man Cmd_man_func;

/*-----------------------------------------------------------------------------------------------------
 
 \param func 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_Set_freemaster_cmd_man(T_cmd_man func)
{
  Cmd_man_func = func;
}

/*-----------------------------------------------------------------------------------------------------
  Обработка пользовательских комманд поступающих движку FreeMaster
 
 \param app_command 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
uint8_t U3HB01_Freemaster_Command_Manager(uint16_t app_command)
{
  uint8_t  res   = 0;
  uint16_t len;
  uint8_t  *dbuf;
  int32_t  ret;

  // Порлучаем указатель на буфер с данными команды
  dbuf = FMSTR_GetAppCmdData(&len);


  ret = -1;
  if (Cmd_man_func != 0)
  {
    ret = Cmd_man_func(app_command, &res);
  }

  if (ret < 0)
  {
    switch (app_command)
    {
    case FMCMD_CHANGE_RECORDER_SMPS_SRC:
      {
        uint8_t  *arg;
        uint32_t v;
        uint16_t sz;
        arg = FMSTR_GetAppCmdData(&sz);
        if (sz == sizeof(v))
        {
          memcpy(&v, arg, sizeof(v));
          Frmstr_Set_recorder_smpls_src(v);
          res = 0;
        }
        else res = 1;
      }
      break;

    case FMCMD_RESET_DEVICE:
      Reset_system();
      break;
    default:
      res = 0;
      break;
    }
  }

  return 0;
}


