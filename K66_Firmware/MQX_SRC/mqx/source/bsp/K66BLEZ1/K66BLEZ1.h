/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This include file is used to provide information needed by
*   an application program using the kernel running on the
*   Freescale TWR-K70F120M Evaluation board.
*
*
*END************************************************************************/

#ifndef _K66BLEZ1_h_
  #define _K66BLEZ1_h_  1


/*----------------------------------------------------------------------
**                  HARDWARE INITIALIZATION DEFINITIONS
*/

/*
** Define the board type
*/
  #define BSP_K66BLEZ1                         1
  #define BSP_NAME                            "K66BLEZ1"

/*
** PROCESSOR MEMORY BOUNDS
*/
  #define BSP_PERIPH_BASE                     (CORTEX_PERIPH_BASE)

typedef void (*vector_entry)(void);

  #define BSP_INTERNAL_FLASH_BASE    0x00000000
  #define BSP_INTERNAL_FLASH_SIZE    0x00100000
  #define BSP_INTERNAL_FLASH_SECTOR_SIZE 0x1000

  #ifdef __CC_ARM
/* Keil compiler */

extern unsigned char Image$$USEDFLASH_END$$Base[];
    #define __FLASHX_START_ADDR     ((void *)Image$$USEDFLASH_END$$Base)
    #define __FLASHX_END_ADDR       ((BSP_INTERNAL_FLASH_BASE) + (BSP_INTERNAL_FLASH_SIZE))
    #define __FLASHX_SECT_SIZE      BSP_INTERNAL_FLASH_SECTOR_SIZE

    #define __INTERNAL_SRAM_BASE    0x1FFF0000
    #define __INTERNAL_SRAM_SIZE    0x00020000
    #define __SRAM_POOL             (void *)0x1FFF0000

    #define __DEFAULT_PROCESSOR_NUMBER 1
    #define __DEFAULT_INTERRUPT_STACK_SIZE 2048

extern unsigned char Image$$VECTORS$$Base[];
    #define __VECTOR_TABLE_ROM_START ((void *)Image$$VECTORS$$Base)

extern unsigned char Image$$RAM_VECTORS$$Base[];
    #define __VECTOR_TABLE_RAM_START ((void *)Image$$RAM_VECTORS$$Base)

  #else /* __CC_ARM */

extern const unsigned char __FLASHX_START_ADDR[];
extern const unsigned char __FLASHX_END_ADDR[];
extern const unsigned char __FLASHX_SECT_SIZE[];

extern unsigned char __INTERNAL_SRAM_BASE[],        __INTERNAL_SRAM_SIZE[];
extern unsigned long __SRAM_POOL[];

extern vector_entry __VECTOR_TABLE_RAM_START[]; // defined in linker command file
extern vector_entry __VECTOR_TABLE_ROM_START[]; // defined in linker command file

extern unsigned char __DEFAULT_PROCESSOR_NUMBER[];
extern unsigned char __DEFAULT_INTERRUPT_STACK_SIZE[];

  #endif /* __CC_ARM */

/* Enable modification of flash configuration bytes during loading for flash targets */
  #ifndef BSPCFG_ENABLE_CFMPROTECT
    #define BSPCFG_ENABLE_CFMPROTECT        1
  #endif
  #if !BSPCFG_ENABLE_CFMPROTECT && defined(__ICCARM__)
    #error Cannot disable CFMPROTECT field on IAR compiler. Please define BSPCFG_ENABLE_CFMPROTECT to 1.
  #endif

  #ifndef BSP_CLOCK_CONFIGURATION_STARTUP
    #define BSP_CLOCK_CONFIGURATION_STARTUP (BSP_CLOCK_CONFIGURATION_180MHZ)
  #endif

