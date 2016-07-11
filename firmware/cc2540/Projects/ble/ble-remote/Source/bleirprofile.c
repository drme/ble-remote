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
#include "hal_irgen_NEC.h"
#include "hal_irgen_RC5.h"
#include "bleirprofile.h"
#include "bleremote.h"
#include "dl_util.h"
#include "osal_snv.h"
#include "bleremote.h"

#define DECLARE_UUID(name, id)                     static CONST uint8 name[DL_UUID_SIZE] = { DL_UUID(id) };
#define DECLARE_ATTR_DESCRIPTION(description)      { { ATT_BT_UUID_SIZE, charUserDescUUID }, GATT_PERMIT_READ, 0, description }
#define DECLARE_ATTR_VALUE(ID, permissions, value) { { DL_UUID_SIZE, ID }, permissions, 0, (uint8*)&value }
#define DECLARE_ATTR_PROPERTIES(properties)        { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, &properties }
#define DECLARE_ATTR(bleProperties, bleUUID, blePermission, bleValue, bleDescription)	DECLARE_ATTR_PROPERTIES(bleProperties), DECLARE_ATTR_VALUE(bleUUID, blePermission, bleValue), DECLARE_ATTR_DESCRIPTION(bleDescription)

DECLARE_UUID(irProfileServiceUUID,        BLE_IR_SERVICE_UUID);
DECLARE_UUID(irProfileCharTypeUUID,       BLE_IR_TYPE_CHAR_UUID);
DECLARE_UUID(irProfileCharCommandUUID,    BLE_IR_COMMAND_CHAR_UUID);
DECLARE_UUID(irProfileCharAddressUUID,    BLE_IR_ADDRESS_CHAR_UUID);
DECLARE_UUID(irProfileCharDevicePassUUID, BLE_IR_DEVICE_PASS_CHAR_UUID);

static CONST gattAttrType_t	irProfileService	= { DL_UUID_SIZE, irProfileServiceUUID };

static uint8 irTypeDescription[17]        = "Modulation Type\0";
static uint8 irCommandDescription[17]     = "Command\0";
static uint8 irAddressDescription[17]     = "Address\0";
static uint8 bleDevicePassDescription[22] = "Device Pass\0";

static uint8  irType    = 2;
static uint16 irAddress = 0x1;
static uint8  irCommand = 0;
static uint32 encodedCommand = 0;

uint32 devicePassword = 0x0;

static uint8 irTypeProperties       = GATT_PROP_WRITE | GATT_PROP_READ;
static uint8 irCommandProperties    = GATT_PROP_WRITE | GATT_PROP_READ;
static uint8 irAddressProperties    = GATT_PROP_WRITE | GATT_PROP_READ;
static uint8 irDevicePassProperties = GATT_PROP_WRITE | GATT_PROP_READ;

static uint8 irTaskId      = 0x0;
static uint8 currentIrType = 1;
static uint8 sircRepeat    = 0;

static gattAttribute_t irProfileAttributesTable[] =
{
	{ { ATT_BT_UUID_SIZE, primaryServiceUUID }, GATT_PERMIT_READ, 0, (uint8*)&irProfileService },

	DECLARE_ATTR(irTypeProperties,       irProfileCharTypeUUID,       GATT_PERMIT_WRITE | GATT_PERMIT_READ, irType,         irTypeDescription),
	DECLARE_ATTR(irCommandProperties,    irProfileCharCommandUUID,    GATT_PERMIT_WRITE | GATT_PERMIT_READ, irCommand,      irCommandDescription),
	DECLARE_ATTR(irAddressProperties,    irProfileCharAddressUUID,    GATT_PERMIT_WRITE | GATT_PERMIT_READ, irAddress,      irAddressDescription),
	DECLARE_ATTR(irDevicePassProperties, irProfileCharDevicePassUUID, GATT_PERMIT_WRITE | GATT_PERMIT_READ, devicePassword, bleDevicePassDescription),
};

