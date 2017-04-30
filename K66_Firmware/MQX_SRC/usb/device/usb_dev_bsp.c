/**HEADER********************************************************************
*
* Copyright (c) 2013 - 2014 Freescale Semiconductor;
* All Rights Reserved
*
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************
*
* Comments:
*
*END************************************************************************/
#include "adapter.h"
#include "usb.h"
#include "usb_device_config.h"
#include "usb_misc.h"
#include "fsl_usb_ehci_phy_hal.h"
#include "MK65F18.h"


extern uint8_t soc_get_usb_vector_number(uint8_t controller_id);
extern uint32_t soc_get_usb_base_address(uint8_t controller_id);
#ifdef __cplusplus
extern "C"
{
#endif
extern _WEAK_FUNCTION(usb_status bsp_usb_dev_board_init(uint8_t controller_id));
#ifdef __cplusplus
}
#endif


#define BSP_USB_INT_LEVEL                (4)
#define USB_CLK_RECOVER_IRC_EN (*(volatile unsigned char *)0x40072144)
#define SIM_SOPT2_IRC48MSEL_MASK                 0x30000u
#ifdef BSPCFG_USB_USE_IRC48M
  #undef BSPCFG_USB_USE_IRC48M
#endif
#define BSPCFG_USB_USE_IRC48M            (1)
#define crystal_val                       12000000
#define USBHS_USBMODE_CM_IDLE_MASK    USBHS_USBMODE_CM(0)
#define USBHS_USBMODE_CM_DEVICE_MASK  USBHS_USBMODE_CM(2)
#define USBHS_USBMODE_CM_HOST_MASK    USBHS_USBMODE_CM(3)


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : bsp_usb_dev_soc_init
* Returned Value   : USB status
* Comments         :
*    This function performs BSP-specific I/O initialization related to USB
*
*END*----------------------------------------------------------------------*/
static usb_status bsp_usb_dev_soc_init
(
 int32_t controller_id
)
{
  usb_status ret = 0;

  /* MPU is disabled. All accesses from all bus masters are allowed */
  MPU_CESR = 0;
  if (USB_CONTROLLER_KHCI_0 == controller_id)
  {
#if (OS_ADAPTER_ACTIVE_OS == OS_ADAPTER_MQX)
  #if BSPCFG_USB_USE_IRC48M

    /*
    * Configure SIM_CLKDIV2: USBDIV = 0, USBFRAC = 0
    */
    SIM_CLKDIV2 = (uint32_t)0x0UL; /* Update USB clock pre-scalers */

    /* Configure USB to be clocked from IRC 48MHz */
    SIM_SOPT2_REG(SIM_BASE_PTR) |= SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_IRC48MSEL_MASK;

    /* Enable USB-OTG IP clocking */
    SIM_SCGC4_REG(SIM_BASE_PTR) |= SIM_SCGC4_USBOTG_MASK;
    /* Enable IRC 48MHz for USB module */
    USB_CLK_RECOVER_IRC_EN = 0x03;
  #else
    /* Enable USB-OTG IP clocking */
    SIM_SCGC4_REG(SIM_BASE_PTR) |= SIM_SCGC4_USBOTG_MASK;
    /* Configure USB to be clocked from PLL0 */
    SIM_SOPT2_REG(SIM_BASE_PTR) |= SIM_SOPT2_USBSRC_MASK;
    SIM_SOPT2_REG(SIM_BASE_PTR) |= SIM_SOPT2_PLLFLLSEL(1);
    /* Configure USB divider to be 120MHz * 2 / 5 = 48 MHz */
    SIM_CLKDIV2_REG(SIM_BASE_PTR) = SIM_CLKDIV2_USBDIV(4) | SIM_CLKDIV2_USBFRAC_MASK;
  #endif
#else
    /* Configure USB to be clocked from PLL0 */
    SIM_SOPT2_REG(SIM_BASE_PTR) |= SIM_SOPT2_USBSRC_MASK;
    /* Configure USB divider to be 120MHz * 2 / 5 = 48 MHz */
    SIM_CLKDIV2_REG(SIM_BASE_PTR) = SIM_CLKDIV2_USBDIV(4) | SIM_CLKDIV2_USBFRAC_MASK;

    /* Enable USB-OTG IP clocking */
    SIM_SCGC4_REG(SIM_BASE_PTR) |= SIM_SCGC4_USBOTG_MASK;

#endif
    /* Configure enable USB regulator for device */
    SIM_SOPT1_REG(SIM_BASE_PTR) |= SIM_SOPT1CFG_URWE_MASK;
    SIM_SOPT1_REG(SIM_BASE_PTR) |= SIM_SOPT1_USBREGEN_MASK;

    /* reset USB CTRL register */
    USB_USBCTRL_REG(USB0_BASE_PTR) = 0;

    /* Enable internal pull-up resistor */
    USB_CONTROL_REG(USB0_BASE_PTR) = USB_CONTROL_DPPULLUPNONOTG_MASK;
    USB_USBTRC0_REG(USB0_BASE_PTR) |= 0x40; /* Software must set this bit to 1 */
    /* setup interrupt */
    OS_intr_init(soc_get_usb_vector_number(controller_id), BSP_USB_INT_LEVEL, 0, TRUE);
  }
  else if (USB_CONTROLLER_EHCI_0 == controller_id)
  {

    /*
    * Four conditions need to be set for using USB HS PHY PLL
    * 1. 32kHz IRC clock enable
    * 2. external reference clock enable on XTAL
    * 3. USB PHY 1.2V PLL regulator enabled
    * 4. 3.3V USB regulator enabled, which means either VREGIN0 or VREGIN1
    *    should be connected with 5V input
    */
    MCG_C1 |= MCG_C1_IRCLKEN_MASK;    //32kHz IRC enable
    OSC_CR |= OSC_CR_ERCLKEN_MASK;   //external reference clock enable

    // Configure EXT_PLL from USB HS PHY
    SIM_SOPT2 |= SIM_SOPT2_USBREGEN_MASK; // | SIM_SOPT2_PLLFLLSEL(2); //enable USB PHY PLL regulator, needs to be enabled before enable PLL
    SIM_SCGC3 |= SIM_SCGC3_USBHS_MASK | SIM_SCGC3_USBHSPHY_MASK;  //open HS USB PHY clock gate
    OS_Time_delay(1);
    SIM_USBPHYCTL = SIM_USBPHYCTL_USB3VOUTTRG(6) | SIM_USBPHYCTL_USBVREGSEL_MASK; //trim the USB regulator output to be 3.13V


    usb_hal_ehci_phy_trim_override_enable(controller_id); //override IFR value

    usb_hal_ehci_phy_pll_enalbe_power_up(controller_id); //power up PLL

    usb_hal_ehci_phy_select_pll_reference_clock(controller_id, crystal_val);

    usb_hal_ehci_phy_pll_clear_bypass(controller_id);  //clear bypass bit

    usb_hal_ehci_phy_enable_usb_clock(controller_id);  //enable USB clock output from USB PHY PLL

    usb_hal_ehci_phy_wait_pll_lock(controller_id);

    usb_hal_ehci_phy_release_from_reset(controller_id);  //release PHY from reset

    usb_hal_ehci_phy_run_clock(controller_id); //Clear to 0 to run clocks

    usb_hal_ehci_phy_enable_utmi_level2(controller_id);
    usb_hal_ehci_phy_enable_utmi_level3(controller_id);

    usb_hal_ehci_phy_set_power_state_to_normal(controller_id);   //for normal operation

    usb_hal_ehci_phy_set_pfd_frac_value(controller_id, 24); //N=24

    usb_hal_ehci_phy_select_pfd_clock_divider(controller_id, 4);      //div by 4

    usb_hal_ehci_phy_disable_pulldown_resistor(controller_id);
    usb_hal_ehci_phy_disable_pfd_clock_gate(controller_id);
    usb_hal_ehci_phy_wait_pfd_stable(controller_id);

    USBPHY_TX |= 1 << 24;
    USBHS_USBCMD |= USBHS_USBCMD_RST_MASK;
    while (USBHS_USBCMD & USBHS_USBCMD_RST_MASK)
    {/* delay while resetting USB controller */}

    USBHS_USBMODE = USBHS_USBMODE_CM_DEVICE_MASK;
    /* Set interrupt threshold control = 0 */
    USBHS_USBCMD &= ~(USBHS_USBCMD_ITC(0xFF));
    /* Setup Lockouts Off */
    USBHS_USBMODE |= USBHS_USBMODE_SLOM_MASK;

    /* setup interrupt */
    OS_intr_init(soc_get_usb_vector_number(controller_id), BSP_USB_INT_LEVEL, 0, TRUE);

  }
  else
  {
    ret = USBERR_BAD_STATUS; //unknown controller
  }

  return ret;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : bsp_usb_dev_board_init
* Returned Value   : USB status
* Comments         :
*    This function performs board-specific initialization related to USB
*
*END*----------------------------------------------------------------------*/
_WEAK_FUNCTION(usb_status bsp_usb_dev_board_init(uint8_t controller_id))
{
  usb_status ret = USB_OK;

  if (USB_CONTROLLER_KHCI_0 == controller_id)
  {

  }
  else if (USB_CONTROLLER_EHCI_0 == controller_id)
  {
    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
    PORT_PCR_REG(PORTD_BASE_PTR, 8) = PORT_PCR_MUX(0x01);
    GPIO_PDDR_REG(PTD_BASE_PTR) |= 1 << 8; // PB8 as output
    GPIO_PDOR_REG(PTD_BASE_PTR) &= ~(1 << 8); // PB8 in low level

  }
  else
  {
    ret = USBERR_BAD_STATUS;
  }

  return ret;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : bsp_usb_dev_init
* Returned Value   : USB status
* Comments         :
*    This function performs BSP-specific initialization related to USB
*
*END*----------------------------------------------------------------------*/
usb_status bsp_usb_dev_init(uint8_t controller_id)
{
  usb_status ret = USB_OK;

  ret = bsp_usb_dev_soc_init(controller_id);
  if (ret == USB_OK)
  {
    ret = bsp_usb_dev_board_init(controller_id);
  }
  return ret;
}

/* EOF */
