// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.05.27
// 11:29:26
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "app.h"


#define LEDS_NUM  1

T_LED_ptrn ledsm_cbl[LEDS_NUM];

/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Set_LED_voltage(uint8_t val, uint8_t num)
{
  if (val == 0) GPIOA_PCOR = BIT(1);
  else GPIOA_PSOR = BIT(1);
}


/*-------------------------------------------------------------------------------------------------------------
  Инициализация шаблона для машины состояний сигнала на соленоиде

  Шаблон состоит из массива груп слов.
  Первое слово в группе - значение напряжения
  Второе слово в группе - длительность интервала времени в десятках мс
    интервал равный 0x00000000 - означает возврат в начало шаблона
    интервал равный 0xFFFFFFFF - означает застывание состояния

  n - индекс светодиода 0..(LEDS_CNT - 1)
-------------------------------------------------------------------------------------------------------------*/
void Set_LED_pattern(const int32_t *pttn, uint32_t n)
{

  if (n > (LEDS_NUM - 1)) return;

  _int_disable();
  if ((pttn != 0) && (ledsm_cbl[n].pattern_start_ptr != (int32_t *)pttn))
  {
    ledsm_cbl[n].pattern_start_ptr = (int32_t *)pttn;
    ledsm_cbl[n].pttn_ptr = (int32_t *)pttn;
    Set_LED_voltage(*ledsm_cbl[n].pttn_ptr, n);
    ledsm_cbl[n].pttn_ptr++;
    ledsm_cbl[n].counter = Conv_ms_to_ticks((*ledsm_cbl[n].pttn_ptr));
    ledsm_cbl[n].pttn_ptr++;
  }
  _int_enable();
}


/*------------------------------------------------------------------------------
   Автомат состояний светодиодов
   Вызывается каждый тик
 ------------------------------------------------------------------------------*/
void LEDS_state_automat(void)
{
  uint32_t        duration;
  uint32_t        voltage;
  uint32_t        n;


  for (n = 0; n < LEDS_NUM; n++)
  {
    // Управление состоянием сигнала соленоида
    if (ledsm_cbl[n].counter) // Отрабатываем шаблон только если счетчик не нулевой
    {
      ledsm_cbl[n].counter--;
      if (ledsm_cbl[n].counter == 0)  // Меняем состояние сигнала при обнулении счетчика
      {
        if (ledsm_cbl[n].pattern_start_ptr != 0) // Проверяем есть ли назначенный шаблон
        {
          voltage = *ledsm_cbl[n].pttn_ptr;        // Выборка значения напряжения
          ledsm_cbl[n].pttn_ptr++;
          duration = *ledsm_cbl[n].pttn_ptr;       // Выборка длительности состояния
          ledsm_cbl[n].pttn_ptr++;                 // Переход на следующий элемент шаблона
          if (duration != 0xFFFFFFFF)
          {
            if (duration == 0)  // Длительность равная 0 означает возврат указателя элемента на начало шаблона и повторную выборку
            {
              ledsm_cbl[n].pttn_ptr = ledsm_cbl[n].pattern_start_ptr;
              voltage = *ledsm_cbl[n].pttn_ptr;
              ledsm_cbl[n].pttn_ptr++;
              ledsm_cbl[n].counter = Conv_ms_to_ticks(*ledsm_cbl[n].pttn_ptr);
              ledsm_cbl[n].pttn_ptr++;
              Set_LED_voltage(voltage, n);
            }
            else
            {
              ledsm_cbl[n].counter = Conv_ms_to_ticks(duration);
              Set_LED_voltage(voltage, n);
            }
          }
          else
          {
            // Обнуляем счетчик и таким образом выключаем обработку паттерна
            Set_LED_voltage(voltage, n);
            ledsm_cbl[n].counter = 0;
            ledsm_cbl[n].pattern_start_ptr = 0;
          }
        }
        else
        {
          // Если нет шаблона обнуляем напряжение на соленоиде
          Set_LED_voltage(0, n);
        }
      }
    }
  }
}


