#ifndef _FAN_CONTROL_H_
#define _FAN_CONTROL_H_

#include "hal_types.h"

#define FAN_START_DEVICE_EVT	0x0001
#define FAN_UPDATE_STATUS_EVT	0x0004

#ifdef __cplusplus
extern "C"
{
#endif

extern void FanServiceInit(uint8 taskId);
extern void FanSetSpeed(uint16 value);

#ifdef __cplusplus
}
#endif

#endif