/* Init startup clock configuration is CPU_CLOCK_CONFIG_0 */
  #define BSP_CLOCK_SRC                   (CPU_XTAL_CLK_HZ)
  #define BSP_CORE_CLOCK                  (CPU_CORE_CLK_HZ_CONFIG_3)
  #define BSP_SYSTEM_CLOCK                (CPU_CORE_CLK_HZ_CONFIG_3)
  #define BSP_CLOCK                       (CPU_BUS_CLK_HZ_CONFIG_3)
  #define BSP_BUS_CLOCK                   (CPU_BUS_CLK_HZ_CONFIG_3)
  #define BSP_FLEXBUS_CLOCK               (CPU_FLEXBUS_CLK_HZ_CONFIG_3)
  #define BSP_FLASH_CLOCK                 (CPU_FLASH_CLK_HZ_CONFIG_3)

/** MGCT: <category name="BSP Hardware Options"> */

/*
** The clock tick rate in miliseconds - be cautious to keep this value such
** that it divides 1000 well
**
** MGCT: <option type="number" min="1" max="1000"/>
*/
  #ifndef BSP_ALARM_FREQUENCY
    #define BSP_ALARM_FREQUENCY             (200)
  #endif

/*
** System timer definitions
*/
  #define BSP_SYSTIMER_DEV          systick_devif
  #define BSP_SYSTIMER_ID           0
  #define BSP_SYSTIMER_SRC_CLK      CM_CLOCK_SOURCE_CORE
  #define BSP_SYSTIMER_ISR_PRIOR    2
/* We need to keep BSP_TIMER_INTERRUPT_VECTOR macro for tests and watchdog.
 * Will be removed after hwtimer expand to all platforms */
  #define BSP_TIMER_INTERRUPT_VECTOR INT_SysTick

/** MGCT: </category> */

/*
** Old clock rate definition in MS per tick, kept for compatibility
*/
  #define BSP_ALARM_RESOLUTION                (1000 / BSP_ALARM_FREQUENCY)

/*
** Define the location of the hardware interrupt vector table
*/
  #if MQX_ROM_VECTORS
    #define BSP_INTERRUPT_VECTOR_TABLE              ((uint32_t)__VECTOR_TABLE_ROM_START)
  #else
    #define BSP_INTERRUPT_VECTOR_TABLE              ((uint32_t)__VECTOR_TABLE_RAM_START)
  #endif

  #ifndef BSP_FIRST_INTERRUPT_VECTOR_USED
    #define BSP_FIRST_INTERRUPT_VECTOR_USED     (0)
  #endif

  #ifndef BSP_LAST_INTERRUPT_VECTOR_USED
    #define BSP_LAST_INTERRUPT_VECTOR_USED      (250)
  #endif

/*
** EDMA interrupt level
*/
  #define BSP_EDMA_INT_LEVEL                     (3)

/*
** RTC interrupt level
*/
  #define BSP_RTC_INT_LEVEL                      (4)

/*
** LPM related
*/
  #define BSP_LLWU_INTERRUPT_VECTOR              INT_LLW
  #define BSP_LLWU_INTERRUPT_PRIORITY            3

  #define BSP_LPM_DEPENDENCY_LEVEL_SERIAL_POLLED (30)
  #define BSP_LPM_DEPENDENCY_LEVEL_SERIAL_INT    (31)


/*
** SAI/audio specifications
*/
  #define BSP_HAS_AUDIO_CODEC         (1)

/*
** EDMA interrupt level
*/
  #define BSP_EDMA_INT_LEVEL                     (3)

/* HWTIMER definitions for user applications */
  #define BSP_HWTIMER1_DEV        pit_devif
  #define BSP_HWTIMER1_SOURCE_CLK (CM_CLOCK_SOURCE_BUS)
  #define BSP_HWTIMER1_ID         (0)

  #define BSP_HWTIMER2_DEV        lpt_devif
  #define BSP_HWTIMER2_SOURCE_CLK (CM_CLOCK_SOURCE_LPO)
  #define BSP_HWTIMER2_ID         (0)

