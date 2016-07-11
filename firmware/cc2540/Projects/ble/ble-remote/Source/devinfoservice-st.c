#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "devinfoservice.h"

CONST uint8 devInfoServUUID[ATT_BT_UUID_SIZE]         = { LO_UINT16(DEVINFO_SERV_UUID),              HI_UINT16(DEVINFO_SERV_UUID) };
CONST uint8 devInfoSystemIdUUID[ATT_BT_UUID_SIZE]     = { LO_UINT16(DEVINFO_SYSTEM_ID_UUID),         HI_UINT16(DEVINFO_SYSTEM_ID_UUID) };
CONST uint8 devInfoModelNumberUUID[ATT_BT_UUID_SIZE]  = { LO_UINT16(DEVINFO_MODEL_NUMBER_UUID),      HI_UINT16(DEVINFO_MODEL_NUMBER_UUID) };
CONST uint8 devInfoSerialNumberUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(DEVINFO_SERIAL_NUMBER_UUID),     HI_UINT16(DEVINFO_SERIAL_NUMBER_UUID) };
CONST uint8 devInfoFirmwareRevUUID[ATT_BT_UUID_SIZE]  = { LO_UINT16(DEVINFO_FIRMWARE_REV_UUID),      HI_UINT16(DEVINFO_FIRMWARE_REV_UUID) };
CONST uint8 devInfoHardwareRevUUID[ATT_BT_UUID_SIZE]  = { LO_UINT16(DEVINFO_HARDWARE_REV_UUID),      HI_UINT16(DEVINFO_HARDWARE_REV_UUID) };
CONST uint8 devInfoSoftwareRevUUID[ATT_BT_UUID_SIZE]  = { LO_UINT16(DEVINFO_SOFTWARE_REV_UUID),      HI_UINT16(DEVINFO_SOFTWARE_REV_UUID) };
CONST uint8 devInfoMfrNameUUID[ATT_BT_UUID_SIZE]      = { LO_UINT16(DEVINFO_MANUFACTURER_NAME_UUID), HI_UINT16(DEVINFO_MANUFACTURER_NAME_UUID) };
CONST uint8 devInfo11073CertUUID[ATT_BT_UUID_SIZE]    = { LO_UINT16(DEVINFO_11073_CERT_DATA_UUID),   HI_UINT16(DEVINFO_11073_CERT_DATA_UUID) };
CONST uint8 devInfoPnpIdUUID[ATT_BT_UUID_SIZE]        = { LO_UINT16(DEVINFO_PNP_ID_UUID),            HI_UINT16(DEVINFO_PNP_ID_UUID) };

static CONST gattAttrType_t devInfoService = { ATT_BT_UUID_SIZE, devInfoServUUID };
static uint8 devInfoSystemIdProps = GATT_PROP_READ;
static uint8 devInfoSystemId[DEVINFO_SYSTEM_ID_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};
static uint8 devInfoModelNumberProps = GATT_PROP_READ;
static const uint8 devInfoModelNumber[] = "1.0";
static uint8 devInfoSerialNumberProps = GATT_PROP_READ;
static uint8 devInfoSoftwareRevProps = GATT_PROP_READ;
static const uint8 devInfoSoftwareRev[] = "1.0";
static uint8 devInfoFirmwareRevProps = GATT_PROP_READ;

static uint8 devInfoHardwareRevProps = GATT_PROP_READ;

static uint8 devInfoMfrNameProps = GATT_PROP_READ;
static const uint8 devInfoMfrName[] = "sarunas.eu";
static uint8 devInfo11073CertProps = GATT_PROP_READ;
static uint8 devInfoPnpIdProps = GATT_PROP_READ;

static const uint8 devInfoSerialNumber[] = "BLETELLY ("__DATE__")("__TIME__")";

#if defined(CC2540)
	static const uint8 devInfoFirmwareRev[]  = "1.0.0 ("__DATE__" "__TIME__")";
#else
	static const uint8 devInfoFirmwareRev[]  = "2.0.0 ("__DATE__" "__TIME__")";
#endif

#if defined(HAL_IMAGE_A)
	static const uint8 devInfoHardwareRev[]  = "1.0 - A";
#elif defined(HAL_IMAGE_B)
	static const uint8 devInfoHardwareRev[]  = "1.0 - B";
#else
	static const uint8 devInfoHardwareRev[]  = "1.0 - 0";
#endif

static const uint8 devInfo11073Cert[] =
{
	DEVINFO_11073_BODY_EXP,     // authoritative body type
	0x00,                       // authoritative body structure type authoritative body data follows below:
	'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'
};

static uint8 devInfoPnpId[DEVINFO_PNP_ID_LEN] =
{
	1,                                      // Vendor ID source (1=Bluetooth SIG)
	LO_UINT16(0x000D), HI_UINT16(0x000D),   // Vendor ID (Texas Instruments)
	LO_UINT16(0x0000), HI_UINT16(0x0000),   // Product ID (vendor-specific)
	LO_UINT16(0x0110), HI_UINT16(0x0110)    // Product version (JJ.M.N)
};

