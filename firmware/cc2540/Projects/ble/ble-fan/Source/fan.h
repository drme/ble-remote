#ifndef _FAN_H_
#define _FAN_H_

#ifdef __cplusplus
extern "C"
{
#endif
  
extern void FanInit(uint8 taskId);
extern uint16 FanProcessEvent(uint8 taskId, uint16 events);

#ifdef __cplusplus
}
#endif

#endif
