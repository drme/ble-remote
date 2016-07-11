/**************************************************************************************************
  Filename:       hal_irgen_RC5.c
  Revised:        $Date$
  Revision:       $Revision$

  Description:    This file contains the implementation of the IR signal generation driver for the
                  RC5 format. The driver is capable of generating non-modulated and modulated
                  signals. The driver is leveraging special hardware for IR signal generation in CC253x.

                  The module requires HAL_IRGEN compile flag to be set to be built in. It also
                  requires the HAL_IRGEN_CARRIER to be set to generate a modulated signal

                  The distinct feature of this driver is that it minimizes interaction with CPU
                  by relying on DMA to reprogram a timer to generate signals. Timer 3 is used to
                  generate carrier pulse signals while Timer 1 is used to generate bit signals on
                  top of the carrier signals. The driver will use two DMA channels, and hence it will
                  have conflict with any other drivers that uses the same resources. The actual 
                  DMA channels used can be configures.

                  The output signal will be generated on the Timer 1 channel 1 pin. This sample code
                  configures the output to be on alt. 2 (Port 1 Pin 1). This can be changes to alt. 1
                  (Port 0 pin 3). 

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

/*********************************************************************
 * INCLUDES
 */
#ifdef HAL_IRGEN

// Hardware Abstraction Layer, HAL
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_drivers.h"
#include "hal_dma.h"

// IR singal generation for the RC5 format
#include "hal_irgen_RC5.h"

/******************************************************************************
 * CONSTANTS
 */



/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

// These buffers will contain the timing information for the IR signal
// Add one item to hold a dummy value to complete signal generation
static uint16 halIrGenCc0Buf[HAL_IRGEN_RC5_CMD_SIZE + 1];
static uint16 halIrGenCc1Buf[HAL_IRGEN_RC5_CMD_SIZE + 1];

// Index into CCO and CC1 buffer
static uint8 idx; 

// Timer status
//static uint8 halIrGenTimerRunning;

// DMA channel descriptors
static halDMADesc_t *pDmaDescCc0;
static halDMADesc_t *pDmaDescCc1;

/******************************************************************************
 * GLOBAL VARIABLES
 */


/******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

static void startIrGenRc5(uint8 IrBufSize);

/******************************************************************************
 * EXPORTED FUNCTIONS
 */

/******************************************************************************
 * @fn      HalIrGenInitRc5
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
void HalIrGenInitRc5(void)
{
	/*
	
	T1CTL = 0;
	
  // Set TICKSPD
  CLKCONCMD &= ~HAL_IRGEN_CLKCON_TICKSPD_MASK;
  CLKCONCMD |= HAL_IRGEN_TICKSPD_8MHZ;

  // Select port direction to output
  P1DIR |= HAL_IRGEN_P1SEL_PORT;

  // Initially clear the port so that there will be no conflict
  P1 &= ~HAL_IRGEN_P1SEL_PORT;

  // Select port function to peripheral
  P1SEL |= HAL_IRGEN_P1SEL_PORT;

  // Select alternative 2 location for T1 CH1 output (P1.1)
  PERCFG |= HAL_IRGEN_PERCFG_T1CFG;
	
	
	
	P0DIR = 0xff;//(1 << 3);
	P0 = 0x00;
	P0SEL = 0xff;//|= (1 << 3);
	PERCFG &= ~(1 << 6);
	PERCFG |= (1 << 0);
	PERCFG |= (1 << 1);	
	
  
  // -- set up bit signal generation timer --
  // -- run timer once to make sure output is deactivated
  
  // Halt timer 1
  T1CTL = HAL_IRGEN_T1CTL_MODE_SUSPEND;
  
  // Set up timer 1 channel 0 to compare mode 4
  T1CCTL0 = HAL_IRGEN_TxCCTLx_CMP_CLR_SET | HAL_IRGEN_TxCCTLx_MODE_COMPARE;

#if !defined (HAL_IRGEN_CARRIER) && defined (HAL_IRGEN_ACTIVE_LOW)
  // Set up timer 1 channel 1 to compare mode 3 (active low output)
  T1CCTL1 = HAL_IRGEN_TxCCTLx_CMP_SET_CLR | HAL_IRGEN_TxCCTLx_MODE_COMPARE;
#else
  // Set up timer 1 channel 1 to compare mode 4 (active high output)
  T1CCTL1 = HAL_IRGEN_TxCCTLx_CMP_CLR_SET | HAL_IRGEN_TxCCTLx_MODE_COMPARE;
#endif

  // Run one timer 1 until output is pulled low. 
  T1CC0L = 2;
  T1CC0H = 0;
  T1CC1L = 1;
  T1CC1H = 0;
  
  // Clear timer 1
  // this will activate the output pin so start timer immediately.
  T1CNTL = 0;
  
  // Start timer 1
  T1CTL = HAL_IRGEN_BIT_TIMING_PRESCALER_DIV1 | HAL_IRGEN_T1CTL_MODE_MODULO;
  
  // wait till the single bit is cleared
  while (T1CNTL == 0);
  
  // stop timer 1
  T1CTL = HAL_IRGEN_T1CTL_MODE_SUSPEND;
*/
  
