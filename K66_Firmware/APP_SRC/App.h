#include   "ctype.h"
#include   "stdint.h"
#include   "math.h"
#include   "limits.h"
#include   "arm_itm.h"
//#include   "cmsis_iar.h"
//#include   "MK66_CMSIS.h"
//#include   "core_cm4.h"

#include   "mqx.h"
#include   "bsp.h"
#include   "message.h"
#include   "timer.h"


#include   "K66BLEZ1_PERIPHERIAL.h"
#include   "bsp_prv.h"
#include   "message.h"
#include   "mutex.h"
#include   "sem.h"
#include   "lwmsgq.h"
#include   "lwevent_prv.h"
#include   "mfs.h"
#include   "part_mgr.h"
#include   "SEGGER_RTT.h"
#include   "Debug_io.h"
#include   "Inputs_processing.h"
#include   "Supervisor_task.h"
#include   "WatchDogCtrl.h"
#include   "RTOS_utils.h"
#include   "MonitorVT100.h"
#include   "Monitor_serial_drv.h"
#include   "Monitor_usb_drv.h"
#include   "App_logger.h"
#include   "MFS_man.h"
#include   "MFS_Shell.h"
#include   "LED_control.h"
#include   "Filters.h"

// Макросы определяются в настройках IDE
//#define    USB_1VCOM      // Конфигурация с одним виртуальным COM портом
//#define    USB_2VCOM      // Конфигурация с 2-я виртуальными COM портами

#include   "usb_device_config.h"
#include   "usb.h"
#include   "usb_device_stack_interface.h"
#include   "usb_class_cdc.h"
#include   "usb_device_descriptor.h"

#ifdef     USB_2VCOM
  #include   "usb_class_composite.h"
  #include   "usb_composite_dev.h"
  #include   "USB_Virtual_com1.h"
  #include   "USB_Virtual_com2.h"
#endif

#ifdef     USB_1VCOM
  #include   "USB_Virtual_com1.h"
#endif

#include   "USB_debug.h"


#include   "Task_FreeMaster.h"
#include   "CRC_utils.h"
#include   "CAN_IO_exchange.h"
#include   "QuadDecoder.h"
#include   "MKW40_Channel.h"

#ifdef U3HB01_APP
  #include   "U3HB01_Params.h"
  #include   "U3HB01_app/U3HB01.h"
#endif


#include   "Parameters.h"


#define PARAMS_FILE_NAME DISK_NAME"Params.ini"


#define USB_VT100  // Определяем если терминал работает через USB

//#define ENABLE_CAN_LOG  Определить если нужен LOG CAN
//#define ENABLE_ACCESS_CONTROL  Определить если нужен контроль доступа



#define ADC_PERIOD          500  // Период выборки цифро-анлогового преобразователя (ADC) в мкс



#define BIT(n) (1u << n)
#define LSHIFT(v,n) (((unsigned int)(v) << n))

#define MQX_MFS   // Пределить если используется MQX MFS
#define MQX_SHELL // Определить если используется MQX SHELL в терминале VT100
#define MFS_TEST  // Определить если компилируется процедура тестирования файловой системы MFS


#define MAIN_TASK_IDX           1
#define CAN_TX_IDX              2
#define CAN_RX_IDX              3
#define MKW40_IDX               4
#define VT100_IDX               5
#define FREEMASTER_IDX          6
#define USB_IDX                 7
#define SUPERVISOR_IDX          8
#define SHELL_IDX               9
#define FILELOG_IDX             10
#define BACKGR_IDX              11


// Установка приоритетов задач
// Чем больше число тем меньше приоритет.
#define MAIN_TASK_PRIO          8
#define CAN_RX_ID_PRIO          9
#define CAN_TX_ID_PRIO          9
#define MKW_ID_PRIO             9
#define FREEMASTER_ID_PRIO      10
#define SUPRVIS_ID_PRIO         10
#define VT100_ID_PRIO           11
#define SHELL_ID_PRIO           12
#define FILELOG_ID_PRIO         13 // Приоритет задачи записи лога в файл
#define TIMERS_ID_PRIO          7
#define BACKGR_ID_PRIO          100


#define MAX_MQX_PRIO        BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX

// Приоритеты для ISR
// Чем меньше число тем выше приоритет ISR
// ISR с более высоким приоритетом вытесняют ISR с более низким приоритетом
// Приоритет ISR использующий сервисы RTOS не может быть меньше значения BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX!!!
#define WDT_ISR_PRIO        MAX_MQX_PRIO      //
#define FTM3_ISR_PRIO       MAX_MQX_PRIO      // Приоритет прерываний модулятора PWM мотора
#define FTM1_ISR_PRIO       MAX_MQX_PRIO + 1  // Приоритет прерываний квадратичного энкодера
#define CAN_ISR_PRIO        MAX_MQX_PRIO + 1  // Приоритет процедуры прерывания от контроллера CAN шины

#define SPI0_PRIO           MAX_MQX_PRIO + 1
#define SPI1_PRIO           MAX_MQX_PRIO + 1
#define SPI2_PRIO           MAX_MQX_PRIO + 1
#define PORTB_PRIO          MAX_MQX_PRIO + 1

// Проритеты прерываний уровня ядра
// Чем меньше число тем выше приоритет ISR
#define DMA_ADC_PRIO        MAX_MQX_PRIO-2     // Проритет прерывания DMA после обработки всех каналов ADC
#define ADC_PRIO            MAX_MQX_PRIO-2     // Проритет прерывания ADC
#define FTM2_PRIO           MAX_MQX_PRIO-1     // Приоритет прерываний измерителя скорости с hall сенсоров




#define REF_TIME_INTERVAL   5  // Интервал времени в милисекундах на котором производлится калибровка и измерение загруженности процессора

#define CAN_TX_QUEUE        8  // Уникальный номер очереди на данном процессоре. Нельзя устанавливать меньше 8-и. Согласовать с номерами в приложении



//#define dbg_printf RTT_printf
#define dbg_printf printf

// Определения для файловой системы
#define    PARTMAN_NAME   "pm:"
#define    DISK_NAME      "a:"
#define    PARTITION_NAME "pm:1"

// Определения статуса записей для логгера приложения
#define   SEVERITY_RED             0
#define   SEVERITY_GREEN           1
#define   SEVERITY_YELLOW          2
#define   SEVERITY_DEFAULT         4



#ifdef _MAIN_GLOBALS_

WVAR_TYPE                  wvar;

uint32_t                   ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
volatile uint32_t          cpu_usage;            // Процент загрузки процессора

#else

extern WVAR_TYPE           wvar;

extern uint32_t            ref_time;
extern volatile uint32_t   cpu_usage;


#endif

#define  DELAY_1us       Delay_m7(25)           // 1.011     мкс при частоте 180 МГц
#define  DELAY_4us       Delay_m7(102)          // 4.005     мкс при частоте 180 МГц
#define  DELAY_8us       Delay_m7(205)          // 8.011     мкс при частоте 180 МГц
#define  DELAY_32us      Delay_m7(822)          // 32.005    мкс при частоте 180 МГц
#define  DELAY_ms(x)     Delay_m7(25714*x-1)    // 999.95*N  мкс при частоте 180 МГц

extern void Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля недопускается


_mqx_int  TimeManInit(void);
void      Get_time_counters(HWTIMER_TIME_STRUCT *t);
uint32_t  Eval_meas_time(HWTIMER_TIME_STRUCT t1, HWTIMER_TIME_STRUCT t2);
uint32_t  Get_usage_time(void);
