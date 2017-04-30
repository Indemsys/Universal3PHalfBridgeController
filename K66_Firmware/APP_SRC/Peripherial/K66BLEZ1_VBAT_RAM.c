// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.01
// 09:28:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*------------------------------------------------------------------------------
   Проверить валидность данных в VBAT RAM

   Возвращает MQX_OK если данные валидные
 ------------------------------------------------------------------------------*/
uint32_t K66BLEZ1_VBAT_RAM_validation(void)
{
  uint32_t i;
  uint16_t crc;
  uint16_t *ram = (uint16_t *)VBAT_RAM_ptr;
  crc = Get_CRC_of_block(VBAT_RAM_ptr, sizeof(T_VBAT_RAM) - 2, 0xFFFF);

  if (crc == ram[VBAT_RAM_WRD_SZ * 2 - 1])
  {
    return MQX_OK;
  }
  return MQX_ERROR;
}

/*------------------------------------------------------------------------------
   Подписать контрольной суммой данные в VBAT RAM
   Контрольная сумма записывается в последнее 2-х байтное слово VBAT RAM
 ------------------------------------------------------------------------------*/
void K66BLEZ1_VBAT_RAM_sign_data(void)
{
  uint32_t i;
  uint16_t crc;
  uint16_t *ram = (uint16_t *)VBAT_RAM_ptr;
  crc = Get_CRC_of_block(VBAT_RAM_ptr, sizeof(T_VBAT_RAM) - 2, 0xFFFF);

  ram[VBAT_RAM_WRD_SZ * 2 - 1] = crc;

}

