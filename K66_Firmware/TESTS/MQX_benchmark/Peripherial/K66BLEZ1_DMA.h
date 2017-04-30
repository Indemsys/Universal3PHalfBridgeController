#ifndef __K66BLEZ_DMA
#define __K66BLEZ_DMA

#define DMA_CHANNELS_COUNT  32


#define MAX_DMA_SPI_BUFF    2048

// Ќомера источников запросов дл€ мультиплесора MUX запросов DMA
// 32-а регистра DMAMUX->CHCFG управл€ют мультиплексированием запросов с 63 периферийных сигналов на 32 входа контролера DMA

#define DMUX_SRC_DISABLED               0   // Channel disabled1
#define DMUX_SRC_TSI0                   1   //
#define DMUX_SRC_UART0_RX               2   // Receive
#define DMUX_SRC_UART0_TX               3   // Transmit
#define DMUX_SRC_UART1_RX               4   // Receive
#define DMUX_SRC_UART1_TX               5   // Transmit
#define DMUX_SRC_UART2_RX               6   // Receive
#define DMUX_SRC_UART2_TX               7   // Transmit
#define DMUX_SRC_UART3_RX               8   // Receive
#define DMUX_SRC_UART3_TX               9   // Transmit
#define DMUX_SRC_UART4_RX               10  // Transmit or Receive
#define DMUX_SRC_RESERVED               11  //
#define DMUX_SRC_I2S0_RX                12  // Receive
#define DMUX_SRC_I2S0_TX                13  // Transmit
#define DMUX_SRC_SPI0_RX                14  // Receive
#define DMUX_SRC_SPI0_TX                15  // Transmit
#define DMUX_SRC_SPI1_RX                16  // Receive
#define DMUX_SRC_SPI1_TX                17  // Transmit
#define DMUX_SRC_I2C0_I2C3              18  //
#define DMUX_SRC_I2C1_I2C2              19  //
#define DMUX_SRC_FTM0_CH0               20  // Channel 0
#define DMUX_SRC_FTM0_CH1               21  // Channel 1
#define DMUX_SRC_FTM0_CH2               22  // Channel 2
#define DMUX_SRC_FTM0_CH3               23  // Channel 3
#define DMUX_SRC_FTM0_CH4               24  // Channel 4
#define DMUX_SRC_FTM0_CH5               25  // Channel 5
#define DMUX_SRC_FTM0_CH6               26  // Channel 6
#define DMUX_SRC_FTM0_CH7               27  // Channel 7
#define DMUX_SRC_FTM1_TPM1_CH0          28  // Channel 0
#define DMUX_SRC_FTM1_TPM1_CH1          29  // Channel 1
#define DMUX_SRC_FTM2_TPM2_CH0          30  // Channel 0
#define DMUX_SRC_FTM2_TPM2_CH1          31  // Channel 1
#define DMUX_SRC_FTM3_CH0               32  //
#define DMUX_SRC_FTM3_CH1               33  //
#define DMUX_SRC_FTM3_CH2               34  //
#define DMUX_SRC_FTM3_CH3               35  //
#define DMUX_SRC_FTM3_CH4               36  //
#define DMUX_SRC_FTM3_CH5               37  //
#define DMUX_SRC_FTM3_CH6_SPI2_RX       38  //
#define DMUX_SRC_FTM3_CH7_SPI2_TX       39  //
#define DMUX_SRC_ADC0                   40  // Ч
#define DMUX_SRC_ADC1                   41  // Ч
#define DMUX_SRC_CMP0                   42  // Ч
#define DMUX_SRC_CMP1                   43  // Ч
#define DMUX_SRC_CMP2_CMP3              44  // Ч
#define DMUX_SRC_DAC0                   45  // Ч
#define DMUX_SRC_DAC1                   46  // Ч
#define DMUX_SRC_CMT                    47  // Ч
#define DMUX_SRC_PDB                    48  // Ч
#define DMUX_SRC_PortA_control_module   49  // Port A
#define DMUX_SRC_PortB_control_module   50  // Port B
#define DMUX_SRC_PortC_control_module   51  // Port C
#define DMUX_SRC_PortD_control_module   52  // Port D
#define DMUX_SRC_PortE_control_module   53  // Port E
#define DMUX_SRC_IEEE1588_TIMER0        54  //
#define DMUX_SRC_IEEE1588_TIMER1_OVF    55  //
#define DMUX_SRC_IEEE1588_TIMER2_OVF    56  //
#define DMUX_SRC_IEEE1588_TIMER3        57  //
#define DMUX_SRC_LPUART0_RX             58  //
#define DMUX_SRC_LPUART0_TX             59  //
#define DMUX_SRC_Always_enabled1        60  // Always enabled
#define DMUX_SRC_Always_enabled2        61  // Always enabled
#define DMUX_SRC_Always_enabled3        62  // Always enabled
#define DMUX_SRC_Always_enabled4        63  // Always enabled


