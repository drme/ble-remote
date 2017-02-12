#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "pcprofile.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hci.h"

#define DECLARE_UUID(name, ID)							CONST uint8 name[ATT_BT_UUID_SIZE] = { LO_UINT16(ID), HI_UINT16(ID)};
#define DECLARE_ATTR_DESCRIPTION(description)			{ { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value)		{ { ATT_BT_UUID_SIZE, ID }, permissions, 0, &value }
#define DECLARE_ATTR_PROPERTIES(properties)				{ { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define SET_VALUE(x)									if (len == sizeof(uint8)) { x = *((uint8*)value); } else { ret = bleInvalidRange; };

DECLARE_UUID(buttonsProfileServUUID, BLE_BUTTONS_SERVICE_UUID);
DECLARE_UUID(buttonsProfileCharSwitch1UUID, BLE_BUTTONS_SWITCH1_CHAR_UUID);
DECLARE_UUID(buttonsProfileCharSwitch2UUID, BLE_BUTTONS_SWITCH2_CHAR_UUID);
DECLARE_UUID(ledProfileCharSwitch1UUID, BLE_BUTTONS_LED1_CHAR_UUID);
DECLARE_UUID(ledProfileCharSwitch2UUID, BLE_BUTTONS_LED2_CHAR_UUID);

static buttonsProfileCBs_t*	buttonsProfile_AppCBs	= NULL;
static CONST gattAttrType_t	buttonsProfileService	= { ATT_BT_UUID_SIZE, buttonsProfileServUUID };

static uint8 buttonsCharSwitch1Description[17]	= "Switch 1\0";
static uint8 buttonsCharSwitch2Description[17]	= "Switch 2\0";
static uint8 ledCharSwitch1Description[17]		= "Led 1\0";
static uint8 ledCharSwitch2Description[17]		= "Led 2\0";

static uint8 buttonsCharSwitch1					= 0;
static uint8 buttonsCharSwitch2					= 0;
uint8 ledCharSwitch1						= 0;
static uint8 ledCharSwitch2						= 0;

static uint8 buttonsCharSwitch1Properties		= GATT_PROP_WRITE;
static uint8 buttonsCharSwitch2Properties		= GATT_PROP_WRITE;
static uint8 ledCharSwitch1Properties			= GATT_PROP_READ;
static uint8 ledCharSwitch2Properties			= GATT_PROP_READ;

#define DECLARE_BUTTON(properties, uuid, value, description) DECLARE_ATTR_PROPERTIES(properties), DECLARE_ATTR_VALUE(uuid, GATT_PERMIT_WRITE, value), DECLARE_ATTR_DESCRIPTION(description)

#define SERVAPP_NUM_ATTR_SUPPORTED	13

/**
 * Profile Attributes - Table
 */
static gattAttribute_t buttonsProfileAttributesTable[SERVAPP_NUM_ATTR_SUPPORTED] =
{
	// Simple Profile Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID },	/* type */
		GATT_PERMIT_READ,							/* permissions */
		0,											/* handle */
		(uint8*)&buttonsProfileService				/* pValue */
	},

	DECLARE_ATTR_PROPERTIES(buttonsCharSwitch1Properties),
	DECLARE_ATTR_VALUE(buttonsProfileCharSwitch1UUID, GATT_PERMIT_WRITE, buttonsCharSwitch1),
	DECLARE_ATTR_DESCRIPTION(buttonsCharSwitch1Description),

	DECLARE_ATTR_PROPERTIES(buttonsCharSwitch2Properties),
	DECLARE_ATTR_VALUE(buttonsProfileCharSwitch2UUID, GATT_PERMIT_WRITE, buttonsCharSwitch2),
	DECLARE_ATTR_DESCRIPTION(buttonsCharSwitch2Description),

	DECLARE_ATTR_PROPERTIES(ledCharSwitch1Properties),
	DECLARE_ATTR_VALUE(ledProfileCharSwitch1UUID, GATT_PERMIT_READ, ledCharSwitch1),
	DECLARE_ATTR_DESCRIPTION(ledCharSwitch1Description),

	DECLARE_ATTR_PROPERTIES(ledCharSwitch2Properties),
	DECLARE_ATTR_VALUE(ledProfileCharSwitch2UUID, GATT_PERMIT_READ, ledCharSwitch2),
	DECLARE_ATTR_DESCRIPTION(ledCharSwitch2Description)
};

