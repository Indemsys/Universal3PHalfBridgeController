#ifndef U3HB01__APP_IDS
  #define U3HB01__APP_IDS


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой платформы BA100
//******************************************************************************************************************************************************
#define PLATFLR0_BTPSH_ID                0x0101
#define PLATFLR0_BTRLS_ID                0x0201
#define PLATFLR1_BTPSH_ID                0x0102
#define PLATFLR1_BTRLS_ID                0x0202
#define PLATFLR2_BTPSH_ID                0x0103
#define PLATFLR2_BTRLS_ID                0x0203
#define PLATFLR3_BTPSH_ID                0x0104
#define PLATFLR3_BTRLS_ID                0x0204
#define PLATFLR4_BTPSH_ID                0x0105
#define PLATFLR4_BTRLS_ID                0x0205
#define PLATFLR5_BTPSH_ID                0x0106
#define PLATFLR5_BTRLS_ID                0x0206
#define PLATFORM_BELL_BTPSH_ID           0x0110
#define PLATFORM_BELL_BTRLS_ID           0x0210
#define PLATFORM_STOP_BTPSH_ID           0x0111
#define PLATFORM_STOP_BTRLS_ID           0x0211

// Идентификаторы входных сигналов для платформы
#define PLATFORM_PLAY_SOUND_ID           0x0500 // Команда на проигрывание файла
                                                // data[0] - номер файла

#define PLATFORM_SET_LED_STATE_ID        0x0501 // Управление состоянием светодиодов на платформе
                                                // data[0] - тип светодиода
                                                // data[1] - номер светодиода
                                                // data[2] - состояние
                                                // data[3] - цвет

#define PLATFORM_EMERG_LIGHT_CTRL        0x0502 // Команда управления аварийной подсветкой
                                                // data[0] - состояние подсветки (1 - включен, 0 - выключено)


#define PLATFORM_SEND_MSG_TO_SPORT       0x503  //
                                                // data[0] - hi byte of cmd num
                                                // data[1] - lo byte of cmd num

// Идентификаторы выходных сигналов из платформы
#define PLATFORM_SIG_STATE_ID           0x0600 // Сообщение о состоянии сигналовв цепочке Safety circuit
                                                // data[0].0 - Upper emergency limit switch
                                                // data[0].1

                                                // data[0].2 - Safety gear
                                                // data[0].3

                                                // data[0].4 - Safety bar 4
                                                // data[0].5

                                                // data[0].6 - Safety bar 3
                                                // data[0].7

                                                // data[1].0 - Safety bar 2
                                                // data[1].1

                                                // data[1].2 - Safety bar 1
                                                // data[1].3

                                                // data[1].4 - Emergency stop button
                                                // data[1].5

#define PLATF_UPGRADE_RX_ID              0x0100FFFF  // Идентификатор пакетов от платы платформы BA100  к программирующему устройству
#define PLATF_UPGRADE_TX_ID              0x0101FFFF  // Идентификатор пакетов от программирующего устройства к плате платформы BA100

//******************************************************************************************************************************************************
// Идентификаторы для работы с платой FrontEnd (обслуживание  пульта и датчиков периметра на платформе )системы SB200M
//******************************************************************************************************************************************************
#define SB200MFE_ESC_EVENT_ID            0x0010FFFF  // Отсылка платой события изменеия состояния цепи безопасност
                                                     // data[0] - Битовая маска состояния сигналов
                                                     // data[1] - Битовая маска состояния сигналов
                                                     #define FE_ESC1  8// BIT(8) - ESC1 log.state
                                                     #define FE_ESC2  7// BIT(7) - ESC2 log.state
                                                     #define FE_ESC3  6// BIT(6) - ESC3 log.state
                                                     #define FE_ESC4  5// BIT(5) - ESC4 log.state
                                                     #define FE_ESC5  4// BIT(4) - ESC5 log.state
                                                     #define FE_ESC6  3// BIT(3) - ESC6 log.state
                                                     #define FE_ESC7  2// BIT(2) - ESC7 log.state
                                                     #define FE_ESC8  1// BIT(1) - ESC8 log.state
                                                     #define FE_ESC9  0// BIT(0) - ESC9 log.state

