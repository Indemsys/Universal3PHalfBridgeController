// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 15:08:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define MKW_BUF_MAX_SZ 128
static uint8_t mkw_buf[MKW_BUF_MAX_SZ];

/*------------------------------------------------------------------------------



 \param parameter
 ------------------------------------------------------------------------------*/
void Task_MKW40(uint32_t parameter)
{

  Init_MKW40_channel();

  mkw_buf[0] = 0x55;
  mkw_buf[1] = 0x01;

  for(;;)
  {

    MKW40_SPI_send_buf(mkw_buf, 2);
    _time_delay_ticks(1);
  }


}