#ifdef HAL_IRGEN_CARRIER
   // -- set up carrier signal generation timer --
  // Clear counter and halt the timer
  T3CTL = HAL_IRGEN_T3CTL_CLR;
  
  // Set up timer 3 channel 0 to compare mode 4
  T3CCTL0 = HAL_IRGEN_TxCCTLx_CMP_CLR_SET | HAL_IRGEN_TxCCTLx_MODE_COMPARE;

  // Set up timer 3 channel 1 to compare mode 4
  T3CCTL1 = HAL_IRGEN_TxCCTLx_CMP_CLR_SET | HAL_IRGEN_TxCCTLx_MODE_COMPARE;
  
  // Configure 36kHz carrier with 33% duty cycle
  T3CC0 = HAL_IRGEN_RC5_CARRIER_DUTY_CYCLE;
  T3CC1 = HAL_IRGEN_RC5_CARRIER_ACTIVE_PER;

  // Combine carrier signal (Timer 1 CH 1 and Timer 3 CH 1 output)
  IRCTL |= 1;
#endif // HAL_IRGEN_CARRIER
  
  // -- Configure DMA --
  
  // Set up DMA channel for CC0
#if HAL_IRGEN_DMA_CH_CC0 == 0
  pDmaDescCc0 = HAL_DMA_GET_DESC0();
#else
  pDmaDescCc0 = HAL_DMA_GET_DESC1234(HAL_IRGEN_DMA_CH_CC0);
#endif
  
  // The start address of the destination.
  HAL_DMA_SET_DEST(pDmaDescCc0, HAL_IRGEN_T1CC0L_ADDR);

  // Using the length field to determine how many bytes to transfer.
  HAL_DMA_SET_VLEN(pDmaDescCc0, HAL_DMA_VLEN_USE_LEN);

  // Two bytes are transferred each time.
  HAL_DMA_SET_WORD_SIZE(pDmaDescCc0, HAL_DMA_WORDSIZE_WORD);

  // One word is transferred each time
  HAL_DMA_SET_TRIG_MODE(pDmaDescCc0, HAL_DMA_TMODE_SINGLE);
  
  // Timer 1 channel 1 trigger DMA xfer.
  HAL_DMA_SET_TRIG_SRC(pDmaDescCc0, HAL_DMA_TRIG_T1_CH1);

  // The source address is incremented by 1 word for each transfer.
  HAL_DMA_SET_SRC_INC(pDmaDescCc0, HAL_DMA_SRCINC_1);

  // The destination address is constant - T1CC0.
  HAL_DMA_SET_DST_INC(pDmaDescCc0, HAL_DMA_DSTINC_0);

  // IRQ handler is set up to tigger on CC1
  HAL_DMA_SET_IRQ(pDmaDescCc0, HAL_DMA_IRQMASK_DISABLE);

  // Xfer all 8 bits of a byte xfer.
  HAL_DMA_SET_M8(pDmaDescCc0, HAL_DMA_M8_USE_8_BITS);

  // Set highest priority
  HAL_DMA_SET_PRIORITY(pDmaDescCc0, HAL_DMA_PRI_HIGH);
  
  // Set source data stream
  HAL_DMA_SET_SOURCE(pDmaDescCc0, &halIrGenCc0Buf[1]);
  
  // Set up DMA channel for CC1