/* HMI Touch TWRPI daughter cards */
  #define BSP_TWRPI_VOID   0
  #define BSP_TWRPI_ROTARY 1
  #define BSP_TWRPI_KEYPAD 2

  #define BSP_HWTIMER_LPT0_DEFAULT_PCSCLK     (1)

/* Port IRQ levels */
  #define BSP_PORTA_INT_LEVEL         (3)
  #define BSP_PORTB_INT_LEVEL         (3)
  #define BSP_PORTC_INT_LEVEL         (3)
  #define BSP_PORTD_INT_LEVEL         (3)
  #define BSP_PORTE_INT_LEVEL         (3)



  #define BSP_TSI_INT_LEVEL       (4)
/*-----------------------------------------------------------------------------
**                      Ethernet Info
*/
  #define BSP_ENET_DEVICE_COUNT                  MACNET_DEVICE_COUNT

/*
** MACNET interrupt levels and vectors
*/
  #define BSP_MACNET0_INT_TX_LEVEL           (4)
  #define BSP_MACNET0_INT_RX_LEVEL           (4)

  #define BSP_DEFAULT_ENET_DEVICE             0
  #define BSP_DEFAULT_ENET_OUI                { 0x00, 0x00, 0x5E, 0, 0, 0 }

/*
** The Ethernet PHY device number 0..31
*/
  #ifndef BSP_ENET0_PHY_ADDR
    #define BSP_ENET0_PHY_ADDR                 0
  #endif

/*
** PHY MII Speed (MDC - Management Data Clock)
*/
  #define BSP_ENET0_PHY_MII_SPEED             (2500000L)

/** MGCT: <category name="BSP Ethernet Options"> */

/*
** Number of receive BD's.
** MGCT: <option type="number"/>
*/
  #ifndef BSPCFG_RX_RING_LEN
    #define BSPCFG_RX_RING_LEN              8
  #endif

/*
** Number of transmit BD's.
** MGCT: <option type="number"/>
*/
  #ifndef BSPCFG_TX_RING_LEN
    #define BSPCFG_TX_RING_LEN              4
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ENET_STATS
    #define BSPCFG_ENABLE_ENET_STATS        1
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_RESTORE
    #define BSPCFG_ENET_RESTORE             1
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_SRAM_BUF
    #define BSPCFG_ENET_SRAM_BUF            1
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_CPP
    #define BSPCFG_ENABLE_CPP               0
  #endif

/*
** Insertion of IPv4 header checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_HW_TX_IP_CHECKSUM
    #define BSPCFG_ENET_HW_TX_IP_CHECKSUM           0
  #endif

/*
** Insertion of protocol checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    #define BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM     0
  #endif

/*
** Insertion of IPv4 header checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_HW_RX_IP_CHECKSUM
    #define BSPCFG_ENET_HW_RX_IP_CHECKSUM           0
  #endif

/*
** Discard of frames with wrong protocol checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM
    #define BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM     0
  #endif

/*
** Discard of frames with MAC layer errors by ENET-module.
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENET_HW_RX_MAC_ERR
    #define BSPCFG_ENET_HW_RX_MAC_ERR               0
  #endif

/** MGCT: </category> */

/*-----------------------------------------------------------------------------
**                      DSPI
*/

  #define BSP_DSPI_INT_LEVEL              (4)
  #define BSP_SPI_MEMORY_CHANNEL          (2)

  #define BSP_DSPI0_DMA_RX_CHANNEL        (0)
  #define BSP_DSPI0_DMA_TX_CHANNEL        (1)
  #define BSP_DSPI0_DMA_RX_SOURCE         (16)
  #define BSP_DSPI0_DMA_TX_SOURCE         (17)

  #define BSP_DSPI1_DMA_RX_CHANNEL        (2)
  #define BSP_DSPI1_DMA_TX_CHANNEL        (3)
  #define BSP_DSPI1_DMA_RX_SOURCE         (18)
  #define BSP_DSPI1_DMA_TX_SOURCE         (19)

  #define BSP_DSPI2_DMA_RX_CHANNEL        (4)
  #define BSP_DSPI2_DMA_TX_CHANNEL        (5)
  #define BSP_DSPI2_DMA_RX_SOURCE         (20)
  #define BSP_DSPI2_DMA_TX_SOURCE         (21)

