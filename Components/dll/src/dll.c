#include "includes/dll.h"
//#include "hal/includes/hal_gpio.h"
#include "includes/defines.h"
#include "hal_uart.h"
#include "includes/circular_buffer.h"

#include "hal_lcd.h"
#include "OnBoard.h"
//#include <stdlib.h>
//#include "stdlib.h"
//#include "cmsis_os.h"

#include <stdio.h>
#include <stdint.h>
//#include <stdlib.h>
#include "hal_types.h"
//*****************************************************************************
//
// A mapping of devices IDs to signal_id number.
//
//*****************************************************************************
static const uint16 signalID[] = {
                                  TEMP_SENSOR_INSIDE,
                                  TEMP_SENSOR_OUTSIDE,
                                  PRESSURE_SENSOR_INSIDE,
                                  PRESSURE_SENSOR_OUTSIDE,
                                  HUMIDITY_SENSOR_INSIDE,
                                  HUMIDITY_SENSOR_OUTSIDE,
                                  DOOR,
                                  WINDOW,
                                  MOTION_SENSOR
                                  };




//*****************************************************************************
//
// Internal/local functions
// \just defining functionality/
//
//*****************************************************************************
static uint8 getSignalID(const uint16 devicesAddress);
static uint8 checksum(DLLPacket_t *frame);

//static uint8 processFrameRx(uint8 *frame, uint8 len, uint16 Address);
//static uint8 devAddressSupport(uint16 devAddress);


//static uint8 halUARTWrite(CircularBuffer_t *txBuffer, uint8 len);
static void packFrameToStell(uint8 *buff, DLLPacket_t emPacket);
//*****************************************************************************
//
// CMSIS-RTOS tread definition and tread  function declaration
//
//*****************************************************************************
/*
void communicationThread(void const *arg);
osThreadId tID_PC_Communication;
osThreadId tID_CC2530_Communication;
*/

//*****************************************************************************
//
// Extern variables
//
// treba promijeniti naziv ova dva bafera u circularBuffer.h
//
//*****************************************************************************
CircularBuffer_t rxBufferPC;
CircularBuffer_t rxBufferCC2530;

//*****************************************************************************
//
// Local variables
//
//*****************************************************************************
//DLLState_t statePC;
DLLState_t stateCC2530;
//uint8 emFlagPC;
uint8 emFlagCC2530;

//char uiTxBufferPC[MAX_BUFFER_LENGHT];
//uint8 uiRxBufferPC[MAX_BUFFER_LENGHT];

char uiTxBufferCC2530[MAX_BUFFER_LENGHT];
uint8 uiRxBufferCC2530[MAX_BUFFER_LENGHT];

//uint8 rxBufferPCIndex;
//uint8 txBufferPCIndex;

uint8 rxBufferCC2530Index;
uint8 txBufferCC2530Index;

//uint8 tmpPC;
uint8 tmpCC2530;

//CircularBuffer_t cTxBufferPC;
CircularBuffer_t cTxBufferCC2530;

DLLPacket_t recPack;

static CallBack_t callBack;

//*****************************************************************************
//
// Function definition
//
//*****************************************************************************