#define DMA_1BYTE_MINOR_TRANSFER    1 
#define DMA_2BYTE_MINOR_TRANSFER    2 
#define DMA_4BYTE_MINOR_TRANSFER    4 


typedef struct {
  uint32_t SADDR;
  uint16_t SOFF;
  uint16_t ATTR;
  union {
    uint32_t NBYTES_MLNO;
    uint32_t NBYTES_MLOFFNO;
    uint32_t NBYTES_MLOFFYES;
  };
  uint32_t SLAST;
  uint32_t DADDR;
  uint16_t DOFF;
  union {
    uint16_t CITER_ELINKNO;
    uint16_t CITER_ELINKYES;
  };
  uint32_t DLAST_SGA;
  uint16_t CSR;
  union {
    uint16_t BITER_ELINKNO;
    uint16_t BITER_ELINKYES;
  };
} T_DMA_TCD;


typedef struct
{
  uint8_t             ch;         // номер канала DMA пересылающий данные из FIFO SPI в буффер с данными
  uint8_t             *databuf;   // указатель на исходный буфер с данными
  uint32_t            datasz;     // размер исходного буфера данных
  uint32_t            spi_popr;   // адрес регистра POPR SPI
  DMAMUX_MemMapPtr    DMAMUX;
  uint32_t            dmux_src;   // номер входа периферии дл€ выбранного мультиплексора DMAMUX дл€ передачи на DMA. Ќапример DMUX0_SRC_SPI0_RX
  uint8_t             minor_tranf_sz; //  оличество пересылаемых байт в минорном цикле 

} T_DMA_SPI_RX_config;

typedef struct
{
  uint8_t             ch;         // номер канала DMA пересылающий данные из буффер с данными в FIFO SPI
  uint8_t             *databuf;   // указатель на исходный буфер с данными
  uint32_t            datasz;     // размер исходного буфера данных
  uint32_t            spi_pushr;  // адрес регистра PUSHR SPI
  DMAMUX_MemMapPtr    DMAMUX;
  uint32_t            dmux_src;   // номер входа периферии дл€ выбранного мультиплексора DMAMUX дл€ передачи на DMA. Ќапример DMUX0_SRC_SPI0_TX
  uint8_t             minor_tranf_sz; //  оличество пересылаемых байт в минорном цикле 

} T_DMA_SPI_TX_config;


typedef struct
{
  FTM_MemMapPtr       FTM;        // ”казатель на таймер
  uint8_t             ftm_ch;     // Ќомер канала таймера
  uint8_t             dma_ch;     // номер канала DMA пересылающий данные из буффер с данными канал таймера
  uint32_t            saddr;      // указатель на массив с данными
  uint32_t            arrsz;      // размер массива данных
  uint32_t            daddr;      // адрес назаначени€ - регистр канала таймера
  DMAMUX_MemMapPtr    DMAMUX;     // ”казатель на структуру мультиплексора
  uint32_t            dmux_src;   // номер входа периферии дл€ выбранного мультиплексора DMAMUX дл€ передачи на DMA.

} T_DMA_WS2812B_config;



typedef struct
{
  uint8_t       tx_ch;            // Ќомер дескриптора DMA на который указывает mem_ch
  uint8_t       rx_ch;            // Ќомер дескриптора DMA предлназначенного дл€ приема
  uint8_t       minor_tranf_sz;   //  оличество пересылаемых байт в минорном цикле 

} T_DMA_SPI_cbl;


void Init_DMA(void);


void Config_DMA_for_SPI_TX(T_DMA_SPI_TX_config *cfg, T_DMA_SPI_cbl *pDS_cbl);
void Config_DMA_for_SPI_RX(T_DMA_SPI_RX_config *cfg, T_DMA_SPI_cbl *pDS_cbl);

void Start_DMA_for_SPI_TX(T_DMA_SPI_cbl *pDS_cbl, void *buf, uint32_t sz);
void Start_DMA_for_SPI_RX(T_DMA_SPI_cbl *pDS_cbl, void *buf, uint32_t sz);
void Start_DMA_for_SPI_RXTX(T_DMA_SPI_cbl *pDS_cbl, void *outbuf, void *inbuf, uint32_t sz);

#endif
