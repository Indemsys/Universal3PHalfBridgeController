#include "App.h"
#include "freemaster.h"
#include "freemaster_os.h"

/*

Анализ протокола FreeMaster



Пример пакета чтения переменной в Scope

 [2b] - начало пакета
 [04] - команда FMSTR_CMD_READMEM_EX        0x04U
 [05] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [04] - длина адреса
 [f8][18][00][20] - адрес
 [c3] - контрольная сумма


Пример команды начала чтения для элемента Scope
 [2b]
 [0a] - команда FMSTR_CMD_SETUPSCOPE_EX     0x0aU
 [10] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [03] - количество переменных в Scope
 [04] - Размер первой переменной
 [84 1a 00  20] - Адрес первой переменной
 [04] - Размер второй переменной
 [f8 18  00 20] - Адрес второй переменной
 [04] - Размер третьей переменной
 [98  1a 00 20] - Адрес третьей переменной
 [17] - контрольная сумма

Пример команды запроса следующего значения для элемента Scope
 [2b] - начало пакета
 [c5] - FMSTR_CMD_READSCOPE         0xc5U
 [3b] - контрольная сумма

Пример ответа на запрос
 [2b] - начало пакета
 [00] - константа
 [00 00 00 00] - Значение переменной
 [00] - контрольная сумма. Вычисляется как:  ~(сумма всех байт кроме 2b) + 1

Пример команды начала чтения для элемента Recorder

 [2b] - начало пакета
 [0b] - команда FMSTR_CMD_SETUPREC_EX       0x0bU
 [21] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [01] - тип применяемого тригера для начала записи
 [18 15] - количество сэмплов
 [03 15] - количество сэмплов записываемых после триггера
 [01 00] - предделитель частоты выборки
 [ac 2f  00 20] - адрес переменной триггера
 [04] - размер переменной триггера
 [00] - способ сравнения триггера (знаковый, беззнаковый)
 [01 00 00 00] - значение величины переменной триггера
 [03] - количество записываемых переменных
 [04] - размер первой переменной
 [98 1a  00 20] - Адрес первой переменной
 [04] - Размер второй переменной
 [a0  1a 00 20] - Адрес второй переменной
 [04] - Размер третьей переменной
 [ec 2f 00 20] - Адрес третьей переменной
 [97] - контрольная сумма

Пример команды запроса состоянпия триггера элемента Recorder
 [2b] - начало пакета
 [c3] - FMSTR_CMD_GETRECSTS         0xc3U    get the recorder status
 [3d] - контрольная сумма

Ответ при отсутствии данных
 [2b] - начало пакета
 [01] - FMSTR_STS_RECRUN            0x01U
 [ff] - контрольная сумма

Ответ при наличии данных
 [2b] - начало пакета
 [02] - FMSTR_STS_RECDONE           0x02U
 [fe] - контрольная сумма

Пример команды запроса данных для элемента Recorder
 [2b] - начало пакета
 [c9] - FMSTR_CMD_GETRECBUFF_EX     0xc9U
 [37] - контрольная сумма

Пример ответа на запрос данных для элемента Recorder
 [2b] - начало пакета
 [00] - FMSTR_STS_OK                0x00U
 [b8 16 ff 1f] - адрес буфера с данными
 [96  02] - индекс последнего отсчета в буфере + 1 (pcm_wRecBuffStartIx)
 [7c] - контрольная сумма

Пример запроса на чтение данных из буффера для элемента Recorder
 [2b] - начало пакета
 [04] - FMSTR_CMD_READMEM_EX        0x04U    read a block of memory
 [05] - количество байт в пакете не учитывая предыдущие два байта и байт контрольной суммы
 [3c] - количество читаемых байт
 [b8 16 ff 1f] - начальный адрес
 [cf] - контрольная сумма


 */



uint8_t myhandler(uint8_t /*nAppcmd*/, uint8_t* /*pData*/, uint16_t /*nDataLen*/);


FMSTR_TSA_TABLE_LIST_BEGIN()
FMSTR_TSA_TABLE(tbl_vars)
#ifdef DOORCTL_APP
FMSTR_TSA_TABLE(tbl_doorctl_vars)
#endif
FMSTR_TSA_TABLE_LIST_END()

T_monitor_driver *frm_serial_drv;
  
/*-------------------------------------------------------------------------------------------------------------
  Функция обратного вызова команды. Регистрируется функцией FMSTR_RegisterAppCmdCall
-------------------------------------------------------------------------------------------------------------*/
uint8_t myhandler(uint8_t code/*nAppcmd*/, uint8_t* pdata/*pData*/, uint16_t size/*nDataLen*/)
{
  return 0x10;  // Возвращать можем любой результат
}


/*-----------------------------------------------------------------------------------------------------
  Сменить источник сыжмплирования рекордера FreeMaster
 
 \param src 
-----------------------------------------------------------------------------------------------------*/
void Frmstr_Set_recorder_smpls_src(uint32_t src)
{

  switch (src)
  {
  case FMSTR_SMPS_ADC_ISR:
    g_fmstr_rate_src = FMSTR_SMPS_ADC_ISR;
    g_fmstr_smpls_period = 1.0/(float)(MOT_PWM_FREQ);
    break;
  case FMSTR_SMPS_HALL_TMR_OVERFL_ISR:
    g_fmstr_rate_src = FMSTR_SMPS_HALL_TMR_OVERFL_ISR;
    g_fmstr_smpls_period = (float)(HALL_TMR_OVERFL_PERIOD_US)/1000000.0;
    break;
  case FMSTR_SMPS_SUPERVISOR_TASK:
    g_fmstr_rate_src = FMSTR_SMPS_SUPERVISOR_TASK;
    g_fmstr_smpls_period = 1.0/(float)(_time_get_ticks_per_sec());
    break;
  default:
    g_fmstr_rate_src = FMSTR_SMPS_ADC_ISR;
    g_fmstr_smpls_period = 1.0/(float)(MOT_PWM_FREQ);
    break;
  }
}


/*-------------------------------------------------------------------------------------------------------------
  Цикл движка FreeMaster
-------------------------------------------------------------------------------------------------------------*/
void Task_FreeMaster(uint32_t initial_data)
{

  uint16_t app_command;
  uint8_t  res;

  // Получаем указатель на драйвер последовательного интерфейса  
  frm_serial_drv = (T_monitor_driver *)_task_get_parameter();

  Frmstr_Set_recorder_smpls_src(FMSTR_SMPS_ADC_ISR);

  if ( !FMSTR_Init() )
  {
    return;
  }

  // registering the App.Command handler
  // FMSTR_RegisterAppCmdCall(10, myhandler); Не регистрируем. Такие команды требуют дополнительной установки флага Wait for result в диалоге, иначе блокируют дальнейшие команды

  while (1)
  {
    app_command = FMSTR_GetAppCmd();

    if ( app_command != FMSTR_APPCMDRESULT_NOCMD )
    {
      res = U3HB01_Freemaster_Command_Manager(app_command);
      FMSTR_AppCmdAck(res);
    }
    FMSTR_Poll();
  }
}