/*-----------------------------------------------------------------------------
**                      I2C
*/
  #define BSP_I2C_CLOCK                       (BSP_BUS_CLOCK)
  #define BSP_I2C0_ADDRESS                    (0x6E)
  #define BSP_I2C1_ADDRESS                    (0x6F)
  #define BSP_I2C0_BAUD_RATE                  (100000)
  #define BSP_I2C1_BAUD_RATE                  (100000)
  #define BSP_I2C0_MODE                       (I2C_MODE_MASTER)
  #define BSP_I2C1_MODE                       (I2C_MODE_MASTER)
  #define BSP_I2C0_INT_LEVEL                  (5)
  #define BSP_I2C1_INT_LEVEL                  (5)
  #define BSP_I2C0_RX_BUFFER_SIZE             (64)
  #define BSP_I2C0_TX_BUFFER_SIZE             (64)
  #define BSP_I2C1_RX_BUFFER_SIZE             (64)
  #define BSP_I2C1_TX_BUFFER_SIZE             (64)

/*-----------------------------------------------------------------------------
**                      ADC
*/
  #define ADC_MAX_MODULES                 (4)
  #define BSP_ADC_POTENTIOMETER           (ADC1_SOURCE_AD20)
  #define BSP_ADC_CH_POT                  (ADC1_SOURCE_AD20)

  #define BSP_ADC0_VECTOR_PRIORITY        (3)
  #define BSP_ADC1_VECTOR_PRIORITY        (3)
  #define BSP_ADC2_VECTOR_PRIORITY        (3)
  #define BSP_ADC3_VECTOR_PRIORITY        (3)

  #define BSP_PDB_VECTOR_PRIORITY         (3)
  #define BSP_ADC_VREF_DEFAULT            (3300)
  #define BSP_ADC_POTENTIOMETER_MAX       (3300)

/*-----------------------------------------------------------------------------
**                      LWADC
*/
  #define BSP_DEFAULT_LWADC_MODULE        lwadc1_init

/*-----------------------------------------------------------------------------
**                      USB
*/
  #define USBCFG_MAX_DRIVERS               (4)

  #define BSP_USB_INT_LEVEL                (4)
  #define BSP_USB_TWR_SER2                 (0) //set to 1 if TWR-SER2 (2 eth) board used (only host)

  #if BSP_USB_TWR_SER2
/* If the TWR-SER2 board is used, the default USB host controller is EHCI. This gives the application
** developer the possibility to run USB at high speed.
*/
    #define USBCFG_DEFAULT_HOST_CONTROLLER    (&_bsp_usb_host_ehci0_if)
  #else //BSP_USB_TWR_SER2
/* Use KHCI host controller as default controller for other configurations. */
    #define USBCFG_DEFAULT_HOST_CONTROLLER    (&_bsp_usb_host_khci0_if)
  #endif //BSP_USB_TWR_SER2

  #define USBCFG_DEFAULT_DEVICE_CONTROLLER  (&_bsp_usb_dev_khci0_if)

/* This will be removed in the future and runtime option in the initialization struct will be used */
  #define USBCFG_REGISTER_ENDIANNESS        MQX_LITTLE_ENDIAN
  #define USBCFG_MEMORY_ENDIANNESS          MQX_LITTLE_ENDIAN

  #define USBCFG_KHCI 1 //for USB DDK
  #define USBCFG_EHCI 1 //for USB DDK

/*-----------------------------------------------------------------------------
**                  IO DEVICE DRIVERS CONFIGURATION
*/

