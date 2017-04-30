#ifndef __K66BLEZ_SPI
  #define __K66BLEZ_SPI


#define  SPI0 0
#define  SPI1 1
#define  SPI2 2


#define  MSTR       31
#define  CONT_SCKE  30
#define  DCONF      28
#define  FRZ        27
#define  MTFE       26
#define  PCSSE      25
#define  ROOE       24
#define  PCSIS      16
#define  DOZE       15
#define  MDIS       14
#define  DIS_TXF    13
#define  DIS_RXF    12
#define  CLR_TXF    11
#define  CLR_RXF    10
#define  SMPL_PT    8
#define  HALT       0

#define  TCF        31
#define  TXRXS      30
#define  EOQF       28
#define  TFUF       27
#define  TFFF       25
#define  RFOF       19
#define  RFDF       17
#define  TXCTR      12
#define  TXNXTPTR    8
#define  RXCTR       4
#define  POPNXTPTR   0

#define  CONT       31
#define  CTAS       28
#define  EOQ        27
#define  CTCNT      26
#define  PCS        16
#define  TXDATA     0

#define  TCF_RE     31
#define  EOQF_RE    28
#define  TFUF_RE    27
#define  TFFF_RE    25
#define  TFFF_DIRS  24
#define  RFOF_RE    19
#define  RFDF_RE    17
#define  RFDF_DIRS  16

typedef enum
{
  SPI_BAUD_20MHZ  ,
  SPI_BAUD_15MHZ  ,
  SPI_BAUD_10MHZ  ,
  SPI_BAUD_6MHZ   ,
  SPI_BAUD_4MHZ   ,
  SPI_BAUD_1_5MHZ,
  SPI_BAUD_3MHZ ,

} T_SPI_bauds;

typedef enum
{
  SPI_4_BITS = 4  ,
  SPI_5_BITS  ,
  SPI_6_BITS  ,
  SPI_7_BITS  ,
  SPI_8_BITS  ,
  SPI_9_BITS  ,
  SPI_10_BITS  ,
  SPI_11_BITS  ,
  SPI_12_BITS  ,
  SPI_13_BITS  ,
  SPI_14_BITS  ,
  SPI_15_BITS  ,
  SPI_16_BITS  ,

} T_SPI_bits;


typedef enum
{
  SPI_CS0  ,
  SPI_CS1  ,
  SPI_CS2  ,
  SPI_CS3  ,
  SPI_CS4  ,
  SPI_CS5  ,

} T_SPI_CS;

// Возможные значения поля PBR
#define PBR_2        0
#define PBR_3        1
#define PBR_5        2
#define PBR_7        3


// Возможные значения поля PCSSCK
#define PCSSCK_1     0
#define PCSSCK_3     1
#define PCSSCK_5     2
#define PCSSCK_7     3


// Возможные значения поля CSSCK
#define CSSCK_2      0
#define CSSCK_4      1
#define CSSCK_8      2
#define CSSCK_16     3
#define CSSCK_32     4
#define CSSCK_64     5
#define CSSCK_128    6
#define CSSCK_256    7
#define CSSCK_512    8
#define CSSCK_1024   9
#define CSSCK_2048   10
#define CSSCK_4096   11
#define CSSCK_8192   12
#define CSSCK_16384  13
#define CSSCK_32768  14
#define CSSCK_65536  15

// Возможные значения поля BR
#define BR_2         0
#define BR_4         1
#define BR_6         2
#define BR_8         3
#define BR_16        4
#define BR_32        5
#define BR_64        6
#define BR_128       7
#define BR_256       8
#define BR_512       9
#define BR_1024      10
#define BR_2048      11
#define BR_4096      12
#define BR_8192      13
#define BR_16384     14
#define BR_32768     15



typedef struct
{
   SPI_MemMapPtr spi;
   uint32_t        int_num;
   uint32_t        prio;
   void          (*spi_isr)(void*);
} T_SPI_modules;

typedef struct
{
  int rx_overfl_cnt;
  int tx_uderfl_cnt;

} T_SPI_errors;


#define  SPI_TX_COMPLETE BIT(16)  // Флаг окончания передачи 

typedef struct
{
   LWEVENT_STRUCT     spi_event; // При использовании RTOS
   uint32_t           tx_err_cnt;
   uint32_t           rx_err_cnt;

} T_SPI_cbls;

#define SPI_CS0 BIT(16)
#define SPI_CS1 BIT(17)
#define SPI_CS2 BIT(18)
#define SPI_CS3 BIT(19)
#define SPI_CS4 BIT(20)
#define SPI_CS5 BIT(21)

void      SPI_master_init(uint8_t modn, T_SPI_bits fsz, uint8_t cpol, uint8_t cpha, T_SPI_bauds baud, int enint);
void      SPI_slave_init(uint8_t modn, T_SPI_bits fsz, uint8_t cpol, uint8_t cpha,  int enint);
void      SPI_push_16bit_word(uint8_t modn, uint32_t cs, uint16_t data);
_mqx_uint SPI_wait_tx_complete(uint8_t modn, uint16_t *pdata, _mqx_uint ticks );

#endif