#if HAL_IRGEN_DMA_CH_CC1 == 0
  pDmaDescCc1 = HAL_DMA_GET_DESC0();
#else
  pDmaDescCc1 = HAL_DMA_GET_DESC1234(HAL_IRGEN_DMA_CH_CC1);
#endif  
  
  // The start address of the destination.
  HAL_DMA_SET_DEST(pDmaDescCc1, HAL_IRGEN_T1CC1L_ADDR);

  // Using the length field to determine how many bytes to transfer.
  HAL_DMA_SET_VLEN(pDmaDescCc1, HAL_DMA_VLEN_USE_LEN);

  // Two bytes are transferred each time.
  HAL_DMA_SET_WORD_SIZE(pDmaDescCc1, HAL_DMA_WORDSIZE_WORD);

  // One word is transferred each time
  HAL_DMA_SET_TRIG_MODE(pDmaDescCc1, HAL_DMA_TMODE_SINGLE);
  
  // Timer 1 channel 1 trigger DMA xfer.
  HAL_DMA_SET_TRIG_SRC(pDmaDescCc1, HAL_DMA_TRIG_T1_CH1);

  // The source address is incremented by 1 word for each transfer.
  HAL_DMA_SET_SRC_INC(pDmaDescCc1, HAL_DMA_SRCINC_1);

  // The destination address is constant - T1CC1.
  HAL_DMA_SET_DST_INC(pDmaDescCc1, HAL_DMA_DSTINC_0);

  // IRQ handler is set up so that sleep enable/disable can be determined.
  HAL_DMA_SET_IRQ(pDmaDescCc1, HAL_DMA_IRQMASK_ENABLE);

  // Xfer all 8 bits of a byte xfer.
  HAL_DMA_SET_M8(pDmaDescCc1, HAL_DMA_M8_USE_8_BITS);

  // Set highest priority
  HAL_DMA_SET_PRIORITY(pDmaDescCc1, HAL_DMA_PRI_HIGH);
  
  // Set source data stream
  HAL_DMA_SET_SOURCE(pDmaDescCc1, &halIrGenCc1Buf[1]);
 
  // Timer is not running
  halIrGenTimerRunning = FALSE;
}

/******************************************************************************
 * @fn      HalIrGenCommandRc5
 *
 * @brief   Generate IR signal corresponding to a command
 *
 * input parameters
 *
 * @param   command - IR signal generation command
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
void HalIrGenCommandRc5(uint16 command)
{
  uint8 i, currentBit;
  
  idx = 0; // start of new command
  currentBit = 1; // Start bit must be always '1'. Compose _-
  halIrGenCc1Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
  halIrGenCc0Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
  
  
  // Build signal format corresponding to command bits
  for (i = 1; i < HAL_IRGEN_RC5_CMD_SIZE; i++)
  {
    
    if (command & (((uint16)1) << i)) // bit value '1'
    {
      if (currentBit)  // 1 (current) -> 1: _-_-
      {
        halIrGenCc0Buf[idx] += HAL_IRGEN_RC5_BIT_BURST_PER;
        idx++;
        halIrGenCc0Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
        halIrGenCc1Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
      }
      else  // 0 (current) -> 1: -__- 
      {
        halIrGenCc0Buf[idx] += HAL_IRGEN_RC5_BIT_BURST_PER;
        idx++;
        halIrGenCc0Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
        halIrGenCc1Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
        currentBit = 1;
      }
    }
    else  // bit value '0'
    {
      if(currentBit)  // 1 (current) -> 0: _--_
      {
        halIrGenCc0Buf[idx] += 2 * HAL_IRGEN_RC5_BIT_BURST_PER;
        halIrGenCc1Buf[idx] += HAL_IRGEN_RC5_BIT_BURST_PER;
        currentBit=0;
      }
      else  // // 0 (current) -> 0: -_-_
      {
        idx++;
        halIrGenCc0Buf[idx] = 2 * HAL_IRGEN_RC5_BIT_BURST_PER;
        halIrGenCc1Buf[idx] = HAL_IRGEN_RC5_BIT_BURST_PER;
      }
    }
  }
  
  //Post processing to ensure propoer format of last CC1
  halIrGenCc0Buf[idx++] += HAL_IRGEN_RC5_BIT_BURST_PER;

  //subract 1 for each CC0 value to account for 0th clock cycle
  for (i = 0; i< idx; i++)
  {
    halIrGenCc0Buf[i] -= 1;
  }
  
  // Inlcude dummy value as last entry in buffer to make sure all bits are generate
  halIrGenCc0Buf[idx] = 0xFFFF;
  halIrGenCc1Buf[idx] = 0xFFFF;
  
  
  // Generate Signal
  startIrGenRc5(idx + 1);
   
}


/******************************************************************************
 * @fn      HalIrGenRepeatRc5
 *
 * @brief   Generate repeat IR signal corresponding to SIRC protocol
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
void HalIrGenRepeatRc5(void)
{
  // Generate repeat signal 
  startIrGenRc5(idx + 1);
}

  
/******************************************************************************
 * @fn      startIrGenRc5
 *
 * @brief   Generate IR signal corresponding to RC5 protocol
 *
 * input parameters
 *
 * @param   IrBufSize - Size of the buffer containing the IR signal
 *
 * output parameters
 *
 * None.
 *
 * @return  None.
 *
 */