//*****************************************************************************
//
//! Data link layer initialization function
//!
//! This function initializes states and indexes,
//! configures communication ports, one of \b PC or \b CC2530 and their
//! baudrates, one of \b BAUDRATE_PC \b BAUDRATE_CC2530. Creates two threads
//! for communication wia UART interface.
//!
//! \return None.
//
//*****************************************************************************
void dllInit(void)
{
	//osThreadDef(communicationThread, osPriorityHigh, 2, 0);

	//uint8 portPC = PC;
	//uint8 portCC2530 = CC2530;

	//statePC = IDLE;
	stateCC2530 = IDLE;
	//rxBufferPCIndex = 0;
	//txBufferPCIndex = 0;
	rxBufferCC2530Index = 0;
	txBufferCC2530Index = 0;

	//circularInit(&cTxBufferPC);
	circularInit(&cTxBufferCC2530);
        /*CircularBuffer_t *cRxBuffer;
	CircularBuffer_t *cTxBuffer;
        cRxBuffer = &rxBufferCC2530;
        cTxBuffer = &cTxBufferCC2530;
        */
     /*   
        
        uint8 port  = *(uint8 *)(arg);
	uint8 *tmp;
	uint8 *rxBuffer;
        uint8 *txBuffer;
	uint8 *txIndex;
	uint8 *rxIndex;
	uint8 *emFlag;
	DLLState_t *state;
	CircularBuffer_t *cRxBuffer;
	CircularBuffer_t *cTxBuffer;
        
        tmp = &tmpCC2530;
        state = &stateCC2530;
        txBuffer = (uint8 *)uiTxBufferCC2530;
        rxBuffer = uiRxBufferCC2530;
        txIndex = &txBufferCC2530Index;
        rxIndex = &rxBufferCC2530Index;
        emFlag = &emFlagCC2530;
        cRxBuffer = &rxBufferCC2530;
        cTxBuffer = &cTxBufferCC2530;
*/
	//halUARTOpenPort(PC, BAUDRATE_PC , 1, 0);
	//halUARTOpenPort(CC2530, BAUDRATE_CC2530, 1, 0);
        
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Ovdje treba initUart
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//tID_PC_Communication = osThreadCreate( osThread(communicationThread), (void *)(&portPC));
	//tID_CC2530_Communication = osThreadCreate( osThread(communicationThread), (void *)(&portCC2530));
}

//*****************************************************************************
//
//! Register callback function.
//!
//! \param cb is pointer on the function.
//!
//! \return None.
//
//*****************************************************************************
void CallBackRegister(CallBack_t cb)
{
	callBack = cb;
}

//*****************************************************************************
//
//! Function handles comminucation. Handles RX or TX on active ports.
//!
//! \param arg is pointer on port.
//!
//! This function sends data from app layer forward and delivers data to app
//! layer.
//!
//! \return None.
//
//*****************************************************************************
/*
void communicationThread(void const *arg)
{
	uint8 port  = *(uint8 *)(arg);
	uint8 *tmp;
	uint8 *rxBuffer;
        uint8 *txBuffer;
	uint8 *txIndex;
	uint8 *rxIndex;
	uint8 *emFlag;
	DLLState_t *state;
	CircularBuffer_t *cRxBuffer;
	CircularBuffer_t *cTxBuffer;

	if (PC == port)
	{
		tmp = &tmpPC;
		state = &statePC;
		txBuffer = (uint8 *)uiTxBufferPC;
		rxBuffer = uiRxBufferPC;
		txIndex = &txBufferPCIndex;
		rxIndex = &rxBufferPCIndex;
		emFlag = &emFlagPC;
		cRxBuffer = &rxBufferPC;
		cTxBuffer = &cTxBufferPC;
	}
	else if (CC2530 == port)
	{
  	tmp = &tmpCC2530;
		state = &stateCC2530;
		txBuffer = (uint8 *)uiTxBufferCC2530;
		rxBuffer = uiRxBufferCC2530;
		txIndex = &txBufferCC2530Index;
		rxIndex = &rxBufferCC2530Index;
		emFlag = &emFlagCC2530;
		cRxBuffer = &rxBufferCC2530;
		cTxBuffer = &cTxBufferCC2530;
	}

	while (1)
	{
		osDelay(10);
		switch ((*state))
		{
			case IDLE:
				if (circularGet(cRxBuffer, tmp))	//zastita!!!
				{
					if (START_DELIMITER == (*tmp))
					{
						*rxIndex = 0;
						*state = RECEPTION;
					}
					else
					{
						*state = IDLE;

					}
				}
				else if (EMISSION_START == (*emFlag))
				{
					*state = EMISSION;
				}

				else
				{
					*state = IDLE;

				}
				break;
			case RECEPTION:
				do
				{
					if (circularGet(cRxBuffer, tmp)) // zastita!!!
					{
						if (START_DELIMITER == (*tmp))
						{
							*rxIndex = 0;
						}
						else
						{
							rxBuffer[(*rxIndex)++] = *tmp;
						}
					}
				} while(STOP_DELIMITER != *tmp);
				*state = IDLE;				//
				processFrameRx(rxBuffer, *rxIndex, port);
			break;
			case EMISSION:
				array2circular(cTxBuffer, txBuffer, *txIndex);
				halUARTWrite(port, cTxBuffer, 0);
				*txIndex = 0;
				*state = IDLE;
			break;
			default:
				// error state!!!
			break;
		}
	}
}
*/
//*****************************************************************************
//
//! Get the signal ID.
//!
//! \param deviceAddress is device address.
//!
//! For given a device address, this function returns the corresponding
//! signal ID.
//!
//! \return Returns signal ID, or -1 if \e deviceAddress is not supported.
//
//*****************************************************************************
static uint8 getSignalID(const uint16 deviceAddress)
{
	uint8 i;
        //uint8 j;
        char *id;
        //char idde;
        uint16 idde;
        
	uint8 retVal = 255;
	for (i = 0; i < sizeof(signalID); i+=2)
	{
            
		if (deviceAddress == signalID[i])
		{
			retVal = i;
                        
                        
		}
                //idde = deviceAddress;//i + 0x30;//signalID[i];
                idde = signalID[i];//i + 0x30;//signalID[i];
                id = (char*)&idde;
                //id[0] = idde;
               // id[1] = '\0';
                id[2] = '\0';
                //HalLcdWriteString("-----------------------------SignalID----------------------",0);
               // HalLcdWriteString(id,0);
                /*for(j=0;j<3;j++)
                {
                  uartSend(*(id + j));
                  
                }*/
                //HalLcdWriteString("-----------------------------SignalID----------------------",0);
                idde = deviceAddress;//i + 0x30;
                //signalID[i];id = (char*)&idde;
                //HalLcdWriteString("-----------------------------deviceAddress----------------------",0);
                //HalLcdWriteString(id,0);
                /*for(j=0;j<3;j++)
                {
                  uartSend(*(id + j));
                  
                }*/
                //HalLcdWriteString("-----------------------------deviceAddress----------------------",0);
	}
	return retVal;
}

