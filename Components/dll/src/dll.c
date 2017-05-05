#include "includes/dll.h"
//#include "hal/includes/hal_gpio.h"
#include "includes/defines.h"
#include "hal_uart.h"
#include "includes/circular_buffer.h"

#include "hal_lcd.h"
#include "OnBoard.h"
#include <stdlib.h>
//#include "stdlib.h"
//#include "cmsis_os.h"

#include <stdio.h>
//#include <stdint.h>
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

//halUARTCfg_t uartConfig;
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

	stateCC2530 = IDLE;

	rxBufferCC2530Index = 0;
	txBufferCC2530Index = 0;

	//circularInit(&cTxBufferPC);
	circularInit(&cTxBufferCC2530);
        
      
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
uint8 getSignalID(const uint16 deviceAddress)
{
	uint8 i;
        
	uint8 retVal = 255;
	for (i = 0; i < sizeof(signalID); i+=2)
	{
            
		if (deviceAddress == signalID[i])
		{
			retVal = i;
                        
                        
		}
               
	}
	return retVal;
}
uint16 getSignalAddress(const uint16 ID)
{
  uint8 i;
        
	uint16 retVal = 0;
	for (i = 0; i < sizeof(signalID); i+=2)
	{
            
		if (ID == i)
		{
			retVal = signalID[i];
                        
                        
		}
               
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
uint16 processFrame(uint8 *buff)
{
	uint8 i = 0;
	uint8 j = 0;
        uint8 k = 0;
	
        Data_t *cbData;
        
        // pomocne promjenljive
        char cID[10];
        char cPackNum[10];
        char cCmd[10];
        char cTimeStamp[10];
        char cCheckSum[10];
        
        char dID[10];
        char dCmd[10];
          
        //uint8 newLine = 0x0A;

        //uint16 broj = 0;
        
	while(*(buff +  i) != 0x3E)//'>')
	{
		i++;
	}
	if(*(buff + i) ==  0x3E)//'>')
	{
		i++;
		if(*(buff + i) ==  0x23)//'#')
		{
			i++;
     			while(*(buff + i) !=  0x2C)//',')
			{
				cID[j]=  *(buff + i);
                                j++;
				i++;
			}
                        cID[j]= '\0';
		}
                else
                {
                  return 0;
                }
		if(*(buff + i) == 0x2C)//',')
		{
			i++;
                        if(*(buff + i) == 0x23)//'#')
                        {
                          i++;
                          j = 0;
                          while(*(buff + i) != 0x2C)//',')
                          {
                                cPackNum[j]=  *(buff + i);
				i++;
				j++;
                          }
                          cPackNum[j]= '\0';
                        }
                        
		}
                else
                {
                  return 0;
                }
		if(*(buff + i) == 0x2C)//',')
		{
			i++;
                        if(*(buff + i) == 0x23)//'#')
                        {
                          i++;
                          j = 0;
                          while(*(buff + i) != 0x2C)//',')
                          {
				cCmd[j]=  *(buff + i);
				i++;
				j++;
                          }
                          cCmd[j]= '\0';
                        }
		}
                else
                {
                  return 0;
                }
                if(*(buff + i) == 0x2C)//',')
		{
			i++;
                        if(*(buff + i) == 0x23)//'#')
                        {
                          i++;
                          j = 0;
                          while(*(buff + i) != 0x2C)//',')
                          {
				cTimeStamp[j]=  *(buff + i);
				i++;
				j++;
                          }
                          cTimeStamp[j]= '\0';
                        }
		}
                else
                {
                  return 0;
                }
                if(*(buff + i) == 0x2C)//',')
		{
			i++;
                        if(*(buff + i) == 0x23)//'#')
                        {
                          i++;
                          j = 0;
                          while(*(buff + i) != 0x3C)//'<')
                          {
				cCheckSum[j]=  *(buff + i);
				i++;
				j++;
                          }
                          cCheckSum[j]= '\0';
                        }
		}
                else
                {
                  return 0;
                }
		
	}
        else
        {
          return 0;
        }
	
        
        
        for(k = 0; k < 2; k++)
        {
          dID[k] = cID[1-k];
          
        }
        dID[2] = '\0';
        
        for(k = 0; k < 4; k++)
        {
          dCmd[k] = cCmd[3-k];
          
        }
        dCmd[4] = '\0';
        
        
        recPack.appData.devID = (uint16)atoi(dID);
	recPack.appData.packNum = (uint8)atoi(cPackNum);
        recPack.appData.data = (uint32)atol(dCmd);
        //recPack.appData.devID = (uint16)atoi(cID);
	//recPack.appData.packNum = (uint8)atoi(cPackNum);
       // recPack.appData.data = (uint32)atol(cCmd);
	recPack.timeStamp = (uint8)atoi(cTimeStamp);
	recPack.checkSum = (uint8)atoi(cCheckSum);
        
       
        
        
        
        cbData = (struct sData *)(&recPack);
        callBack(cbData);
/*
        _itoa(recPack.appData.data, (uint8 *)dCmd, 10);
        for(k = 0; k < 4; k++)
        {
          cCmd[k] = dCmd[3-k];
          
        }
        cCmd[4] = '\0';
       
       HalUARTWrite(HAL_UART_PORT_1, "ID", 2);
       HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
       //HalUARTWrite(HAL_UART_PORT_1, (uint8 *)cCmd, k+1);
       HalUARTWrite(HAL_UART_PORT_1, (uint8 *)cCmd, 5);
       HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
       HalUARTWrite(HAL_UART_PORT_1, "ID", 2);
       HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
*/
        return recPack.appData.devID;
        
        
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
     
        
	DLLPacket_t emPacket;

        
        emPacket.appData.devID = aData->devID;
        emPacket.appData.data = aData->data;
        emPacket.appData.packNum = aData->packNum;
        emPacket.timeStamp = 0;
	emPacket.checkSum = checksum(&emPacket);
        
       
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
        
        //HalLcdWriteString(uiTxBufferCC2530, 0);
        
     ////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
       //HalUARTWrite(HAL_UART_PORT_1, (uint8 *)uiTxBufferCC2530, 19);
     ////////////////////////////////////////////////////////////////////////////////////////////////////////////////   

        
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