#define SB200MFE_BTN_EVENT_ID            0x0011FFFF  // Отсылка платой события кнопок
                                                     // data[0..3] - Идетификатор события
                                                     #define  PRESS_PLATFORM_BUTTON       0x00010000 // +n  n - номер этажа
                                                     #define  RELEASE_PLATFORM_BUTTON     0x00020000 // +n
                                                     #define  LONG_PRESS_PLATFORM_BUTTON  0x00060000 // +n
                                                     #define  PRESS_BELL_BUTTON           0x00070000
                                                     #define  RELEASE_BELL_BUTTON         0x00080000
                                                     #define  PRESS_STOP_BUTTON           0x00090000
                                                     #define  RELEASE_STOP_BUTTON         0x000A0000
                                                     #define  PRESS_FLOOR_BUTTON          0x000B0000
                                                     #define  RELEASE_FLOOR_BUTTON        0x000C0000
                                                     #define  ORDER_TO_MOVE_BUTTON        0x000D0000 // Сигнал безусловного движения, используется при автопаркинге
                                                     #define  IBUTTON_CODE                0x000E0000 // Сообщение о приеме кода iButton от считывателя


#define SB200MFE_LEDS_GET_ID             0x0012FFFF  // Сообщение плате с запросом состояния сигналов светодиодов
                                                     // Плата отсылает с тем же идентификатором данные
                                                     // data[0]..[1] - Битовая маска состояний типа unsigned short
                                                       #define FE_LED_STOP      0  //
                                                       #define FE_LED_OVERLOAD  1  //
                                                       #define FE_COLR_RED      2  //
                                                       #define FE_COLR_GREEN    3  //
                                                       #define FE_COLR_BLUE     4  //
                                                       #define FE_LED_BTN0      5  //
                                                       #define FE_LED_BTN1      6  //
                                                       #define FE_LED_BTN2      7  //
                                                       #define FE_LED_BTN3      8  //
                                                       #define FE_LED_BTN4      9  //
                                                       #define FE_LED_BTN5      10 //

#define SB200MFE_LEDS_SET_ID             0x0013FFFF  // Сообщение плате для установки состояний светодиодов
                                                     // data[0]..[1] - кодовое слово типа unsigned short
                                                     // Если бит 15 = 1, то кодовое слово представляет маску установки состояния всех сигналов светодиодов
                                                     // аналогичную маске в команде SB200MFE_LEDS_GET_ID
                                                     // Если бит 15 = 0, то в бите 14 содержится состояние сигнала, а в битах 3..0 его номер в соответствии с
                                                     // номерами в команде SB200MFE_LEDS_GET_ID



#define SB200MFE_VAL_GET_ID              0x0014FFFF  // Сообщение плате с запросом значения переменной
                                                     // data[0] - Идетификатор переменной
                                                     //  0 -  температура CPU (float)
                                                     //  1 -  ESC1_v (float)
                                                     //  2 -  ESC2_v (float)
                                                     //  3 -  ESC3_v (float)
                                                     //  4 -  ESC4_v (float)
                                                     //  5 -  ESC5_v (float)
                                                     //  6 -  ESC6_v (float)
                                                     //  7 -  ESC7_v (float)
                                                     //  8 -  ESC8_v (float)
                                                     //  9 -  ESC9_v (float)
                                                     // Плата отсылает данные с тем же идентификатором данные

#define SB200MFE_VAL_SET_ID              0x0015FFFF  // Сообщение плате установить значение переменной
                                                     // data[0] - Идетификатор переменной
                                                     //  0 -  яркость светодиодной подсветки
                                                     //  1 -  Включение (data[1] = 1) и выключение (data[1] = 0) светодиодной подсветки
                                                     #define FE_LIGHT_INTENS      0 // Яркость светодиодной подсветки
                                                     #define FE_LIGHT_CTRL        1 // Включение (data[1] = 1) и выключение (data[1] = 0) светодиодной подсветки
                                                     #define FE_OVERLOAD_CTRL     2 // Включение (data[1] = 1) и выключение (data[1] = 0) режима перегрузки


#define SB200MFE_REQ_BTN_EVENT_ID        0x0016FFFF  // Запрос на повторную отсылку сотояния кнопок на пульте

