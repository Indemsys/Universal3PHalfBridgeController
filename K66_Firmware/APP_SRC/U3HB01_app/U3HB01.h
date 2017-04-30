#ifndef MDC01__H
#define MDC01__H


#define SB200AD49K   1
#define RB150AD49K   2
#define SB200AD92K   3 
#define RB150AD92K   4
#define SB200AD92KV  5 

#define APPLICATION_TASK   U3HB01_Task_selector
#define CAN_RX_PROC        U3HB01_CAN_rx
#define CAN_TX_PROC        U3HB01_CAN_tx


#define RES_OK     MQX_OK
#define RES_ERROR  MQX_ERROR

#define APP_NO_ZERO_POINT         BIT(0)  // В системе не установлена нудевая точка 
#define APP_ERR_NO_DRV8305        BIT(1)  // Ошибка. Нет связи с дпайвером DRV8305
#define APP_ERR_DRV8305_FAULT     BIT(2)  // Ошибка. Сигнал Fault в драйвере DRV8305
#define APP_ERR_HALL_STATE_FAULT  BIT(3)  // Ошибка. 


#define FMSTR_SMPS_ADC_ISR                 0       // Источником сэмплов для рекордера FreeMaster является процедура прерывания ADC
#define FMSTR_SMPS_HALL_TMR_OVERFL_ISR     1       // Источником сэмплов для рекордера FreeMaster является процедура прерывания по переполнению таймера датчиков Холла
#define FMSTR_SMPS_SUPERVISOR_TASK         2       // Источником сэмплов для рекордера FreeMaster является задача супервизора


#include   "U3HB01_main.h"
#include   "U3HB01_DRV8305.h"
#include   "U3HB01_monitor.h"
#include   "U3HB01_WS2812B.h"
#include   "U3HB01_CAN.h"
#include   "U3HB01_CAN_BUS_IDs.h"
#include   "U3HB01_BLDC.h"
#include   "U3HB01_Pins.h"
#include   "U3HB01_Hall_sensor.h"
#include   "U3HB01_sensless_BLDC.h"
#include   "U3HB01_Hall_BLDC.h"
#include   "U3HB01_FreemasterCmdMan.h"
#include   "PIDController.h"
#include   "U3HB01_PIDcontrol.h"
#include   "U3HB01_SolenoidControl.h"
#include   "U3HB01_RotationControl.h"
#include   "U3HB01_SPISysBus.h"
#include   "U3HB01_ExtEncoder.h"

#ifdef DOORCTL_APP
#include   "U3HB01_DoorControl.h"
#endif

#ifdef _APP_GLOBALS_

uint32_t                           g_app_errors;
uint32_t                           g_bldc_active;       // Флаг включенного управления двигателем 
uint32_t                           g_bldc_forcebrake;   // Флаг форсированного торможения. Коммутация обмоток не выполняется  
TIME_STRUCT                        g_calibr_time;       // Время последней калибровки датчиков тока                   
                                     
T_BLDC_async_ctrl_params           bldca_cbl;    
T_BLDC_sync_ctrl_params            bldcs_cbl;     
T_hall_capture_cbl                 hccbl;                
T_overcur_observer                 ovrc_cbl;             // Структура наблюдателя за перегрузкой по току                                             
T_rotation                         rot_cbl;              // Структура задачи вращения  
                                          
uint32_t                           hall_inputs;          // Состояние сигналов Hall сенсора для отдадочного вывода во FreeMaster  
int16_t                            mot_PWM_lev;          // Уровень модуляции PWM сигнала мотора для отдадочного вывода во FreeMaster               
                                                         
uint32_t                           sns_ia_calibr;        // Калибровочные константы каналов измерения тока чипа DRV8305
uint32_t                           sns_ib_calibr;        //
uint32_t                           sns_ic_calibr;        //
                                                         
float                              current_a;            // Ток в фазе A 
float                              current_b;            // Ток в фазе B 
float                              current_c;            // Ток в фазе C 
float                              dc_current;           // 
float                              int_temp;             // Температура CPU
float                              drv_temp;             // Температура силового модуля
float                              motor_temp;           // Температура мотора 
float                              dc_voltage;           // Напряжени питания
float                              mot_speed_rps;        // Скорость вращения мотора в обротах в секунду  
float                              mot_speed_rps_raw;    // Скорость вращения мотора в обротах в секунду  без фильтрации
                                                         
uint32_t                           g_fmstr_rate_src;     // Переменная управляющая источником сэмплов для рекордера FreeMaster 
float                              g_fmstr_smpls_period; // Период сэмплирования рекордера FreeMaster в секундах  

float                              ext_enc_angle;
uint32_t                           ext_enc_errors;


#else                              
                                   
extern uint32_t                    g_app_errors;
extern uint32_t                    g_bldc_active;
extern uint32_t                    g_bldc_forcebrake;   // Флаг форсированного торможения. Коммутация обмоток не выполняется  
extern TIME_STRUCT                 g_calibr_time;         // Время последней калибровки датчиков тока                   


extern T_BLDC_async_ctrl_params    bldca_cbl;    
extern T_BLDC_sync_ctrl_params     bldcs_cbl;     
extern T_hall_capture_cbl          hccbl;                
extern T_overcur_observer          ovrc_cbl;             // Структура наблюдателя за перегрузкой по току                                             
extern T_rotation                  rot_cbl;              // Структура задачи вращения  
                                                         

extern uint32_t                    hall_inputs;          
extern int16_t                     mot_PWM_lev;                      
                                                         
extern uint32_t                    sns_ia_calibr;        // Калибровочные константы каналов измерения тока чипа DRV8305
extern uint32_t                    sns_ib_calibr;        //
extern uint32_t                    sns_ic_calibr;        //
                                                         
extern float                       current_a;            // Ток в фазе A 
extern float                       current_b;            // Ток в фазе B 
extern float                       current_c;            // Ток в фазе C 
extern float                       dc_current;           // 
extern float                       int_temp;             // Температура CPU
extern float                       drv_temp;             // Температура силового модуля
extern float                       motor_temp;           // Температура мотора 
extern float                       dc_voltage;           // Напряжени питания
extern float                       mot_speed_rps;        // Скорость вращения мотора в обротах в секунду  
extern float                       mot_speed_rps_raw;    // Скорость вращения мотора в обротах в секунду  без фильтрации
                                                         
                                                         
extern uint32_t                    g_fmstr_rate_src;     // Переменная управляющая источником сэмплов для рекордера FreeMaster 
extern float                       g_fmstr_smpls_period; // Период сэмплирования рекордера FreeMaster в секундах  

extern float                       ext_enc_angle;
extern uint32_t                    ext_enc_errors;


#endif

#endif // MDC01__H