/** MGCT: <category name="I/O Subsystem"> */

/*
** Enable I/O subsystem features in MQX. No I/O drivers and file I/O will be
** possible without this feature.
**
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_IO_SUBSYSTEM
    #define BSPCFG_ENABLE_IO_SUBSYSTEM      1
  #endif

/** MGCT: </category> */

/** MGCT: <category name="Default Driver Installation in BSP startup"> */

/** MGCT: <category name="UART0 Settings"> */

/*
** Polled TTY device (UART0)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYA
    #define BSPCFG_ENABLE_TTYA              0
  #endif

/*
** Interrupt-driven TTY device (UART0)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYA
    #define BSPCFG_ENABLE_ITTYA             0
  #endif

/*
** TTYA and ITTYA baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI0_BAUD_RATE
    #define BSPCFG_SCI0_BAUD_RATE             1250000
  #endif

/*
** TTYA and ITTYA buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI0_QUEUE_SIZE
    #define BSPCFG_SCI0_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/** MGCT: <category name="UART1 Settings"> */

/*
** Polled TTY device (UART1)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYB
    #define BSPCFG_ENABLE_TTYB              0
  #endif

/*
** Interrupt-driven TTY device (UART1)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYB
    #define BSPCFG_ENABLE_ITTYB             0
  #endif

/*
** TTYB and ITTYB baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI1_BAUD_RATE
    #define BSPCFG_SCI1_BAUD_RATE             115200
  #endif

/*
** TTYB and ITTYB buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI1_QUEUE_SIZE
    #define BSPCFG_SCI1_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/** MGCT: <category name="UART2 Settings"> */

/*
** Polled TTY device (UART2)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYC
    #define BSPCFG_ENABLE_TTYC              0
  #endif

/*
** Interrupt-driven TTY device (UART2)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYC
    #define BSPCFG_ENABLE_ITTYC             0
  #endif

/*
** TTYC and ITTYC baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI2_BAUD_RATE
    #define BSPCFG_SCI2_BAUD_RATE             115200
  #endif

/*
** TTYC and ITTYC buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI2_QUEUE_SIZE
    #define BSPCFG_SCI2_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/** MGCT: <category name="UART3 Settings"> */

/*
** Polled TTY device (UART3)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYD
    #define BSPCFG_ENABLE_TTYD              1
  #endif

/*
** Interrupt-driven TTY device (UART3)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYD
    #define BSPCFG_ENABLE_ITTYD             0
  #endif

/*
** TTY device HW signals (UART3).
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYD_HW_SIGNALS
    #define BSPCFG_ENABLE_TTYD_HW_SIGNALS   0
  #endif

/*
** TTYD and ITTYD baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI3_BAUD_RATE
    #define BSPCFG_SCI3_BAUD_RATE             115200
  #endif

/*
** TTYD and ITTYD buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI3_QUEUE_SIZE
    #define BSPCFG_SCI3_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/** MGCT: <category name="UART4 Settings"> */

/*
** Polled TTY device (UART4)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYE
    #define BSPCFG_ENABLE_TTYE              0
  #endif

/*
** Interrupt-driven TTY device (UART4)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYE
    #define BSPCFG_ENABLE_ITTYE             0
  #endif

/*
** TTYE and ITTYE baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI4_BAUD_RATE
    #define BSPCFG_SCI4_BAUD_RATE             115200
  #endif

/*
** TTYE and ITTYE buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI4_QUEUE_SIZE
    #define BSPCFG_SCI4_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/** MGCT: <category name="UART5 Settings"> */

/*
** Polled TTY device (UART5)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_TTYF
    #define BSPCFG_ENABLE_TTYF              1
  #endif

/*
** Interrupt-driven TTY device (UART5)
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ITTYF
    #define BSPCFG_ENABLE_ITTYF             0
  #endif

/*
** TTYF and ITTYF baud rate
** MGCT: <option type="number" min="0" max="115200"/>
*/
  #ifndef BSPCFG_SCI5_BAUD_RATE
    #define BSPCFG_SCI5_BAUD_RATE             115200
  #endif