#define SB200MFE_PLAYER_CMD_ID           0x0017FFFF  // Команда проигрывателю информационных сообщений
                                                     // data[0] - Тип команды
                                                     #define FE_PLR_CMD_SETT      0 // Настройка параметров проигрывателя
                                                        // data[1] - Номер параметра. data[2..5] - значение параметра
                                                        #define FEPLCMD_ARRIVAL_GONG          0 // Установка значения переменной app_vars.en_arrival_gong
                                                        #define FEPLCMD_FLOORS_ANNOUNCER      1 // Установка значения переменной app_vars.en_floors_announcer
                                                        #define FEPLCMD_EN_VOICE_ANNOUNCER    2 // Установка значения переменной app_vars.en_voice_announcer
                                                        #define FEPLCMD_VOICE_ATTENUATION     3 // Установка значения переменной app_vars.voice_attenuation
                                                        #define FEPLCMD_FLOOR_NUM_OFFSET      4 // Установка значения переменной app_vars.floor_num_offset
                                                     #define FE_PLR_CMD_PLAY      1 // Команда на проигрывание определенного сообщения
                                                        // data[1] - номер сообщения, data[2..5] - аргумент сообщения
                                                        #define FEPLCMD_MSG_ERROR_            0 //
                                                        #define FEPLCMD_MSG_EMERGENCY_STOP    1 //
                                                        #define FEPLCMD_MSG_SENSOR_NUMBER     2 //
                                                        #define FEPLCMD_MSG_OVERLOAD          3 //
                                                        #define FEPLCMD_MSG_OPENED_LOCK       4 //
                                                        #define FEPLCMD_MSG_SERVICE_MODE      5 //
                                                        #define FEPLCMD_MSG_WORK_MODE         6 //
                                                        #define FEPLCMD_MSG_CALIBRATING_DONE  7 //
                                                        #define FEPLCMD_MSG_FIRE_ALARM        8 //
                                                        #define FEPLCMD_ARRIVAL_SND           9 //

#define SB200FE_UPGRADE_RX_ID            0x001CFFFF  // Идентификатор пакетов от платы FrontEnd  к программирующему устройству
#define SB200FE_UPGRADE_TX_ID            0x001DFFFF  // Идентификатор пакетов от программирующего устройства к плате FrontEnd

#define SB200MFE_HEARTBEAT               0x001FFFFF  // Периодическое сообщение от платы FrontEnf о ее работоспособности


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой BackEnd  системы SB200M
//******************************************************************************************************************************************************
#define SB200BE_UPGRADE_RX_ID            0x001EFFFF  // Идентификатор пакетов от платы BackEnd  к программирующему устройству
#define SB200BE_UPGRADE_TX_ID            0x001FFFFF  // Идентификатор пакетов от программирующего устройства к плате BackEnd


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой BaseUnit
//******************************************************************************************************************************************************
#define SB200BU_ESC_EVENT_ID             0x0020FFFF  // Отсылка платой события изменеия состояния цепи безопасности
                                                     // data[0]..[1] - Битовая маска состояния сигналов типа unsigned short
                                                     // Пересылка содержимого переменной ESC_state
                                                     // BIT(5) - ESC1
                                                     // BIT(4) - ESC2
                                                     // BIT(3) - ESC3
                                                     // BIT(2) - ESC4
                                                     // BIT(1) - ESC5
                                                     // BIT(0) - ESC6

#define SB200BU_BTN_EVENT_ID             0x0021FFFF  // Отсылка платой события кнопок
                                                     // data[0] - Битовые флаги событий
                                                     #define  PRESS_BT1_AUTO                 BIT(0)
                                                     #define  RELEASE_BT1_AUTO               BIT(1)
                                                     #define  PRESS_BT2_UP                   BIT(2)
                                                     #define  RELEASE_BT2_UP                 BIT(3)
                                                     #define  PRESS_BT3_MANUAL               BIT(4)
                                                     #define  RELEASE_BT3_MANUAL             BIT(5)
                                                     #define  PRESS_BT4_DOWN                 BIT(6)
                                                     #define  RELEASE_BT4_DOWN               BIT(7)
#define SB200BU_SET_LED_STATE            0x0022FFFF  // Команда установки состояния светодиодов кнопки
                                                     // data[0..1] - Битовые флаги установки состояний светодиодов
                                                     #define  LED_BT1_AUTO_STATE             BIT(0) // Установка состояния: 0 - выключен, 1- включен
                                                     #define  LED_BT1_AUTO_MASK              BIT(1) // Маска установки состояния: 0 - состояние игнорируется, 1- состояние изменяется согласно установке
                                                     #define  LED_BT2_UP_STATE               BIT(2)
                                                     #define  LED_BT2_UP_MASK                BIT(3)
                                                     #define  LED_BT3_MANUAL_STATE           BIT(4)
                                                     #define  LED_BT3_MANUAL_MASK            BIT(5)
                                                     #define  LED_BT4_DOWN_STATE             BIT(6)
                                                     #define  LED_BT4_DOWN_MASK              BIT(7)


