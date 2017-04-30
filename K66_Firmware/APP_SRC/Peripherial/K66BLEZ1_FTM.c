// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.08.31
// 10:47:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


static void        FTM1_isr(void *user_isr_ptr);
static T_qdec_isr  qdec_isr;



/*-------------------------------------------------------------------------------------------------------------
  Обработчик прерывания таймера FTM1
  Прерывания происходят при переполнении счетчика
  Функция обработчика вести счетчик переполнений
-------------------------------------------------------------------------------------------------------------*/
static void FTM1_isr(void *user_isr_ptr)
{
  FTM_MemMapPtr FTM1 = FTM1_BASE_PTR;
  uint32_t      sc;

  sc =  FTM1->SC; // Читаем состояние
  if (sc & BIT(7))
  {
    FTM1->SC = sc & ~(BIT(7)); // Сбрасываем бит TOF

    if (qdec_isr != 0) qdec_isr(FTM1->QDCTRL & BIT(1));
  }
}

/*-------------------------------------------------------------------------------------------------------------
  Обработчик прерывания таймера FTM3Р 
  Триггер счетчика используется для запуска ADC 
  Прерывания происходят при переполнении счетчика
 
  В случае счета вверх и вниз прерывания будут присходить только внизу
-------------------------------------------------------------------------------------------------------------*/
static void FTM3_isr(void *user_isr_ptr)
{
  FTM_MemMapPtr FTM3 = FTM3_BASE_PTR;
  uint32_t      sc;

  sc =  FTM3->SC; // Читаем состояние
  if (sc & BIT(7))
  {
    FTM3->SC = sc & ~(BIT(7)); // Сбрасываем бит TOF
  }
// Чтобы триггер активизировал ADC его сбрасывать не обязательно
//  sc =  FTM3->EXTTRIG; // Читаем состояние
//  if (sc & BIT(7))
//  {
//    FTM3->EXTTRIG = sc & ~(BIT(7)); // Сбрасываем бит TRIGF
//  }

}

/*------------------------------------------------------------------------------
  Конфигурирование FTM1 для работы в режиме квадратурного декодера 


 \param isr
 ------------------------------------------------------------------------------*/
