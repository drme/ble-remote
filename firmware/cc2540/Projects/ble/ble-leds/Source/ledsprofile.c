#include <string.h>
#include <stdio.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "ledsprofile.h"
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "gatt.h"
#include "hci.h"
#include "osal_snv.h"
#include "hal_uart.h"

#define SERVAPP_NUM_ATTR_SUPPORTED	46

#define DECLARE_UUID(name, ID)							CONST uint8 name[ATT_BT_UUID_SIZE] = { LO_UINT16(ID), HI_UINT16(ID)};
#define DECLARE_ATTR_DESCRIPTION(description)			{ { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value)		{ { ATT_BT_UUID_SIZE, ID }, permissions, 0, (uint8*)&value }
#define DECLARE_ATTR_PROPERTIES(properties)				{ { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define SET_VALUE(x)									if (len == sizeof(uint16)) { x = *((uint16*)value); } else { ret = bleInvalidRange; };

#define DECLARE_ATTR(bleProperties, bleUUID, blePermission, bleValue, bleDescription)	DECLARE_ATTR_PROPERTIES(bleProperties), DECLARE_ATTR_VALUE(bleUUID, blePermission, bleValue), DECLARE_ATTR_DESCRIPTION(bleDescription)

DECLARE_UUID(bleLedsProfileServUUID,	BLE_LEDS_SERVICE_UUID);
DECLARE_UUID(bleLedsPixelCharUUID,		BLE_LEDS_PIXEL_CHAR_UUID);

//static rcProfileCBs_t*		rcProfile_AppCBs	= NULL;
static CONST gattAttrType_t	bleLedsProfileService = { ATT_BT_UUID_SIZE, bleLedsProfileServUUID };

static uint8 blePixelColorDescription[22] = "Pixel Color\0";

static uint32 blePixelColor			= 0;

static uint8 bleLedsPixelColorProperties = GATT_PROP_WRITE_NO_RSP;
//static uint8 bleLedsPixelColorProperties = GATT_PROP_WRITE;

/**
 * Profile Attributes - Table
 */
static gattAttribute_t bleRCProfileAttributesTable[SERVAPP_NUM_ATTR_SUPPORTED] =
{
	// Simple Profile Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID },	/* type */
		GATT_PERMIT_READ,							/* permissions */
		0,											/* handle */
		(uint8*)&bleLedsProfileService				/* pValue */
	},

	DECLARE_ATTR(bleLedsPixelColorProperties, bleLedsPixelCharUUID, GATT_PERMIT_WRITE, blePixelColor, blePixelColorDescription)
};

static uint8 LedsProfileReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen);
static bStatus_t LedsProfileWriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);
static void LedsProfileHandleConnStatusCB(uint16 connHandle, uint8 changeType);

/**
 * Service Callbacks
 */
CONST gattServiceCBs_t rcProfileCBs =
{
	LedsProfileReadAttrCB,	// Read callback function pointer
	LedsProfileWriteAttrCB,	// Write callback function pointer
	NULL					// Authorization callback function pointer
};

/**
 * Initializes the Profile service by registering GATT attributes with the GATT server.
 *
 * @param services - services to add. This is a bit map and can contain more than one service.
 * @return Success or Failure
 */
bStatus_t LedsProfileAddService(uint32 services)
{
	uint8 status = SUCCESS;

	// Register with Link DB to receive link status change callback
	VOID linkDB_Register(LedsProfileHandleConnStatusCB);

	if (services & BLE_LEDS_PROFILE_SERVICE)
	{
		// Register GATT attribute list and CBs with GATT Server App
		status = GATTServApp_RegisterService(bleRCProfileAttributesTable, GATT_NUM_ATTRS(bleRCProfileAttributesTable), &rcProfileCBs);
	}

	return (status);
};

/**
 * Registers the application callback function. Only call this function once.
 *
 * @param callbacks - pointer to application callbacks.
 * @return SUCCESS or bleAlreadyInRequestedMode.
 */
bStatus_t LedsProfileRegisterAppCBs(ledsProfileCBs_t* appCallbacks)
{
	if (appCallbacks)
	{
		//rcProfile_AppCBs = appCallbacks;

		return (SUCCESS);
	}
	else
	{
		return (bleAlreadyInRequestedMode);
	}
};

/**
 * Read an attribute.
 *
 * @param connHandle - connection message was received on.
 * @param pAttr - pointer to attribute.
 * @param pValue - pointer to data to be read.
 * @param pLen - length of data to be read.
 * @param offset - offset of the first octet to be read.
 * @param maxLen - maximum length of data to be read.
 * @return Success or Failure.
 */
