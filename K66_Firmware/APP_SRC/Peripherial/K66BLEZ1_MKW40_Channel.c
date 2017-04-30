// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.29
// 13:50:42
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern const T_SPI_modules spi_mods[3];
extern T_SPI_cbls  spi_cbl[3];

#define  MKW40_RXEND   BIT(1)  // Флаг передаваемый из ISR об окончании приема по SPI

static T_DMA_SPI_cbl DS_cbl;
/*------------------------------------------------------------------------------
  Обработчик прерывания от модуля DMA по завершению чтения данных из SPI связанного с MKW40

 \param user_isr_ptr
 ------------------------------------------------------------------------------*/
static void DMA_SPI_MKW40_rx_isr(void *user_isr_ptr)
{
   DMA_MemMapPtr    DMA     = DMA_BASE_PTR;

   DMA->INT = BIT(DMA_MKW40_FM_CH); // Сбрасываем флаг прерываний  канала

   // Очистим FIFO приемника и передатчика
   spi_mods[MKW40_SPI].spi->MCR  |= BIT(CLR_RXF) + BIT(CLR_TXF);
   // Сбросим все флаги у SPI
   spi_mods[MKW40_SPI].spi->SR =  spi_mods[MKW40_SPI].spi->SR;

   _lwevent_set(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND); // Уведомляем об окончании пересылки по DMA из SPI в память
}

/*------------------------------------------------------------------------------
  Конфигурирование 2-х каналов DMA на прием и на передачу для работы с модулем SPI


 ------------------------------------------------------------------------------*/
void Init_MKW40_SPI_DMA(void)
{
   T_DMA_SPI_RX_config   rx_cfg;
   T_DMA_SPI_TX_config   tx_cfg;

   tx_cfg.ch       = DMA_MKW40_MF_CH;                           // номер канала DMA
   tx_cfg.spi_pushr = (uint32_t)&(spi_mods[MKW40_SPI].spi->PUSHR); // адрес регистра PUSHR SPI
   tx_cfg.DMAMUX   = DMA_MKW40_DMUX_PTR;                        // Указатель на мультиплексор входных сигналов для DMA
   tx_cfg.dmux_src = DMA_MKW40_DMUX_TX_SRC;                     // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA.
   tx_cfg.minor_tranf_sz = DMA_1BYTE_MINOR_TRANSFER;
   Config_DMA_for_SPI_TX(&tx_cfg, &DS_cbl);


   rx_cfg.ch        = DMA_MKW40_FM_CH;                          // номер канала DMA
   rx_cfg.spi_popr  = (uint32_t)&(spi_mods[MKW40_SPI].spi->POPR); // адрес регистра POPR SPI
   rx_cfg.DMAMUX    = DMA_MKW40_DMUX_PTR;                       // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA.
   rx_cfg.dmux_src  = DMA_MKW40_DMUX_RX_SRC;
   rx_cfg.minor_tranf_sz = DMA_1BYTE_MINOR_TRANSFER;
   Config_DMA_for_SPI_RX(&rx_cfg, &DS_cbl);
   Install_and_enable_isr(DMA_MKW40_RX_INT_NUM, spi_mods[MKW40_SPI].prio, DMA_SPI_MKW40_rx_isr); // Прерывание по завершению приема по DMA
}



/*-------------------------------------------------------------------------------------------------------------
  Отправка данных по SPI с использованием DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_send_buf(const uint8_t *buff, uint32_t sz)
{
   _mqx_uint    res = MQX_OK;
   uint32_t       s;
   int          i;

   Set_MKW40_CS_state(0);
   while (sz > 0)
   {
      if (sz >= MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF-1;
      else s = sz;

      Start_DMA_for_SPI_TX(&DS_cbl, (void*)buff, s);
      // Ожидаем флага окончания передачи буфера по DMA
      if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
      {
         spi_cbl[MKW40_SPI].tx_err_cnt++;
         res = MQX_ERROR;
      }
      buff = buff + s;
      sz -= s;
   }
   Set_MKW40_CS_state(1);
   return res;
}

/*-------------------------------------------------------------------------------------------------------------
  Прием данных по SPI с использованием DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_read_buf(const uint8_t *buff, uint32_t sz)
{
   _mqx_uint   res = MQX_OK;
   uint32_t      s;
   int         i;

   Set_MKW40_CS_state(0);
   while (sz > 0)
   {
      if (sz >= MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF-1;
      else s = sz;

      Start_DMA_for_SPI_RX(&DS_cbl,  (void*)buff, s);
      // Ожидаем флага окончания передачи очереди
      if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
      {
         spi_cbl[MKW40_SPI].rx_err_cnt++;
         res = MQX_ERROR;
      }
      buff = buff + s;
      sz -= s;
   }
   Set_MKW40_CS_state(1);
   return res;
}

/*-------------------------------------------------------------------------------------------------------------
  Прием данных по SPI с использованием DMA
-------------------------------------------------------------------------------------------------------------*/
_mqx_uint MKW40_SPI_write_read_buf(const uint8_t *wbuff, uint32_t wsz, const uint8_t *rbuff, uint32_t rsz)
{
   _mqx_uint   res = MQX_OK;
   uint32_t      s;
   int         i;

   Set_MKW40_CS_state(0);
   while (wsz > 0)
   {
      if (wsz > MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF;
      else s = wsz;

      Start_DMA_for_SPI_TX(&DS_cbl,  (void*)wbuff, s);
      // Ожидаем флага окончания передачи буфера по DMA
      if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
      {
         spi_cbl[MKW40_SPI].tx_err_cnt++;
         res = MQX_ERROR;
      }
      wbuff = wbuff + s;
      wsz -= s;
   }

   while (rsz > 0)
   {
      if (rsz > MAX_DMA_SPI_BUFF) s = MAX_DMA_SPI_BUFF;
      else s = rsz;

      Start_DMA_for_SPI_RX(&DS_cbl,  (void*)rbuff, s);
      // Ожидаем флага окончания передачи очереди
      if (_lwevent_wait_ticks(&spi_cbl[MKW40_SPI].spi_event, MKW40_RXEND, FALSE, 10) != MQX_OK)
      {
         spi_cbl[MKW40_SPI].rx_err_cnt++;
         res = MQX_ERROR;
      }
      rbuff = rbuff + s;
      rsz -= s;
   }
   Set_MKW40_CS_state(1);
   return res;
}


/*------------------------------------------------------------------------------
  Инициализация SPI модуля и 2-х каналов DMA для него

 ------------------------------------------------------------------------------*/
void Init_MKW40_channel(void)
{
  //SPI_master_init(MKW40_SPI, SPI_8_BITS, 0, 0, SPI_BAUD_20MHZ, 0);
  SPI_slave_init(MKW40_SPI, SPI_8_BITS, 0, 0, 0);
  Init_MKW40_SPI_DMA(); // Конфигурирование 2-х каналов DMA на прием и на передачу для работы с модулем SPI

}