#define SB200BU_VAL_GET_ID               0x0023FFFF  // Сообщение плате BaseUnit с запросом значения переменной
                                                     // data[0] - Идетификатор переменной
                                                     #define  CPU_TEMP_VID 0  //   температура CPU (float)
                                                     #define  ESC1_VID     1  //   ESC1_v (float)
                                                     #define  ESC2_VID     2  //   ESC2_v (float)
                                                     #define  ESC3_VID     3  //   ESC3_v (float)
                                                     #define  ESC4_VID     4  //   ESC4_v (float)
                                                     #define  ESC5_VID     5  //   ESC5_v (float)
                                                     #define  ESC6_VID     6  //   ESC6_v (float)
                                                     #define  V5V_VID      7  //   V5V_v  (float)
                                                     #define  V10V_VID     8  //   V10V_v (float)
                                                     #define  V24V_VID     9  //   V24V_v (float)
                                                     #define  VACC_VID     10 //   VACC_v (float)
                                                     #define  IACC_VID     11 //   IACC_v (float)
                                                     #define  EXALR_VID    12 //   EXALR_v(float)
                                                     // Плата отсылает данные с тем же идентификатором данные

#define SB200BU_SET_OUTPUT_STATE         0x0024FFFF  // Сообщение плате BaseUnit с командой на установку состояня выходного сигнала
                                                     // data[0..1] - Битовые флаги установки состояний выходных сигналов
                                                     #define  OUT_LIGHTING_STATE             BIT(0) // Установка состояния: 0 - выключен, 1- включен
                                                     #define  OUT_LIGHTING_MASK              BIT(1) // Маска установки состояния: 0 - состояние игнорируется, 1- состояние изменяется согласно установке
                                                     #define  OUT_HEATING_STATE              BIT(2)
                                                     #define  OUT_HEATING_MASK               BIT(3)
                                                     #define  OUT_MISC1_STATE                BIT(4)
                                                     #define  OUT_MISC1_MASK                 BIT(5)
                                                     #define  OUT_MISC2_STATE                BIT(6)
                                                     #define  OUT_MISC2_MASK                 BIT(7)
                                                     #define  OUT_EMLW_DIR_STATE             BIT(8)
                                                     #define  OUT_EMLW_DIR_MASK              BIT(9)
                                                     #define  OUT_EMLW_MODE_STATE            BIT(10)
                                                     #define  OUT_EMLW_MODE_MASK             BIT(11)
                                                     #define  OUT_ACC_ACT_STATE              BIT(12) // Шунтирование диода в цепи подключения аккумулятора
                                                     #define  OUT_ACC_ACT_MASK               BIT(13)
                                                     #define  OUT_ACC_ON_STATE               BIT(14)
                                                     #define  OUT_ACC_ON_MASK                BIT(15)

#define SB200BU_SET_DIAGN_STRIP_STATE    0x0025FFFF  // Сообщение плате BaseUnit о состянии сигналов диагностической полосы
                                                     // data[0] - Тип диагностического сообщения
                                                     #define  DIAGNM_LANDING 0x80 // Идентификатор сообщение о состоянии остановки. В младших битах номер остановки - 0..5
                                                     #define  BACKEND_DMSG   0x40 // Диагностическое сообщение от BackEnd платы
                                                        // Позиции информационных битов в байте data[1] сообщения BACKEND_DMSG
                                                        #define BEDMSG_ESC               BIT(0)
                                                        #define BEDMSG_UP_EM_LIM_SW      BIT(1)
                                                        #define BEDMSG_LW_EM_LIM_SW      BIT(2)
                                                        #define BEDMSG_MOT_OVERHEAT      BIT(3)
                                                        #define BEDMSG_REFERENCE_SW      BIT(4)
                                                        #define BEDMSG_WEAR_SW           BIT(5)
                                                        #define BEDMSG_UP_LIM_SW         BIT(6)
                                                        #define BEDMSG_LOST_FRNTND_CAN   BIT(7) // Потеря связи BackEnd с FrontEnd
                                                        // Позиции информационных битов в байте data[2] сообщения BACKEND_DMSG
                                                        #define BEDMSG_LOST_INVERT_CAN   BIT(0) // Потеря связи BackEnd с Inverter
                                                        #define BEDMSG_SOLENOID1_ERR     BIT(1)
                                                        #define BEDMSG_SOLENOID2_ERR     BIT(2)
                                                        #define BEDMSG_SOLENOID1_LIMERR  BIT(3)
                                                        #define BEDMSG_SOLENOID2_LIMERR  BIT(4)
                                                        #define BEDMSG_SPEED_ERROR       BIT(5)

                                                     #define  FRONTEND_DMSG  0x20 // Диагностическое сообщение от FrontEnd платы


                                                     #define  GENERAL_DMSG   0x10 // Общесистемные сообщения
                                                        #define GENMSG_CRITMOTTEMP     BIT(0)  // Критическая температура мотора
                                                        #define GENMSG_PHASEFAULT      BIT(1)  // Ошибка сетевого напряжени
                                                        #define GENMSG_BACKUPSTATE     BIT(2)  // Режим резервного питания
                                                        #define GENMSG_CALIBRATING     BIT(4)  // Режим калибровки
                                                        #define GENMSG_OVERLOAD        BIT(6)  // Режим перегрузки
                                                        #define GENMSG_FIREALARM       BIT(7)  // Пожарный режим
                                                        // Позиции информационных битов в байте data[2] сообщения BACKEND_DMSG
                                                        #define GENMSG_BLOCKMODE       BIT(0)  // Режим блокировки
                                                        #define GENMSG_SERVMODE        BIT(1)  // Сервисный режим
                                                        #define GENMSG_NORMMODE        BIT(2)  // Рабочий режим

