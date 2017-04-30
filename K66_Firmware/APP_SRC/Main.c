#define _MAIN_GLOBALS_
#include "App.h"


void Main_task(unsigned int initial_data);



const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
  /* Task Index,     Function,              Stack,  Priority,                   Name,        Attributes,                                                          Param, Time Slice */
  { MAIN_TASK_IDX,      Main_task,          2000,   MAIN_TASK_PRIO,            "Main",       /*MQX_FLOATING_POINT_TASK + */ MQX_AUTO_START_TASK,                       0,     0 },
  { VT100_IDX,          Task_VT100,         2000,   VT100_ID_PRIO,             "VT100",      /*MQX_FLOATING_POINT_TASK + */ MQX_TIME_SLICE_TASK,                       0,     2 },
  { CAN_TX_IDX,         Task_CAN_Tx,        500,    CAN_TX_ID_PRIO,            "CAN_TX",     0,                                                                   0,     0 },
  { CAN_RX_IDX,         Task_CAN_Rx,        500,    CAN_RX_ID_PRIO,            "CAN_RX",     0,                                                                   0,     0 },
  { MKW40_IDX,          Task_MKW40,         1000,   MKW_ID_PRIO,               "MKW40",      MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { FILELOG_IDX,        Task_file_log,      1500,   FILELOG_ID_PRIO,           "FileLog",    MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { SHELL_IDX,          Task_shell,         2000,   SHELL_ID_PRIO,             "Shell",      MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { FREEMASTER_IDX,     Task_FreeMaster,    2000,   FREEMASTER_ID_PRIO,        "FreeMaster", MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { SUPERVISOR_IDX,     Task_supervisor,    500,    SUPRVIS_ID_PRIO,           "SUPRVIS",    MQX_TIME_SLICE_TASK,                                                 0,     2 },
  { BACKGR_IDX,         Task_background,    1000,   BACKGR_ID_PRIO,            "BACKGR",     /*MQX_FLOATING_POINT_TASK +*/ 0,                                             0,     0 },
  { 0 }
};

volatile uint32_t tdelay, tmin, tmax;

extern void PID_controller_subsystem_step(void);

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Main_task(unsigned int initial_data)
{
  unsigned int   cycle_cnt;
  uint32_t       rtc_time;
  TIME_STRUCT    mqx_time;

  // Устанавливаем время операционной системы
  _rtc_get_time(&rtc_time);
  mqx_time.SECONDS = rtc_time;
  mqx_time.MILLISECONDS = 0;
  _time_set(&mqx_time); // Установка времени RTOS

  Set_LED_pattern(LED_BLINK, 0);

  AppLogg_init();
  Write_start_log_rec();
  Get_reference_time();

  ADC_calibr_config_start();    // Инициализируем работу многоканального АЦП

  #ifdef MQX_MFS
  if (Init_mfs() == MQX_OK)
  {
    _task_create(0, FILELOG_IDX, 0);
  }
  #endif

  Restore_parameters_and_settings();


  _task_create(0, BACKGR_IDX, 0);                  // Фоновая задача. Измеряет загрузку процессора
  _task_create(0, SUPERVISOR_IDX, 0);              // Задача сброса watchdog и наблюдения за работоспособностью системы
  WatchDog_init(WATCHDOG_TIMEOUT, WATCHDOG_WIN);   // Инициализируем Watchdog

  _task_create(0, MKW40_IDX, 0);

  #ifdef USB_2VCOM
  Composite_USB_device_init();
  #elif USB_1VCOM
  Init_USB();
  #endif

  #ifdef USB_VT100
  _task_create(0, VT100_IDX, (uint32_t)Mnudrv_get_USB_vcom_driver1());       // Создаем задачу VT100_task с дайвером  интерфейса и делаем ее активной
  #else
  _task_create(0, VT100_IDX, (uint32_t)Mnsdrv_get_ser_std_driver());         // Создаем задачу VT100_task с дайвером  интерфейса и делаем ее активной
  #endif

  _task_create(0, FREEMASTER_IDX, (uint32_t)Mnudrv_get_USB_vcom_driver2());  // Создаем задачу FreeMaster с дайвером  интерфейса и делаем ее активной


  APPLICATION_TASK();
}


