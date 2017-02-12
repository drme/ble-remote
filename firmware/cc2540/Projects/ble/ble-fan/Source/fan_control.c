#include "comdef.h"
#include "bcomdef.h"
#include "gatt.h"
#include "fan_profile.h"
#include "fan_control.h"
#include "hal_adc.h"
#include "att.h"
#include "gapgattserver.h"
#include "battservice.h"
#include "osal_snv.h"
#include "gapbondmgr.h"

#define DEVICE_NAME_FLASH_ADDRESS		0x91
#define DEVICE_PASSWORD_FLASH_ADDRESS	0x92
#define STATUS_UPDATE_EVENT_PERIOD		5000

static uint8 mainTaskId = 0;
static bool  connected  = false;
Settings settings;

void FanControlInit(uint8 taskId, uint8* deviceScanName)
{
	settings.deviceScanNameAddress = deviceScanName;

  	mainTaskId = taskId;
  
	FanLoadDeviceName(settings.deviceScanNameAddress);
	
	FanServiceInit(taskId);
};

void FanConnected(uint8 taskId)
{
	osal_start_timerEx(mainTaskId, FAN_UPDATE_STATUS_EVT, STATUS_UPDATE_EVENT_PERIOD);
	
	connected = true;
};

void FanDisConnected()
{
	connected = false;
};

void FanUpdateStatus()
{
	Batt_MeasLevel();

	if (true == connected)
	{
		osal_start_timerEx(mainTaskId, FAN_UPDATE_STATUS_EVT, STATUS_UPDATE_EVENT_PERIOD);
	}
};

void FanLoadDeviceName(uint8* scanResponseName)
{
	if (SUCCESS == osal_snv_read(DEVICE_NAME_FLASH_ADDRESS, sizeof(uint8) * GAP_DEVICE_NAME_LEN, scanResponseName))
	{
	}
	else
	{
		osal_memcpy(scanResponseName, "Radio Controlled Fan\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", GAP_DEVICE_NAME_LEN);
	}
};

bStatus_t FanSaveDevicePassword(uint16 connectionHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	if (offset == 0)
	{
		if (len != 4)
		{
			return ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		return ATT_ERR_ATTR_NOT_LONG;
	}

	for (int i = 0; i < 4; i++)
	{
		((uint8*)settings.devicePassCode)[i] = pValue[i];
	}

	if (SUCCESS == osal_snv_write(DEVICE_PASSWORD_FLASH_ADDRESS, sizeof(uint32), &settings.devicePassCode))
	{
	}
				   
	GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32), &settings.devicePassCode);
					
	return SUCCESS;
};

bStatus_t FanSaveDeviceName(uint16 connectionHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	if (offset == 0)
	{
		if ((len >= GAP_DEVICE_NAME_LEN) || (len <= 0))
		{
			return ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		return ATT_ERR_ATTR_NOT_LONG;
	}

	for (int i = 0; i < len; i++)
	{
		settings.deviceScanNameAddress[i] = pValue[i];
	}

	for (int i = len; i < GAP_DEVICE_NAME_LEN; i++)
	{
		settings.deviceScanNameAddress[i] = 0x0;
	}

	if (SUCCESS == osal_snv_write(DEVICE_NAME_FLASH_ADDRESS, sizeof(uint8) * GAP_DEVICE_NAME_LEN, settings.deviceScanNameAddress))
	{
	}
				   
   	GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, settings.deviceScanNameAddress);

	return SUCCESS;
};

void FanServiceInit(uint8 taskId)
{
	mainTaskId = taskId;
	
	P0SEL	= 0x00;
	P0DIR	= 0x00;
	P0		= 0x00;
  
 	P1SEL	= 0x00;
	P1DIR	= 0x00;
	P1		= 0x00;

	P2SEL	= 0x00;
	P2DIR	= 0x00;
	P2		= 0x00;
};

static void SetP20PWM(int dutyCycle)
{
	if (dutyCycle <= 0)
	{
		T4CTL = 0x00; // stop timer
		
		P2DIR |= (1 << 0);
		P2SEL &= ~(1 << 0);
		P2_0 = 0;
	}
	else
	{
		PERCFG |= (1 << 4);  // Timer 4 on alternative 2 location		
		P2DIR |= (1 << 0);
		P2SEL |= (1 << 0);
		
		T4CC0 = 0xff - dutyCycle;
		T4CCTL0 = 0x1C;
		T4CTL = 0xF0; // start, free running 0-255, tick/128
	}
};

void FanSetSpeed(uint16 value)
{
  SetP20PWM(value & 0x00ff);
  
  /*
  
	P2DIR = (1 << 0);
	P2SEL = 0x00;
	P2 = 0x00;
	
	if (value > 0)
	{
		P2_0 = 0xff;
	}
	else
	{
		P2_0 = 0x00;
	}
 
  */
};
