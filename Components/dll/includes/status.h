#ifndef __STATUS_H__
#define __STATUS_H__


typedef enum sStatus
{
	ERROR = -1,
	SUCCESS = 1,
	GPIO_ERROR =-2,
	GPIO_SUCCES = 2
} Status_t;

typedef enum
{
	TX_ERROR = -1,
	TX_START = 1,
	TX_COMPLETE = 3,
	TX_SUCCESS = 5,
	RX_ERROR = 0,
	RX_START = 2,
	RX_COMPLETE = 4,
	Rx_SUCCESS = 6
} TxRxStatus_t;


							
// uint8_t getPortStatus(Port_t port);
// uint8_t getPinStatus(Port_t port, uint8_t pinNum);


#endif //__STATUS_H__
