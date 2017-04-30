// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017.03.15
// 11:46:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

static uint32_t portb_pcr3;

// Идентификаторы слов синхронизаторов
#define H2D_SYNC_WR 0x43211234 //
#define H2D_SYNC_RD 0x87655678 //
#define D2H_SYNC    0xABCDDCBA //

// Хидеры пакетов отправляемых хосту
#define ACK_PACKET              0x13
#define RD_PACKET               0x14


// Хидеры пакетов получаемых от хоста
#define CMD_WR_VAL              0x01
#define CMD_RD_VAL              0x02
#define CMD_UPDATE_RCOUNT_OFFS  0x03
#define CMD_UPDATE_ALL_DEVS     0x04
#define CMD_SAVE_APP_SETTINGS   0x05
#define CMD_RESET               0x06

#define MAX_EENC_BUF_SZ  32
uint32_t        ebuf_w[MAX_EENC_BUF_SZ];
uint32_t        ebuf_r[MAX_EENC_BUF_SZ];

uint8_t         ext_enc_pack[6];
/*-----------------------------------------------------------------------------------------------------
  Обрабтчик прерывания по сигналу на порту B. Нарастающий фронт сигнала READY говорит о том что внешний энкодер подготовил данные для чтения 
 
 \param user_isr_ptr 
-----------------------------------------------------------------------------------------------------*/
static void ExtEnc_IRQHandler(void *user_isr_ptr)
{
  PORTB_ISFR = 0xFFFFFFFF;
  U3HB01_Rot_post_evt(EVT_ROTATION_ENC_DAT_READY);
}


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void ExtEnc_init(void)
{
  // Разрешаем и конфигурируем прерываний по линии READY от внешнего энкодера
  PORTB_ISFR = 0xFFFFFFFF;         // Сбросим флаги прерываний на порту D
  NVIC_ClearPendingIRQ(INT_PORTB); // Сбросим запрос на прерывание в NVIC от порта D

  // Установим свойство линии PTB3 для генерации прерываний
  PORTB_PCR3 =  LSHIFT(IRQ_IRE,   16) |
               LSHIFT(0,         15) |
               LSHIFT(GPIO,      8) |
               LSHIFT(DSE_HI,    6) |
               LSHIFT(OD_DIS,    5) |
               LSHIFT(PFE_DIS,   4) |
               LSHIFT(FAST_SLEW, 2) |
               LSHIFT(PULL__UP,  0);
  GPIOB_PDDR = (GPIOD_PDDR & ~LSHIFT(1, 3)) | LSHIFT(GP_INP, 3); // Линия работает как вход
                                                                 // Установливаем прерываний от порта B
  Install_and_enable_isr(INT_PORTB, PORTB_PRIO, ExtEnc_IRQHandler); // Регитсрация процедуры прерывания

}

/*-----------------------------------------------------------------------------------------------------
 
 \param addr 
 \param pval 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint ExtEnc_write_register(uint16_t val)
{
  _mqx_uint res = MQX_OK;

  res = SPISysBus_write_16bit_word(SPI_CS0, val);
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Перемещение 16-и битных данных из 32-х битного буфера в 8-и битный
 
 
 \param buf32 
 \param buf8 
 \param sz - размер 32-х битного буфера
-----------------------------------------------------------------------------------------------------*/
void _Copy16_32bit_buf_to8bit_buf(uint32_t *buf32, uint8_t *buf8, uint32_t sz)
{
  uint32_t i;
  uint32_t w;
  for (i = 0; i < sz; i++)
  {
    w = *buf32;
    *buf8 = (w >> 8) & 0xFF;
    buf8++;
    *buf8 = (w >> 0) & 0xFF;
    buf8++;

    buf32++;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Получение 8-и битной контрольной суммы буфера
 
 \param buf 
 \param sz 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
uint8_t _Get_crc8(uint8_t *buf, uint32_t sz)
{
  uint32_t i;
  uint8_t  crc = 0;

  for (i = 0; i < sz; i++)
  {
    crc += *buf;
    buf++;
  }
  return 0x100 - crc;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
uint32_t ExtEnc_Read_angle(void)
{
  uint32_t res;
  uint32_t i;

  uint8_t  crc;
  uint32_t sz  = 3; // Количество 2-х байтных слов в пакете

  // Читаем 16-и байтными словами, поскольку так настроен интерфейс SPI
  // Подготавливаем чистый буфер чтобы во время приема не передавались никакие случайные данные
  for (i = 0; i < sz; i++) ebuf_w[i] = 0xFFFF + SPI_CS0 + BIT(31);;
  ebuf_w[sz - 1] = 0xFFFF + SPI_CS0;

  res = SPISysBus_write_read_buf_dma(ebuf_w,  ebuf_r, sz);
  if (res != MQX_OK) 
  {
    ext_enc_errors++;
    return res;
  }

  _Copy16_32bit_buf_to8bit_buf(ebuf_r, ext_enc_pack, sz);  // Конвертируем 32-х битный буфер в 8-и битный

  if (ext_enc_pack[0] != 0) 
  {
    ext_enc_errors++;
    return MQX_ERROR;         // Проверяем индекс переменной для значения угла, он должен быть равен 0
  }
  
  if (_Get_crc8(ext_enc_pack, sz * 2 ) != 0)
  {
    ext_enc_errors++;
    return MQX_ERROR;
  }
  memcpy(&ext_enc_angle, &ext_enc_pack[1], 4);             // Копируем значение переменной

  return MQX_OK;
}
