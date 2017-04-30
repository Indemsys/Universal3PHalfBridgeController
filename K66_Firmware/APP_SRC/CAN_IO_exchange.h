#ifndef CAN_IO_EXCHANGE_H
  #define CAN_IO_EXCHANGE_H


// Определения типа сигнала для поля T_input_cbl.itype
#define   GEN_SW     0
#define   ESC_SW     1


// Структура для обработчика подавления дребезга
typedef struct
{
  int8_t    val;
  int8_t    curr;
  int8_t    prev;
  uint32_t  cnt;
  uint8_t   init;

} T_bncf;


// Структура алгоритма определения состояния сигналов
typedef struct
{
  uint8_t            itype;     // Тип  сигнала. 0 - простой контакт бистабильный, 1 - контакт в цепи безопасности с 3-я состояниями
  uint16_t           *p_smpl1;  // Указатель на результат сэмплирования напряжения на контакте с более высоким напряжением
  uint16_t           *p_smpl2;  // Указатель на результат сэмплирования напряжения на контакте с более низким напряжением
  uint16_t           log_bound; // Граница между логическим 0 и 1 на входе
  uint32_t           l0_time;   // Время устоявшегося состояния для фиксации низкого уровня сигнала
  uint32_t           l1_time;   // Время устоявшегося состояния для фиксации высокого уровня сигнала
  uint32_t           lu_time;   // Время устоявшегося состояния для фиксации неопределенного уровня сигнала
  int8_t             *val;      // Указатель на переменную для сохранения вычисленного состояния входа
  int8_t             *val_prev; // Указатель на переменную для сохранения предыдущего состояния входа
  int8_t             *flag;     // Указатель на флаг переменной. Флаг не равный нулю указывает на произошедшее изменение состояния переменной
  T_bncf             pbncf;     // Структура для алгоритма фильтрации дребезга
} T_input_cbl;


// Управляющая структура для алгоритма упаковки состояния сигналов в 8-и байтный пакет CAN
typedef struct
{
  uint8_t            itype;    // Тип  сигнала. 0 - простой контакт бистабильный, 1 - контакт в цепи безопасности с 3-я состояниями
  uint8_t            nbyte;    // Номер байта
  uint8_t            nbit;     // Номер бита в байте
  int8_t             *val;     // Указатель на переменную состояния сигнала
  int8_t             *val_prev;// Указатель на переменную состояния сигнала
  int8_t             *flag;    // Указатель на флаг переменной. Флаг не равный нулю указывает на произошедшее изменение состояния переменной
  const uint8_t      *name;    // Имя переменной
} T_can_inp_pack;


// Структура алгоритма упаковки состояния выходов в 8-и байтный пакет CAN
typedef struct
{
  uint8_t            mask;     // Битовая маска переменной отражающая ее размер в битах
  uint8_t            nbyte;    // Номер байта
  uint8_t            nbit;     // Номер бита в байте
  int8_t             *val;     // Указатель на переменную состояния сигнала
  const uint8_t      *name;    // Имя переменной
} T_can_out_pack;




uint32_t  Do_input_processing(T_input_cbl *scbl);
uint32_t  CAN_pack_inputs(T_can_inp_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen);
int32_t   CAN_unpack_received_inputs(T_can_rx *rx, T_can_inp_pack *parr, uint32_t sz);
uint32_t  CAN_pack_outputs(T_can_out_pack *parr, uint32_t sz, uint8_t *canbuf, uint8_t *packlen);
uint32_t  CAN_unpack_received_outputs(T_can_rx *rx, T_can_out_pack *parr, uint32_t sz);

#endif // CAN_IO_EXCHANGE_H



