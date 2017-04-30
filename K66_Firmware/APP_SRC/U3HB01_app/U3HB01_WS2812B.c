// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-07
// 15:58:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

//#define RESET_BITS_CNT 50
#define   COLRS          3
#define   LEDS_NUM       122//78
#define   WS2812B_BITS_NUM (8*COLRS*LEDS_NUM)

#define   MAX_PTTRN_LEN 64 // Максимальная длина шаблона для машины состояний светодиода


uint32_t  enable_led_strip;

static   T_DMA_WS2812B_config  ws2812B_DMA_cfg;

// Структура массива хранения представления потока бит для пересылки с помощью DMA
typedef struct
{
  uint16_t  buf[LEDS_NUM][COLRS][8];
  uint16_t  bend;
} T_WS2812B_bits;

// Управляющая структура машины состояний  для каждого светодиода WS2812B
typedef struct
{
  uint32_t  cnt;         // Счетчик автомата состояний
  uint32_t  *chain_ptr;  // Указатель на начало цепочки управляющих кодов
  uint32_t  *curr_ptr;   // Текущая позиция в цепочке управляющих кодов
  uint32_t  code;
  uint32_t  data;
  uint32_t  prev_hsv;
  uint32_t  hsv;
  uint32_t  duration;
  uint32_t  jmp_done;    // Флаг выполненного перехода в управляющей цепочке
} T_WS2812B_sm_cbl;


// Структура записи в управляющем шаблоне режима работы светодиода
#define  B_JMP   BIT(31)
#define  B_STOP  BIT(30)
#define  B_RAMP  BIT(29)

typedef struct
{
  uint32_t  code;
  // Описание формата поля code
  // Номер бита     Описание
  //  31            Флаг перехода, при наличии этого флага поле data содержит адрес структуры на которую следует перейти автомату состояний
  //  30            Флаг остановки. Наличие этого флага указывает прекратить выполнение автомата состояний
  //  29            Флаг плавного перехода. Параметры цвета линейно изменяются от текущих к заданным в течении интервала заданного в поле data
  //  24..0         Код цвета в формате   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)

  uint32_t  data;  // Интервал времени удержания данного состояния или адрес перехода в зависмости от управляющих флагов

} T_pattrn_item;

#pragma data_alignment= 64
static T_WS2812B_bits WS2812B_bits; // Массив представления потока бит для пересылки с помощью DMA

static T_WS2812B_sm_cbl lcbl[LEDS_NUM]; // Структуры машин состояний светодиодов
static uint32_t g_atten; // Аттенюация яркости

static uint32_t ptrns_arr[LEDS_NUM][MAX_PTTRN_LEN];  // Массив хранения шаблонов

