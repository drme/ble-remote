#ifndef _BLE_FAN_PROFILE_H_
#define _BLE_FAN_PROFILE_H_

#define BLE_FAN_SERVICE_UUID				0xBCC0
#define BLE_FAN_SPEED_CHAR_UUID				0xBCC1
#define BLE_FAN_DEVICE_NAME_CHAR_UUID		0xF101
#define BLE_FAN_DEVICE_PASS_CHAR_UUID		0xF102
#define BLE_FAN_PROFILE_SERVICE				0x00000002

#ifdef __cplusplus

extern "C"
{
#endif

typedef struct Settings
{
	uint16 deviceName;
	uint16 devicePassword;
	uint32 devicePassCode;
	uint8* deviceScanNameAddress;
} Settings;

extern Settings settings;

extern bStatus_t FanProfileAddService(uint32 services);
extern void FanProfileInit(uint8 taskId, uint8* deviceName, uint32* passCode);
extern void FanProfileStart(uint8 taskId);
extern void FanProfileStop();
extern void FanProfileUpdateStatus();
extern void UpdateDeviceName(uint8* deviceName);

extern void FanConnected(uint8 taskId);
extern void FanDisConnected();
extern bStatus_t FanSaveDevicePassword(uint16 connectionHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);
extern bStatus_t FanSaveDeviceName(uint16 connectionHandle, gattAttribute_t* pAttr, uint8* pValue, uint8 len, uint16 offset);
extern void FanLoadDeviceName(uint8* scanResponseName);
extern void FanUpdateStatus();
extern void FanControlInit(uint8 taskId, uint8* deviceScanName);

#ifdef __cplusplus
}
#endif

#endif
