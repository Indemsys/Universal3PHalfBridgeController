#include "mqx.h"
#include "bsp.h"
#include "bsp_prv.h"

#define     BIT(n) (1u << n)
#define     LSHIFT(v,n) (((unsigned int)(v) << n))

extern uint32_t ___VECTOR_RAM[];         //Get vector table that was copied to RAM
extern int Init_pins(void);
extern void Init_DMA(void);
void Init_cpu(void);
//-------------------------------------------------------------------------------------------------------
//  Инициализация чипа MK66FN2M0VLQ18 на плате K66BLEZ1
//-------------------------------------------------------------------------------------------------------
int Init_MK66FN2M0VLQ18_K66BLEZ1(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  // Выключать WATCHDOG надо сразу, иначе может возникнуть нестабильный запуск кода после холодного рестарта
  //--------------------------------------------------------------------------------------------------------------------------------------
  WDOG->UNLOCK = 0xC520; // Откроем доступ на запись в регитры управления WDOG
  WDOG->UNLOCK = 0xD928;
  WDOG->STCTRLH = 0
                  + LSHIFT(0x00, 14) // DISTESTWDOG | Allows the WDOG’s functional test mode to be disabled permanently| 0 WDOG functional test mode is not disabled.
                  + LSHIFT(0x00, 12) // BYTESEL[1:0]| This 2-bit field select the byte to be tested ...                | 00 Byte 0 selected
                  + LSHIFT(0x00, 11) // TESTSEL     | Selects the test to be run on the watchdog timer                 | 0 Quick test
                  + LSHIFT(0x00, 10) // TESTWDOG    | Puts the watchdog in the functional test mode                    |
                  + LSHIFT(0x01, 8)  // Reserved    |
                  + LSHIFT(0x01, 7)  // WAITEN      | Enables or disables WDOG in wait mode.                           | 1 WDOG is enabled in CPU wait mode.
                  + LSHIFT(0x01, 6)  // STOPEN      | Enables or disables WDOG in stop mode                            | 1 WDOG is enabled in CPU stop mode.
                  + LSHIFT(0x00, 5)  // DBGEN       | Enables or disables WDOG in Debug mode                           | 0 WDOG is disabled in CPU Debug mode.
                  + LSHIFT(0x01, 4)  // ALLOWUPDATE | Enables updates to watchdog write once registers                 | 1 WDOG write once registers can be unlocked for updating
                  + LSHIFT(0x00, 3)  // WINEN       | Enable windowing mode.                                           | 0 Windowing mode is disabled.
                  + LSHIFT(0x00, 2)  // IRQRSTEN    | Used to enable the debug breadcrumbs feature                     | 0 WDOG time-out generates reset only.
                  + LSHIFT(0x01, 1)  // CLKSRC      | Selects clock source for the WDOG                                | 1 WDOG clock sourced from alternate clock source
                  + LSHIFT(0x00, 0)  // WDOGEN      | Enables or disables the WDOG’s operation                         | 1 WDOG is enabled.
  ;

  #if MQX_ENABLE_LOW_POWER
  /* Reset from LLWU wake up source */
  if (_lpm_get_reset_source() == MQX_RESET_SOURCE_LLWU)
  {
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
  }
  #endif


  Init_pins();
  Init_cpu();
  Init_DMA();  //DMA также может инициализироваться в файле init_bsp.c  Проверять!!!
//  Debug_LED_pulse_gen();

  return 1;
}


