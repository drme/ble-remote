/**************************************************************************************************
  Filename:       hal_irgenHw_NSIRC.h
  Revised:        $Date: 2010-11-01 16:39:15 -0700 (Mon, 01 Nov 2010) $
  Revision:       $Revision: 24304 $

  Description:    This file contains the interface of the IR signal generation driver for the
                  SIRC format. The driver is capable of generating non-modulated and modulated
                  signals. The driver is leveraging special hardware for IR signal generation in CC253x.

                  The module requires HAL_IRGEN compile flag to be set to TRUE to be built in. It also
                  requires the HAL_IRGEN_CARRIER to be set to generate a modulated signal

                  The distinct feature of this driver is that it minimizes interaction with CPU
                  by relying on DMA to reprogram a timer to generate signals.

                  The output signal will be generated onto Port 1 Pin 1.
                  The driver will use two DMA channels, Timer 1 and also Timer 3 for modulated signals,
                  and hence it will have conflict with any other drivers that uses the same
                  resources. (Note that AES driver must not be configured to use DMA).

                  Timer 3 is used to generate carrier pulse signals while
                  Timer 1 is used to generate bit signals on top of the carrier signals.

  Copyright 2009 Texas Instruments Incorporated. All rights reserved.

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

#ifndef HAL_IRGENHW_SIRC_H
#define HAL_IRGENHW_SIRC_H

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "hal_irgen.h"

/******************************************************************************
 * MACROS
 */
#define HAL_IRGEN_CMD_SIRC(_addr,_cmd) \
  (((uint16)((_addr) & 0x1F) << 7) | \
    (uint16)((_cmd) & 0x7F))


/******************************************************************************
 * CONSTANTS
 */

// Signal repeat interval
#define HAL_IRGEN_SIRC_REPEAT_INTERVAL  45 // ms


// SIRC 12-bit format: 5 bits for addr, 7 bits for data
#define HAL_IRGEN_SIRC_CMD_SIZE    12

#ifdef HAL_IRGEN_CARRIER
  // 40kHz carrier generation (Timer 3)
  // Duty cycle counter = ticksp / prescaler / 40kHz
  // 8000k / 1 / 40k = 200 = 0xC8
  // 25% Duty cycle; 0xC8/4 = 0x32
  #define HAL_IRGEN_SIRC_CARRIER_DUTY_CYCLE  0xC7 // Subtract one for 0th clk cycle
  #define HAL_IRGEN_SIRC_CARRIER_ACTIVE_PER  0x32

  // Bit signal generation (Timer 1)
  // Timer 1 tick input from Timer3 Ch0 (40kHz)
  // bit counter : Active Periode * tickspeed / prescaler
  // '1' : 1.2 mSec * 40kHz / 1 = 48 = 0x30
  // '0' : 0.6 mSec * 40kHz / 1 = 24 = 0x18
  #define HAL_IRGEN_SIRC_BIT_1_PER   0x0030
  #define HAL_IRGEN_SIRC_BIT_0_PER   HAL_IRGEN_SIRC_BIT_1_PER/2

#else
  // Bit signal generation (Timer 1)
  // Timer 1 tick input configured directly to tickspeed 8MHz
  // bit counter : Active Periode * tickspeed / prescaler
  // '1' : 1.2 mSec * 8MHz / 1 = 9600 = 0x2580
  // '0' : 0.6 mSec * 8MHz / 1 = 4800 = 0x12C0
  #define HAL_IRGEN_SIRC_BIT_1_PER   0x2580
  #define HAL_IRGEN_SIRC_BIT_0_PER   HAL_IRGEN_SIRC_BIT_1_PER/2
#endif // HAL_IRGEN_CARRIER

// Bit signal duty cycle, subract 1 for 0th clk cycle
#define HAL_IRGEN_SIRC_BIT_1_DUTY_CYCLE (3 * HAL_IRGEN_SIRC_BIT_0_PER - 1)
#define HAL_IRGEN_SIRC_BIT_0_DUTY_CYCLE (2 * HAL_IRGEN_SIRC_BIT_0_PER - 1)


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
 * @fn      HalIrGenInitSirc
 *
 * @brief   Initialize driver
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
extern void HalIrGenInitSirc(void);


/******************************************************************************
 * @fn      HalIrGenCommandSirc
 *
 * @brief   Generate IR signal corresponding to a command
 *
 * input parameters
 *
 * @param   command - upto 32 bit command. Note that the type of this parameter
 *          is determined at compile time according to the HAL_IRGEN_CMD_LENGTH
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
extern void HalIrGenCommandSirc(uint16 command);

/******************************************************************************
 * @fn      HalIrGenRepeatNec
 *
 * @brief   Generate repeat IR signal corresponding to NEC protocol
 *
 * input parameters
 *
 * @param   command - upto 32 bit command. Note that the type of this parameter
 *          is determined at compile time according to the HAL_IRGEN_CMD_LENGTH
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
extern void HalIrGenRepeatSirc(void);


#endif // HAL_IRGENHW_SIRC_H
/**************************************************************************************************
 **************************************************************************************************/