/*
** TTYF and ITTYF buffer size
** MGCT: <option type="number" min="0" max="256"/>
*/
  #ifndef BSPCFG_SCI5_QUEUE_SIZE
    #define BSPCFG_SCI5_QUEUE_SIZE             64
  #endif

/** MGCT: </category> */

/*
** Polled I2C0 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_I2C0
    #define BSPCFG_ENABLE_I2C0                  0
  #endif

/*
** Polled I2C1 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_I2C1
    #define BSPCFG_ENABLE_I2C1                  0
  #endif

/*
** Interrupt-driven I2C0 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_II2C0
    #define BSPCFG_ENABLE_II2C0                 0
  #endif

/*
** Interrupt-driven I2C1 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_II2C1
    #define BSPCFG_ENABLE_II2C1                 0
  #endif

  #ifdef BSPCFG_ENABLE_GPIODEV
    #if BSPCFG_ENABLE_GPIODEV
      #error The GPIO driver was replaced by faster and smaller LWGPIO - see <MQX_INST_DIR>\mqx\examples\lwgpio\lwgpio.c example application
    #endif
  #endif

/*
** RTC device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_RTCDEV
    #define BSPCFG_ENABLE_RTCDEV                0
  #endif

/*
** PCFLASH device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_PCFLASH
    #define BSPCFG_ENABLE_PCFLASH               0
  #endif

/*
** Enable SPI0 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_SPI0
    #define BSPCFG_ENABLE_SPI0                  0
  #endif

/*
** Use DMA transfers on SPI0
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_DSPI0_USE_DMA
    #define BSPCFG_DSPI0_USE_DMA                1
  #endif

/*
** Enable SPI1 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_SPI1
    #define BSPCFG_ENABLE_SPI1                  0
  #endif

/*
** Use DMA transfers on SPI1
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_DSPI1_USE_DMA
    #define BSPCFG_DSPI1_USE_DMA                1
  #endif

/*
** Enable SPI2 device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_SPI2
    #define BSPCFG_ENABLE_SPI2                  0
  #endif

/*
** Use DMA transfers on SPI2
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_DSPI2_USE_DMA
    #define BSPCFG_DSPI2_USE_DMA                1
  #endif

/*
** ADC0
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ADC0
    #define BSPCFG_ENABLE_ADC0                  0
  #endif

/*
** ADC1
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ADC1
    #define BSPCFG_ENABLE_ADC1                  0
  #endif

/*
** ADC2
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ADC2
    #define BSPCFG_ENABLE_ADC2                  0
  #endif

/*
** ADC3
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ADC3
    #define BSPCFG_ENABLE_ADC3                  0
  #endif

/*
** FTFL
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_FLASHX
    #define BSPCFG_ENABLE_FLASHX                0
  #endif

/*
** ESDHC device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_ESDHC
    #define BSPCFG_ENABLE_ESDHC                 0
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_NANDFLASH
    #define BSPCFG_ENABLE_NANDFLASH             0
  #endif

/*
** IODEBUG device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_IODEBUG
    #define BSPCFG_ENABLE_IODEBUG               1
  #endif

/*
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_HAS_SRAM_POOL
    #define BSPCFG_HAS_SRAM_POOL                1
  #endif

/*
** SAI device
** MGCT: <option type="bool"/>
*/
  #ifndef BSPCFG_ENABLE_SAI
    #define BSPCFG_ENABLE_SAI                   0
  #endif
/** MGCT: </category> */

/*----------------------------------------------------------------------
**                  DEFAULT MQX INITIALIZATION DEFINITIONS
** MGCT: <category name="Default MQX initialization parameters">
*/