#define SB200BU_PWR_EVENT_ID             0x0026FFFF  // Отсылка платой события подсистемы управления питанием
                                                     // data[0] - Битовые флаги событий
                                                     #define  PREPARE_TO_POWER_OFF           BIT(0) // Напряжение упало ниже критического, через 1 сек питание будет отключено
                                                     #define  POWER_FROM_ACCUM               BIT(1) // Система питается от аккумулятора
                                                     #define  POWER_FROM_MAIN                BIT(2) // Система питается от сети

#define SB200BU_FIREAL_EVENT_ID          0x0027FFFF  // Отсылка платой события изменения состояния входа FireAlarm
                                                     // data[0] - BIT(0) - состояние входа

#define SB200BU_RAINBOW_EVENT_ID         0x0028FFFF  // Команда плате управления режимом Rainbow
                                                     // data[0] = 1 - включить, data[0] = 0 - выключить

#define SB200BU_HEARTBEAT                0x002FFFFF  // Периодическое сообщение от платы BaseUnit о ее работоспособности




//******************************************************************************************************************************************************
// Идентификаторы для работы с платами остановок
//******************************************************************************************************************************************************
#define FLOOR0_ONBUS_MSG                 0x0C00FFFF  //
#define FLOOR0_ERR_ACK_MSG               0x0C04FFFF  //
#define FLOOR0_DATA_MSG                  0x0C08FFFF  //
#define FLOOR0_INF_REQ                   0x1040FFFF  //
#define FLOOR0_INP_MSG                   0x1080FFFF  //
#define FLOOR0_UPGRADE_RX_ID             0x1081FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR0_UPGRADE_TX_ID             0x1082FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR1_ONBUS_MSG                 0x0C40FFFF  //
#define FLOOR1_ERR_ACK_MSG               0x0C44FFFF  //
#define FLOOR1_DATA_MSG                  0x0C48FFFF  //
#define FLOOR1_INF_REQ                   0x10C0FFFF  //
#define FLOOR1_INP_MSG                   0x1100FFFF  //
#define FLOOR1_UPGRADE_RX_ID             0x1101FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR1_UPGRADE_TX_ID             0x1102FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR2_ONBUS_MSG                 0x0C80FFFF  //
#define FLOOR2_ERR_ACK_MSG               0x0C84FFFF  //
#define FLOOR2_DATA_MSG                  0x0C88FFFF  //
#define FLOOR2_INF_REQ                   0x1140FFFF  //
#define FLOOR2_INP_MSG                   0x1180FFFF  //
#define FLOOR2_UPGRADE_RX_ID             0x1181FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR2_UPGRADE_TX_ID             0x1182FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR3_ONBUS_MSG                 0x0CC0FFFF  //
#define FLOOR3_ERR_ACK_MSG               0x0CC4FFFF  //
#define FLOOR3_DATA_MSG                  0x0CC8FFFF  //
#define FLOOR3_INF_REQ                   0x11A0FFFF  //
#define FLOOR3_INP_MSG                   0x1200FFFF  //
#define FLOOR3_UPGRADE_RX_ID             0x1201FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR3_UPGRADE_TX_ID             0x1202FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR4_ONBUS_MSG                 0x0D00FFFF  //
#define FLOOR4_ERR_ACK_MSG               0x0D04FFFF  //
#define FLOOR4_DATA_MSG                  0x0D08FFFF  //
#define FLOOR4_INF_REQ                   0x1240FFFF  //
#define FLOOR4_INP_MSG                   0x1280FFFF  //
#define FLOOR4_UPGRADE_RX_ID             0x1281FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR4_UPGRADE_TX_ID             0x1282FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR5_ONBUS_MSG                 0x0D40FFFF  //
#define FLOOR5_ERR_ACK_MSG               0x0D44FFFF  //
#define FLOOR5_DATA_MSG                  0x0D48FFFF  //
#define FLOOR5_INF_REQ                   0x12C0FFFF  //
#define FLOOR5_INP_MSG                   0x1300FFFF  //
#define FLOOR5_UPGRADE_RX_ID             0x1301FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR5_UPGRADE_TX_ID             0x1302FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей

