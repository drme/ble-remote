#ifndef _BLE_IR_PROFILE_H_
#define _BLE_IR_PROFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define BLE_IR_SERVICE_UUID           0xEEEE
#define BLE_IR_TYPE_CHAR_UUID         0xEEE0
#define BLE_IR_COMMAND_CHAR_UUID      0xEEE1
#define BLE_IR_ADDRESS_CHAR_UUID      0xEEE2
#define BLE_IR_DEVICE_PASS_CHAR_UUID  0xF102
#define BLE_IR_PROFILE_SERVICE        0x00000004

extern uint32 devicePassword;

extern bStatus_t IrProfileAddService(uint32 services);
extern bStatus_t IrInit(uint8 taskId);
extern void IrChangeType();
extern void IrRepeatCommand();

#ifdef __cplusplus
}
#endif

#endif