/* Defined in link.xxx */
extern unsigned char __KERNEL_DATA_START[];
extern unsigned char __KERNEL_DATA_END[];

extern unsigned char __UNCACHED_DATA_START[];
extern unsigned char __UNCACHED_DATA_END[];
extern unsigned char __UNCACHED_DATA_SIZE[];

  #if MQX_ENABLE_USER_MODE
extern unsigned char __KERNEL_AREA_START[];
extern unsigned char __KERNEL_AREA_END[];
extern unsigned char __USER_AREA_START[];
extern unsigned char __USER_AREA_END[];

    #if __IAR_SYSTEMS_ICC__
      #pragma section="USER_RW_MEMORY"
      #pragma section="USER_RO_MEMORY"
      #pragma section="USER_NO_MEMORY"
      #pragma section="USER_HEAP"
      #pragma section="USER_DEFAULT_MEMORY"
    #endif

    #define BSP_DEFAULT_START_OF_KERNEL_AREA                    ((void *)__KERNEL_AREA_START)
    #define BSP_DEFAULT_END_OF_KERNEL_AREA                      ((void *)__KERNEL_AREA_END)

    #define BSP_DEFAULT_START_OF_USER_DEFAULT_MEMORY            ((void *)__sfb("USER_DEFAULT_MEMORY"))
    #define BSP_DEFAULT_END_OF_USER_DEFAULT_MEMORY              ((void *)__sfe("USER_DEFAULT_MEMORY"))

    #define BSP_DEFAULT_START_OF_USER_HEAP                      ((void *)__sfb("USER_HEAP"))
    #define BSP_DEFAULT_END_OF_USER_HEAP                        ((void *)__USER_AREA_END)

    #define BSP_DEFAULT_START_OF_USER_RW_MEMORY                 ((void *)__sfb("USER_RW_MEMORY"))
    #define BSP_DEFAULT_END_OF_USER_RW_MEMORY                   ((void *)__sfe("USER_RW_MEMORY"))

    #define BSP_DEFAULT_START_OF_USER_RO_MEMORY                 ((void *)__sfb("USER_RO_MEMORY"))
    #define BSP_DEFAULT_END_OF_USER_RO_MEMORY                   ((void *)__sfe("USER_RO_MEMORY"))

    #define BSP_DEFAULT_START_OF_USER_NO_MEMORY                 ((void *)__sfb("USER_NO_MEMORY"))
    #define BSP_DEFAULT_END_OF_USER_NO_MEMORY                   ((void *)__sfe("USER_NO_MEMORY"))

    #define BSP_DEFAULT_MAX_USER_TASK_PRIORITY                  (0)
    #define BSP_DEFAULT_MAX_USER_TASK_COUNT                     (0)

  #endif // MQX_ENABLE_USER_MODE

  #define BSP_DEFAULT_START_OF_KERNEL_MEMORY                    ((void *)__KERNEL_DATA_START)
  #define BSP_DEFAULT_END_OF_KERNEL_MEMORY                      ((void *)__KERNEL_DATA_END)
  #define BSP_DEFAULT_PROCESSOR_NUMBER                          ((uint32_t)__DEFAULT_PROCESSOR_NUMBER)

/* MGCT: <option type="string" quoted="false" allowempty="false"/> */
  #ifndef BSP_DEFAULT_INTERRUPT_STACK_SIZE
    #define BSP_DEFAULT_INTERRUPT_STACK_SIZE                  ((uint32_t)__DEFAULT_INTERRUPT_STACK_SIZE)
  #endif

/* MGCT: <option type="list">
** <item name="1 (all levels disabled)" value="(1L)"/>
** <item name="2" value="(2L)"/>
** <item name="3" value="(3L)"/>
** <item name="4" value="(4L)"/>
** <item name="5" value="(5L)"/>
** <item name="6" value="(6L)"/>
** <item name="7" value="(7L)"/>
** </option>
*/
  #ifndef BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX
    #define BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX      (2L)
  #endif

