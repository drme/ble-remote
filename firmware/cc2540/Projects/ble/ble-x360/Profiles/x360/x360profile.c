#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "x360profile.h"
#include "hal_ad5206.h"
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "gatt.h"
#include "hci.h"

#define DECLARE_UUID(name, ID)							CONST uint8 name[ATT_BT_UUID_SIZE] = { LO_UINT16(ID), HI_UINT16(ID)};
#define DECLARE_ATTR_DESCRIPTION(description)			{ { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value)		{ { ATT_BT_UUID_SIZE, ID }, permissions, 0, &value }
#define DECLARE_ATTR_PROPERTIES(properties)				{ { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define SET_VALUE(x)									if (len == sizeof(uint8)) { x = *((uint8*)value); } else { ret = bleInvalidRange; };

DECLARE_UUID(buttonsProfileServUUID, BLE_BUTTONS_SERVICE_UUID);

DECLARE_UUID(aUUID, BLE_BUTTONS_A_CHAR_UUID);
DECLARE_UUID(bUUID, BLE_BUTTONS_B_CHAR_UUID);
DECLARE_UUID(xUUID, BLE_BUTTONS_X_CHAR_UUID);
DECLARE_UUID(yUUID, BLE_BUTTONS_Y_CHAR_UUID);

DECLARE_UUID(upUUID, BLE_BUTTONS_UP_CHAR_UUID);
DECLARE_UUID(dnUUID, BLE_BUTTONS_DN_CHAR_UUID);
DECLARE_UUID(lUUID, BLE_BUTTONS_L_CHAR_UUID);
DECLARE_UUID(rUUID, BLE_BUTTONS_R_CHAR_UUID);

DECLARE_UUID(lbUUID, BLE_BUTTONS_LB_CHAR_UUID);
DECLARE_UUID(rbUUID, BLE_BUTTONS_RB_CHAR_UUID);

DECLARE_UUID(lsUUID, BLE_BUTTONS_LS_CHAR_UUID);
DECLARE_UUID(rsUUID, BLE_BUTTONS_RS_CHAR_UUID);

DECLARE_UUID(startUUID, BLE_BUTTONS_START_CHAR_UUID);
DECLARE_UUID(backUUID, BLE_BUTTONS_BACK_CHAR_UUID);
DECLARE_UUID(guideUUID, BLE_BUTTONS_GUIDE_CHAR_UUID);

DECLARE_UUID(rtUUID, BLE_BUTTONS_RT_CHAR_UUID);
DECLARE_UUID(rxUUID, BLE_BUTTONS_RX_CHAR_UUID);
DECLARE_UUID(ryUUID, BLE_BUTTONS_RY_CHAR_UUID);
DECLARE_UUID(ltUUID, BLE_BUTTONS_LT_CHAR_UUID);
DECLARE_UUID(lxUUID, BLE_BUTTONS_LX_CHAR_UUID);
DECLARE_UUID(lyUUID, BLE_BUTTONS_LY_CHAR_UUID);

static buttonsProfileCBs_t*	buttonsProfile_AppCBs	= NULL;
static CONST gattAttrType_t	buttonsProfileService	= { ATT_BT_UUID_SIZE, buttonsProfileServUUID };

static uint8 aDescription[17]		= "Button A\0";
static uint8 bDescription[17]		= "Button B\0";
static uint8 xDescription[17]		= "Button X\0";
static uint8 yDescription[17]		= "Button Y\0";

static uint8 upDescription[17]		= "DPad Up\0";
static uint8 dnDescription[17]		= "DPad Down\0";
static uint8 lDescription[17]		= "DPad Left\0";
static uint8 rDescription[17]		= "DPad Right\0";

static uint8 lsDescription[20]		= "Left Stick Button\0";
static uint8 rsDescription[20]		= "Right Stick Button\0";

static uint8 lbDescription[17]		= "LB Button\0";
static uint8 rbDescription[17]		= "RB Button\0";

static uint8 startDescription[17]	= "Start Button\0";
static uint8 backDescription[17]	= "Back Button\0";
static uint8 guideDescription[17]	= "Guide Button\0";