//-------------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------------
void Init_cpu(void)
{
  SCB_MemMapPtr  SCB  = SystemControl_BASE_PTR;
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  SIM_MemMapPtr  SIM  = SIM_BASE_PTR;
  RTC_MemMapPtr  RTC  = RTC_BASE_PTR;
  MCG_MemMapPtr  MCG  = MCG_BASE_PTR;
  PIT_MemMapPtr  PIT  = PIT_BASE_PTR;
  FMC_MemMapPtr  FMC  = FMC_BASE_PTR;
  CRC_MemMapPtr  CRC  = CRC_BASE_PTR;
  RCM_MemMapPtr  RCM  = RCM_BASE_PTR;
  PMC_MemMapPtr  PMC  = PMC_BASE_PTR;

  MPU_BASE_PTR->CESR = 0;    // 0 MPU is disabled. All accesses from all bus masters are allowed.




  //--------------------------------------------------------------------------------------------------------------------------------------
  SIM->SCGC6 |= BIT(29); // RTC | RTC clock gate control | 1 Clock is enabled.
  if ((RTC->CR & BIT(8)) == 0u) // If 0, 32.768 kHz oscillator is disabled.
  {
    RTC->CR = 0
              + LSHIFT(0x00, 13) // SC2P | Oscillator 2pF load configure  | 0 Disable the load.
              + LSHIFT(0x00, 12) // SC4P | Oscillator 4pF load configure  | 0 Disable the load.
              + LSHIFT(0x00, 11) // SC8P | Oscillator 8pF load configure  | 0 Disable the load.
              + LSHIFT(0x00, 10) // SC16P| Oscillator 16pF load configure | 0 Disable the load.
              + LSHIFT(0x00, 9)  // CLKO | Clock Output                   | 1 The 32kHz clock is not output to other peripherals
              + LSHIFT(0x01, 8)  // OSCE | Oscillator Enable              | 1 32.768 kHz oscillator is enabled.
              + LSHIFT(0x00, 3)  // UM   | Update Mode                    | 0 Registers cannot be written when locked.
              + LSHIFT(0x00, 2)  // SUP  | Supervisor Access              | 0 Non-supervisor mode write accesses are not supported and generate a bus error.
              + LSHIFT(0x00, 1)  // WPE  | Wakeup Pin Enable              | 0 Wakeup pin is disabled.
              + LSHIFT(0x00, 0)  // SWR  | Software Reset                 | 0 No effect
    ;
  }




  //--------------------------------------------------------------------------------------------------------------------------------------
  // Загрузка регистров из области чипа с заводскими установками
  if (*((uint8_t *)0x03FFU) != 0xFFU)
  {
    MCG->C3 = *((uint8_t *)0x03FFU);
    MCG->C4 = (MCG_C4 & 0xE0U) | ((*((uint8_t *)0x03FEU)) & 0x1FU);
  }

  //--------------------------------------------------------------------------------------------------------------------------------------
  SIM->CLKDIV1 = 0
                 + LSHIFT(0x00, 28) // OUTDIV1 | Divide value for the core/system clock                                  | 0000 Divide-by-1.  | core/system clock = 180 MHz = CPU_CORE_CLK_HZ_CONFIG_3
                 + LSHIFT(0x02, 24) // OUTDIV2 | Divide value for the peripheral clock                                   | 0010 Divide-by-3.  | bus clock = 60 MHz = CPU_BUS_CLK_HZ_CONFIG_3
                 + LSHIFT(0x06, 20) // OUTDIV3 | Divide value for the FlexBus clock driven to the external pin (FB_CLK). | 0110 Divide-by-7.  | FlexBus clock = 25.7 MHz = CPU_FLEXBUS_CLK_HZ_CONFIG_3
                 + LSHIFT(0x06, 16) // OUTDIV4 | Divide value for the flash clock                                        | 0110 Divide-by-7.  | flash clock = 25.7 MHz = CPU_FLASH_CLK_HZ_CONFIG_3
  ;

  SIM->CLKDIV4 = 0
                 + LSHIFT(0x01, 01) // TRACEDIV   | Trace clock divider divisor
                 + LSHIFT(0x00, 00) // TRACEFRAC  | Trace clock divider fraction
  ;
  //
  SIM->SOPT2 = 0
               + LSHIFT(0x00, 28) // ESDHCSRC    | ESDHC perclk source select            | 00 Core/system clock
               + LSHIFT(0x01, 26) // LPUARTSRC   | LPUART clock source select            | 01 MCGFLLCLK , or MCGPLLCLK, or IRC48M, or USB1 PFD
               + LSHIFT(0x01, 24) // TPMSRC      | TPM clock source select               | 01 MCGFLLCLK , or MCGPLLCLK, or IRC48M, or USB1 PFD
               + LSHIFT(0x00, 20) // TIMESRC     | Ethernet timestamp clock source select| 00 System platform clock
               + LSHIFT(0x00, 19) // RMIISRC     | RMII clock source select              | 0 EXTAL clock
               + LSHIFT(0x01, 18) // USBSRC      | USB clock source select               | 1 MCGFLLCLK, or MCGPLLCLK, or IRC48M, or USB1 PFD
               + LSHIFT(0x01, 16) // PLLFLLSEL   | PLL/FLL clock select                  | 01 MCGPLL0CLK !!!
               + LSHIFT(0x01, 12) // TRACECLKSEL | Debug trace clock select              | 0 MCGCLKOUT
               + LSHIFT(0x03, 8)  // FBSL        | Flexbus security level                | 11 Off-chip op code accesses and data accesses are allowed.
               + LSHIFT(0x02, 5)  // CLKOUTSEL   | Clock out select                      | 010 Flash ungated clock
               + LSHIFT(0x00, 4)  // RTCCLKOUTSEL| RTC clock out select                  | 0 RTC 1 Hz clock drives RTC CLKOUT.
               + LSHIFT(0x00, 1)  // USBREGEN    | USB PHY PLL Regulator Enable          | 1 USB PHY PLL Regulator enabled.
               + LSHIFT(0x00, 0)  // USBSLSRC    | USB Slow Clock Source                 | 0 MCGIRCLK
  ;
  SIM->SOPT1 = 0
               + LSHIFT(0x01, 31) // USBREGEN  | USB voltage regulator enable
               + LSHIFT(0x00, 30) // USBSSTBY  | UUSB voltage regulator in standby mode during Stop, VLPS, LLS or VLLS
               + LSHIFT(0x00, 29) // USBVSTBY  | USB voltage regulator in standby mode during VLPR or VLPW
               + LSHIFT(0x02, 18) // OSC32KSEL | 32K oscillator clock select | 10 RTC 32.768kHz oscillator
  ;


  //--------------------------------------------------------------------------------------------------------------------------------------

  MCG->C7 = 0; // OSCSEL | MCG OSC Clock Select | 0 Selects System Oscillator (OSCCLK). 1 Selects 32 kHz RTC Oscillator.
               // При подключении к FLL синала 32 kHz RTC не происходит установка битов IREFS и CLKS
               // Поэтому подключается сигнал с OSC0 который в данной схеме принимает внешний тактовый сигнал 50 МГц


  // Конфигурируем осцилятор 0 (кварц 32 МГц)
  MCG->C2 = 0
            + LSHIFT(0x00, 7) // LOCRE0 | Loss of Clock Reset Enable     | 0 Interrupt request is generated on a loss of OSC0 external reference clock.
            + LSHIFT(0x00, 6) // FCFTRIM| Fast Internal Reference Clock Fine Trim     | FCFTRIM controls the smallest adjustment of the fast internal reference clock frequency
            + LSHIFT(0x02, 4) // RANGE0 | Frequency Range Select         | 1X Encoding 2 — Very high frequency range selected for the crystal oscillator ..
            + LSHIFT(0x00, 3) // HGO0   | High Gain Oscillator Select    | 1 Configure crystal oscillator for high-gain operation.
            + LSHIFT(0x01, 2) // EREFS0 | External Reference Select      | 1 Oscillator requested.
            + LSHIFT(0x00, 1) // LP     | Low Power Select               | 0 FLL (or PLL) is not disabled in bypass modes.
            + LSHIFT(0x01, 0) // IRCS   | Internal Reference Clock Select| 1 Fast internal reference clock selected.
  ;


  // Установка емкостей на осциляторе 0
  OSC_CR = 0
           + LSHIFT(0x01, 7) // ERCLKEN  | External Reference Enable (OSCERCLK)       | 1 External reference clock is enabled.
           + LSHIFT(0x00, 5) // EREFSTEN | External Reference Stop Enable             | 0 External reference clock is disabled in Stop mode.
           + LSHIFT(0x00, 3) // SC2P     | Oscillator 2  pF Capacitor Load Configure  | 1 Add 2 pF capacitor to the oscillator load.
           + LSHIFT(0x00, 2) // SC4P     | Oscillator 4  pF Capacitor Load Configure  | 1 Add 4 pF capacitor to the oscillator load.
           + LSHIFT(0x00, 1) // SC8P     | Oscillator 8  pF Capacitor Load Configure  | 1 Add 8 pF capacitor to the oscillator load.
           + LSHIFT(0x01, 0) // SC16P    | Oscillator 16 pF Capacitor Load Configure  | 1 Add 16 pF capacitor to the oscillator load.
  ;

  // Переход в режим FBE - FLL Bypassed External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C1 = 0
            + LSHIFT(0x02, 6) // CLKS     | Clock Source Select             | 10 Encoding 2 — External reference clock is selected.
            + LSHIFT(0x03, 3) // FRDIV    | FLL External Reference Divider  | 011 If RANGE 0 = 0 or OSCSEL=1 , Divide Factor is 32; for all other RANGE 0 values, Divide Factor is 1024.
            + LSHIFT(0x00, 2) // IREFS    | Internal Reference Select       | 0 External reference clock is selected.
            + LSHIFT(0x01, 1) // IRCLKEN  | Internal Reference Clock Enable | 1 MCGIRCLK active.
            + LSHIFT(0x00, 0) // IREFSTEN | Internal Reference Stop Enable  | 0 Internal reference clock is disabled in Stop mode.
  ;

  // Настройка FLL
  MCG->C4 = 0
            + LSHIFT(0x00, 7) // DMX32    | DCO Maximum Frequency with 32.768 kHz Reference  | 0 DCO has a default range of 25%.
            + LSHIFT(0x00, 5) // DRST_DRS | DCO Range Select                                 | 00 Encoding 0 — Low range (reset default).
            + LSHIFT(0x00, 1) // FCTRIM   | Fast Internal Reference Clock Trim Setting       |
            + LSHIFT(0x00, 0) // SCFTRIM  | Slow Internal Reference Clock Fine Trim          |
  ;


  //
  // Конфигурация PLL0 на 180 МГц
  MCG->C5 = 0
            + LSHIFT(0x00, 6) // PLLCLKEN  | PLL Clock Enable                | 0 MCGPLL0CLK and MCGPLL0CLK2X are inactive
            + LSHIFT(0x00, 5) // PLLSTEN   | PLL Stop Enable                 | 0 MCGPLL0CLK and MCGPLL0CLK2X are disabled in any of the Stop modes.
            + LSHIFT(0x00, 0) // PRDIV     | PLL External Reference Divider  | 011 Divide Factor 4. Selects the amount to divide down the external reference clock for the PLL0. The resulting frequency must be in the range of 8 MHz to 16 MHz.
  ;

  MCG->C6 = 0
            + LSHIFT(0x00, 7) // LOLIE0  | Loss of Lock Interrrupt Enable | 0 No interrupt request is generated on loss of lock.
            + LSHIFT(0x00, 6) // PLLS    | PLL Select                     | 0 FLL is selected.
            + LSHIFT(0x00, 5) // CME0    | Clock Monitor Enable           | 0 External clock monitor is disabled for OSC0.
            + LSHIFT(0x0E, 0) // VDIV    | VCO Divider                    | Умножаем на 30 (чтобы получить 180 МГц от 12 МГц кварца)
  ;


  MCG->C11 = 0
             + LSHIFT(0x00, 4) // PLLCS      | PLL Clock Select                | 0 PLL0 output clock is selected
  ;


  while ((MCG->S & BIT(1)) == 0) // OSCINIT0 Ждем стабилизации осцилятора OSC0
  {
  }
  while ((MCG->S & BIT(4)) != 0) // IREFST Ждем пока FLL начнет работать от внешнего источника тактирования
  {
  }
  while ((MCG->S & 0x0CU) != 0x08U)  // CLKST  Ждем пока мультиплексор CLKS установиться в режим FBE - FLL Bypassed External
  {
  }

  // Переход в режим PBE - PLL Bypassed External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C6 = 0
            + LSHIFT(0x00, 7) // LOLIE0  | Loss of Lock Interrrupt Enable | 0 No interrupt request is generated on loss of lock.
            + LSHIFT(0x01, 6) // PLLS    | PLL Select                     | 1 PLLCS output clock is selected
            + LSHIFT(0x00, 5) // CME0    | Clock Monitor Enable           | 0 External clock monitor is disabled for OSC0.
            + LSHIFT(0x0E, 0) // VDIV0   | VCO0 Divider                   | Умножаем на 30 (чтобы получить 180 МГц от 12 МГц кварца)
  ;


  while ((MCG->S & 0x0CU) != 0x08U)  // CLKST  Ждем пока мультиплексор CLKS установиться в режим FBE - FLL Bypassed External
  {
  }

  while ((MCG->S & BIT(6)) == 0x00U) // LOCK0 Ждем пока PLL0 защелкнется
  {
  }

  // Переход в режим PEE - PLL Engaged External ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MCG->C1 = 0
            + LSHIFT(0x00, 6) // CLKS     | Clock Source Select             | 00 Encoding 0 — Output of FLL or PLLCS is selected (depends on PLLS control bit).
            + LSHIFT(0x03, 3) // FRDIV    | FLL External Reference Divider  | 101 If RANGE 0 = 0 or OSCSEL=1 , Divide Factor is 32; for all other RANGE 0 values, Divide Factor is 1024.
            + LSHIFT(0x00, 2) // IREFS    | Internal Reference Select       | 0 External reference clock is selected.
            + LSHIFT(0x01, 1) // IRCLKEN  | Internal Reference Clock Enable | 1 MCGIRCLK active.
            + LSHIFT(0x00, 0) // IREFSTEN | Internal Reference Stop Enable  | 0 Internal reference clock is disabled in Stop mode.
  ;

  while ((MCG->S & 0x0CU) != 0x0CU)  // CLKST  Ждем пока мультиплексор CLKS установиться в режим PEE - PLL Engaged External
  {
  }

  MCG->C6 |= BIT(5);     // CME0 = 1 | 1 External clock monitor is enabled for OSC0.


  //--------------------------------------------------------------------------------------------------------------------------------------
  // Установки  Reset Control Module (RCM)
  RCM->RPFW = 0
              + LSHIFT(0x1F, 0) // RSTFLTSEL | Selects the reset pin bus clock filter width.| 11111 Bus clock filter count is 32
  ;
  RCM->RPFC = 0
              + LSHIFT(0x00, 2) // RSTFLTSS  | Selects how the reset pin filter is enabled in STOP and VLPS modes. | 0 All filtering disabled
              + LSHIFT(0x01, 0) // RSTFLTSRW | Selects how the reset pin filter is enabled in run and wait modes.  | 01 Bus clock filter enabled for normal operation
  ;

  if ((PMC->REGSC & BIT(3)) != 0) PMC->REGSC |= BIT(3); // Сброс бита ACKISO, чтобы разблокировать некоторую периферию

  //--------------------------------------------------------------------------------------------------------------------------------------
  // Установки Power Management Controller (PMC)
  PMC->REGSC = 0
               + LSHIFT(0x00, 3) // ACKISO | Acknowledge Isolation | 0 Peripherals and I/O pads are in normal run state|
                                 //          Writing one to this bit when it is set releases the I/O pads and certain peripherals to their normal run mode state
               + LSHIFT(0x00, 0) // BGBE   | Bandgap Buffer Enable | 0 Bandgap buffer not enabled
  ;


  PMC->LVDSC1 = 0
                + LSHIFT(0x01, 6) // LVDACK | Low-Voltage Detect Acknowledge     | This write-only bit is used to acknowledge low voltage detection errors (write 1 to clear LVDF). Reads always return 0.
                + LSHIFT(0x00, 5) // LVDIE  | Low-Voltage Detect Interrupt Enable| 0 Hardware interrupt disabled (use polling)
                + LSHIFT(0x01, 4) // LVDRE  | Low-Voltage Detect Reset Enable    | 1 Force an MCU reset when LVDF = 1
                + LSHIFT(0x00, 0) // LVDV   | Low-Voltage Detect Voltage Select  | 00 Low trip point selected (V LVD = V LVDL )
  ;

  PMC->LVDSC2 = 0
                + LSHIFT(0x01, 6) // LVWACK | Low-Voltage Warning Acknowledge      |
                + LSHIFT(0x00, 5) // LVWIE  | Low-Voltage Warning Interrupt Enable | 0 Hardware interrupt disabled (use polling)
                + LSHIFT(0x00, 0) // LVWV   | Low-Voltage Warning Voltage Select   | 00 Low trip point selected (V LVW = V LVW1 )
  ;


  //--------------------------------------------------------------------------------------------------------------------------------------
  // Инициализация Periodic Interrupt timer
  SIM->SCGC6 |= BIT(23); // PIT | PIT clock gate control | 1 Clock is enabled.

  PIT->MCR  = 0
              + LSHIFT(0x00, 1) // MDIS | Module Disable | 0 Clock for PIT Timers is enabled.
              + LSHIFT(0x01, 0) // FRZ  | Freeze         | 1 Timers are stopped in debug mode.
  ;



  //--------------------------------------------------------------------------------------------------------------------------------------
  // Инициализация Flash Access Protection Register для разрешения доступа к Flash по DMA и от прочих мастеров
  FMC->PFAPR = 0
               + LSHIFT(0x00, 23) // M7PFD     | 1 Prefetching for this master is disabled.                       7 ( )
               + LSHIFT(0x01, 22) // M6PFD     | 1 Prefetching for this master is disabled.                       6 (USB HS/FS/LS OTG)
               + LSHIFT(0x00, 21) // M5PFD     | 1 Prefetching for this master is disabled.                       5 (SDHC)
               + LSHIFT(0x01, 20) // M4PFD     | 1 Prefetching for this master is disabled.                       4 (USB FS/LS OTG)
               + LSHIFT(0x01, 19) // M3PFD     | 0 Prefetching for this master is enabled.                        3 (Ethernet)
               + LSHIFT(0x01, 18) // M2PFD     | 0 Prefetching for this master is enabled.                        2 (DMA, EzPort)
               + LSHIFT(0x01, 17) // M1PFD     | 0 Prefetching for this master is enabled.                        1 (ARM core system bus)
               + LSHIFT(0x01, 16) // M0PFD     | 0 Prefetching for this master is enabled.                        0 (ARM core code bus)
               + LSHIFT(0x00, 14) // M7AP[1:0] | 00 No access may be performed by this master.                    7 ( )
               + LSHIFT(0x03, 12) // M6AP[1:0] | 00 Both read and write accesses may be performed by this master  6 (USB HS/FS/LS OTG)
               + LSHIFT(0x00, 10) // M5AP[1:0] | 00 No access may be performed by this master.                    5 (SDHC)
               + LSHIFT(0x03, 8)  // M4AP[1:0] | 11 Both read and write accesses may be performed by this master  4 (USB FS/LS OTG)
               + LSHIFT(0x03, 6)  // M3AP[1:0] | 11 Both read and write accesses may be performed by this master  3 (Ethernet)
               + LSHIFT(0x03, 4)  // M2AP[1:0] | 11 Both read and write accesses may be performed by this master  2 (DMA, EzPort)
               + LSHIFT(0x03, 2)  // M1AP[1:0] | 11 Both read and write accesses may be performed by this master  1 (ARM core system bus)
               + LSHIFT(0x03, 0)  // M0AP[1:0] | 11 Both read and write accesses may be performed by this master  0 (ARM core code bus)
  ;

  SIM->SCGC6 |= BIT(18); // Включаем модуль CRC

  SIM->SCGC4 |= BIT(19); // Включаем модуль CMP (аналоговые компараторы)


  MCM_CR = 0
           + LSHIFT(0x00, 30)  // SRAMLWP | SRAM_L Write Protect         | When this bit is set, writes to SRAM_L array generates a bus error.
           + LSHIFT(0x01, 28)  // SRAMLAP | SRAM_L arbitration priority  | 01 Special round robin (favors SRAM backoor accesses over the processor)
           + LSHIFT(0x00, 26)  // SRAMUWP | SRAM_U write protect         | When this bit is set, writes to SRAM_U array generates a bus error.
           + LSHIFT(0x01, 24)  // SRAMUAP | SRAM_U arbitration priority  | 01 Special round robin (favors SRAM backoor accesses over the processor)
  ;

  PMC_REGSC = BIT(0); // BGBE | 1 Bandgap buffer enabled
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void _bsp_get_unique_identificator(unsigned int *uid)
{
  SIM_MemMapPtr  SIM  = SIM_BASE_PTR;

  uid[0] = SIM->UIDH;
  uid[1] = SIM->UIDMH;
  uid[2] = SIM->UIDML;
  uid[3] = SIM->UIDL;
}

#define __NVIC_PRIO_BITS          4

/*------------------------------------------------------------------------------



 \param IRQn
 ------------------------------------------------------------------------------*/
void NVIC_EnableIRQ(int32_t irq_index)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  int32_t        IRQn = irq_index - 16;
  if  (IRQn < 0) return; // Системные исключения нельзя не разрешить, не запретить
  NVIC->ISER[(uint32_t)((int32_t)IRQn) >> 5] = (uint32_t)(1 << ((uint32_t)((int32_t)IRQn) & (uint32_t)0x1F)); /* enable interrupt */
}