/*
** MGCT: <option type="number"/>
*/
  #ifndef BSP_DEFAULT_MAX_MSGPOOLS
    #define BSP_DEFAULT_MAX_MSGPOOLS                          (51L)
  #endif

/*
** MGCT: <option type="number"/>
*/
  #ifndef BSP_DEFAULT_MAX_MSGQS
    #define BSP_DEFAULT_MAX_MSGQS                             (100L)
  #endif

/*
 * Other Serial console options:(do not forget to enable BSPCFG_ENABLE_TTY define if changed)
 *      "ttyc:"      TWR-SER and OSJTAG-COM     polled mode
 *      "ittyc:"     TWR-SER and OSJTAG-COM    interrupt mode
 *      "iodebug:"   IDE debug console
 ** MGCT: <option type="string" maxsize="256" quoted="false" allowempty="false"/>
 */
  #ifndef BSP_DEFAULT_IO_CHANNEL
    #if BSPCFG_ENABLE_ITTYE
      #define BSP_DEFAULT_IO_CHANNEL                        "ittye:"
      #define BSP_DEFAULT_IO_CHANNEL_DEFINED
    #else
      #define BSP_DEFAULT_IO_CHANNEL                        NULL
    #endif
  #else
/* undef is for backward compatibility with user_configh.h files which have already had it defined */
    #undef  BSP_DEFAULT_IO_CHANNEL_DEFINED
    #define BSP_DEFAULT_IO_CHANNEL_DEFINED
  #endif

/*
** MGCT: <option type="string" maxsize="1024" quoted="false" allowempty="false"/>
*/
  #ifndef BSP_DEFAULT_IO_OPEN_MODE
//    #define BSP_DEFAULT_IO_OPEN_MODE                          (void *) (IO_SERIAL_XON_XOFF | IO_SERIAL_TRANSLATION | IO_SERIAL_ECHO) // Предыдущая конфигурация
    #define BSP_DEFAULT_IO_OPEN_MODE                          (void *) ( 0 )
  #endif

/*
** FLASHX flash memory pool minimum size
** MGCT: <option type="string" maxsize="1024" quoted="false" allowempty="false"/>
*/
  #if BSPCFG_ENABLE_FLASHX
    #ifndef BSPCFG_FLASHX_SIZE
      #define BSPCFG_FLASHX_SIZE      0x4000
    #endif
  #else
    #undef  BSPCFG_FLASHX_SIZE
    #define BSPCFG_FLASHX_SIZE      0x0
  #endif

/*
** FLASHX swap indicator address
** MGCT: <option type="string" maxsize="1024" quoted="false" allowempty="false"/>
*/
  #if BSPCFG_ENABLE_FLASHX
    #ifndef BSPCFG_SWAP_INDICATOR_ADDR
/* defines last sector of first half of flash space */
      #define BSPCFG_SWAP_INDICATOR_ADDR      (((BSP_INTERNAL_FLASH_SIZE)/2) - BSP_INTERNAL_FLASH_SECTOR_SIZE)
    #endif
  #endif

/*
** FLASHX flash use Programming Acceleration RAM
** MGCT: <option type="bool"/>
*/
  #if BSPCFG_ENABLE_FLASHX
    #ifndef BSPCFG_FLASHX_USE_PA_RAM
      #define BSPCFG_FLASHX_USE_PA_RAM 0
    #endif
  #else
    #undef  BSPCFG_FLASHX_USE_PA_RAM
    #define BSPCFG_FLASHX_USE_PA_RAM 0
  #endif

/** MGCT: </category> */

//#define BSP_FLASH_SIZE                  BSP_BURST_FLASH_SIZE
//#define BSP_FLASH_WIDTH                 (16)
  #define BSP_FLASH_DEVICES               (1)


#endif /* _EmSysK70_h_  */
