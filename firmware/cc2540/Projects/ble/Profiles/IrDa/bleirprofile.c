#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "gatt.h"
#include "hci.h"
#include "hal_irgen_SIRC.h"
#include "bleirprofile.h"

#define SERVAPP_NUM_ATTR_SUPPORTED						10
#define DECLARE_UUID(name, ID)							CONST uint8 name[ATT_BT_UUID_SIZE] = { LO_UINT16(ID), HI_UINT16(ID)};
#define DECLARE_ATTR_DESCRIPTION(description)			{ { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value)		{ { ATT_BT_UUID_SIZE, ID }, permissions, 0, (uint8*)&value }
#define DECLARE_ATTR_PROPERTIES(properties)				{ { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define SET_VALUE(x)									if (len == sizeof(uint16)) { x = *((uint16*)value); } else { ret = bleInvalidRange; };
#define NOP()											asm("NOP")

DECLARE_UUID(irProfileServUUID,			BLE_IR_SERVICE_UUID);
DECLARE_UUID(irProfileCharTypeUUID,		BLE_IR_TYPE_CHAR_UUID);
DECLARE_UUID(irProfileCharCommandUUID,	BLE_IR_COMMAND_CHAR_UUID);
DECLARE_UUID(irProfileCharAddressUUID,	BLE_IR_ADDRESS_CHAR_UUID);

static CONST gattAttrType_t	irProfileService	= { ATT_BT_UUID_SIZE, irProfileServUUID };

static uint8 irTypeDescription[17]				= "Modulation Type\0";
static uint8 irCommandDescription[17]			= "Command\0";
static uint8 irAddressDescription[17]			= "Address\0";

static uint8 irType								= 0;
static uint8 irRepeat							= 0;
static uint8 irAddress							= 0x1;
static uint16 irCommand							= 0;

static uint8 irTypeProperties					= GATT_PROP_WRITE;
static uint8 irCommandProperties				= GATT_PROP_WRITE;
static uint8 irAddressProperties				= GATT_PROP_WRITE;

static uint8 irTaskId							= 0x0;

/**
 * Profile Attributes - Table
 */
static gattAttribute_t irProfileAttributesTable[SERVAPP_NUM_ATTR_SUPPORTED] =
{
	// Simple Profile Service
	{ 
		{ ATT_BT_UUID_SIZE, primaryServiceUUID },	/* type */
		GATT_PERMIT_READ,							/* permissions */
		0,											/* handle */
		(uint8*)&irProfileService					/* pValue */
	},

	DECLARE_ATTR_PROPERTIES(irTypeProperties),
	DECLARE_ATTR_VALUE(irProfileCharTypeUUID, GATT_PERMIT_AUTHEN_WRITE, irType),
	DECLARE_ATTR_DESCRIPTION(irTypeDescription),

	DECLARE_ATTR_PROPERTIES(irCommandProperties),
	DECLARE_ATTR_VALUE(irProfileCharCommandUUID, GATT_PERMIT_AUTHEN_WRITE, irCommand),
	DECLARE_ATTR_DESCRIPTION(irCommandDescription),

	DECLARE_ATTR_PROPERTIES(irAddressProperties),
	DECLARE_ATTR_VALUE(irProfileCharAddressUUID, GATT_PERMIT_AUTHEN_WRITE, irAddress),
	DECLARE_ATTR_DESCRIPTION(irAddressDescription)
};

static bStatus_t irProfileWriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);

/**
 * Service Callbacks
 */
CONST gattServiceCBs_t irProfileCBs =
{
	NULL,					// Read callback function pointer
	irProfileWriteAttrCB,	// Write callback function pointer
	NULL					// Authorization callback function pointer
};

/**
 * Initializes the Profile service by registering GATT attributes with the GATT server.
 *
 * @param services - services to add. This is a bit map and can contain more than one service.
 *
 * @return Success or Failure
 */
bStatus_t irProfileAddService(uint32 services)
{
	uint8 status = SUCCESS;

	if (services & BLE_IR_PROFILE_SERVICE)
	{
		// Register GATT attribute list and CBs with GATT Server App
		status = GATTServApp_RegisterService(irProfileAttributesTable, GATT_NUM_ATTRS(irProfileAttributesTable), &irProfileCBs);
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
 * @param oper - whether to validate and/or write attribute value .
 *
 * @return  Success or Failure.
 */
static bStatus_t assignValue8(gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset, uint8* result)
{
	if (offset == 0)
	{
		if (len != 1)
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

	*result = pValue[0];

	return SUCCESS;
};

void delayMicroseconds(uint16 duration)
{
	duration >>= 1;

	while (duration-- > 0)
	{
		NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
		NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
	}
};

void HalIrGenIsrCback()
{
	if (irRepeat > 0)
	{
		delayMicroseconds(45000);

		// Set TICKSPD
		CLKCONCMD &= ~HAL_IRGEN_CLKCON_TICKSPD_MASK;
		CLKCONCMD |= HAL_IRGEN_TICKSPD_8MHZ;

		HalIrGenCommandSirc(irCommand);

		irRepeat--;
	}
	else
	{
		HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT);

		#if defined (POWER_SAVING)
			osal_pwrmgr_task_state(irTaskId, PWRMGR_CONSERVE);
			osal_pwrmgr_device(PWRMGR_BATTERY);
		#endif
	}
};

void sendCommand(uint8 command)
{
	switch (irType)
	{
		case 0:
			if (irRepeat == 0)
			{
				//HalIrGenInitSirc();
			  
				HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT);

				#if defined (POWER_SAVING)
					osal_pwrmgr_task_state(irTaskId, PWRMGR_HOLD);
					osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
				#endif

				irRepeat = 3;

				// Set TICKSPD
				CLKCONCMD &= ~HAL_IRGEN_CLKCON_TICKSPD_MASK;
				CLKCONCMD |= HAL_IRGEN_TICKSPD_8MHZ;
				
				irCommand = HAL_IRGEN_CMD_SIRC(irAddress, command);
				
				HalIrGenCommandSirc(irCommand);
			}
			break;
		case 1:
		  	// not implemented
			break;
		case 2:
			// not implemented
			break;
		default:
			break;
	}
};

static bStatus_t irProfileWriteAttrCB(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset)
{
	bStatus_t	status		= SUCCESS;
	uint8		tmp			= 0;
  
	 // If attribute permissions require authorization to write, return error
	if (gattPermitAuthorWrite(pAttr->permissions))
	{
		// Insufficient authorization
		return ATT_ERR_INSUFFICIENT_AUTHOR;
	}
  
	if (linkDB_Encrypted( connHandle ) == FALSE)
	{
		//   return ATT_ERR_INSUFFICIENT_AUTHEN;
	}	
	
	if (pAttr->type.len == ATT_BT_UUID_SIZE)
	{
    	// 16-bit UUID
		uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

		switch (uuid)
		{
			case BLE_IR_TYPE_CHAR_UUID:
				status = assignValue8(pAttr, pValue, len, offset, &irType);
				break;
			case BLE_IR_ADDRESS_CHAR_UUID:
				status = assignValue8(pAttr, pValue, len, offset, &irAddress);
				break;
			case BLE_IR_COMMAND_CHAR_UUID:
				status = assignValue8(pAttr, pValue, len, offset, &tmp);
				sendCommand(tmp);
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

bStatus_t irInit(uint8 taskId)
{
	irTaskId = taskId;

	HalIrGenInitSirc();

	return SUCCESS;
};
