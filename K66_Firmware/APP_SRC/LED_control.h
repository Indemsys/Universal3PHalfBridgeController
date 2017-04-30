#ifndef LED_CONTROL_H
  #define LED_CONTROL_H



// Управляющая структура машины состояний управляемой шаблоном
typedef struct
{
  uint32_t  init_state;
  uint32_t  counter;
  int32_t  *pattern_start_ptr;  // Указатель на массив констант являющийся цепочкой состояний (шаблоном)
                                // Если значение в массиве = 0xFFFFFFFF, то процесс обработки завершается
                                // Если значение в массиве = 0x00000000, то вернуть указатель на начало цепочки
  int32_t   *pttn_ptr;          // Текущая позиция в цепочке состояний

} T_LED_ptrn;




void Set_LED_pattern(const int32_t *pttn, uint32_t n);
void LEDS_state_automat(void);


#define LED__ON  0
#define LED_OFF  1

#ifdef _MAIN_GLOBALS_

//  Шаблон состоит из массива груп слов.
//  Первое слово в группе - значение напряжения
//  Второе слово в группе - длительность интервала времени в мс или специальный маркер остановки(0xFFFFFFFF) или цикла(0x00000000)

const int32_t   LED_BLINK[] =
{
  LED__ON, 30,
  LED_OFF, 470,
  0, 0
};

const int32_t   LED_ON[] =
{
  LED__ON, 100,
  0, 0xFFFFFFFF
};

const int32_t   LED_2_BLINK[] =
{
  LED__ON, 50,
  LED_OFF, 50,
  LED__ON, 50,
  LED_OFF, 350,
  0, 0
};

const int32_t   LED_3_BLINK[] =
{
  LED__ON, 50,
  LED_OFF, 100,
  LED__ON, 50,
  LED_OFF, 100,
  LED__ON, 50,
  LED_OFF, 350,
  0, 0
};

#else

extern const int32_t   LED_BLINK[];
extern const int32_t   LED_ON[];
extern const int32_t   LED_2_BLINK[];
extern const int32_t   LED_3_BLINK[];

#endif


void Set_LED_voltage(uint8_t val, uint8_t num);


#endif // LED_CONTROL_H