//*****************************************************************************
//
//! Process received data.
//!
//! \param frame is pointer on the received frame.
//! \param len is lenght of received frame.
//! \param port is number of port.
//!
//! Function process received frame, converts, checks checksum and validation
//! of signal ID (supported devices)
//!
//! \return Returns signal ID, or -1 if \e deviceAddress is not supported.
//
//*****************************************************************************
uint16 processFrameRx(uint8 *frame)
{
	Data_t *cbData;
	int iSignalID;
	int iData;
	int iPackNum;
	int iTimeStamp;
	int iCheckSum;
	sscanf((char *)frame,
                      "%*[^0123456789]%d\
                       %*[^0123456789]%d\
                       %*[^0123456789]%d\
                       %*[^0123456789]%d\
                       %*[^0123456789]%d",
                       &iSignalID,
                       &iPackNum,
                       &iData,
                       &iTimeStamp,
                       &iCheckSum
                       );
	recPack.appData.devID = signalID[iSignalID];
	recPack.appData.packNum = (uint32)iPackNum & 0xff;
	recPack.appData.data = (uint32)iData & 0xffffffff;
	recPack.timeStamp = (uint32)iTimeStamp & 0xff;
	recPack.checkSum = (uint32)iCheckSum & 0xff;

	cbData = (struct sData *)(&recPack);


//if (checksum(recPack) == recPack->checkSum)
//{
//	if (devAddressSupport(recPack->appData.devID))
//	{
			callBack(cbData);

//	}
		//return 1;
                return recPack.appData.devID;
//}
//else
//{
//	error_checkSum();
//	recPack = NULL;
//	return 0;
//}

}


//*****************************************************************************
//
//! Calculating checksum.
//!
//! \param frame is pointer on frame on which one is needed to calculated.
//!
//! Function caluculates checksum executing XOR operation on evert member of
//! \b frame.
//!
//! \return Returns result of sumary operation.
//
//*****************************************************************************
static uint8 checksum(DLLPacket_t *frame)
{
	uint8 sum  = 0x00;
	uint8 *ptr = (uint8 *)(frame);
	uint8 *end = &frame->checkSum;
	while(ptr != end)
	{
		sum ^= *ptr;
		ptr++;
	}

	return sum;

}

