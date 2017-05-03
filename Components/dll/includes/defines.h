#ifndef __DEFINES_H__
#define __DEFINES_H__

#define PART_LM3S9B90



//#define HAL_UART0			true			// for interrupts enable - \b true, else \b false
//#define HAL_UART1			true			// for interrupts enable - \b true, else \b false
//#define	HAL_UART2			true			// for interrupts enable - \b true, else \b false




//*****************************************************************************
// just for testing
//*****************************************************************************
//#define __CMSIS_RTOS__
#define HAL_UART0	        true
#define HAL_UART1	        false
#define PORT_D
#define PC			0
#define CC2530			1
#define BAUDRATE_PC	        115200
#define BAUDRATE_CC2530	        9600

#endif // __DEFINES_H__
