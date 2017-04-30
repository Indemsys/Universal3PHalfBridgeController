// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.04.29
// 14:38:37
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*-------------------------------------------------------------------------------------------------------------
  Инициализация DAC0 и установка в 0
-------------------------------------------------------------------------------------------------------------*/
void Init_DAC0(void)
{
  DAC_MemMapPtr DAC0 = DAC0_BASE_PTR;
  SIM_MemMapPtr SIM = SIM_BASE_PTR;

  SIM->SCGC2 |= BIT(12);  // 12BDAC0 clock gate control

  DAC0->C0 = 0
             + LSHIFT(1, 7) // DACEN     | Starts the Programmable Reference Generator operation
             + LSHIFT(1, 6) // DACRFS    | DAC Reference Select. VDDA is connected to the DACREF_2 input.
             + LSHIFT(0, 5) // DACTRGSEL | DAC Trigger Select
             + LSHIFT(0, 4) // DACSWTRG  | DAC Software Trigger
             + LSHIFT(0, 3) // LPEN      | DAC Low Power Control
             + LSHIFT(0, 2) // DACBWIEN  | DAC Buffer Watermark Interrupt Enable
             + LSHIFT(0, 1) // DACBTIEN  | DAC Buffer Read Pointer Top Flag Interrupt Enable
             + LSHIFT(0, 0) // DACBBIEN  | DAC Buffer Read Pointer Bottom Flag Interrupt Enable
  ;
  DAC0->C1 = 0
             + LSHIFT(0, 7) // DMAEN     | DMA Enable Select
             + LSHIFT(0, 3) // DACBFWM   | DAC Buffer Watermark Select
             + LSHIFT(0, 1) // DACBFMD   | DAC Buffer Work Mode Select
             + LSHIFT(0, 0) // DACBFEN   | DAC Buffer Enable
  ;
  DAC0->C2 = 0
             + LSHIFT(0, 4) // DACBFRP  | DAC Buffer Read Pointer
             + LSHIFT(0, 0) // DACBFUP  | DAC Buffer Upper Limit
  ;

  DAC0->DAT[0].DATH=0;
  DAC0->DAT[0].DATL=0;
}

/*-------------------------------------------------------------------------------------------------------------
  Инициализация DAC1 и установка в 0
-------------------------------------------------------------------------------------------------------------*/
void Init_DAC1(void)
{
  DAC_MemMapPtr DAC1 = DAC1_BASE_PTR;
  SIM_MemMapPtr SIM = SIM_BASE_PTR;

  SIM->SCGC2 |= BIT(13);  // 12BDAC1 clock gate control

  DAC1->C0 = 0
             + LSHIFT(1, 7) // DACEN     | Starts the Programmable Reference Generator operation
             + LSHIFT(1, 6) // DACRFS    | DAC Reference Select. VDDA is connected to the DACREF_2 input.
             + LSHIFT(0, 5) // DACTRGSEL | DAC Trigger Select
             + LSHIFT(0, 4) // DACSWTRG  | DAC Software Trigger
             + LSHIFT(0, 3) // LPEN      | DAC Low Power Control
             + LSHIFT(0, 2) // DACBWIEN  | DAC Buffer Watermark Interrupt Enable
             + LSHIFT(0, 1) // DACBTIEN  | DAC Buffer Read Pointer Top Flag Interrupt Enable
             + LSHIFT(0, 0) // DACBBIEN  | DAC Buffer Read Pointer Bottom Flag Interrupt Enable
  ;
  DAC1->C1 = 0
             + LSHIFT(0, 7) // DMAEN     | DMA Enable Select
             + LSHIFT(0, 3) // DACBFWM   | DAC Buffer Watermark Select
             + LSHIFT(0, 1) // DACBFMD   | DAC Buffer Work Mode Select
             + LSHIFT(0, 0) // DACBFEN   | DAC Buffer Enable
  ;
  DAC1->C2 = 0
             + LSHIFT(0, 4) // DACBFRP  | DAC Buffer Read Pointer
             + LSHIFT(0, 0) // DACBFUP  | DAC Buffer Upper Limit
  ;

  DAC1->DAT[0].DATH=0;
  DAC1->DAT[0].DATL=0;
}

/*-------------------------------------------------------------------------------------------------------------
  Установка значения в DAC0
  Значение является 12-и битной величиной
-------------------------------------------------------------------------------------------------------------*/
void Set_DAC0_val(uint16_t val)
{
  DAC_MemMapPtr DAC0 = DAC0_BASE_PTR;
  DAC0->DAT[0].DATH= val >> 8;
  DAC0->DAT[0].DATL= val & 0xFF;
}

/*-------------------------------------------------------------------------------------------------------------
  Получение значения из DAC0
  Значение является 12-и битной величиной
-------------------------------------------------------------------------------------------------------------*/
uint16_t Pin_DAC0_state(void)
{
  DAC_MemMapPtr DAC0 = DAC0_BASE_PTR;
  uint16_t val;

  val = (DAC0->DAT[0].DATH << 8) | (DAC0->DAT[0].DATL & 0xFF);
  return val;
}
/*-------------------------------------------------------------------------------------------------------------
  Установка значения в DAC1
  Значение является 12-и битной величиной
-------------------------------------------------------------------------------------------------------------*/
void Set_DAC1_val(uint16_t val)
{
  DAC_MemMapPtr DAC1 = DAC1_BASE_PTR;
  DAC1->DAT[0].DATH= val >> 8;
  DAC1->DAT[0].DATL= val & 0xFF;
}

/*-------------------------------------------------------------------------------------------------------------
  Получение значения из DAC1
  Значение является 12-и битной величиной
-------------------------------------------------------------------------------------------------------------*/
uint16_t Pin_DAC1_state(void)
{
  DAC_MemMapPtr DAC1 = DAC1_BASE_PTR;
  uint16_t val;

  val = (DAC1->DAT[0].DATH << 8) | (DAC1->DAT[0].DATL & 0xFF);
  return val;
}