static uint8 ltDescription[17]		= "Left Trigger\0";
static uint8 lxDescription[17]		= "Left Stick X\0";
static uint8 lyDescription[17]		= "Left Stick Y\0";
static uint8 rtDescription[17]		= "Right Trigger\0";
static uint8 rxDescription[17]		= "Right Stick X\0";
static uint8 ryDescription[17]		= "Right Stick Y\0";

static uint8 aVal		= 0;
static uint8 bVal		= 0;
static uint8 xVal		= 0;
static uint8 yVal		= 0;

static uint8 upVal		= 0;
static uint8 dnVal		= 0;
static uint8 lVal		= 0;
static uint8 rVal		= 0;

static uint8 rbVal		= 0;
static uint8 lbVal		= 0;

static uint8 lsVal		= 0;
static uint8 rsVal		= 0;

static uint8 startVal		= 0;
static uint8 backVal		= 0;
static uint8 guideVal		= 0;

static uint8 ltVal		= 0;
static uint8 lxVal		= 0;
static uint8 lyVal		= 0;
static uint8 rtVal		= 0;
static uint8 rxVal		= 0;
static uint8 ryVal		= 0;

static uint8 aProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 bProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 xProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 yProperties		= GATT_PROP_WRITE_NO_RSP;

static uint8 upProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 dnProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 lProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 rProperties		= GATT_PROP_WRITE_NO_RSP;

static uint8 lbProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 rbProperties		= GATT_PROP_WRITE_NO_RSP;

static uint8 lsProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 rsProperties		= GATT_PROP_WRITE_NO_RSP;

static uint8 startProperties	= GATT_PROP_WRITE_NO_RSP;
static uint8 backProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 guideProperties	= GATT_PROP_WRITE_NO_RSP;

static uint8 ltProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 lxProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 lyProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 rtProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 rxProperties		= GATT_PROP_WRITE_NO_RSP;
static uint8 ryProperties		= GATT_PROP_WRITE_NO_RSP;

#define DECLARE_BUTTON(properties, uuid, value, description) DECLARE_ATTR_PROPERTIES(properties), DECLARE_ATTR_VALUE(uuid, GATT_PERMIT_WRITE, value), DECLARE_ATTR_DESCRIPTION(description)

#define SERVAPP_NUM_ATTR_SUPPORTED	1 + (21 * 3)

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

	DECLARE_BUTTON(aProperties, aUUID, aVal, aDescription),
	DECLARE_BUTTON(bProperties, bUUID, bVal, bDescription),
	DECLARE_BUTTON(xProperties, xUUID, xVal, xDescription),
	DECLARE_BUTTON(yProperties, yUUID, yVal, yDescription),

	DECLARE_BUTTON(upProperties, upUUID, upVal, upDescription),
	DECLARE_BUTTON(dnProperties, dnUUID, dnVal, dnDescription),
	DECLARE_BUTTON(lProperties, lUUID, lVal, lDescription),
	DECLARE_BUTTON(rProperties, rUUID, rVal, rDescription),

	DECLARE_BUTTON(lbProperties, lbUUID, lbVal, lbDescription),
	DECLARE_BUTTON(rbProperties, rbUUID, rbVal, rbDescription),

	DECLARE_BUTTON(lsProperties, lsUUID, lsVal, lsDescription),
	DECLARE_BUTTON(rsProperties, rsUUID, rsVal, rsDescription),

	DECLARE_BUTTON(startProperties, startUUID, startVal, startDescription),
	DECLARE_BUTTON(backProperties, backUUID, backVal, backDescription),
	DECLARE_BUTTON(guideProperties, guideUUID, guideVal, guideDescription),

	DECLARE_BUTTON(ltProperties, ltUUID, ltVal, ltDescription),
	DECLARE_BUTTON(lxProperties, lxUUID, lxVal, lxDescription),
	DECLARE_BUTTON(lyProperties, lyUUID, lyVal, lyDescription),
	DECLARE_BUTTON(rtProperties, rtUUID, rtVal, rtDescription),
	DECLARE_BUTTON(rxProperties, rxUUID, rxVal, rxDescription),
	DECLARE_BUTTON(ryProperties, ryUUID, ryVal, ryDescription)
};