void FTM1_init_QDEC(T_qdec_isr isr)
{
  FTM_MemMapPtr FTM1 = FTM1_BASE_PTR;

  // Разрешаем тактирование FTM1
  SIM_SCGC6 |= BIT(25);

  // Включаем полную функциональность модуля таймера и разрешаем запись во все поля регистров
  FTM1->MODE = 0
               + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
               + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
               + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
               + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode
               + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
               + LSHIFT(0, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
               + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM1->CONF  = 0
                + LSHIFT(0, 10) // GTBEOUT | Global time base output
                + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
                + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | В режиме отладки сохраняется полная выходов FTM
                + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // Таймер остановлен
  FTM1->SC = 0
             + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | Сброс в 0
             + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.
             + LSHIFT(0, 3) // CLKS  | 00 Модуль таймера остановлен
             + LSHIFT(0, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. Предделитель = 1
  ;



  FTM1->MOD      = QDEC_CNT_MOD;    // Установка регистра перезагрузки. Прерывание через 4-е импульса. Таким образом нивелируем разницу в длительности импульсов тахометра
  FTM1->CNTIN    = 0;    // Начальное значение счетчика
  FTM1->CNT      = 0;    // Запись в регистр счетчка любого значения приводит к записи значения из CNTIN и установке начального состояния выходов

  FTM1->FILTER  = LSHIFT(0xF, 4) + LSHIFT(0xF, 0); // Фильтуем максимально по 16 отсчетов  в каждом канале

  // Устанавливаем режим квадратурный декодера
  FTM1->QDCTRL  = 0
                  + LSHIFT(1, 7) // PHAFLTREN | Phase A Input Filter Enable
                  + LSHIFT(1, 6) // PHBFLTREN | Phase B Input Filter Enable
                  + LSHIFT(0, 5) // PHAPOL    | Phase A Input Polarity
                  + LSHIFT(0, 4) // PHBPOL    | Phase B Input Polarity
                  + LSHIFT(0, 3) // QUADMODE  | Quadrature Decoder Mode  | 0 Phase A and phase B encoding mode.
                  + LSHIFT(0, 2) // QUADIR    | Read Only. FTM Counter Direction in Quadrature Decoder Mode    | 1 Counting direction is increasing (FTM counter increment).
                  + LSHIFT(0, 1) // TOFDIR    | Read Only. Timer Overflow Direction in Quadrature Decoder Mode | 1 TOF bit was set on the top of counting.
                  + LSHIFT(1, 0) // QUADEN    | Quadrature Decoder Mode Enable
  ;



  // Запускаем системный тактирующий сигнал на таймер
  FTM1->SC = 0
             + LSHIFT(1, 6) // TOIE. 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS. 1 -FTM counter operates in up-down counting mode.
             + LSHIFT(3, 3) // CLKS. 11 External clock
             + LSHIFT(0, 0) // PS. Prescale Factor Selection. 000 Divide by 1
  ;
  // Кофигурируем прерывани таймера
  qdec_isr = isr;
  Install_and_enable_isr(INT_FTM1, FTM1_ISR_PRIO, FTM1_isr);
}



/*-----------------------------------------------------------------------------------------------------
 Инициализация заданного FTM для генерации PWM сигналов управления чипами WS2812B
 Только для таймеров 0 и 3 !!!
 FTM пареводится при этом в режим TPM с упрощенной синхронизацией, поскольку только в этом режиме работает DMA    
 
 Тактирование таймеров производится от частоты шины = 60 МГц
 Таймер считает до максимума и обнуляется 
 Период PWM = 1.25 мкс 
 
 
 
 \param FTM   - Указатель на регистровую структуру таймера
 \param presc - Предделитель входной частоты таймера
 
-----------------------------------------------------------------------------------------------------*/
void FTM_init_PWM_DMA(FTM_MemMapPtr FTM)
{
  uint32_t sc_reg;
  uint8_t  presc  = FTM_PRESC_1;

  if (FTM == FTM0_BASE_PTR)
  {
    // Разрешаем тактирование FTM0
    SIM_SCGC6 |= BIT(24);
  }
  else if (FTM == FTM3_BASE_PTR)
  {
    // Разрешаем тактирование FTM3
    SIM_SCGC3 |= BIT(25);
  }
  else return;

  FTM->OUTINIT = 0    // Начальные значения для выходов каналов
                 + LSHIFT(0, 7)  // CH7OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 6)  // CH6OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 5)  // CH5OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 4)  // CH4OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 3)  // CH3OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 2)  // CH2OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 1)  // CH1OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 0)  // CH0OI | 0 The initialization value is 0. 1 The initialization value is 1.
  ;

  // Включаем полную функциональность модуля таймера и разрешаем запись во все поля регистров
  FTM->MODE = 0
              + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
              + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
              + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
              + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode | 0 No restrictions. Software and hardware triggers can be used by MOD, CnV, OUTMASK, and FTM counter synchronization.
              + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
              + LSHIFT(1, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
                             // Здесь включаем режим совместимости с TPM в котором работает DMA
                             // В режиме FTM невозможно запустить DMA поскольку требуется сигнал тригера для загрузки регистров CnV из их буферов
              + LSHIFT(0, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM->CONF = 0
              + LSHIFT(0, 10) // GTBEOUT | Global time base output
              + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
              + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | В режиме отладки сохраняется полная выходов FTM
              + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // Таймер остановлен
  sc_reg = 0
           + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | Сброс в 0
           + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
           + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.0 FTM counter operates in Up Counting mode.
           + LSHIFT(0, 3) // CLKS  | 00 Модуль таймера остановлен
           + LSHIFT(presc, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 001 Divide by 2
  ;
  FTM->SC = sc_reg;

  // В режиме совместимости с TPM установки синхронизации в регистре  FTM->SYNCONF не работают

  FTM->SYNCONF = 0
                 + LSHIFT(0, 20) // HWSOC.      1 A hardware trigger activates the SWOCTRL register synchronization.
                 + LSHIFT(0, 19) // HWINVC.     1 A hardware trigger activates the INVCTRL register synchronization.
                 + LSHIFT(0, 18) // HWOM.       1 A hardware trigger activates the OUTMASK register synchronization.
                 + LSHIFT(0, 17) // HWWRBUF.    1 A hardware trigger activates MOD, CNTIN, and CV registers synchronization.
                 + LSHIFT(0, 16) // HWRSTCNT.   1 A hardware trigger activates the FTM counter synchronization.
                 + LSHIFT(0, 12) // SWSOC.      1 The software trigger activates the SWOCTRL register synchronization.
                 + LSHIFT(0, 11) // SWINVC.     1 The software trigger activates the INVCTRL register synchronization.
                 + LSHIFT(0, 10) // SWOM.       1 The software trigger activates the OUTMASK register synchronization.
                 + LSHIFT(0, 9) // SWWRBUF.    1 The software trigger activates MOD, CNTIN, and CV registers synchronization
                 + LSHIFT(0, 8) // SWRSTCNT.   1 The software trigger activates the FTM counter synchronization. Если 1 то сбрасываем таймер каждый раз когда пишем 1 в SWSYNC
                 + LSHIFT(1, 7) // SYNCMODE.   1 Enhanced PWM synchronization is selected.
                 + LSHIFT(0, 5) // SWOC.       1 SWOCTRL регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(0, 4) // INVC.       1 INVCTRL регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(0, 2) // CNTINC.     1 CNTIN регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(0, 1) // HWTRIGMODE. 1 FTM does not clear the TRIGj bit when the hardware trigger j is detected.
  ;



  FTM->MOD     = FTM_WS2812B_MOD;   // Установка регистра перезагрузки.
  FTM->CNTIN   = 0;    // Начальное значение счетчика
  FTM->CNT     = 0;    // Запись в регистр счетчка любого значения приводит к записи значения из CNTIN и установке начального состояния выходов
  FTM->POL     = 0;    // Полярность у сигналов

  FTM->OUTMASK = 0    // Маскировка реакции выходов на событие совпадения, т.е. маска отключения PWM на заданных выходах
                 + LSHIFT(1, 7)  // CH7OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 6)  // CH6OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 5)  // CH5OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 4)  // CH4OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 3)  // CH3OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 2)  // CH2OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 1)  // CH1OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 0)  // CH0OM | 1 Channel output is masked. It is forced to its inactive state.
  ;
  FTM->COMBINE = 0     // Связанную работу каналов не используем
                       // Установки для  каналов 6 и 7
                 + LSHIFT(0, 30) // FAULTEN3. 1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 29) // SYNCEN3.  1 The PWM synchronization in this pair of channels is enabled. Должна быть обязательно разрешена чтобы можно было делать запись в регистры CnV
                 + LSHIFT(0, 28) // DTEN3.    0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 27) // DECAP3.   0 The dual edge captures are inactive.
                 + LSHIFT(0, 26) // DECAPEN3  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0, 25) // COMP3     1 The channel (7) output is the complement of the channel (6) output.
                 + LSHIFT(0, 24) // COMBINE3  1 Channels (6) and (7) are combined.
                                 // Установки для  каналов 4 и 5
                 + LSHIFT(0, 22) // FAULTEN2  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 21) // SYNCEN2   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(0, 20) // DTEN2     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 19) // DECAP2    0 The dual edge captures are inactive.
                 + LSHIFT(0, 18) // DECAPEN2  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0, 17) // COMP2     1 The channel (5) output is the complement of the channel (4) output.
                 + LSHIFT(0, 16) // COMBINE2  1 Channels (4) and (5) are combined.
                                 // Установки для  каналов 2 и 3
                 + LSHIFT(0, 14) // FAULTEN1  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 13) // SYNCEN1   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(0, 12) // DTEN1     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 11) // DECAP1    0 The dual edge captures are inactive.
                 + LSHIFT(0, 10) // DECAPEN1  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0,  9) // COMP1     1 The channel (3) output is the complement of the channel (2) output.
                 + LSHIFT(0,  8) // COMBINE1  1 Channels (2) and (3) are combined.
                                 // Установки для  каналов 0 и 1
                 + LSHIFT(0,  6) // FAULTEN0  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1,  5) // SYNCEN0   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(0,  4) // DTEN0     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0,  3) // DECAP0    0 The dual edge captures are inactive.
                 + LSHIFT(0,  2) // DECAPEN0  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0,  1) // COMP0     1 The channel (1) output is the complement of the channel (0) output.
                 + LSHIFT(0,  0) // COMBINE0  1 Channels (0) and (1) are combined.
  ;
  FTM->DEADTIME = 0; // Мертове время не используем
  FTM->INVCTRL = 0;  // Управление инверитированием пар каналов не используем

  FTM->FILTER = 0;     // Фильтры для входных каналов не используем


  // Установка работы каналов в режиме PWM выровненному по срезу
  // Включение в 1 в начале периода, сброс в 0 после сработки компаратора

  FTM->CONTROLS[0].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[1].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[2].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[3].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[4].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[5].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[6].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[7].CnSC = 0
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  // Загрузку по событиям компараторов не используем
  FTM->PWMLOAD = 0
                 + LSHIFT(0, 9)  // LDOK   | Автоматическая загрузка в регистры MOD, CNTIN, и CnV  момент перегрузки таймера. Флаг однократный. Действует до ближайшего события синхронизации.
                 + LSHIFT(0, 7)  // CH7SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 7
                 + LSHIFT(0, 6)  // CH6SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 6
                 + LSHIFT(0, 5)  // CH5SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 5
                 + LSHIFT(0, 4)  // CH4SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 4
                 + LSHIFT(0, 3)  // CH3SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 3
                 + LSHIFT(0, 2)  // CH2SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 2
                 + LSHIFT(0, 1)  // CH1SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 1
                 + LSHIFT(0, 0)  // CH0SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 0
  ;
  // Выходами програмно не управляем
  FTM->SWOCTRL = 0
                 + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 14) // CH6OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 13) // CH5OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 12) // CH4OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 11) // CH3OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 10) // CH2OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 9)  // CH1OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 8)  // CH0OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 5)  // CH5OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 4)  // CH4OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 3)  // CH3OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 2)  // CH2OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 1)  // CH1OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 0)  // CH0OC  | 1 The channel output is affected by software output control.
  ;

  // В режиме совместимости с TPM установки синхронизации в регистре  FTM->SYNC не работают

  // Программно активизируем синхроную загрузку регистров по событиям
  FTM->SYNC = 0
              + LSHIFT(0, 7)  // SWSYNC  | PWM Synchronization Software Trigger. The software trigger happens when a 1 is written to this bit. Флаг однократный. Действует до ближайшего события синхронизации.
              + LSHIFT(0, 6)  // TRIG2   | PWM Synchronization Hardware Trigger 2
              + LSHIFT(0, 5)  // TRIG1   | PWM Synchronization Hardware Trigger 1
              + LSHIFT(0, 4)  // TRIG0   | PWM Synchronization Hardware Trigger 0
              + LSHIFT(0, 3)  // SYNCHOM | 0 OUTMASK register is updated with the value of its buffer in all rising edges of the system clock. 1 OUTMASK register is updated with the value of its buffer only by the PWM synchronization.
              + LSHIFT(0, 2)  // REINIT  | FTM Counter Reinitialization by Synchronization | 0 FTM counter continues to count normally. (Не имеет значения в данном режиме таймера)
              + LSHIFT(0, 1)  // CNTMAX  | 1 The maximum loading point is enabled. Один из  двух флагов (CNTMAX, CNTMIN) должен быть обязательно установлен для работы синхронной записи в регистры
              + LSHIFT(0, 0)  // CNTMIN  | 1 The minimum loading point is enabled.
  ;

  FTM->CONTROLS[0].CnV = 0; // Обнулем регистры компараторов для всех каналов
  FTM->CONTROLS[1].CnV = 0;
  FTM->CONTROLS[2].CnV = 0;
  FTM->CONTROLS[3].CnV = 0;
  FTM->CONTROLS[4].CnV = 0;
  FTM->CONTROLS[5].CnV = 0;
  FTM->CONTROLS[6].CnV = 0;
  FTM->CONTROLS[7].CnV = 0;

  // Запускаем таймер
  FTM->SC = sc_reg | LSHIFT(1, 3); // CLKS  | 01 System clock;

}

/*-----------------------------------------------------------------------------------------------------
 Инициализация FTM3 для генерации 3-х фазного PWM сигнала управления мотором
 Применяется центрально-симметричная модуляция
 
 Тактирование таймеров производится от частоты шины = 60 МГц
-----------------------------------------------------------------------------------------------------*/
void FTM3_init_motor_PWM(void)
{
  FTM_MemMapPtr FTM    = FTM3_BASE_PTR;
  uint32_t      sc_reg;
  uint8_t       presc  = FTM_PRESC_1;

  // Разрешаем тактирование FTM3
  SIM_SCGC3 |= BIT(25);

  FTM->OUTINIT = 0    // Начальные значения для выходов каналов
                 + LSHIFT(0, 7)  // CH7OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 6)  // CH6OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 5)  // CH5OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 4)  // CH4OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 3)  // CH3OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 2)  // CH2OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 1)  // CH1OI | 0 The initialization value is 0. 1 The initialization value is 1.
                 + LSHIFT(0, 0)  // CH0OI | 0 The initialization value is 0. 1 The initialization value is 1.
  ;

  // Включаем полную функциональность модуля таймера и разрешаем запись во все поля регистров
  FTM->MODE = 0
              + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
              + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
              + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
              + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode | 0 No restrictions. Software and hardware triggers can be used by MOD, CnV, OUTMASK, and FTM counter synchronization.
              + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
              + LSHIFT(1, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
              + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM->CONF = 0
              + LSHIFT(0, 10) // GTBEOUT | Global time base output
              + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
              + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | В режиме отладки сохраняется полная выходов FTM
              + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // Таймер остановлен
  sc_reg = 0
           + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | Сброс в 0
           + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
           + LSHIFT(1, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.0 FTM counter operates in Up Counting mode.
           + LSHIFT(0, 3) // CLKS  | 00 Модуль таймера остановлен
           + LSHIFT(presc, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 001 Divide by 2
  ;
  FTM->SC = sc_reg;

  // В режиме совместимости с TPM установки синхронизации в регистре  FTM->SYNCONF не работают

  FTM->SYNCONF = 0
                 + LSHIFT(0, 20) // HWSOC.      1 A hardware trigger activates the SWOCTRL register synchronization.
                 + LSHIFT(0, 19) // HWINVC.     1 A hardware trigger activates the INVCTRL register synchronization.
                 + LSHIFT(0, 18) // HWOM.       1 A hardware trigger activates the OUTMASK register synchronization.
                 + LSHIFT(0, 17) // HWWRBUF.    1 A hardware trigger activates MOD, CNTIN, and CV registers synchronization.
                 + LSHIFT(0, 16) // HWRSTCNT.   1 A hardware trigger activates the FTM counter synchronization.
                 + LSHIFT(1, 12) // SWSOC.      1 The software trigger activates the SWOCTRL register synchronization.
                 + LSHIFT(1, 11) // SWINVC.     1 The software trigger activates the INVCTRL register synchronization.
                 + LSHIFT(1, 10) // SWOM.       1 The software trigger activates the OUTMASK register synchronization.
                 + LSHIFT(1, 9) // SWWRBUF.    1 The software trigger activates MOD, CNTIN, and CV registers synchronization
                 + LSHIFT(0, 8) // SWRSTCNT.   1 The software trigger activates the FTM counter synchronization. Если 1 то сбрасываем таймер каждый раз когда пишем 1 в SWSYNC
                 + LSHIFT(1, 7) // SYNCMODE.   1 Enhanced PWM synchronization is selected.
                 + LSHIFT(0, 5) // SWOC.       1 SWOCTRL регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(1, 4) // INVC.       1 INVCTRL регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(1, 2) // CNTINC.     1 CNTIN регистр будет обновлен в точке зугрузки при наличии флага синхронизации, если 0 то будет обновлен сразу
                 + LSHIFT(0, 1) // HWTRIGMODE. 1 FTM does not clear the TRIGj bit when the hardware trigger j is detected.
  ;



  FTM->MOD     = FTM_MOTOR_MOD;   // Установка регистра перезагрузки.
  FTM->CNTIN   = 0;    // Начальное значение счетчика
  FTM->CNT     = 0;    // Запись в регистр счетчка любого значения приводит к записи значения из CNTIN и установке начального состояния выходов
  FTM->POL     = 0;    // Полярность у сигналов

  FTM->OUTMASK = 0    // Маскировка реакции выходов на событие совпадения, т.е. маска отключения PWM на заданных выходах
                 + LSHIFT(1, 7)  // CH7OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 6)  // CH6OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 5)  // CH5OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 4)  // CH4OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 3)  // CH3OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 2)  // CH2OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 1)  // CH1OM | 1 Channel output is masked. It is forced to its inactive state.
                 + LSHIFT(1, 0)  // CH0OM | 1 Channel output is masked. It is forced to its inactive state.
  ;
  FTM->COMBINE = 0     // Связанную работу каналов не используем
                       // Установки для  каналов 6 и 7
                 + LSHIFT(0, 30) // FAULTEN3. 1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 29) // SYNCEN3.  1 The PWM synchronization in this pair of channels is enabled. Должна быть обязательно разрешена чтобы можно было делать запись в регистры CnV
                 + LSHIFT(1, 28) // DTEN3.    0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 27) // DECAP3.   0 The dual edge captures are inactive.
                 + LSHIFT(0, 26) // DECAPEN3  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0, 25) // COMP3     1 The channel (7) output is the complement of the channel (6) output. Работает только при COMBINE3=1
                 + LSHIFT(0, 24) // COMBINE3  1 Channels (6) and (7) are combined.
                                 // Установки для  каналов 4 и 5
                 + LSHIFT(0, 22) // FAULTEN2  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 21) // SYNCEN2   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(1, 20) // DTEN2     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 19) // DECAP2    0 The dual edge captures are inactive.
                 + LSHIFT(0, 18) // DECAPEN2  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0, 17) // COMP2     1 The channel (5) output is the complement of the channel (4) output. Работает только при COMBINE2=1
                 + LSHIFT(0, 16) // COMBINE2  1 Channels (4) and (5) are combined.
                                 // Установки для  каналов 2 и 3
                 + LSHIFT(0, 14) // FAULTEN1  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1, 13) // SYNCEN1   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(1, 12) // DTEN1     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0, 11) // DECAP1    0 The dual edge captures are inactive.
                 + LSHIFT(0, 10) // DECAPEN1  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0,  9) // COMP1     1 The channel (3) output is the complement of the channel (2) output. Работает только при COMBINE1=1
                 + LSHIFT(0,  8) // COMBINE1  1 Channels (2) and (3) are combined.
                                 // Установки для  каналов 0 и 1
                 + LSHIFT(0,  6) // FAULTEN0  1 The fault control in this pair of channels is enabled.
                 + LSHIFT(1,  5) // SYNCEN0   1 The PWM synchronization in this pair of channels is enabled.
                 + LSHIFT(1,  4) // DTEN0     0 The deadtime insertion in this pair of channels is disabled.
                 + LSHIFT(0,  3) // DECAP0    0 The dual edge captures are inactive.
                 + LSHIFT(0,  2) // DECAPEN0  0 The dual edge capture mode in this pair of channels is disabled.
                 + LSHIFT(0,  1) // COMP0     1 The channel (1) output is the complement of the channel (0) output. Работает только при COMBINE0=1
                 + LSHIFT(0,  0) // COMBINE0  1 Channels (0) and (1) are combined.
  ;
  FTM->DEADTIME = 0 // Мертове время
                  + LSHIFT(0,  6)  // DTPS  | Deadtime Prescaler Value | 0x Divide the system clock by 1.
                  + LSHIFT(15,  0) // DTVAL | Deadtime Value           | 0.25 мкс
  ;
  FTM->INVCTRL = 0x07;  // Управление взаимным обменом выходов пар каналов
                        // Предварительно устанавливаем инверсию каналов поскольку фазы коммутации начинаются с инверсии

  FTM->FILTER = 0;   // Фильтры для входных каналов не используем


  // Установка работы каналов в режиме PWM выровненному по срезу
  // Включение в 1 в начале периода, сброс в 0 после сработки компаратора

  FTM->CONTROLS[0].CnSC = 0 // Установка выхода в 1 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[1].CnSC = 0 // Установка выхода в 0 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[2].CnSC = 0 // Установка выхода в 1 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[3].CnSC = 0 // Установка выхода в 0 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[4].CnSC = 0 // Установка выхода в 1 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[5].CnSC = 0 // Установка выхода в 0 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[6].CnSC = 0 // Установка выхода в 1 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;
  FTM->CONTROLS[7].CnSC = 0 // Установка выхода в 0 при совпадении
                          + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                          + LSHIFT(0, 6) // CHIE. 0 Disable channel interrupts. Use software polling.
                          + LSHIFT(1, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                          + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                          + LSHIFT(1, 3) // ELSB. Edge or Level Select. ELSB=1,ELSA=0 - установка низкого уровня  при совпадении или счетчик выше
                          + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - установка высокого уровня  при совпадении или счетчик выше
                          + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  // Загрузку по событиям компараторов не используем
  FTM->PWMLOAD = 0
                 + LSHIFT(0, 9)  // LDOK   | Автоматическая загрузка в регистры MOD, CNTIN, и CnV  момент перегрузки таймера. Флаг однократный. Действует до ближайшего события синхронизации.
                 + LSHIFT(0, 7)  // CH7SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 7
                 + LSHIFT(0, 6)  // CH6SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 6
                 + LSHIFT(0, 5)  // CH5SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 5
                 + LSHIFT(0, 4)  // CH4SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 4
                 + LSHIFT(0, 3)  // CH3SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 3
                 + LSHIFT(0, 2)  // CH2SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 2
                 + LSHIFT(0, 1)  // CH1SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 1
                 + LSHIFT(0, 0)  // CH0SEL | Автоматическая загрузка в регистры MOD, CNTIN, и CnV при сработке компаратора канала 0
  ;
  // Выходами програмно не управляем
  FTM->SWOCTRL = 0
                 + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 14) // CH6OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 13) // CH5OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 12) // CH4OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 11) // CH3OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 10) // CH2OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 9)  // CH1OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 8)  // CH0OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 5)  // CH5OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 4)  // CH4OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 3)  // CH3OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 2)  // CH2OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 1)  // CH1OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 0)  // CH0OC  | 1 The channel output is affected by software output control.
  ;

  // Программно активизируем синхроную загрузку регистров по событиям
  FTM->SYNC = 0
              + LSHIFT(1, 7)  // SWSYNC  | PWM Synchronization Software Trigger. The software trigger happens when a 1 is written to this bit. Флаг однократный. Действует до ближайшего события синхронизации.
              + LSHIFT(0, 6)  // TRIG2   | PWM Synchronization Hardware Trigger 2
              + LSHIFT(0, 5)  // TRIG1   | PWM Synchronization Hardware Trigger 1
              + LSHIFT(0, 4)  // TRIG0   | PWM Synchronization Hardware Trigger 0
              + LSHIFT(1, 3)  // SYNCHOM | 0 OUTMASK register is updated with the value of its buffer in all rising edges of the system clock. 1 OUTMASK register is updated with the value of its buffer only by the PWM synchronization.
              + LSHIFT(0, 2)  // REINIT  | FTM Counter Reinitialization by Synchronization | 0 FTM counter continues to count normally. (Не имеет значения в данном режиме таймера)
              + LSHIFT(1, 1)  // CNTMAX  | 1 The maximum loading point is enabled. Один из  двух флагов (CNTMAX, CNTMIN) должен быть обязательно установлен для работы синхронной записи в регистры
              + LSHIFT(1, 0)  // CNTMIN  | 1 The minimum loading point is enabled.
  ;

  FTM->EXTTRIG = 0
                 + LSHIFT(0, 7)  // TRIGF      | Channel Trigger Flag
                 + LSHIFT(1, 6)  // INITTRIGEN | Initialization Trigger Enable
                 + LSHIFT(0, 5)  // CH1TRIG    | Channel 1 Trigger Enable
                 + LSHIFT(0, 4)  // CH0TRIG    | Channel 0 Trigger Enable
                 + LSHIFT(0, 3)  // CH5TRIG    | Channel 5 Trigger Enable
                 + LSHIFT(0, 2)  // CH4TRIG    | Channel 4 Trigger Enable
                 + LSHIFT(0, 1)  // CH3TRIG    | Channel 3 Trigger Enable
                 + LSHIFT(0, 0)  // CH2TRIG    | Channel 2 Trigger Enable
  ;


  FTM->CONTROLS[0].CnV = 0; // Обнулем регистры компараторов для всех каналов
  FTM->CONTROLS[1].CnV = 0;
  FTM->CONTROLS[2].CnV = 0;
  FTM->CONTROLS[3].CnV = 0;
  FTM->CONTROLS[4].CnV = 0;
  FTM->CONTROLS[5].CnV = 0;
  FTM->CONTROLS[6].CnV = 0;
  FTM->CONTROLS[7].CnV = 0;

  //  Install_and_enable_isr(INT_FTM3, FTM3_ISR_PRIO, FTM3_isr);
  //  Запускаем таймер
  FTM->SC = sc_reg | LSHIFT(1, 3); // CLKS  | 01 System clock;

}


/*-----------------------------------------------------------------------------------------------------
 
 \param n 
 \param val 
-----------------------------------------------------------------------------------------------------*/
void FTM3_set_CnV(uint32_t val)
{
  FTM3_C0V = val;
  FTM3_C1V = val;
  FTM3_C2V = val;
  FTM3_C3V = val;
  FTM3_C4V = val;
  FTM3_C5V = val;
}

/*-----------------------------------------------------------------------------------------------------
  Пересчитать константу модуляции таймера в процентное значение 
 
 \param val 
 
 \return int8_t 
-----------------------------------------------------------------------------------------------------*/
int8_t Get_percent_from_modulation(uint32_t val)
{
  uint32_t zv;
  float    v;
  int8_t   r;
  // Находим значение модуляции для нулевого процента
  zv = FTM_MOTOR_MOD / 2;
  if (val < zv)
  {
    v = zv - val;
    v = v * (100.0 + MOD_MARGIN) / zv;
    if (v > 100.0) v = 100.0;
    r = (int8_t)v;
  }
  else
  {
    v = val - zv;
    v = v * (100.0 + MOD_MARGIN) / zv;
    if (v > 100.0) v = 100.0;
    r = (int8_t)-v;
  }
  mot_PWM_lev = r;
  return r;
}

/*-----------------------------------------------------------------------------------------------------
  Установить константу модуляции по заданному значению в процентах
 
 \param val 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_modulation_from_percent(float val)
{
  float    zv = ((float)FTM_MOTOR_MOD / 2.0);
  int32_t  sv;
  uint32_t v;

  if (val < -100.0) val = -100.0;
  if (val > 100.0) val = 100.0;
  val = (zv * val) / (100.0 + MOD_MARGIN);
  sv = (int32_t)(FTM_MOTOR_MOD / 2) - (int32_t)(val);
  v = (uint32_t)sv;
  return v;
}

/*------------------------------------------------------------------------------
  Конфигурирование FTM2 канал 1 и FTM1 для работы в режиме обработки импульсов с 3-х фазного датчика холла 
  Частота счета таймера = 60 МГц/16 =  3.75 МГц  
 
 ------------------------------------------------------------------------------*/
void FTM1_FTM2_init_hall_sens_meas(void)
{
  FTM_MemMapPtr FTM2 = FTM2_BASE_PTR;
  FTM_MemMapPtr FTM1 = FTM1_BASE_PTR;

  SIM_SCGC6 |= BIT(26);  // Разрешаем тактирование FTM2 на AIPS0 (Peripheral bridge 0)
  SIM_SCGC6 |= BIT(25);  // Разрешаем тактирование FTM1 на AIPS0 (Peripheral bridge 0)
  SIM_SCGC3 |= BIT(24);  // Разрешаем тактирование FTM2 на AIPS1 (Peripheral bridge 1)

  // Включаем полную функциональность модуля таймера и разрешаем запись во все поля регистров
  FTM2->MODE = 0
               + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
               + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
               + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
               + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode
               + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
               + LSHIFT(0, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
               + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;
  FTM1->MODE = 0
               + LSHIFT(0, 7) // FAULTIE | Fault Interrupt Enable   |
               + LSHIFT(0, 5) // FAULTM  | Fault Control Mode       | 00 Fault control is disabled for all channels.
               + LSHIFT(0, 4) // CAPTEST | Capture Test Mode Enable
               + LSHIFT(0, 3) // PWMSYNC | PWM Synchronization Mode
               + LSHIFT(1, 2) // WPDIS   | Write Protection Disable | 1 Write protection is disabled.
               + LSHIFT(0, 1) // INIT    | Initialize the Channels Output | When a 1 is written to INIT bit the channels output is initialized according to the state of their corresponding bit in the OUTINIT register
               + LSHIFT(1, 0) // FTMEN   | FTM Enable | 1 All registers including the FTM-specific registers (second set of registers) are available for use with no restrictions.
  ;

  FTM2->CONF  = 0
                + LSHIFT(0, 10) // GTBEOUT | Global time base output
                + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
                + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | В режиме отладки сохраняется полная выходов FTM
                + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;
  FTM1->CONF  = 0
                + LSHIFT(0, 10) // GTBEOUT | Global time base output
                + LSHIFT(0, 9)  // GTBEEN  | Global time base enable
                + LSHIFT(3, 6)  // BDMMODE | Selects the FTM behavior in BDM mode | В режиме отладки сохраняется полная выходов FTM
                + LSHIFT(0, 0)  // NUMTOF  | TOF Frequency | NUMTOF = 0: The TOF bit is set for each counter overflow.
  ;

  // Таймер остановлен
  FTM2->SC = 0
             + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | Сброс в 0
             + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.
             + LSHIFT(0, 3) // CLKS  | 00 Модуль таймера остановлен
             + LSHIFT(FTM2_PRESC_VAL, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 100 Divide by 16
  ;
  FTM2->COMBINE = 0;     // Связанную работу каналов не используем

  // Таймер остановлен
  FTM1->SC = 0
             + LSHIFT(0, 7) // TOF   | 1 FTM counter has overflowed. | Сброс в 0
             + LSHIFT(0, 6) // TOIE  | 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS | 1 FTM counter operates in up-down counting mode.
             + LSHIFT(0, 3) // CLKS  | 00 Модуль таймера остановлен
             + LSHIFT(FTM2_PRESC_VAL, 0) // PS    | Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 100 Divide by 16
  ;
  FTM1->COMBINE = 0;     // Связанную работу каналов не используем

  FTM2->CONTROLS[0].CnSC = 0 // Установка выхода в 0 при совпадении
                           + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                           + LSHIFT(0, 6) // CHIE. Запрещаем прерывания от канала
                           + LSHIFT(0, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                           + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                           + LSHIFT(1, 3) // ELSB. Edge or Level Select.
                           + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - Capture on Rising or Falling Edge
                           + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  FTM2->CONTROLS[1].CnSC = 0 // Установка выхода в 0 при совпадении
                           + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                           + LSHIFT(1, 6) // CHIE. Разрешаем прерывания от канала
                           + LSHIFT(0, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                           + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                           + LSHIFT(1, 3) // ELSB. Edge or Level Select.
                           + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - Capture on Rising or Falling Edge
                           + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  FTM1->CONTROLS[1].CnSC = 0 // Установка выхода в 0 при совпадении
                           + LSHIFT(0, 7) // CHF.  1 A channel event has occurred.
                           + LSHIFT(0, 6) // CHIE. Запрещаем прерывания от канала
                           + LSHIFT(0, 5) // MSB.  Channel Mode Select. 00 - Input Capture, 01- Output Compare, 1X - Edge-Aligned PWM, XX (if CPWMS = 1)- Center-Aligned PWM
                           + LSHIFT(0, 4) // MSA.  Channel Mode Select.
                           + LSHIFT(1, 3) // ELSB. Edge or Level Select.
                           + LSHIFT(1, 2) // ELSA. Edge or Level Select. ELSB=1,ELSA=1 - Capture on Rising or Falling Edge
                           + LSHIFT(0, 0) // DMA.  0 Disable DMA transfers.
  ;

  FTM2->MOD      = FTM2_CAPTURE_MOD;    // Установка регистра перезагрузки.
  FTM2->CNTIN    = 0;         // Начальное значение счетчика
  FTM2->CNT      = 0;         // Запись в регистр счетчка любого значения приводит к записи значения из CNTIN и установке начального состояния выходов

  FTM1->MOD      = FTM2_CAPTURE_MOD;    // Установка регистра перезагрузки.
  FTM1->CNTIN    = 0;         // Начальное значение счетчика
  FTM1->CNT      = 0;         // Запись в регистр счетчка любого значения приводит к записи значения из CNTIN и установке начального состояния выходов

  FTM2->FILTER  = LSHIFT(0xF, 4); // Фильтуем максимально по 16 отсчетов  в канале 1
  FTM2->FILTER  = LSHIFT(0xF, 0); // Фильтуем максимально по 16 отсчетов  в канале 0
  FTM1->FILTER  = LSHIFT(0xF, 4); // Фильтуем максимально по 16 отсчетов  в канале 1

  FTM2->QDCTRL  = 0; // Квадратурный декодер не используем
  FTM1->QDCTRL  = 0; // Квадратурный декодер не используем

  // Запускаем системный тактирующий сигнал на таймер
  FTM2->SC = 0
             + LSHIFT(1, 6) // TOIE. 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS. 1 -FTM counter operates in up-down counting mode.
             + LSHIFT(1, 3) // CLKS. 01 System clock
             + LSHIFT(FTM2_PRESC_VAL, 0) // PS. Prescale Factor Selection. Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 100 Divide by 16
  ;
  // Запускаем системный тактирующий сигнал на таймер
  FTM1->SC = 0
             + LSHIFT(0, 6) // TOIE. 1 Enable TOF interrupts. An interrupt is generated when TOF equals one.
             + LSHIFT(0, 5) // CPWMS. 1 -FTM counter operates in up-down counting mode.
             + LSHIFT(1, 3) // CLKS. 01 System clock
             + LSHIFT(FTM2_PRESC_VAL, 0) // PS. Prescale Factor Selection. Prescale Factor Selection. 000 Divide by 1. Предделитель = 1, 100 Divide by 16
  ;
  Install_and_enable_kernel_isr(INT_FTM2, FTM2_PRIO, Hall_sensor_kernel_isr);
}