#define FLOOR6_ONBUS_MSG                 0x1303FFFF  //
#define FLOOR6_ERR_ACK_MSG               0x1304FFFF  //
#define FLOOR6_DATA_MSG                  0x1305FFFF  //
#define FLOOR6_INF_REQ                   0x1306FFFF  //
#define FLOOR6_INP_MSG                   0x1307FFFF  //
#define FLOOR6_UPGRADE_RX_ID             0x1308FFFF  // Идентификатор пакетов от контроллера дверей  к программирующему устройству
#define FLOOR6_UPGRADE_TX_ID             0x1309FFFF  // Идентификатор пакетов от программирующего устройства к контроллеру дверей


//******************************************************************************************************************************************************
//  Частотный преобразователь SB200
//******************************************************************************************************************************************************
#define INVERT_ONBUS_MSG                 0x1BF0FFFF  // Асинхронная посылка из платы инвертера
#define INVERT_REQ                       0x1BF1FFFF  // Посылка к плате с запросом данных или командой
#define INVERT_ANS                       0x1BF2FFFF  // Посылка из платы в ответ на запрос

// Идентификатором INVERT_REQ вызываются следующие команды (передаются в байте 0 блока данных)
#define START_MOVING                     0x01 // Начало движения
                                              // В байте  1 - направление (вниз - 1, вверх - 0)
                                              // В байтах 2 -  целевая частота вращения (Гц)
                                              // В байтах 3 -  время ускорения (в десятых долях секунды)

#define STOP_MOVING                      0x02 // Окончание движения
                                              // В байте  1 - время замедления (в десятых долях секунды)
#define EMERGENCY_STOP_MOVING            0x03 // Аварийная остановка


#define INVERT_HEARTBEAT                 0x1BF3FFFF  // Периодическое сообщение от платы Inverter о ее работоспособности

#define INVERT_UPGRADE_RX_ID             0x1BF6FFFF  // Идентификатор пакетов от частотного прелобразователя к программирующему устройству
#define INVERT_UPGRADE_TX_ID             0x1BF7FFFF  // Идентификатор пакетов от программирующего устройства к частотному прелобразователю


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой подсветки
// Номер платы записывается в биты 20...23 идентификатора
//******************************************************************************************************************************************************

#define LIGHTx_ONBUS_MSG                 0x1C00FFFF  // Асинхронная посылка из платы подсветки
#define LIGHTx_REQ                       0x1C01FFFF  // Посылка к плате подсветки
#define LIGHTx_ANS                       0x1C02FFFF  // Посылка из платы подсветки
#define LIGHTx_UPGRADE_RX_ID             0x1C03FFFF  // Идентификатор пакетов от платы подсветки к программирующему устройству
#define LIGHTx_UPGRADE_TX_ID             0x1C04FFFF  // Идентификатор пакетов от прогаммирующего устройства к плате подсветки

// Список команд посылаемых плате подсветки в байте 0 блока данных
#define LIGHT_SET_STATIC_RGB   0x01 // Установить цвет в байтах 1,2,3 передаются значения R, G, B соответственно
#define LIGHT_SET_RUN_RGB      0x02 // В байтах 1,2,3 передаются значения R, G, B фона в байтах 4,5,6 - цвет волны , в байте 6 в бите 0 передается направление
#define LIGHT_SET_RANDOM       0x03 // Установить динамический режим хаотичных миганий
#define LIGHT_SET_OFF          0x04 // Выключить светодиоды


//******************************************************************************************************************************************************
// Идентификаторы для работы с адаптером 1Wire-CAN
// Номер платы записывается в биты 20...23 идентификатора
//******************************************************************************************************************************************************
#define D1WCANx_ONBUS_MSG                0x1D00FFFF  // Асинхронная посылка из платы подсветки
#define D1WCANx_NEW_ID                   0x1D01FFFF  // Асинхронная посылка из адаптера с кодом 1Wire устройства
#define D1WCANx_REQ                      0x1D02FFFF  // Посылка к адаптеру с запросом данных или командой
#define D1WCANx_ANS                      0x1D03FFFF  // Посылка из адаптера в ответ на запрос
#define D1WCANx_UPGRADE_RX_ID            0x1D04FFFF  // Идентификатор пакетов от адаптера 1Wire-CAN к программирующему устройству
#define D1WCANx_UPGRADE_TX_ID            0x1D05FFFF  // Идентификатор пакетов от прогаммирующего устройства к адаптеру 1Wire-CAN