//*****************************************************************************
//
//! Checks a devices address.
//!
//! \param devAddress is device address.
//!
//! This function determines if a device address is supported.
//!
//! \return Returns \b true if address is supported, or \b false otherwise.
//
//*****************************************************************************
/*
static uint8 devAddressSupport(uint16 devAddress)
{
	return( (devAddress == TEMP_SENSOR_INSIDE) ||
					(devAddress == TEMP_SENSOR_OUTSIDE) ||
					(devAddress == PRESSURE_SENSOR_INSIDE) ||
 					(devAddress == PRESSURE_SENSOR_OUTSIDE) ||
					(devAddress == HUMIDITY_SENSOR_INSIDE) ||
					(devAddress == HUMIDITY_SENSOR_OUTSIDE) ||
					(devAddress == DOOR) ||
					(devAddress == WINDOW) ||
					(devAddress == MOTION_SENSOR)
					);
}
*/
//*****************************************************************************
//
//! Function that handles data services.
//!
//! \param aData is pointer on data structure.
//! \param port indicates port number.
//!
//! This function prepare data to be ready to send and sets state flag to
//! signals the thread to perform an action.
//!
//! \return None.
//
//*****************************************************************************
/*
static uint8 halUARTWrite(CircularBuffer_t *txBuffer, uint8 len)
{
	uint8 cnt = 0;
	uint8 i;
	uint8 data;

	if (0 == len)
	{
		while(!circularIsEmpty(txBuffer))
		{
			circularGet(txBuffer, &data);
                        uartSend(data);
			cnt++;

		}// end while
	}// end if

	else if (len < circularSize(txBuffer))
	{
		for (i = 0; i < len; i++)
		{
			circularGet(txBuffer, &data);
			uartSend(data);
		}// end for
		cnt = i;

	}// end else if

	else
	{
		// error_uart();

	}// end else

	return cnt;
}
*/
static void packFrameToStell(uint8 *buff, DLLPacket_t emPacket)
{
  
  
  //uint32 da;
  
  int i = 0;
  int j = 0;
  int k = 0;
  uint8 *pBuf = buff;
 
  uint32 pom = emPacket.appData.data;
  
  
  char dat[4] = {'0','0','0','0'};
  char daaat[4] = {'0','0','0','0'};
 // char *id; //= dat;
  
 
    while(pom)
    {
      dat[i] = (pom % 16) + '0';
      pom/=256;
      i++;
    }
  
    k = 4 - i;
    for(j = 0; j < k; j++)
    {
      daaat[j] = '0';
      
    }
    i = 0;
    for(; j < 4; j++)
    {   
      daaat[k] = dat[i];
      k++;
      i++;
      
    }
 
      //uint8 len = 1 + 1 + 4 + 1 + 1;
  *pBuf++ = 0x3E;
  
  *pBuf++ = 0x23;
  
  
  *pBuf++ = getSignalID(emPacket.appData.devID) + 0x30;
  
  *pBuf++ = 0x2C;
  *pBuf++ = 0x23;
  
  
  *pBuf++ = emPacket.appData.packNum + 0x30;
  
  *pBuf++ = 0x2C;
  *pBuf++ = 0x23;
  
  
  for(i = 0; i < 4; i++)
  {
    *pBuf++ =daaat[i] ;
     //uartSend(data[i]);//+'0');
    //HalLcdWriteString((char *)data[i], 0);
    //HalLcdWriteString("-----------Simic-----------", 0);
  }
  
  *pBuf++ = 0x2C;
  *pBuf++ = 0x23;
  
  
  *pBuf++ = emPacket.timeStamp + 0x30;  
  
  *pBuf++ = 0x2C;
  *pBuf++ = 0x23;
  
  
  *pBuf++ = emPacket.checkSum + 0x30;
  *pBuf++ = 0x3C;
  *pBuf++ = 0x00;
  /*
  da = emPacket.appData.data;
  id = (char*)&da;
  id[4] = '\0';
  HalLcdWriteString("-----------------------------Paket----------------------",0);
  HalLcdWriteString(id,0);
  HalLcdWriteString("-----------------------------Paket----------------------",0);
  */
} 