static void startIrGenRc5(uint8 IrBufSize)
{
  halIntState_t intState;
  
  //check if IR generation timers are already in use
  if (halIrGenTimerRunning)
  {
    // Timer is already running. Cannot generate the signals.
    return;
  }
  
  halIrGenTimerRunning = TRUE;
  
  // Set data length, number of items to be xfered by DMA
  HAL_DMA_SET_LEN(pDmaDescCc0, IrBufSize - 1);
  HAL_DMA_SET_LEN(pDmaDescCc1, IrBufSize - 1);
  
  // ARM both DMA channels
  HAL_DMA_CLEAR_IRQ(HAL_IRGEN_DMA_CH_CC0);
  HAL_DMA_ARM_CH(HAL_IRGEN_DMA_CH_CC0);

  HAL_DMA_CLEAR_IRQ(HAL_IRGEN_DMA_CH_CC1);
  HAL_DMA_ARM_CH(HAL_IRGEN_DMA_CH_CC1);
 
  // Program the first bit timing information
  T1CC0L = halIrGenCc0Buf[0] & 0xff;
  T1CC0H = halIrGenCc0Buf[0] >> 8;
  T1CC1L = halIrGenCc1Buf[0] & 0xff;
  T1CC1H = halIrGenCc1Buf[0] >> 8;  

  // Can't be interrupted when clearing and starting the timer(s)
  HAL_ENTER_CRITICAL_SECTION(intState);
  
  // Clear timer counter.  Execution of this command will activate the output pin
  // so important to start timer immediately afterwards
  T1CNTL = 0;

  // Start timers. Note the order of the timer start sequence
  T1CTL = HAL_IRGEN_BIT_TIMING_PRESCALER_DIV1 | HAL_IRGEN_T1CTL_MODE_MODULO;
 
#ifdef HAL_IRGEN_CARRIER
  T3CTL = HAL_IRGEN_CARRIER_PRESCALER_DIV1 | HAL_IRGEN_T3CTL_START | HAL_IRGEN_T3CTL_CLR |
    HAL_IRGEN_T3CTL_MODE_MODULO;
#endif //HAL_IRGEN_CARRIER
  
  HAL_EXIT_CRITICAL_SECTION(intState);
}

/******************************************************************************
 * @fn      HalIrGenDmaIsr
 *
 * @brief   Handles DMA interrupt that comes upon completion of transmission.
 *          This function is called from DMA interrupt service routine.
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
/*
void HalIrGenDmaIsr(void)
{
  
  // Stop timers
#ifdef HAL_IRGEN_CARRIER
  T3CTL = HAL_IRGEN_T3CTL_CLR;
#endif //HAL_IRGEN_CARRIER
  
  T1CTL = HAL_IRGEN_T1CTL_MODE_SUSPEND;
  
  // Clear CC0 DMA interrupt flag, CC1 is cleared in hal_dma isr.
  HAL_DMA_CLEAR_IRQ(HAL_IRGEN_DMA_CH_CC0);
    
  // Update status
  halIrGenTimerRunning = FALSE;
    
  // Call the callback function
  HalIrGenIsrCback();  
}*/
#endif // HAL_IRGEN == TRUE
