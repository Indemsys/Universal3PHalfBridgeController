#ifndef __K66BLEZ_H
  #define __K66BLEZ_H

#include "K66BLEZ1_INIT_SYS.h"
#include "K66BLEZ1_ADC.h"
#include "K66BLEZ1_DMA.h"
#include "K66BLEZ1_FTM.h"
#include "K66BLEZ1_DAC.h"
#include "K66BLEZ1_PIT.h"
#include "K66BLEZ1_SPI.h"
#include "K66BLEZ1_CAN.h"
#include "K66BLEZ1_VBAT_RAM.h"
#include "K66BLEZ1_MKW40_Channel.h"

#define  VREF (3.3)
#define  ADC_PREC (4096.0)


// Здесь сведены в одно место все настройки всех каналов DMA 
// Настройка DMA включает:
//   - Выбор канала DMA. Каналов может быть 32. Канал выбирается произвольно, но у каналов есть приоритеты 
//   - Выбор мультиплексора DMA если их в чипе больше одного 
//   - Выбор входа у мультиплексора. Входв мультиплексоров жестко привязаны к определенной периферии
//   - Выбор номера прерывания DMA если использовано несколько каналов DMA для одного процесса обмена  
//


// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с ADC
#define ADC_SCAN_SZ       5

#define DMA_ADC0_RES_CH   20               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC1_RES_CH   21               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC0_CFG_CH   16               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC1_CFG_CH   17               // Занятый канал DMA для обслуживания ADC
#define DMA_ADC_DMUX_PTR  DMAMUX_BASE_PTR  // Указатель на модуль DMUX который используется для передачи сигналов от ADC к DMA
#define DMA_ADC0_DMUX_SRC DMUX_SRC_ADC0    // Входы DMUX используемые для выбранного ADC
#define DMA_ADC1_DMUX_SRC DMUX_SRC_ADC1    // Входы DMUX используемые для выбранного ADC
#define DMA_ADC_INT_NUM   INT_DMA5_DMA21   // Номер вектора прерывания используемый в DMA для обслуживания ADC

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с интерфейсом SPI чипа MKW40

#define MKW40_SPI              SPI2              // Номер SPI модуля используемый для коммуникации с модулем MKW40
#define DMA_MKW40_FM_CH        0                 // Канал DMA для обслуживания приема по   SPI. FIFO->Память.
#define DMA_MKW40_MF_CH        1                 // Канал DMA для обслуживания передачи по SPI. Память->FIFO.
#define MKW40_SPI_CS           0                 // Номер аппаратного внешнего сигнала CS для интерфейса MKW40 SPI
#define DMA_MKW40_DMUX_PTR     DMAMUX_BASE_PTR  // Указатель на модуль DMUX который используется для передачи сигналов от контроллера SPI к DMA
#define DMA_MKW40_DMUX_TX_SRC  DMUX_SRC_FTM3_CH7_SPI2_TX // Вход DMUX используемый для вызова передачи по DMA
#define DMA_MKW40_DMUX_RX_SRC  DMUX_SRC_FTM3_CH6_SPI2_RX // Вход DMUX используемый для вызова приема по DMA

#define DMA_MKW40_RX_INT_NUM   INT_DMA0_DMA16    // Номер вектора прерывания используемый в DMA для обслуживания приема по SPI
#define DMA_MKW40_ISR          DMA0_DMA16_IRQHandler

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с интерфейсом SPI системной шины (SYS BUS) связывающей DRV8305 и другую периферию в проекте DMC01

#define SYSB_SPI                SPI1              // Номер SPI модуля используемый для коммуникации с SYS BUS
#define DMA_SYSB_FM_CH          2                 // Канал DMA для обслуживания приема по   SPI. FIFO->Память.
#define DMA_SYSB_MF_CH          3                 // Канал DMA для обслуживания передачи по SPI. Память->FIFO.
#define DRV8305_SPI_CS          0                 // Номер аппаратного внешнего сигнала CS для интерфейса DRV8305
#define DMA_SYSB_DMUX_PTR       DMAMUX_BASE_PTR   // Указатель на модуль DMUX который используется для передачи сигналов от контроллера SPI к DMA
#define DMA_SYSB_DMUX_TX_SRC    DMUX_SRC_SPI1_TX  // Вход DMUX используемый для вызова передачи по DMA
#define DMA_SYSB_DMUX_RX_SRC    DMUX_SRC_SPI1_RX  // Вход DMUX используемый для вызова приема по DMA
                                
#define DMA_SYSB_RX_INT_NUM     INT_DMA2_DMA18    // Номер вектора прерывания используемый в DMA для обслуживания приема по SPI
#define DMA_SYSB_ISR            DMA2_DMA18_IRQHandler

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с на вывод в LED ленту на основе WS2812B
//  Прерывания DMA не используются 

#define DMA_WS2812B_DMUX_PTR    DMAMUX_BASE_PTR   // Указатель на модуль DMUX который используется для передачи сигналов от контроллера SPI к DMA
#define DMA_WS2812B_DMUX_SRC    DMUX_SRC_FTM0_CH2 // Вход DMUX используемый для вызова передачи по DMA
#define DMA_WS2812B_CH          4                 // Канал DMA для обслуживания передачи в WS2812B


#ifdef ADC_GLOBAL

T_ADC_res   adcs;

#else

extern T_ADC_res   adcs;

#endif


void Set_MKW40_CS_state(int state);


#endif