static uint8 buttonsProfile_ReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen);
static bStatus_t buttonsProfile_WriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);
static void buttonsProfile_HandleConnStatusCB(uint16 connHandle, uint8 changeType);

/**
 * Service Callbacks
 */
CONST gattServiceCBs_t buttonsProfileCBs =
{
	buttonsProfile_ReadAttrCB,	// Read callback function pointer
	buttonsProfile_WriteAttrCB,	// Write callback function pointer
	NULL						// Authorization callback function pointer
};

bStatus_t ButtonsProfileAddService(uint32 services)
{
	uint8 status = SUCCESS;

	VOID linkDB_Register(buttonsProfile_HandleConnStatusCB);

	if (services & BLE_BUTTONS_PROFILE_SERVICE)
	{
		status = GATTServApp_RegisterService(buttonsProfileAttributesTable, GATT_NUM_ATTRS(buttonsProfileAttributesTable), &buttonsProfileCBs);
	}

	return (status);
};

bStatus_t buttonsProfile_RegisterAppCBs(buttonsProfileCBs_t* appCallbacks)
{
	if (appCallbacks)
	{
		buttonsProfile_AppCBs = appCallbacks;

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
static uint8 buttonsProfile_ReadAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen)
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
			case BLE_BUTTONS_SWITCH1_CHAR_UUID:
			case BLE_BUTTONS_SWITCH2_CHAR_UUID:
			case BLE_BUTTONS_LED1_CHAR_UUID:
			case BLE_BUTTONS_LED2_CHAR_UUID:
				*pLen = 1;
				pValue[0] = *pAttr->pValue;
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

uint8 getValue(gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset, bStatus_t* status)
{
	// Validate the value
	// Make sure it's not a blob oper
	if (offset == 0)
	{
		if (len != 1)
		{
			*status = ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		*status = ATT_ERR_ATTR_NOT_LONG;
	}

	//Write the value
	if (*status == SUCCESS)
	{
		uint8* pCurValue = (uint8*)pAttr->pValue;
		*pCurValue = pValue[0];
					
		if (pValue[0] > 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	return 0;
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
static bStatus_t buttonsProfile_WriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	bStatus_t status = SUCCESS;
	uint8 notifyApp = 0xFF;

	// If attribute permissions require authorization to write, return error
	if (gattPermitAuthorWrite(pAttr->permissions))
	{
		// Insufficient authorization
		return (ATT_ERR_INSUFFICIENT_AUTHOR);
	}

	if (pAttr->type.len == ATT_BT_UUID_SIZE)
	{
    	// 16-bit UUID
		uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

		switch (uuid)
		{
			case BLE_BUTTONS_SWITCH1_CHAR_UUID:
				P0_0 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_SWITCH2_CHAR_UUID:
				P0_1 = getValue(pAttr, pValue, len, offset, &status);
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

	// If a charactersitic value changed then callback function to notify application of change
	if ((notifyApp != 0xFF ) && buttonsProfile_AppCBs && buttonsProfile_AppCBs->pfnButtonsProfileChange)
	{
		buttonsProfile_AppCBs->pfnButtonsProfileChange( notifyApp );
	}

	return (status);
};

/**
 * Simple Profile link status change handler function.
 *
 * @param connHandle - connection handle
 * @param changeType - type of change
 */
static void buttonsProfile_HandleConnStatusCB(uint16 connHandle, uint8 changeType)
{
};

void ButtonsInit()
{
	// P1_2, P1_3 as inputs
	P1SEL = 0x00;
	P1DIR = 0x00;

	// P0_0, P0_1 as outputs
	P0SEL = 0x00;
	P0DIR = 0xff;
	
	P0 = 0x0;
};

void ButtonsUpdate()
{
	ledCharSwitch2 = P1_2 > 0;
	ledCharSwitch1 = P1_3 > 0;
};
