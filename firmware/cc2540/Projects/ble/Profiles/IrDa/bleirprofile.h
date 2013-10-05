#ifndef _BLE_IR_PROFILE_H_
#define _BLE_IR_PROFILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define BLE_IR_SERVICE_UUID				0xEEEE
#define BLE_IR_TYPE_CHAR_UUID			0xEEE0
#define BLE_IR_COMMAND_CHAR_UUID		0xEEE1
#define BLE_IR_ADDRESS_CHAR_UUID		0xEEE2

#define BLE_IR_PROFILE_SERVICE			0x00000004

/**
 * Initializes the GATT Profile service by registering GATT attributes with the GATT server.
 *
 * @param services - services to add. This is a bit map and can contain more than one service.
 */
extern bStatus_t irProfileAddService(uint32 services);

/**
 * Initializes IR profile.
 */
extern bStatus_t irInit(uint8 taskId);

#ifdef __cplusplus
}
#endif

#endif