static uint8 LedsProfileReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen)
{
	bStatus_t status = SUCCESS;

	// If attribute permissions require authorization to read, return error
	if (gattPermitAuthorRead(pAttr->permissions))
	{
		// Insufficient authorization
		return (ATT_ERR_INSUFFICIENT_AUTHOR);
	}

	// Make sure it's not a blob operation (no attributes in the profile are long)
	if (offset > 0)
	{
		return (ATT_ERR_ATTR_NOT_LONG);
	}

	if (pAttr->type.len == ATT_BT_UUID_SIZE)
	{
		// 16-bit UUID
		uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

		switch ( uuid )
		{
			case BLE_LEDS_PIXEL_CHAR_UUID:
				*pLen = 4;
				pValue[0] = pAttr->pValue[0];
				pValue[1] = pAttr->pValue[1];
				pValue[2] = pAttr->pValue[2];
				pValue[3] = pAttr->pValue[3];
				break;
			default:
				// Should never get here! (characteristics 3 and 4 do not have read permissions)
				*pLen = 0;
				status = ATT_ERR_ATTR_NOT_FOUND;
				break;
		}
	}
	else
	{
		// 128-bit UUID
		*pLen = 0;
		status = ATT_ERR_INVALID_HANDLE;
	}

	return (status);
};

/**
 * Validate attribute data prior to a write operation.
 *
 * @param connHandle - connection message was received on.
 * @param pAttr - pointer to attribute.
 * @param pValue - pointer to data to be written.
 * @param len - length of data.
 * @param offset - offset of the first octet to be written.
 * @param complete - whether this is the last packet.
 * @param oper - whether to validate and/or write attribute value  .
 * @return  Success or Failure.
 */
static bStatus_t assignValue(gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset, uint32* result)
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

	uint8* pCurValue = (uint8*)pAttr->pValue;
	pCurValue[0] = pValue[0];
	pCurValue[1] = pValue[1];
	pCurValue[2] = pValue[2];
	pCurValue[3] = pValue[3];

	LedsSetPixel(pValue[0], pValue[1], pValue[2], pValue[3]);
	
	// TODO:
	uint16 v = (pCurValue[1] << 8) | pCurValue[0];
	
	*result = v;

	return SUCCESS;
};

static bStatus_t LedsProfileWriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	bStatus_t status = SUCCESS;

	 // If attribute permissions require authorization to write, return error
	if (gattPermitAuthorWrite(pAttr->permissions))
	{
		// Insufficient authorization
		return ATT_ERR_INSUFFICIENT_AUTHOR;
	}

	if (pAttr->type.len == ATT_BT_UUID_SIZE)
	{
    	// 16-bit UUID
		uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

		switch (uuid)
		{
			case BLE_LEDS_PIXEL_CHAR_UUID:
				status = assignValue(pAttr, pValue, len, offset, &blePixelColor);
				//TODO:
				break;
			case GATT_CLIENT_CHAR_CFG_UUID:
				status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY);
				break;
			default:
				status = ATT_ERR_ATTR_NOT_FOUND;
				break;
		}
	}
	else
	{
		// 128-bit UUID
		status = ATT_ERR_INVALID_HANDLE;
	}

	return (status);
};

/**
 * Profile link status change handler function.
 *
 * @param connHandle - connection handle
 * @param changeType - type of change
 */
static void LedsProfileHandleConnStatusCB(uint16 connHandle, uint8 changeType)
{
};

void LedsProfileInit()
{
	P0SEL = (1 << 2) | (1 << 3); // P0.3 -> TX pin

	// Power on pin setup

	P1SEL &= ~0x1;
	P1DIR |= 0x1;

	P1_0 = 0x0;

	HalUARTInit();

	halUARTCfg_t cfg = { 0 };

	cfg.configured = true;
	cfg.baudRate = HAL_UART_BR_9600;//HAL_UART_BR_115200;
	cfg.flowControl = false;
	cfg.flowControlThreshold = 0;
	cfg.idleTimeout = 0;
	//cfg.rx = NULL;
	//cfg.tx = NULL;
	cfg.intEnable = false;
	cfg.rxChRvdTime = 0;
	cfg.callBackFunc = NULL;

	HalUARTOpen(HAL_UART_PORT_0, &cfg);

	for (uint8 i = 0; i < 240; i++)
	{
		LedsSetPixel(i, i < 80 ? i : 0, i > 80 && i < 160 ? i : 0, i > 160 ? i : 0);
		LedsSetPixel(i, 0, 0, 0);
	}
};

void LedsStart()
{
	P1_0 = 0x1;
};

void LedsStop()
{
	P1_0 = 0x0;
};

//uint8 ledId = 0;

void LedsUpdateStatus()
{
/*  uint8 p[5] = { ledId++, 0, 0x7f, 0, 0xff };

HalUARTWrite(HAL_UART_PORT_0 , p, 5);


  // TODO:*/
};

void LedsSetPixel(uint8 pixelId, uint8 r, uint8 g, uint8 b)
{
//	uint8 p[5] = { pixelId, r, g, b, 0xff };
//	HalUARTWrite(HAL_UART_PORT_0 , p, 5);

	char buffer[256];

	if (pixelId == 0xff)
	{
		sprintf(buffer, "c=%d,%d,%d,%d\13", 0xffff, r, g, b);
	}
	else
	{
		sprintf(buffer, "c=%d,%d,%d,%d\13", pixelId, r, g, b);
	}
	
	HalUARTWrite(HAL_UART_PORT_0, (unsigned char*)buffer, strlen(buffer));
};