void dllDataRequest(Data_t *aData)
{
        //char *id;
        //uint16 idde;
        //uint8 num;
        //uint32 da;
        /*
        int iSignalID;
	int iData;
	int iPackNum;
	int iTimeStamp;
	int iCheckSum;
        */
        //uint8 *txBuffer;
        //uint8 *txIndex;
        //CircularBuffer_t *cTxBuffer;
        
	DLLPacket_t emPacket;
	//emPacket.appData = *aData;
        //emPacket.appData = aData;
        
        emPacket.appData.devID = aData->devID;
        emPacket.appData.data = aData->data;
        emPacket.appData.packNum = aData->packNum;
        emPacket.timeStamp = 0;
	emPacket.checkSum = checksum(&emPacket);
        
        //txBuffer = (uint8 *)uiTxBufferCC2530;
        //cTxBuffer = &cTxBufferCC2530;
        //txBuffer = (uint8 *)uiTxBufferCC2530;
        //txIndex = &txBufferCC2530Index;
	// ako bude bilo potrebe za slanjem vremenskog trenutka
	// radi iscrtavanja grafa obezbjedicemo mehanizam, inace
	// timeStamp = 0
	/*
        iSignalID = (int)getSignalID(emPacket.appData.devID);
	iData = (int)emPacket.appData.data;
	iPackNum = (int)emPacket.appData.packNum;
	iTimeStamp = (int)emPacket.timeStamp;
	iCheckSum = (int)emPacket.checkSum;
        */
        /*
        iSignalID = (unsigned)getSignalID(emPacket.appData.devID);
	iData = (unsigned)emPacket.appData.data;
	iPackNum = (unsigned)emPacket.appData.packNum;
	iTimeStamp = (unsigned)emPacket.timeStamp;
	iCheckSum = (unsigned)emPacket.checkSum;
        */
        
	 //HalLcdWriteString("Miso----------------------",0);
        /*
        txBufferCC2530Index = sprintf(uiTxBufferCC2530,
                                      ">#%d,#%c,#%l,#%c,#%c",
                                      getSignalID(emPacket.appData.devID),
                                      emPacket.appData.packNum,
                                      emPacket.appData.data,
                                      emPacket.timeStamp,
                                      emPacket.checkSum
                                      );
        */
        /*
        txBufferCC2530Index = sprintf(uiTxBufferCC2530,
                                      ">#%d,#%d,#%d,#%d,#%d",
                                      iSignalID,
                                      iPackNum,
                                      iData,
                                      iTimeStamp,
                                      iCheckSum
                                      );
        */
        packFrameToStell((uint8 *)uiTxBufferCC2530, emPacket);
        //array2circular(cTxBuffer, txBuffer, *txIndex);
        
        /*HalLcdWriteString("Velicina je:",0);
        uartSend(sizeof(uint32)+'0');
        HalLcdWriteString("",0);
        HalLcdWriteString("Velicina je:",0);
        */
        //halUARTWrite(cTxBuffer, 0);
        //HalLcdWriteString("Miso----------------------",0); 
        
       // halUARTWrite(port, cTxBuffer, 0);
        HalLcdWriteString(uiTxBufferCC2530, 0);
        //HalLcdWriteString(uiTxBufferCC2530, 0);
        
	emFlagCC2530 = EMISSION_START;
	
	//idde = emPacket.appData.devID;//
        
        //idde = aData->devID;
        
        //idde = (uint16)getSignalID(emPacket.appData.devID);//+ 0x30;
        
        //idde = aData->devID;
        //idde = aData.devID;
        
        //id = (char*)&emPacket.appData.data;
        //id = (char*)&emPacket.appData.devID;
        
        //da = emPacket.appData.data;
        
        //num = emPacket.timeStamp + 0x30;
       // num = emPacket.appData.packNum + 0x30;
        //num = emPacket.checkSum + 0x30;
        
        //id = (char*)&idde;
        //id = (char*)&da;
        //id = (char*)&num;
        
        //id[0] += 0x30;
        //id[1] += 0x30;
        
        //id[2] = '\0';
        //id[1] = '\0';
        //id[4] = '\0';
        
        //HalLcdWriteString("-----------------------------Borislav----------------------",0);
        //HalLcdWriteString(id,0);
        //uartSend(emPacket.checkSum+'0');
        //HalLcdWriteString("", 0);
        //HalLcdWriteString("-----------------------------Borislav----------------------",0);

}