/*-----------------------------------------------------------------------------------------------------
 
 \param IRQn 
-----------------------------------------------------------------------------------------------------*/
void NVIC_DisableIRQ(int32_t irq_index)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  int32_t        IRQn = irq_index - 16;
  if  (IRQn < 0) return; // Системные исключения нельзя не разрешить, не запретить
  NVIC->ICER[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}

/*------------------------------------------------------------------------------



 \param irq_index
 \param priority: высший приоритет = 0, низший = 15
 ------------------------------------------------------------------------------*/
void NVIC_SetPriority(int32_t irq_index, uint32_t priority)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  SCB_MemMapPtr  SCB  = SystemControl_BASE_PTR;
  int32_t        IRQn = irq_index - 16;

  if (IRQn < 0)
  {
    SCB->SHP[((uint32_t)(IRQn)&0xF) - 4] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff); } /* set Priority for Cortex-M  System Interrupts */
  else
  {
    NVIC->IP[(uint32_t)(IRQn)] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);    }        /* set Priority for device specific Interrupts  */
}

/*-----------------------------------------------------------------------------------------------------
 
 \param irq_index 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t NVIC_GetPendingIRQ(int32_t irq_index)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  int32_t        IRQn = irq_index - 16;
  return ((uint32_t)(((NVIC->ISPR[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
}

/*-----------------------------------------------------------------------------------------------------
 
 \param irq_index 
-----------------------------------------------------------------------------------------------------*/
void NVIC_SetPendingIRQ(int32_t irq_index)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR;
  int32_t        IRQn = irq_index - 16;
  NVIC->ISPR[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}

/*-----------------------------------------------------------------------------------------------------
 
 \param irq_index 
-----------------------------------------------------------------------------------------------------*/
void NVIC_ClearPendingIRQ(int32_t irq_index)
{
  NVIC_MemMapPtr NVIC = NVIC_BASE_PTR; 
  int32_t        IRQn = irq_index - 16;
  NVIC->ICPR[(((uint32_t)(int32_t)IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL));
}
