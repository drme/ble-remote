#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "fan_profile.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "gatt.h"
#include "hci.h"
#include "osal_snv.h"
#include "fan_control.h"
#include "gapgattserver.h"
#include "dl_util.h"
  
#define DECLARE_UUID(name, id)															static CONST uint8 name[DL_UUID_SIZE] = { DL_UUID(id) };
#define DECLARE_ATTR_DESCRIPTION(description)											{ { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value)										{ { DL_UUID_SIZE, ID }, permissions, 0, (uint8*)&value }
#define DECLARE_ATTR_PROPERTIES(properties)												{ { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define SET_VALUE(x)																	if (len == sizeof(uint16)) { x = *((uint16*)value); } else { ret = bleInvalidRange; };
#define DECLARE_ATTR(bleProperties, bleUUID, blePermission, bleValue, bleDescription)	DECLARE_ATTR_PROPERTIES(bleProperties), DECLARE_ATTR_VALUE(bleUUID, blePermission, bleValue), DECLARE_ATTR_DESCRIPTION(bleDescription)

DECLARE_UUID(fanProfileServiceUUID, 		BLE_FAN_SERVICE_UUID);
DECLARE_UUID(fanProfileCharThrottleUUID,	BLE_FAN_SPEED_CHAR_UUID);
DECLARE_UUID(fanProfileCharDeviceNameUUID,	BLE_FAN_DEVICE_NAME_CHAR_UUID);
DECLARE_UUID(fanProfileCharDevicePassUUID,	BLE_FAN_DEVICE_PASS_CHAR_UUID);

static CONST gattAttrType_t	fanProfileService	= { DL_UUID_SIZE, fanProfileServiceUUID };

static uint8 speedDescription[22]      = "Fan Speed\0";
static uint8 deviceNameDescription[22] = "Device Name\0";
static uint8 devicePassDescription[22] = "Device Pass\0";

static uint8 speedProperties      = GATT_PROP_WRITE_NO_RSP;
static uint8 deviceNameProperties = GATT_PROP_WRITE_NO_RSP | GATT_PROP_READ;
static uint8 devicePassProperties = GATT_PROP_WRITE_NO_RSP | GATT_PROP_READ;

static uint16 fanSpeed = 0;

static gattAttribute_t fanProfileAttributesTable[] =
{
	// Simple Profile Service
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID },	/* type */
		GATT_PERMIT_READ,							/* permissions */
		0,											/* handle */
		(uint8*)&fanProfileService				/* pValue */
	},

	DECLARE_ATTR(speedProperties,      fanProfileCharThrottleUUID,   GATT_PERMIT_WRITE,                    fanSpeed,                speedDescription),
	DECLARE_ATTR(deviceNameProperties, fanProfileCharDeviceNameUUID, GATT_PERMIT_WRITE | GATT_PERMIT_READ, settings.deviceName,     deviceNameDescription),
	DECLARE_ATTR(devicePassProperties, fanProfileCharDevicePassUUID, GATT_PERMIT_WRITE | GATT_PERMIT_READ, settings.devicePassword, devicePassDescription),
};

static uint8 FanProfileReadAttributeCallBack(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen)
{
	bStatus_t status = SUCCESS;

	// If attribute permissions require authorization to read, return error
	if (gattPermitAuthorRead(pAttr->permissions))
	{
		return (ATT_ERR_INSUFFICIENT_AUTHOR);
	}

	// Make sure it's not a blob operation (no attributes in the profile are long)
	if (offset > 0)
	{
		return (ATT_ERR_ATTR_NOT_LONG);
	}

	uint16 uuid;

	if (ExtractUuid16(pAttr,&uuid) == FAILURE)
	{
		*pLen = 0;
		
		return ATT_ERR_INVALID_HANDLE;
	}
	
	switch (uuid)
	{
		case BLE_FAN_SPEED_CHAR_UUID:
			*pLen = 2;
			pValue[0] = pAttr->pValue[0];
			pValue[1] = pAttr->pValue[1];
			break;
		case BLE_FAN_DEVICE_NAME_CHAR_UUID:
		  	*pLen = GAP_DEVICE_NAME_LEN;
			for (int i = 0; i < GAP_DEVICE_NAME_LEN; i++)
			{
				pValue[i] = settings.deviceScanNameAddress[i];
			}
			break;
		case BLE_FAN_DEVICE_PASS_CHAR_UUID:
		  	*pLen = 4;
			for (int i = 0; i < 4; i++)
			{
				pValue[i] = ((uint8*)settings.devicePassCode)[i];
			}
			break;
		default:
			*pLen = 0;
			status = ATT_ERR_ATTR_NOT_FOUND;
			break;
	}

	return status;
};

static bStatus_t AssignValue(gattAttribute_t* pAttr, uint8* value, uint8 length, uint16 offset, uint16* result)
{
	if (offset == 0)
	{
		if (length != 2)
		{
			return ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		return ATT_ERR_ATTR_NOT_LONG;
	}

	uint8* pCurValue = (uint8*)pAttr->pValue;
	pCurValue[0] = value[0];
	pCurValue[1] = value[1];

	uint16 v = (pCurValue[1] << 8) | pCurValue[0];
	
	*result = v;

	return SUCCESS;
};

static bStatus_t FanProfileWriteAttributeCallBack(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	bStatus_t status = SUCCESS;

	if (gattPermitAuthorWrite(pAttr->permissions))
	{
		return ATT_ERR_INSUFFICIENT_AUTHOR;
	}

	uint16 uuid;

	if (ExtractUuid16(pAttr, &uuid) == FAILURE)
	{
		return ATT_ERR_INVALID_HANDLE;
	}
	
	switch (uuid)
	{
		case BLE_FAN_SPEED_CHAR_UUID:
			status = AssignValue(pAttr, pValue, len, offset, &fanSpeed);
			FanSetSpeed(fanSpeed);
			break;
		case BLE_FAN_DEVICE_NAME_CHAR_UUID:
			status = FanSaveDeviceName(connHandle, pAttr, pValue, len, offset);
			break;
		case BLE_FAN_DEVICE_PASS_CHAR_UUID:
			status = FanSaveDevicePassword(connHandle, pAttr, pValue, len, offset);
			break;				
		case GATT_CLIENT_CHAR_CFG_UUID:
			status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY);
			break;
		default:
			status = ATT_ERR_ATTR_NOT_FOUND;
			break;
	}

	return (status);
};

static void FanProfileHandleConnStatusCallBack(uint16 connHandle, uint8 changeType)
{
};

CONST gattServiceCBs_t fanProfileCallBacks =
{
	FanProfileReadAttributeCallBack,	// Read callback function pointer
	FanProfileWriteAttributeCallBack,	// Write callback function pointer
	NULL					// Authorization callback function pointer
};

bStatus_t FanProfileAddService(uint32 services)
{
	uint8 status = SUCCESS;

	// Register with Link DB to receive link status change callback
	VOID linkDB_Register(FanProfileHandleConnStatusCallBack);

	if (services & BLE_FAN_PROFILE_SERVICE)
	{
		// Register GATT attribute list and CBs with GATT Server App
		status = GATTServApp_RegisterService(fanProfileAttributesTable, GATT_NUM_ATTRS(fanProfileAttributesTable), &fanProfileCallBacks);
	}

	return (status);
};
