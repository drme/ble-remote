/**************************************************************************************************
  Filename:       hal_irgen.h
  Revised:        $Date: 2010-10-12 17:32:20 -0700 (Tue, 12 Oct 2010) $
  Revision:       $Revision: 24108 $

  Description:    This file contains generic definition for IR signal generation.


  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

#ifndef HAL_IRGEN_H
#define HAL_IRGEN_H

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"

/******************************************************************************
 * MACROS
 */
// MACRO for swapping bit order in a byte to reverse order.
#define HAL_IRGEN_SWAP_BITS(_data) \
  ((((_data) & 0x80) >> 7) | \
    (((_data) & 0x40) >> 5) | \
      (((_data) & 0x20) >> 3) | \
        (((_data) & 0x10) >> 1) | \
          (((_data) & 0x08) << 1) | \
            (((_data) & 0x04) << 3) | \
              (((_data) & 0x02) << 5) | \
                (((_data) & 0x01) << 7))

/******************************************************************************
 * CONSTANTS
 */

// Select P1.1 (timer 1 ch 1) as output port
#define HAL_IRGEN_P1SEL_PORT    0x02

// Select DMA channels for the IR signal generation timer. 
#define HAL_IRGEN_DMA_CH_CC1    3
#define HAL_IRGEN_DMA_CH_CC0    4

// ISR in hal_dma.c will check on this symbol
#define HAL_IRGEN_DMA_CH        HAL_IRGEN_DMA_CH_CC1


// -- Enumeration values for configuration items --

// tick speed enumeration
// For code optimization the value corresponds to actual register bit value
#define HAL_IRGEN_TICKSPD_32MHZ               0x00
#define HAL_IRGEN_TICKSPD_16MHZ               0x08
#define HAL_IRGEN_TICKSPD_8MHZ                0x10
#define HAL_IRGEN_TICKSPD_4MHZ                0x18
#define HAL_IRGEN_TICKSPD_2MHZ                0x20
#define HAL_IRGEN_TICKSPD_1MHZ                0x28
#define HAL_IRGEN_TICKSPD_500KHZ              0x30
#define HAL_IRGEN_TICKSPD_250KHZ              0x38

// Carrier PWM signal tick prescaler enumeration
// For code optimization the value corresponds to Timer 3 prescaler bits
#define HAL_IRGEN_CARRIER_PRESCALER_DIV1      0x00
#define HAL_IRGEN_CARRIER_PRESCALER_DIV2      0x20
#define HAL_IRGEN_CARRIER_PRESCALER_DIV4      0x40
#define HAL_IRGEN_CARRIER_PRESCALER_DIV8      0x60
#define HAL_IRGEN_CARRIER_PRESCALER_DIV16     0x80
#define HAL_IRGEN_CARRIER_PRESCALER_DIV32     0xA0
#define HAL_IRGEN_CARRIER_PRESCALER_DIV64     0xC0
#define HAL_IRGEN_CARRIER_PRESCALER_DIV128    0xE0

// Bit timing tick prescaler enumeration
// For code optimization the value corresponds to Timer 1 prescaler bits
#define HAL_IRGEN_BIT_TIMING_PRESCALER_DIV1   0x00
#define HAL_IRGEN_BIT_TIMING_PRESCALER_DIV8   0x04
#define HAL_IRGEN_BIT_TIMING_PRESCALER_DIV32  0x08
#define HAL_IRGEN_BIT_TIMING_PRESCALER_DIV128 0x0C

// -- Register values --

#define HAL_IRGEN_PERCFG_T1CFG         BV(6)

#define HAL_IRGEN_CLKCON_TICKSPD_MASK  0x38

#define HAL_IRGEN_TxCCTLx_CMP_SET      0x00
#define HAL_IRGEN_TxCCTLx_CMP_CLEAR    0x08
#define HAL_IRGEN_TxCCTLx_CMP_TOGGLE   0x10
#define HAL_IRGEN_TxCCTLx_CMP_SET_CLR  0x18
#define HAL_IRGEN_TxCCTLx_CMP_CLR_SET  0x20
#define HAL_IRGEN_TxCCTLx_MODE_COMPARE 0x04

#ifdef HAL_IRGEN_CARRIER
#define HAL_IRGEN_T3CTL_START          0x10
#define HAL_IRGEN_T3CTL_CLR            0x04
#define HAL_IRGEN_T3CTL_MODE_MODULO    0x02
#endif // HAL_IRGEN_CARRIER

#define HAL_IRGEN_T1CTL_MODE_SUSPEND   0x00
#define HAL_IRGEN_T1CTL_MODE_MODULO    0x02

#define HAL_IRGEN_IEN1_DMAIE           0x01
#define HAL_IRGEN_IRCON_DMAIF          0x01

// -- Register Addresses --

#define HAL_IRGEN_T1CC0L_ADDR          (&X_T1CC0L)
#define HAL_IRGEN_T1CC1L_ADDR          (&X_T1CC1L)

#ifndef IRCTL
# define IRCTL                          XREG( 0x6281 )
#endif

extern uint8 halIrGenTimerRunning;


/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

/******************************************************************************
 * FUNCTION PROTOTYPES
 */

/******************************************************************************
 * @fn      HalIrGenDmaIsr
 *
 * @brief   Handles DMA interrupt that comes upon completion of IR signal generation.
 *          This function is called from the DMA interrupt service routine and
 *          managed by the IR generation module.
 *
 *          Procssing should be kept to an absolute minimin in this routine since
 *          it's running in IAR context
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
extern void HalIrGenDmaIsr(void);


/******************************************************************************
 * @fn      HalIrGenIsrCback
 *
 * @brief   This function is a callback function that a user application has to
 *          define. HAL IR signal generation driver will call this callback
 *          function upon completion of IR signal generation.
 *
 *          NOTE that this function is called in the ISR context and should as
 *          such do a minimum of processsing. A prudent design will set an
 *          application event to delay further processing.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
extern void HalIrGenIsrCback(void);


#endif // HAL_IRGEN_H
/**************************************************************************************************
 **************************************************************************************************/