// Идентификатором D1WCANx_REQ вызываются следующие команды (передаются в байте 0 блока данных)
#define D1WCAN_SET_SOUND_PTTRN           0x01
#define D1WCAN_SET_LED_PTTRN             0x02
#define D1WCAN_GET_TEMPERATURE           0x03



//******************************************************************************************************************************************************
// Идентификаторы для работы с платой LED дисплея
// Номер платы записывается в биты 20...23 идентификатора
//******************************************************************************************************************************************************
#define PDISPLx_ONBUS_MSG                0x1E00FFFF  // Асинхронная посылка из платы LED дисплея
#define PDISPLx_REQ                      0x1E02FFFF  // Посылка к плате с запросом данных или командой
#define PDISPLx_ANS                      0x1E03FFFF  // Посылка из платы в ответ на запрос
#define PDISPLx_UPGRADE_RX_ID            0x1E04FFFF  // Посылка из платы во время программирования
#define PDISPLx_UPGRADE_TX_ID            0x1E05FFFF  // Посылка к плате во время программирования
#define PDISPLx_SET_RED_SYMB             0x1E06FFFF  // Посылка к плате 8-и байт красного символа
#define PDISPLx_SET_GREEN_SYMB           0x1E07FFFF  // Посылка к плате 8-и байт зеленого символа

// Идентификатором PDISPLx_REQ вызываются следующие команды (передаются в байте 0 блока данных)
#define PDISPLx_SET_SYMBOL                0x01 // Установка статического символа с кодом в байте 1 и цветом в байте 2
#define PDISPLx_SET_SYMBOL_PTRN1          0x02 // Установка 1-й части  паттерна символа с кодом в байте 1 , байты паттерна в байтах 4..7
#define PDISPLx_SET_SYMBOL_PTRN2          0x03 // Установка 2-й части  паттерна символа с кодом в байте 1 , байты паттерна в байтах 4..7
#define PDISPLx_DIN_SYMBOL_SET1           0x04 // Фаза 1 установки динамического символа.
                                               // В байте  1 - номер символа
                                               // В байтах 2..3 -  период смены шагов
                                               // В байтах 4..5 -  количество шагов
#define PDISPLx_DIN_SYMBOL_SET2           0x05 // Фаза 2 установки динамического символа.
                                               // В байте  2..3 - приращение по x
                                               // В байтах 4..5 - приращение по y
#define PDISPLx_DIN_SYMBOL_SET3           0x06 // Фаза 2 установки динамического символа.
                                               // В байте  2..3 - начальное значение x
                                               // В байтах 4..5 - начальное значение y
#define PDISPLx_DIN_SYMBOL_SET4           0x07 // Фаза 2 установки динамического символа.
                                               // В байте  1 - цвет символа (0 - красный, 1 - зеленый)


//******************************************************************************************************************************************************
// Идентификаторы для работы с платой управления питанием BA100
//******************************************************************************************************************************************************
#define PWRMOD_TO_SYS                    0x1E71FFFF  // Посылка данных от платы управления питанием к центральному блоку управления
                                                     // data[0] - тип команды
                                                     // data[1]...data[7] - данные
/*
  Перечень команд и формат

---1.--- От платы управления питанием к центральному блоку управления

  data[0] = 0x01 - событие кнопки
  data[1] = 0x01 - нажатие, 0x02 - отпускание
  data[2] = 0x01 (normal), 0x02 (manual), 0x03 (UP), 0x04 (DOWN)

  В ответ должна быть получена команда подтверждения от центрального блока с теми же данными

---2.--- От центрального блока к плате управления питанием

  data[0] = 0x02 - событие запроса платы управления питанием

  В ответ должно быть послано
  data[0] = 0x02 - событие запроса платы управления питанием
  data[1] = 0xXX - Версия платы

---3.--- От центрального блока к плате управления питанием

  data[0] = 0x03 - запрос от центральной платы на включение силового источника

  В ответ должна быть получена та же команда

---4.--- От платы управления питанием к центральному блоку управления

  data[0] = 0x04 - запрос от центральной платы на выключение силового источника

  В ответ должна быть получена та же команда

*/

// Содержимое байта 0 посылок с идетификатором PWRMOD_TO_SYS
#define PWRMOD_KEY_EVNT_CMD 0x01
#define PWRMOD_BRD_REQ      0x02
#define PWRMOD_TRACTION_ON  0x03
#define PWRMOD_TRACTION_OFF 0x04