static bStatus_t buttonsProfile_WriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);
static void buttonsProfile_HandleConnStatusCB(uint16 connHandle, uint8 changeType);

/**
 * Service Callbacks
 */
CONST gattServiceCBs_t buttonsProfileCBs =
{
	NULL,						// Read callback function pointer
	buttonsProfile_WriteAttrCB,	// Write callback function pointer
	NULL						// Authorization callback function pointer
};

bStatus_t buttonsProfile_AddService(uint32 services)
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

void setChannelValue(uint8 channel, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset, bStatus_t* status)
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
		
		HalAD5206SetValue(channel, pValue[0]);
	}
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
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }

	if (pAttr->type.len == ATT_BT_UUID_SIZE)
	{
    	// 16-bit UUID
		uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

		switch (uuid)
		{
			case BLE_BUTTONS_A_CHAR_UUID:
				P1_7 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_B_CHAR_UUID:
				P1_1 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_X_CHAR_UUID:
				P0_6 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_Y_CHAR_UUID:
				P0_5 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_UP_CHAR_UUID:
				P2_0 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_DN_CHAR_UUID:
				if (getValue(pAttr, pValue, len, offset, &status) > 0)
				{
					I2CIO |= 0x02; // SCL
				}
				else
				{
					I2CIO &= ~0x02; // SCL
				}
				break;
			case BLE_BUTTONS_L_CHAR_UUID:
				P0_7 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_R_CHAR_UUID:
				P1_6 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_LB_CHAR_UUID:
				P0_2 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_RB_CHAR_UUID:
				if (getValue(pAttr, pValue, len, offset, &status) > 0)
				{
					I2CIO |= 0x01; // SDA
				}
				else
				{
					I2CIO &= ~0x01; // SDA
				}			
				break;
			case BLE_BUTTONS_START_CHAR_UUID:
				P0_1 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_BACK_CHAR_UUID:
				P0_0 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_GUIDE_CHAR_UUID:
				P1_0 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_LS_CHAR_UUID:
				P0_3 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_RS_CHAR_UUID:
				P0_4 = getValue(pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_LX_CHAR_UUID:
				setChannelValue(6 - 1, pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_LY_CHAR_UUID:
				setChannelValue(5 - 1, pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_RX_CHAR_UUID:
				setChannelValue(4 - 1, pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_RY_CHAR_UUID:
				setChannelValue(2 - 1, pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_LT_CHAR_UUID:
				setChannelValue(3 - 1, pAttr, pValue, len, offset, &status);
				break;
			case BLE_BUTTONS_RT_CHAR_UUID:
				setChannelValue(1 - 1, pAttr, pValue, len, offset, &status);
				break;
			case GATT_CLIENT_CHAR_CFG_UUID:
				status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY);
				break;
			default:
				// Should never get here! (characteristics 2 and 4 do not have write permissions)
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
  if ( (notifyApp != 0xFF ) && buttonsProfile_AppCBs && buttonsProfile_AppCBs->pfnButtonsProfileChange )
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

void x360Init()
{
	P1SEL	= 0x0;
	P1DIR	= 0xff;
	P1		= 0x0;

	P0SEL	= 0x0;
	P0DIR	= 0xff;
	P0		= 0x0;

	P2SEL	= 0x0;
	P2DIR	= 0x7;
	P2		= 0x0;
	
	// SDA & SCL as output pins
	I2CCFG = 0x0;
	I2CWC = 0x83;
	
	// SDA = SCL = 0;
	I2CIO = 0x0;

	HalAD5206Init();
	
	HalAD5206SetValue(0, 127);
	HalAD5206SetValue(1, 127);
	HalAD5206SetValue(2, 127);
	HalAD5206SetValue(3, 127);
	HalAD5206SetValue(4, 127);
	HalAD5206SetValue(5, 127);
};
