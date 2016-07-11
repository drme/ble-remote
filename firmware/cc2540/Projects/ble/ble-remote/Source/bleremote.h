#ifndef _BLE_REMOTE_H_
#define _BLE_REMOTE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define REMOTE_START_DEVICE_EVENT      0x0001
#define REMOTE_PERIODIC_EVENT          0x0002
#define REMOTE_ADV_IN_CONNECTION_EVENT 0x0004
#define BATT_PERIODIC_EVENT            0x0008
#define CHANGE_TYPE_EVENT              0x0010
#define REPEAT_COMMAND1_EVENT          0x0020
#define REPEAT_COMMAND2_EVENT          0x0040
#define DEVICE_PASSWORD_FLASH_ADDRESS  0x90

extern void RemotePeripheralInit(uint8 taskId);
extern uint16 RemotePeripheralProcessEvent(uint8 taskId, uint16 events);
extern void Timer1Init();

#ifdef __cplusplus
}
#endif

#endif
