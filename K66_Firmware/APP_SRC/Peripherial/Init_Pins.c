#include "App.h"

void Config_pin(const T_IO_pins_configuration pinc);


#include "K66BLEZ1_MDC01_pins.h"



/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Config_pin(const T_IO_pins_configuration pinc)
{
  pinc.port->PCR[pinc.pin_num] = LSHIFT(pinc.irqc, 16) |
                                 LSHIFT(pinc.lock, 15) |
                                 LSHIFT(pinc.mux, 8) |
                                 LSHIFT(pinc.DSE, 6) |
                                 LSHIFT(pinc.ODE, 5) |
                                 LSHIFT(pinc.PFE, 4) |
                                 LSHIFT(pinc.SRE, 2) |
                                 LSHIFT(pinc.PUPD, 0);

  if (pinc.init == 0) pinc.gpio->PCOR = LSHIFT(1, pinc.pin_num);
  else pinc.gpio->PSOR = LSHIFT(1, pinc.pin_num);
  pinc.gpio->PDDR = (pinc.gpio->PDDR & ~LSHIFT(1, pinc.pin_num)) | LSHIFT(pinc.dir, pinc.pin_num);
}


/*------------------------------------------------------------------------------



 \return int
 ------------------------------------------------------------------------------*/
int Init_pins(void)
{
  int i;

  // Включаем тактирование на всех портах
  SIM_SCGC5 |=   SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

  for (i = 0; i < (sizeof(K66BLEZ1_pins_conf) / sizeof(K66BLEZ1_pins_conf[0])); i++)
  {
    Config_pin(K66BLEZ1_pins_conf[i]);
  }

  return 0;
}

/*-------------------------------------------------------------------------------------------------------------
  1 - снимаем сигнал выборки CS
  0 - устанавливаем сигнал выборки CS
-------------------------------------------------------------------------------------------------------------*/
void Set_MKW40_CS_state(int state)
{
  if ( state == 0 ) PTD_BASE_PTR->PCOR = LSHIFT(1, 11); // Устанавливаем бит в Port Clear Output Register
  else PTD_BASE_PTR->PSOR = LSHIFT(1, 11);              // Устанавливаем бит в Port Set Output Register
}

