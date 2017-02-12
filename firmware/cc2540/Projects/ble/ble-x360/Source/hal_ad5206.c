
#include "OSAL.h"

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

//#include "bletempservice.h"

#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
//#include "hal_adc.h"
//#include "hal_led.h"
//#include "hal_key.h"
//#include "hal_lcd.h"

#include "hci.h"






//#include <ioCC2540.h>
#include "hal_ad5206.h"

// Accelerometer connected at (rev0.6/rev1.0):
// P0_6 = DATA_READY/-
// P1_2 = -/CS
// P1_3 = SCK
// P1_4 = MISO
// P1_5 = MOSI
// P1_7 = CS/DATA_READY

//#define SCK             P1_3
//#define MISO            P1_4
//#define MOSI            P1_5

//#define REV_1_0
//
//#ifdef REV_1_0
 // #define CS              P1_2 UPS...
#define CS P2_2
//#elif (defined REV_0_6)
//  #define CS              P1_7
//#endif

#define CS_DISABLED     1
#define CS_ENABLED      0






/** \brief	Initialize SPI interface and CMA3000-D01 accelerometer
*
* This will initialize the SPI interface and CMA3000-D01 accelerometer
*
*/
void accInit(void)
{
    //*** Setup USART 0 SPI at alternate location 2 ***

    // USART 0 at alternate location 2
    PERCFG |= 0x01;
    // Peripheral function on SCK, MISO and MOSI (P1_3-5)
    P1SEL |= 0x38;
    // Configure CS (P1_7/P1_2) as output
//#ifdef REV_1_0
    P1DIR |= 0x04;
//#elif (defined REV_0_6)
//    P1DIR |= 0x80;
//#endif
    CS =  CS_DISABLED;
	
	P2_1 = 1;
	P2_2 = 1;
	P1_2 = 1;

    //*** Setup the SPI interface ***
    // SPI master mode
    U0CSR = 0x00;
    // Negative clock polarity, Phase: data out on CPOL -> CPOL-inv
    //                                 data in on CPOL-inv -> CPOL
//    // MSB first
  //  U0GCR = 0x20 | 0x80;
//    // SCK frequency = 480.5kHz (max 500kHz)
//    U0GCR |= 0x0D;
//    U0BAUD = 0xEC;
	
	    // MSB first
    U0GCR = 0x20;
    // SCK frequency = 3MHz (MBA250 max=10MHz, CC254x max = 4MHz)
    U0GCR |= 0x10;
    U0BAUD = 0x80;

	

  //  uint8 readValue;
  //  accWriteReg(CTRL, RANGE_2G | MODE_100HZ_MEAS);
  //  WAIT_1_3US(80);
  //  do{
   //     accReadReg(STATUS, &readValue);
  //      WAIT_1_3US(80);
 //   }while(readValue & 0x08);
//    acc_initialized = TRUE;
}


/** \brief	Write one byte to SPI interface
*
* Write one byte to SPI interface
*
* \param[in]       write
*     Value to write
*/
void spiWriteByte(uint8 write)
{
        U0CSR &= ~0x02;                 // Clear TX_BYTE
        U0DBUF = write;
        while (!(U0CSR & 0x02));        // Wait for TX_BYTE to be set
}

void HalAD5206Init(void)
{
	accInit();
};

void HalAD5206SetValue(uint8 channel, uint8 value)
{
    CS = CS_ENABLED;
	
	P2_1 = 0;
	P2_2 = 0;
	P1_2 = 0;
	
    spiWriteByte(channel);
    spiWriteByte(value);

    CS = CS_DISABLED;
	
	P2_1 = 1;
	P2_2 = 1;
	P1_2 = 1;
};