// Таблица перекодирования для преобразователя HSV -> RGB
const uint8_t         dim_curve[256] = {
  0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
  6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
  8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11,
  11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15,
  15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20,
  20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26,
  27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35,
  36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 47,
  48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
  63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 81, 82,
  83, 85, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

static void  WS2812B_state_automat(void);
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_bits(void)
{
  uint32_t i, j, k;

  for (i = 0; i < LEDS_NUM; i++)
  {
    for (j = 0; j < COLRS; j++)
    {
      for (k = 0; k < 8; k++)
      {
        WS2812B_bits.buf[i][j][k] = FTM_WS2812B_0;
      }
    }
  }
  WS2812B_bits.bend = 0;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param cfg 
-----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_DMA_TCD(T_DMA_WS2812B_config *cfg)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;

  // Инициализируем канал DMA на непрерывную циклическую передачу из RAM в таймер формирующий последовательность 0 и 1 в виде интервалоа PWM

  DMA->TCD[cfg->dma_ch].SADDR = cfg->saddr;                   // Источник - буфер с данными
  DMA->TCD[cfg->dma_ch].SOFF = 2;                             // Адрес источника смещаем на 2 байта после каждой передачи
                                                              // DMA->TCD[cfg->dma_ch].SLAST = (uint32_t)(-cfg->arrsz * 2);    // Возвращаемся на начальный адрес источника после завершения всего цикла DMA (окончания мажорного цикла)
  DMA->TCD[cfg->dma_ch].SLAST = 0;
  DMA->TCD[cfg->dma_ch].DADDR = cfg->daddr;                   // Адрес приемника - регистр PUSHR SPI
  DMA->TCD[cfg->dma_ch].DOFF = 0;                             // После  записи указатель приемника не смещаем
  DMA->TCD[cfg->dma_ch].DLAST_SGA = 0;                        // Цепочки дескрипторов не применяем
  DMA->TCD[cfg->dma_ch].NBYTES_MLNO = 2;                      // Количество байт пересылаемых за один запрос DMA (в минорном цикле)

  DMA->TCD[cfg->dma_ch].BITER_ELINKNO = 0 //TCD Beginning Minor Loop Link
    + LSHIFT(0, 15)                  // ELINK  | Линковку не применяем
    + LSHIFT(cfg->arrsz, 0)          // BITER  | Starting Major Iteration Count (15 bit) Копируется в CITER на следующей итерации мажорного цикла
  ;
  DMA->TCD[cfg->dma_ch].CITER_ELINKNO = 0 //TCD Current Minor Loop Link
    + LSHIFT(0, 15)                  // ELINK  | Линковку не применяем
    + LSHIFT(cfg->arrsz, 0)          // CITER  | Current Major Iteration Count (15 bit)
  ;
  DMA->TCD[cfg->dma_ch].ATTR = 0
    + LSHIFT(0, 11) // SMOD  | Модуль адреса источника не используем
    + LSHIFT(1, 8)  // SSIZE | 16-и битная пересылка из источника
    + LSHIFT(0, 3)  // DMOD  | Модуль адреса приемника
    + LSHIFT(1, 0)  // DSIZE | 16-и битная пересылка в приемник
  ;
  DMA->TCD[cfg->dma_ch].CSR = 0
    + LSHIFT(0, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
    + LSHIFT(0, 8)  // MAJORLINKCH | Номер прилинкованного канала. Попытки прилинковать канал сам к себе не работают
    + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
    + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
    + LSHIFT(0, 5)  // MAJORELINK  | Линковку не применяем
    + LSHIFT(0, 4)  // ESG         | Цепочки дескрипторов не применяем
    + LSHIFT(1, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
    + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
    + LSHIFT(0, 1)  // INTMAJOR    | НеИспользуем прерывание по окнчании пересылки DMA
    + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;
  DMA->SERQ = cfg->dma_ch; // Разрешаем работу канала DMA
  DMA->SSRT = cfg->dma_ch; // Стартуем обмен по DMA, поскольку сам таймер не выставит флага запроса пока его компаратор содержит 0
}

/*-----------------------------------------------------------------------------------------------------
  Конфигурируем выдачу стрима с помощью DMA в WS2812B
 
----------------------------------------------------------------------------------------------------*/
static void WS2812B_init_DMA_stream(T_DMA_WS2812B_config *cfg)
{
  if (cfg->ftm_ch > 7) return;

  cfg->DMAMUX->CHCFG[cfg->dma_ch] = cfg->dmux_src + BIT(7); // Через мультиплексор связываем сигнал от внешней периферии (здесь от канала SPI) с входом выбранного канала DMA

  WS2812B_init_DMA_TCD(cfg);

  cfg->FTM->OUTMASK &= ~LSHIFT(1, cfg->ftm_ch);             // Активизация выхода канала
  cfg->FTM->CONTROLS[cfg->ftm_ch].CnSC |= 0
    + LSHIFT(1, 6) // CHIE.
    + LSHIFT(1, 0) // DMA.  1 Enable DMA transfers.
  ;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param tid 
 \param data_ptr 
 \param secs 
 \param msecs 
-----------------------------------------------------------------------------------------------------*/
static void  WS2812B_refresh(_timer_id tid, void *data_ptr, uint32_t secs, uint32_t msecs)
{
  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;
  DMA->INT = BIT(DMA_WS2812B_CH); // Сбрасываем флаг прерываний  канала

  WS2812B_init_DMA_TCD(&ws2812B_DMA_cfg);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void WS2812B_periodic_refresh(void)
{
  int32_t           n;
  static uint32_t   color1;
  static uint32_t   color2;
  static uint32_t   color3;


  DMA_MemMapPtr    DMA     = DMA_BASE_PTR;
  DMA->INT = BIT(DMA_WS2812B_CH); // Сбрасываем флаг прерываний  канала

  if (enable_led_strip==1)
  {

    WS2812B_init_DMA_TCD(&ws2812B_DMA_cfg);

    WS2812B_state_automat();

    // Меняем после каждого цикла цвет волн
    for (n = 0; n < LEDS_NUM; n++)
    {
      if (lcbl[n].jmp_done != 0)
      {
        if (n == 0)
        {
          // Если переход на светодиоде 0, то меняем цвета
          // Код цвета в задается в формате   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
          color1 = (rand() & 0x1FF0000) + 0xFFFF;
          color2 = (rand() & 0x1FF0000) + 0xFFFF;
          color3 = (rand() & 0x1FF0000) + 0xFFFF;
        }

        // Здесь производим модификацию командной цепочки
        ptrns_arr[n][2] = color1 + B_RAMP;
        ptrns_arr[n][3] = 400;
        ptrns_arr[n][4] = HSV_NONE + B_RAMP;
        ptrns_arr[n][5] = 400;
        ptrns_arr[n][6] = color2 + B_RAMP;
        ptrns_arr[n][7] = 400;
        ptrns_arr[n][8] = HSV_NONE + B_RAMP;
        ptrns_arr[n][9] = 400;
        ptrns_arr[n][10] = color3 + B_RAMP;
        ptrns_arr[n][11] = 400;
        ptrns_arr[n][12] = HSV_NONE + B_RAMP;
        ptrns_arr[n][13] = 400;

        lcbl[n].jmp_done = 0;
      }
    }
  }

}

/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB в целочисленной арифмерите
 
  Вход - HSV, код цвета в формате   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
 ------------------------------------------------------------------------------*/
uint32_t Convert_H_S_V_to_RGB(uint32_t hue, uint32_t sat, uint32_t val)
{
  uint32_t   r;
  uint32_t   g;
  uint32_t   b;
  uint32_t   base;
  uint32_t   rgb;

  val = dim_curve[val];
  sat = 255 - dim_curve[255 - sat];

  if (sat == 0) // Acromatic color (gray). Hue doesn't mind.
  {
    rgb = val | (val << 8) | (val << 16);
  }
  else
  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60)
    {
    case 0:
      r = val;
      g = (((val - base) * hue) / 60) + base;
      b = base;
      break;
    case 1:
      r = (((val - base) * (60 - (hue % 60))) / 60) + base;
      g = val;
      b = base;
      break;
    case 2:
      r = base;
      g = val;
      b = (((val - base) * (hue % 60)) / 60) + base;
      break;
    case 3:
      r = base;
      g = (((val - base) * (60 - (hue % 60))) / 60) + base;
      b = val;
      break;
    case 4:
      r = (((val - base) * (hue % 60)) / 60) + base;
      g = base;
      b = val;
      break;
    case 5:
      r = val;
      g = base;
      b = (((val - base) * (60 - (hue % 60))) / 60) + base;
      break;
    }
    rgb = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
  }
  return rgb;

}


/*------------------------------------------------------------------------------
  Корнвертер из HSV в RGB в целочисленной арифмерите
 
  Вход - HSV, код цвета в формате   [hue] - 0..360 (9 bit), [saturation] - 0..255 (8 bit),  [value] - 0..255 (8-bit)
 ------------------------------------------------------------------------------*/
uint32_t Convert_HSV_to_RGB(uint32_t hsv)
{
  uint32_t   hue;
  uint32_t   sat;
  uint32_t   val;

  hue = (hsv >> 16) & 0x1FF;
  sat = (hsv >> 8) & 0xFF;
  val = (hsv >> 0) & 0xFF;
  return Convert_H_S_V_to_RGB(hue, sat, val);
}

/*------------------------------------------------------------------------------
   Устанавливаем цвет светодиода
   color - цвет в формате HSV
 ------------------------------------------------------------------------------*/
void WS2812B_set_led_state(uint32_t ledn, uint32_t hue, uint32_t sat, uint32_t val, uint32_t attn)
{
  uint32_t i;
  uint8_t  c_red;
  uint8_t  c_green;
  uint8_t  c_blue;
  uint32_t color;

  color = Convert_H_S_V_to_RGB(hue, sat, val);
  c_red   = ((color >> 16) & 0xFF) >> attn;
  c_green = ((color >> 8) & 0xFF) >> attn;
  c_blue  = ((color >> 0) & 0xFF) >> attn;

  for (i = 0; i < 8; i++)
  {

    // Зеленый
    if ((c_green >> (7 - i)) & 1) WS2812B_bits.buf[ledn][0][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][0][i] = FTM_WS2812B_0;
    // Красный
    if ((c_red >> (7 - i)) & 1) WS2812B_bits.buf[ledn][1][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][1][i] = FTM_WS2812B_0;
    // Синий
    if ((c_blue >> (7 - i)) & 1) WS2812B_bits.buf[ledn][2][i] = FTM_WS2812B_1;
    else WS2812B_bits.buf[ledn][2][i] = FTM_WS2812B_0;
  }
}


/*-------------------------------------------------------------------------------------------------------------
  Инициализация шаблона для машины состояний сигнала на соленоиде

  Шаблон состоит из массива груп слов.
  Первое слово в группе - код цвета 
  Второе слово в группе - длительность интервала времени в десятках мс
    интервал равный 0x00000000 - означает возврат в начало шаблона
    интервал равный 0xFFFFFFFF - означает застывание состояния

  n - индекс светодиода 0..(LEDS_CNT - 1)
-------------------------------------------------------------------------------------------------------------*/
void WS2812B_Set_pattern(uint32_t *pattern, uint32_t n)
{

  if (n >= LEDS_NUM) return;

  _int_disable();
  if ((pattern != 0) && (lcbl[n].chain_ptr != pattern))
  {
    lcbl[n].chain_ptr = pattern;
    lcbl[n].curr_ptr = pattern;
    lcbl[n].prev_hsv = HSV_NONE;
    lcbl[n].hsv      = HSV_NONE;
    lcbl[n].cnt      = 0;
  }
  _int_enable();
}


/*------------------------------------------------------------------------------
   Автомат состояний светодиодов
   Вызывается каждый тик 
 ------------------------------------------------------------------------------*/
static void  WS2812B_state_automat(void)
{
  uint32_t       n;
  int32_t        res = -1;

  for (n = 0; n < LEDS_NUM; n++)
  {
    if (lcbl[n].chain_ptr != 0) // Проверяем наличие указателя цепочки
    {
      if (lcbl[n].cnt == 0)  // Переходим к новому управляющему элементу цепочки
      {
        lcbl[n].code = *lcbl[n].curr_ptr;   // Выборка кодового слова
        lcbl[n].curr_ptr++;
        lcbl[n].data = *lcbl[n].curr_ptr;   // Выборка данных
        lcbl[n].curr_ptr++;                 // Сдвигаем указатель на следующий управляющий элемент


        if (lcbl[n].code & B_STOP)
        {
          // Прекращаем работу автомата состояний
          lcbl[n].cnt = 0;
          lcbl[n].chain_ptr = 0;
        }
        else if (lcbl[n].code & B_JMP)
        {
          // Переход на цепочку по указателю
          lcbl[n].curr_ptr = (uint32_t *)lcbl[n].data;
          lcbl[n].jmp_done = 1;
        }
        else
        {
          lcbl[n].duration = Conv_ms_to_ticks(lcbl[n].data);
          lcbl[n].cnt = lcbl[n].duration;
          lcbl[n].prev_hsv = lcbl[n].hsv;
          lcbl[n].hsv = lcbl[n].code;

          if ((lcbl[n].code & B_RAMP) == 0)
          {
            uint32_t   hue;
            uint32_t   sat;
            uint32_t   val;

            hue = (lcbl[n].hsv >> 16) & 0x1FF;
            sat = (lcbl[n].hsv >> 8) & 0xFF;
            val = (lcbl[n].hsv >> 0) & 0xFF;

            // Если нет рампы, то сразу устанавливаем заданный цвет
            WS2812B_set_led_state(n, hue, sat, val, g_atten);
          }
        }
      }
      else
      {
        lcbl[n].cnt--;
        if (lcbl[n].code & B_RAMP)
        {
          // Расчет нового значения цвета в случае рампы
          uint32_t   prev_hue;
          uint32_t   prev_sat;
          uint32_t   prev_val;

          uint32_t   hue;
          uint32_t   sat;
          uint32_t   val;

          uint32_t   delta;

          hue = (lcbl[n].hsv >> 16) & 0x1FF;
          sat = (lcbl[n].hsv >> 8) & 0xFF;
          val = (lcbl[n].hsv >> 0) & 0xFF;

          prev_hue = (lcbl[n].prev_hsv >> 16) & 0x1FF;
          prev_sat = (lcbl[n].prev_hsv >> 8) & 0xFF;
          prev_val = (lcbl[n].prev_hsv >> 0) & 0xFF;

          if (hue > prev_hue)
          {
            delta = ((hue - prev_hue) * lcbl[n].cnt) / lcbl[n].duration;
            hue = hue - delta;
          }
          else
          {
            delta = ((prev_hue - hue) * lcbl[n].cnt) / lcbl[n].duration;
            hue = hue + delta;
          }
          if (sat > prev_sat)
          {
            delta = ((sat - prev_sat) * lcbl[n].cnt) / lcbl[n].duration;
            sat = sat - delta;
          }
          else
          {
            delta = ((prev_sat - sat) * lcbl[n].cnt) / lcbl[n].duration;
            sat = sat + delta;
          }
          if (val > prev_val)
          {
            delta = ((val - prev_val) * lcbl[n].cnt) / lcbl[n].duration;
            val = val - delta;
          }
          else
          {
            delta = ((prev_val - val) * lcbl[n].cnt) / lcbl[n].duration;
            val = val + delta;
          }
          WS2812B_set_led_state(n, hue, sat, val, g_atten);
        }
      }
    }
    else
    {
      // Если нет шаблона, то выключаем светодиод
      WS2812B_set_led_state(n, 0, 0, 0, g_atten);
    }
  }
}



/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void WS2812B_Demo_DMA(void)
{
  uint32_t i;

  // Таймер не вызывает свою функцию. Причина не ясна
  //refr_tmr_id = _timer_start_periodic_every(WS2812B_refresh, 0, TIMER_KERNEL_TIME_MODE, 10);

  WS2812B_init_bits();

  ws2812B_DMA_cfg.FTM      = FTM0_BASE_PTR;
  ws2812B_DMA_cfg.ftm_ch   = FTM_CH_2;
  ws2812B_DMA_cfg.dma_ch   = DMA_WS2812B_CH;
  ws2812B_DMA_cfg.saddr    = (uint32_t)&WS2812B_bits.buf;
  ws2812B_DMA_cfg.arrsz    = WS2812B_BITS_NUM + 1;
  ws2812B_DMA_cfg.daddr    = (uint32_t)&ws2812B_DMA_cfg.FTM->CONTROLS[FTM_CH_2].CnV;
  ws2812B_DMA_cfg.DMAMUX   = DMA_WS2812B_DMUX_PTR;
  ws2812B_DMA_cfg.dmux_src = DMA_WS2812B_DMUX_SRC;

  WS2812B_init_DMA_stream(&ws2812B_DMA_cfg);

  // Настройка шаблона бегущих разноцветных волн
  for (i = 0; i < LEDS_NUM; i++)
  {
    ptrns_arr[i][0] = HSV_NONE + B_RAMP;
    ptrns_arr[i][1] = 40 * i;

    ptrns_arr[i][2] = HSV_GREEN + B_RAMP;
    ptrns_arr[i][3] = 400;
    ptrns_arr[i][4] = HSV_NONE + B_RAMP;
    ptrns_arr[i][5] = 400;
    ptrns_arr[i][6] = HSV_BLUE + B_RAMP;
    ptrns_arr[i][7] = 400;
    ptrns_arr[i][8] = HSV_NONE + B_RAMP;
    ptrns_arr[i][9] = 400;
    ptrns_arr[i][10] = HSV_RED + B_RAMP;
    ptrns_arr[i][11] = 400;
    ptrns_arr[i][12] = HSV_NONE + B_RAMP;
    ptrns_arr[i][13] = 400;
    ptrns_arr[i][14] = B_JMP;
    ptrns_arr[i][15] = (uint32_t)&ptrns_arr[i][2];
  }
  for (i = 0; i < LEDS_NUM; i++)
  {
    WS2812B_Set_pattern(&ptrns_arr[i][0], i);
  }
}