// Содержимое байта 1 посылки PWRMOD_KEY_EVNT_CMD
#define   PWRMOD_KEY_PRESS     0x01
#define   PWRMOD_KEY_RELEASE   0x02

// Содержимое байта 2 посылки PWRMOD_KEY_EVNT_CMD
#define   PWRMOD_KEY_NORMAL     0x01
#define   PWRMOD_KEY_MANUAL     0x02
#define   PWRMOD_KEY_UP         0x03
#define   PWRMOD_KEY_DOWN       0x04
#define   PWRMOD_KEY_FIRE_ALR   0x05





#define PWRMOD_FROM_SYS                  0x1E72FFFF  //
#define PWRMOD_ONBUS_MSG                 0x1E73FFFF  //

#define PWRMOD_DATA_PACKET_TO_SYS        0x1E74FFFF  // Посылка блока данных произвольной длины от платы управления питанием к центральному блоку управления

#define   COMCH_SET_PARAMS        0x10  // Команда установки параметров задачи приложения
#define   COMCH_GET_PARAMS        0x01  // Команда запроса параметров задачи приложения
#define   COMCH_PARAMS_ANS        0x81  // Пакет данных в центральную плату с параметрами задачи приложения

#define   COMCH_GET_SYSVALS       0x03
#define   COMCH_SYSVALS_ANS       0x83  // Пакет данных в центральную плату со значениями системных величин

#define   COMCH_SET_ALARM         0x8E  // Пакет тревоги в центральную плату с передачей системных величин
#define   COMCH_PREPARE_OFF       0x8F  // Пакет подготовки к выключению в центральную плату
#define   COMCH_RESET_ALARM       0x0F  // Сброс состояния тревоги от центральной платы
#define   COMCH_SYS_OFF           0xFF  // Сообщение от центральной платы о переводе системы в выключенное состояние пока не восстановятся нормальные параметры
#define   COMCH_SYS_RESET         0xFE  // Сообщение от центральной платы о сбросе системы


#define PWRMOD_DATA_PACKET_FROM_SYS      0x1E75FFFF  // Посылка блока данных произвольной длины от центрального блока управления к плате управления питанием к
#define PWRMOD_UPGRADE_RX_ID             0x1E76FFFF  // Идентификатор пакетов от платы управления питанием BA100 к программирующему устройству
#define PWRMOD_UPGRADE_TX_ID             0x1E77FFFF  // Идентификатор пакетов от прогаммирующего устройства к плате управления питанием BA100

//******************************************************************************************************************************************************
// Идентификаторы для работы с платой управления автоматическими дверями 
// Номер платы записывается в биты 20...23 идентификатора
//******************************************************************************************************************************************************
#define DMC01_ONBUS_MSG            0x1A00FFFF  // Асинхронная посылка из платы управления автоматическими дверями 
#define DMC01_REQ                  0x1A01FFFF  // Запрос данных 
                                               // data[0] - Идетификатор команды
#define DMC01_REQ_STATUS                          0x01 // Команда запроса статуса 
#define DMC01_REQ_ZERO                            0x02 // Команда обнуления позиции 
#define DMC01_REQ_OPEN                            0x03 // Команда открыть двери
#define DMC01_REQ_CLOSE                           0x04 // Команда закрыть двери
#define DMC01_REQ_BRAKE                           0x05 // Команда затормозить двери
#define DMC01_REQ_FORCEBRAKE                      0x06 // Команда форсировано затормозить двери

#define DMC01_ANS                  0x1A02FFFF  // Ответ с данными 


//******************************************************************************************************************************************************
// Идентификаторы для общих команд перепрограммирования
//******************************************************************************************************************************************************
#define   ALL_ENTER_TO_WRK_MOD              0x0001FFFF // Команда всем перейти в рабочий режим
#define   ALL_PROLONG_BOOT_MODE             0x0002FFFF // Команда всем продлить загрузочный режим
#define   ALL_ENTER_TO_BOOT_MOD             0x0003FFFF // Команда всем перейти в загрузочный режим

#define   BOOTL_GET_CRC           0x10  // Команда запроса контрольной суммы
#define   BOOTL_ACK_CRC           0x11  // Ответ на запрос контрольной суммы
#define   BOOTL_ACK_ERASE         0x12  // Ответ о завершении стрирания

#define   BOOTL_SET_BOOT_REC      0x20  // Команда с содержимым загрузочной записи
#define   BOOTL_ERASE_FLASH       0x21  // Команда стирания FLASH
#define   BOOTL_DO_PRG            0x30  // Команда выполнить программирование блока
#define   BOOTL_ACK_PRG           0x31  //


#endif