static gattAttribute_t devInfoAttrTbl[] =
{
	{
		{ ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
		GATT_PERMIT_READ,                         /* permissions */
		0,                                        /* handle */
		(uint8*)&devInfoService                  /* pValue */
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoSystemIdProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoSystemIdUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoSystemId
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoModelNumberProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoModelNumberUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoModelNumber
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoSerialNumberProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoSerialNumberUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoSerialNumber
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoFirmwareRevProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoFirmwareRevUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoFirmwareRev
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoHardwareRevProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoHardwareRevUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoHardwareRev
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoSoftwareRevProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoSoftwareRevUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoSoftwareRev
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoMfrNameProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoMfrNameUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoMfrName
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfo11073CertProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfo11073CertUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfo11073Cert
	},
	{
		{ ATT_BT_UUID_SIZE, characterUUID },
		GATT_PERMIT_READ,
		0,
		&devInfoPnpIdProps
	},
	{
		{ ATT_BT_UUID_SIZE, devInfoPnpIdUUID },
		GATT_PERMIT_READ,
		0,
		(uint8*)devInfoPnpId
	}
};

bStatus_t DevInfo_SetParameter(uint8 param, uint8 len, void* value)
{
	bStatus_t ret = SUCCESS;

	switch (param)
	{
		case DEVINFO_SYSTEM_ID:
			osal_memcpy(devInfoSystemId, value, len);
			break;
		default:
			ret = INVALIDPARAMETER;
			break;
	}

	return (ret);
};

bStatus_t DevInfo_GetParameter(uint8 param, void* value)
{
	bStatus_t ret = SUCCESS;

	switch (param)
	{
		case DEVINFO_SYSTEM_ID:
			osal_memcpy(value, devInfoSystemId, sizeof(devInfoSystemId));
			break;
		case DEVINFO_MODEL_NUMBER:
			osal_memcpy(value, devInfoModelNumber, sizeof(devInfoModelNumber));
			break;
		case DEVINFO_SERIAL_NUMBER:
			osal_memcpy(value, devInfoSerialNumber, sizeof(devInfoSerialNumber));
			break;
		case DEVINFO_FIRMWARE_REV:
			osal_memcpy(value, devInfoFirmwareRev, sizeof(devInfoFirmwareRev));
			break;
		case DEVINFO_HARDWARE_REV:
			osal_memcpy(value, devInfoHardwareRev, sizeof(devInfoHardwareRev));
			break;
		case DEVINFO_SOFTWARE_REV:
			osal_memcpy(value, devInfoSoftwareRev, sizeof(devInfoSoftwareRev));
			break;
		case DEVINFO_MANUFACTURER_NAME:
			osal_memcpy(value, devInfoMfrName, sizeof(devInfoMfrName));
			break;
		case DEVINFO_11073_CERT_DATA:
			osal_memcpy(value, devInfo11073Cert, sizeof(devInfo11073Cert));
			break;
		case DEVINFO_PNP_ID:
			osal_memcpy(value, devInfoPnpId, sizeof(devInfoPnpId));
			break;
		default:
			ret = INVALIDPARAMETER;
			break;
	}

	return (ret);
};

static uint8 DevInfoReadAttributeCallBack(uint16 connHandle, gattAttribute_t* pAttr, uint8* pValue, uint8* pLen, uint16 offset, uint8 maxLen)
{
	bStatus_t status = SUCCESS;
	uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

	switch (uuid)
	{
		case DEVINFO_SYSTEM_ID_UUID:
			if (offset >= sizeof(devInfoSystemId))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoSystemId) - offset));
				osal_memcpy(pValue, &devInfoSystemId[offset], *pLen);
			}
			break;
		case DEVINFO_MODEL_NUMBER_UUID:
			if (offset >= sizeof(devInfoModelNumber))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoModelNumber) - offset));
				osal_memcpy(pValue, &devInfoModelNumber[offset], *pLen);
			}
			break;
		case DEVINFO_SERIAL_NUMBER_UUID:
			if (offset >= sizeof(devInfoSerialNumber))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoSerialNumber) - offset));
				osal_memcpy(pValue, &devInfoSerialNumber[offset], *pLen);
			}
			break;
		case DEVINFO_FIRMWARE_REV_UUID:
			if (offset >= sizeof(devInfoFirmwareRev))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoFirmwareRev) - offset));
				osal_memcpy(pValue, &devInfoFirmwareRev[offset], *pLen);
			}
			break;
		case DEVINFO_HARDWARE_REV_UUID:
			if (offset >= sizeof(devInfoHardwareRev))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoHardwareRev) - offset));
				osal_memcpy(pValue, &devInfoHardwareRev[offset], *pLen);
			}
			break;
		case DEVINFO_SOFTWARE_REV_UUID:
			if (offset >= sizeof(devInfoSoftwareRev))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoSoftwareRev) - offset));
				osal_memcpy(pValue, &devInfoSoftwareRev[offset], *pLen);
			}
			break;
		case DEVINFO_MANUFACTURER_NAME_UUID:
			if (offset >= sizeof(devInfoMfrName))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoMfrName) - offset));
				osal_memcpy(pValue, &devInfoMfrName[offset], *pLen);
			}
			break;
		case DEVINFO_11073_CERT_DATA_UUID:
			if (offset >= sizeof(devInfo11073Cert))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfo11073Cert) - offset));
				osal_memcpy(pValue, &devInfo11073Cert[offset], *pLen);
			}
			break;
		case DEVINFO_PNP_ID_UUID:
			if (offset >= sizeof(devInfoPnpId))
			{
				status = ATT_ERR_INVALID_OFFSET;
			}
			else
			{
				*pLen = MIN(maxLen, (sizeof(devInfoPnpId) - offset));
				osal_memcpy(pValue, &devInfoPnpId[offset], *pLen);
			}
			break;
		default:
			*pLen = 0;
			status = ATT_ERR_ATTR_NOT_FOUND;
			break;
	}

	return (status);
};

CONST gattServiceCBs_t devInfoCBs =
{
	DevInfoReadAttributeCallBack, // Read callback function pointer
	NULL,                     // Write callback function pointer
	NULL                      // Authorization callback function pointer
};

bStatus_t DevInfo_AddService(void)
{
	return GATTServApp_RegisterService(devInfoAttrTbl, GATT_NUM_ATTRS(devInfoAttrTbl), &devInfoCBs);
};