static bStatus_t AssignValue8(gattAttribute_t* attribute, uint8* value, uint8 length, uint16 offset, uint8* result)
{
	if (offset == 0)
	{
		if (length != 1)
		{
			return ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		return ATT_ERR_ATTR_NOT_LONG;
	}
        
	uint8* curentValue = (uint8*)attribute->pValue;
	curentValue[0] = value[0];

	*result = value[0];

	return SUCCESS;
};

static bStatus_t AssignValue16(gattAttribute_t* attribute, uint8* value, uint8 length, uint16 offset, uint16* result)
{
	if (offset == 0)
	{
		if ((length != 1) && (length != 2))
		{
			return ATT_ERR_INVALID_VALUE_SIZE;
		}
	}
	else
	{
		return ATT_ERR_ATTR_NOT_LONG;
	}

	uint8* curentValue = (uint8*)attribute->pValue;
	
	if (length == 1)
	{
		curentValue[0] = value[0];
		*result = value[0];
	}
	else
	{
		curentValue[0] = value[0];
		curentValue[1] = value[1];
		*result = (value[1] << 8) | (value[0]);
	}

	return SUCCESS;
};

void HalIrGenIsrCback()
{
	if (sircRepeat == 2)
	{
		sircRepeat = 1;
		osal_start_timerEx(irTaskId, REPEAT_COMMAND1_EVENT, 45);
	}
	else if (sircRepeat == 1)
	{
		sircRepeat = 0;
		osal_start_timerEx(irTaskId, REPEAT_COMMAND2_EVENT, 45);
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






static bool rc5Toggle = false;




void SendCommand(uint8 command)
{
	HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT);

	#if defined (POWER_SAVING)
		osal_pwrmgr_task_state(irTaskId, PWRMGR_HOLD);
		osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
	#endif

	// Set TICKSPD
	CLKCONCMD &= ~HAL_IRGEN_CLKCON_TICKSPD_MASK;
	CLKCONCMD |= HAL_IRGEN_TICKSPD_8MHZ;
				
	switch (irType)
	{
		case 0:
 			encodedCommand = HAL_IRGEN_CMD_SIRC(irAddress, command);
			sircRepeat = 2;
			HalIrGenCommandSirc(encodedCommand);
			break;
		case 1:
			if (irAddress > 0xff)
			{
				encodedCommand = HAL_IRGEN_CMD_NEC_EX(irAddress, command);
			}
			else
			{
				encodedCommand = HAL_IRGEN_CMD_NEC(irAddress, command);
			}
			
			HalIrGenCommandNec(encodedCommand);
			break;
		case 2:
			encodedCommand = HAL_IRGEN_CMD_RC5(rc5Toggle ? 1 : 0, irAddress, command);
			HalIrGenCommandRc5(encodedCommand);
			rc5Toggle = !rc5Toggle;
			break;
		default:
			break;
	}
};

void IrChangeType()
{
	if (irType != currentIrType)
	{
		switch (irType)
		{
			case 0:
				HalIrGenInitSirc();
				break;
			case 1:
				HalIrGenInitNec();
				break;
			default:
				HalIrGenInitRc5();
				break;
		}

		currentIrType = irType;
	}
};

void IrRepeatCommand()
{
	switch (irType)
	{
		case 0:
			CLKCONCMD &= ~HAL_IRGEN_CLKCON_TICKSPD_MASK;
			CLKCONCMD |= HAL_IRGEN_TICKSPD_8MHZ;
			
			HalIrGenCommandSirc(encodedCommand);
			break;
		case 1:
			break;
		case 2:
			break;
		default:
			break;
	}
};

static bStatus_t SaveDevicePassword(uint16 connectionHandle, gattAttribute_t* attribute, uint8* value, uint8 length, uint16 offset)
{
	if (offset == 0)
	{
		if (length != 4)
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
		((uint8*)&devicePassword)[i] = value[i];
	}

	if (SUCCESS == osal_snv_write(DEVICE_PASSWORD_FLASH_ADDRESS, sizeof(uint32), &devicePassword))
	{
	}
	
	//pin change on power reset only			   
	//GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32), &devicePassword);
					
	return SUCCESS;
};

static bStatus_t IrProfileWriteAttributesCallBack(uint16 connHandle, gattAttribute_t* attribute, uint8* value, uint8 length, uint16 offset)
{
	bStatus_t status = SUCCESS;

	if (gattPermitAuthorWrite(attribute->permissions))
	{
		return ATT_ERR_INSUFFICIENT_AUTHOR;
	}

	uint16 uuid;

	if (ExtractUuid16(attribute, &uuid) == FAILURE)
	{
		return ATT_ERR_INVALID_HANDLE;
	}

	uint8 tmp = 0;

	switch (uuid)
	{
		case BLE_IR_TYPE_CHAR_UUID:
			status = AssignValue8(attribute, value, length, offset, &irType);
			osal_start_timerEx(irTaskId, CHANGE_TYPE_EVENT, 10);
			break;
		case BLE_IR_ADDRESS_CHAR_UUID:
			status = AssignValue16(attribute, value, length, offset, &irAddress);
			break;
		case BLE_IR_COMMAND_CHAR_UUID:
			status = AssignValue8(attribute, value, length, offset, &tmp);
			SendCommand(tmp);
			break;
		case BLE_IR_DEVICE_PASS_CHAR_UUID:
			status = SaveDevicePassword(connHandle, attribute, value, length, offset);
			break;				
		default:
			status = ATT_ERR_ATTR_NOT_FOUND;
			break;
	}

	return (status);
};

static uint8 IrProfileReadAttributesCallBack(uint16 connHandle, gattAttribute_t* attribute, uint8* value, uint8* length, uint16 offset, uint8 maxLength)
{
	bStatus_t status = SUCCESS;

	if (gattPermitAuthorRead(attribute->permissions))
	{
		return (ATT_ERR_INSUFFICIENT_AUTHOR);
	}

	if (offset > 0)
	{
		return (ATT_ERR_ATTR_NOT_LONG);
	}

	uint16 uuid;

	if (ExtractUuid16(attribute, &uuid) == FAILURE)
	{
		*length = 0;
		
		return ATT_ERR_INVALID_HANDLE;
	}
	
	switch (uuid)
	{
		case BLE_IR_DEVICE_PASS_CHAR_UUID:
			*length = 4;

			for (int i = 0; i < 4; i++)
			{
				value[i] = ((uint8*)&devicePassword)[i];
			}

			break;
		case BLE_IR_COMMAND_CHAR_UUID:
			*length = 4;

			for (int i = 0; i < 4; i++)
			{
				value[i] = ((uint8*)&encodedCommand)[i];
			}

			break;
		case BLE_IR_TYPE_CHAR_UUID:
			*length = 1;

			for (int i = 0; i < 1; i++)
			{
				value[i] = ((uint8*)&irType)[i];
			}
			
			break;
		case BLE_IR_ADDRESS_CHAR_UUID:
			*length = 2;

			for (int i = 0; i < 2; i++)
			{
				value[i] = ((uint8*)&encodedCommand)[i];
			}
			break;			
		default:
			*length = 0;
			status = ATT_ERR_ATTR_NOT_FOUND;
			break;
	}

	return status;
};

bStatus_t IrInit(uint8 taskId)
{
	irTaskId = taskId;

	Timer1Init();
	
	HalIrGenInitRc5();

	return SUCCESS;
};

CONST gattServiceCBs_t irProfileCallBacks =
{
	IrProfileReadAttributesCallBack,
	IrProfileWriteAttributesCallBack,
	NULL
};

bStatus_t IrProfileAddService(uint32 services)
{
	uint8 status = SUCCESS;

	if (services & BLE_IR_PROFILE_SERVICE)
	{
		status = GATTServApp_RegisterService(irProfileAttributesTable, GATT_NUM_ATTRS(irProfileAttributesTable), &irProfileCallBacks);
	}

	return (status);
};
