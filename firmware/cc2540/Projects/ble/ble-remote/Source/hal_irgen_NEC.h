/**************************************************************************************************
  Filename:       hal_irgenHw_NEC.h
  Revised:        $Date$
  Revision:       $Revision$

  Description:    This file contains the interface of the IR signal generation driver for the
                  NEC format. The driver is capable of generating non-modulated and modulated
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

#ifndef HAL_IRGENHW_NEC_H
#define HAL_IRGENHW_NEC_H

/*********************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "hal_irgen.h"

/******************************************************************************
 * MACROS
 */
#define HAL_IRGEN_CMD_NEC(_addr,_cmd) \
  (((uint32)((~(_cmd))&0xFF) << 24) | \
    ((uint32)(_cmd) << 16) | \
      ((uint32)((~(_addr))&0xFF) << 8) | \
        (uint32)(_addr))

#define HAL_IRGEN_CMD_NEC_EX(_addr,_cmd) ( ((uint32)((~(_cmd))&0xFF) << 24) | ((uint32)(_cmd) << 16) | (uint32)(_addr & 0xFFFF) )

/******************************************************************************
 * CONSTANTS
 */
// Signal repeat interval
#define HAL_IRGEN_NEC_REPEAT_INTERVAL  110 // ms

// NEC format: 2*8 bits for addr, 2*8 bits for data
#define HAL_IRGEN_NEC_CMD_SIZE    32

#ifdef HAL_IRGEN_CARRIER
  // 38kHz carrier generation (Timer 3) 
  // Duty cycle counter = ticksp / prescaler / 38kHz
  // 8000k / 1 / 38k = 210.53 ~= 211 = 0xD3
  // 33% Duty cycle; 0xD3/3 = 0x46
  #define HAL_IRGEN_NEC_CARRIER_DUTY_CYCLE  0xD2 // Subtract one for 0th clk cycle
  #define HAL_IRGEN_NEC_CARRIER_ACTIVE_PER  0x46

  // Bit signal generation (Timer 1)
  // Timer 1 tick input from Timer3 Ch0 (38kHz)
  // bit counter : Active Periode * tickspeed / prescaler
  // '1' : 2.25 mSec * 38kHz / 1 = 85.5 ~= 86 = 0x56
  // '0' : 1.12 mSec * 38kHz / 1 = 42.6 ~= 42 = 0x2A
  #define HAL_IRGEN_NEC_BIT_1_PER   0x0055 //subtract one for 0th clk cycle
  #define HAL_IRGEN_NEC_BIT_0_PER   0x0029 //subtract one for 0th clk cycle
#else
  // Bit signal generation (Timer 1)
  // Timer 1 tick input configured directly to tickspeed 8MHz
  // bit counter : Active Periode * tickspeed / prescaler
  // '1' : 2.25 mSec * 8MHz / 1 = 18000 = 0x4650
  // '0' : 1.12 mSec * 8MHz / 1 = 8960 = 0x2300
  #define HAL_IRGEN_NEC_BIT_1_PER   0x464F //subtract one for 0th clk cycle
  #define HAL_IRGEN_NEC_BIT_0_PER   0x22FF //subtract one for 0th clk cycle
#endif // HAL_IRGEN_CARRIER

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
 * @fn      HalIrGenInitNec
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
extern void HalIrGenInitNec(void);


/******************************************************************************
 * @fn      HalIrGenCommandNec
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
extern void HalIrGenCommandNec(uint32 command);

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
extern void HalIrGenRepeatNec(void);


#endif // HAL_IRGENHW_NEC_H
/**************************************************************************************************
 **************************************************************************************************/
