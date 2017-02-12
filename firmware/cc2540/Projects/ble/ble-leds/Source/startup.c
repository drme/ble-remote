#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"

int main(void)
{
	HAL_BOARD_INIT();
	InitBoard(OB_COLD);
	HalDriverInit();
	osal_snv_init();
	osal_init_system();
	HAL_ENABLE_INTERRUPTS();
	InitBoard(OB_READY);

	#if defined (POWER_SAVING)
		osal_pwrmgr_device(PWRMGR_BATTERY);
	#endif

	osal_start_system();

	return 0;
};
