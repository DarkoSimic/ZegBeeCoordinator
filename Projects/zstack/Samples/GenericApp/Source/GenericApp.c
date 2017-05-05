/******************************************************************************
  Filename:       GenericApp.c
  Revised:        $Date: 2014-09-07 13:36:30 -0700 (Sun, 07 Sep 2014) $
  Revision:       $Revision: 40046 $

  Description:    Generic Application (no Profile).


  Copyright 2004-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
******************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful - it is
  intended to be a simple example of an application's structure.

  This application periodically sends a "Hello World" message to
  another "Generic" application (see 'txMsgDelay'). The application
  will also receive "Hello World" packets.

  This application doesn't have a profile, so it handles everything
  directly - by itself.

  Key control:
    SW1:  changes the delay between TX packets
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "GenericApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 ) || defined( ZBIT )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "hal_adc.h"

/* RTOS */
#if defined( IAR_ARMCM3_LM )
#include "RTOS_App.h"
#endif







////////////////////////////////////////////////////////////////////////////////////////////
//#include "includes/hal_driver.h"
#include "hal_drivers.h"

#include "includes/dll.h"
#include "includes/look_up_table.h"

#include "includes/circular_buffer.h"

#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************************
 * MACROS
 */


#define GENERICAPP_ENDPOINT           10

#define GENERICAPP_PROFID             0x0F04
#define GENERICAPP_DEVICEID           0x0001
#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0

#define GENERICAPP_MAX_CLUSTERS       1
#define GENERICAPP_CLUSTERID          1


#define MAX_NUMBER_OF_ENDDEVICES      10


// magnetic switch macros
#define DOOR_CLOSED_DETECTION P1_2
#define TRUE 1
#define CLOSED 1
#define OPENED 0
// magnetic switch macros end
   

#define HAL_UART_ISR 2

#define HAL_UART_MSECS_TO_TICKS    33

#if !defined HAL_UART_ISR_IDLE
#define HAL_UART_ISR_IDLE         (6 * HAL_UART_MSECS_TO_TICKS)
#endif

#if !defined HAL_UART_ISR_RX_MAX
#define HAL_UART_ISR_RX_MAX        128
#endif
#if !defined HAL_UART_ISR_TX_MAX
#define HAL_UART_ISR_TX_MAX        HAL_UART_ISR_RX_MAX
#endif
   
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
////////////////////////////////////////////////////////////////////////////////////////////
//CircularBuffer_t cMain;
uint8 dataRdy;
LookUpTable_t lutData[20];
uint8 *rxBuffer;
halUARTCfg_t uartConfig;
////////////////////////////////////////////////////////////////////////////////////////////

uint8 coin = 1;
char prevData = 0;


uint8 keyPressSW4 = 1;

uint8 dataBuffer[10];
   
uint8 *buff;

int counter = 0;


uint16 shortAddressOfEndDevice[MAX_NUMBER_OF_ENDDEVICES] = {0,0};
uint16 SAddr[MAX_NUMBER_OF_ENDDEVICES] = {0,0};

uint8 index = 0;

uint8 brojac = 0;


// This list should be filled with Application specific Cluster IDs.
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in GenericApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t GenericApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

  extern void uartInit(void);
  extern void uartSend(char);
 // void LazoInit(void);

/*********************************************************************
 * LOCAL VARIABLES
 */
byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.

devStates_t GenericApp_NwkState;

byte GenericApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GenericApp_DstAddr;
afAddrType_t GenericApp_DstAddress;//GenericApp_DstAddress
//////////////////////////////////////////////////////////////////////////////  Mozda izbrisati ovo

//afAddrType_t GenericApp_DstAddress[MAX_NUMBER_OF_ENDDEVICES] = {0,0};

// Number of recieved messages
static uint16 rxMsgCount;

// Time interval between sending messages
static uint32 txMsgDelay = GENERICAPP_SEND_MSG_TIMEOUT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
//static void GenericApp_HandleKeys( byte shift, byte keys );
static void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
static void GenericApp_SendTheMessage( void );

//static void GenericApp_EndPointList(uint16);

#if defined( IAR_ARMCM3_LM )
static void GenericApp_ProcessRtosMessage( void );
#endif

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */


void GenericApp_Init( uint8 task_id )
{
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;
  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  //GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  //GenericApp_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  //GenericApp_DstAddr.addr.shortAddr = 0xFFFF; //0;// NLME_GetShortAddr();//1;*/

  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( GenericApp_TaskID );

  // Update the display
#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "GenericApp", HAL_LCD_LINE_1 );
#endif

  ZDO_RegisterForZDOMsg( GenericApp_TaskID, End_Device_Bind_rsp );
  ZDO_RegisterForZDOMsg( GenericApp_TaskID, Match_Desc_rsp );

#if defined( IAR_ARMCM3_LM )
  // Register this task with RTOS task initiator
  RTOS_RegisterApp( task_id, GENERICAPP_RTOS_MSG_EVT );
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  //halInitDriver();
    dllInit();
    lookUpInit();
   // circularInit(&cMain);
    HalUARTOpen(HAL_UART_PORT_1, &uartConfig);
    //HalUARTSuspend();
     //halGPIOOutput(HW_PORT_D, 0x03, 0);

    /*circularPut(&cMain, 'T');
    circularPut(&cMain, 'E');
    circularPut(&cMain, 'S');
    circularPut(&cMain, 'T');
    */
   // halUARTWrite(PC, &cMain, 0);
  
////////////////////////////////////////////////////////////////////////////////////////////  
    
  
}
/*********************************************************************
 * @fn      GenericApp_EndPointList
 *
 * @brief   Get's short address of end device and put in the end device 
            array
 *
 * @param   shAddr  - Short Address of connected End Device
 * 
 * @return  none
 */
/*
static void GenericApp_EndPointList(uint16 shAddr)
{
 
  if(index > MAX_NUMBER_OF_ENDDEVICES)
  {
    HalLcdWriteString("Max number of end devices overflow.",0);
  }
  else
  {
    shortAddressOfEndDevice[index];
    index++;
  }
  
  
}
*/
/*********************************************************************
 * @fn      GenericApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 GenericApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;
  zAddrType_t dstAddr;

    //uint8 flag = 0;
    //uint8 i;


  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    
   
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          
          GenericApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          
          break;

        case KEY_CHANGE:                                                        /////////////////////////////////////////////// case KEY_CHANGE: prazno ????????????????????????????

          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
         
          afDataConfirm = (afDataConfirm_t *)MSGpkt;

          sentEP = afDataConfirm->endpoint;
          (void)sentEP;  // This info not used now
          sentTransID = afDataConfirm->transID;
          (void)sentTransID;  // This info not used now

          sentStatus = afDataConfirm->hdr.status;
          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////          

          
         GenericApp_MessageMSGCB( MSGpkt );
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////        
          break;

        case ZDO_STATE_CHANGE:
           
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( (GenericApp_NwkState == DEV_ZB_COORD) ||
               (GenericApp_NwkState == DEV_ROUTER) ||
               (GenericApp_NwkState == DEV_END_DEVICE) )
          {
            
           
            // Start sending "the" message in a regular interval.
            osal_start_timerEx( GenericApp_TaskID,
                                GENERICAPP_SEND_MSG_EVT,
                                txMsgDelay );
          }
                      
          break;

        default:
           
          
            osal_set_event(GenericApp_TaskID,GENERICAPP_SEND_MSG_EVT);
           break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
        
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in GenericApp_Init()).
  
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
  if ( events & RX_PROCCESS_EVENT )
  {
    
    uint8 i;
    //char *id;
    uint8 newLine = 0x0A;
    uint16 ID;
    uint32 Data; 
    uint32 broj;
    char id[3];
    char dat[5];
    char di[3];
    char tad[5];
    char theMessage[25];
    //char doorOpened[] = {'S','E','N','D','\0'};

    HalUARTRead(HAL_UART_PORT_1, uartConfig.rx.pBuffer, 128);
    
    rxBuffer = uartConfig.rx.pBuffer;
    
    ID = processFrame(rxBuffer);
    Data = getCmd(ID);
    
    //itoa(ID, id, 10);
    
     //id = (char*)&ID;
     //id[0]+=0x30;
     //id[1]+=0x30;
    
     _itoa(ID, (uint8 *)id, 10);
     id[2] = '\0';
     for(i = 0; i < 2; i++)
     {
       di[i] = id[1-i];
     }
     di[2] = '\0';
     
     
     
     _itoa(Data, (uint8 *)dat, 10);
     dat[4] = '\0';
     for(i = 0; i < 4; i++)
     {
       tad[i] = dat[3-i];
     }
     tad[4] = '\0';
   /*  
     HalUARTWrite(HAL_UART_PORT_1, "----------Borislav  ID-------", 29);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     HalUARTWrite(HAL_UART_PORT_1, (uint8 *)di, 3);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     HalUARTWrite(HAL_UART_PORT_1, (uint8 *)tad, 5);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     HalUARTWrite(HAL_UART_PORT_1, "----------Borislav  ID-------", 29);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     */
       
  //   id = (char*)&Data;
     //id = (char*)&broj;
     /*id[0]+=0x30;
     id[1]+=0x30;
     id[2]+=0x30;
     id[3]+=0x30;
     id[4] = '\0';
     HalUARTWrite(HAL_UART_PORT_1, "----------Borislav  DATA-----", 29);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     HalUARTWrite(HAL_UART_PORT_1, (uint8 *)id, 5);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
     HalUARTWrite(HAL_UART_PORT_1, "----------Borislav  DATA-----", 29);
     HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
 */
       
     broj = getSignalAddress(ID);
     i = 0;
     while(broj)
     {
      id[i] = (broj % 256);// + '0';
      broj/=256;
      i++;
     }
     id[i] = '0';
       
     //_itoa(getSignalAddress(ID))
    //theMessage[0] = 'T';
    //theMessage[1] = '2';
    theMessage[0] = id[0];
    theMessage[1] = id[1];
    theMessage[2] = tad[0];
    theMessage[3] = tad[1];
    theMessage[4] = tad[2];
    theMessage[5] = tad[3]; 
    theMessage[6] = '\0'; 
    
    HalUARTWrite(HAL_UART_PORT_1, (uint8 *)theMessage, 7);
    HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);

     
     for(i = 0; i < 20; i++)
  {
    
    if(getSignalAddress(ID) == lutData[i].devID)
    //if(0 != lutData[i].devID)
    {
      
    GenericApp_DstAddress.addrMode = (afAddrMode_t)Addr16Bit;
    GenericApp_DstAddress.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_DstAddress.addr.shortAddr = lutData[i].devAddress;
    
    
    
    AF_DataRequest( &GenericApp_DstAddress, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                      (byte)osal_strlen( theMessage ) + 1,
                       //5,    
                      (byte *)&theMessage,
                       //(byte *)&doorOpened,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );

      }
  }

    HalUARTWrite(HAL_UART_PORT_1, (uint8 *)rxBuffer, 29);
    HalUARTWrite(HAL_UART_PORT_1, &newLine, 1);
    
/*
   
    theMessage[GenericApp_DstAddress.addr.shortAddr][0] = (char)((ID >> 8) & 0x00FF);
    theMessage[GenericApp_DstAddress.addr.shortAddr][1] = (char)(ID & 0x00FF);
    theMessage[GenericApp_DstAddress.addr.shortAddr][2] = (char)((Data >> 24) & 0x000000FF);
    theMessage[GenericApp_DstAddress.addr.shortAddr][3] = (char)((Data >> 16) & 0x000000FF); 
    theMessage[GenericApp_DstAddress.addr.shortAddr][4] = (char)((Data >> 8) & 0x000000FF);
    theMessage[GenericApp_DstAddress.addr.shortAddr][5] = (char)(Data & 0x000000FF); 
 
    GenericApp_DstAddress.addrMode = (afAddrMode_t)Addr16Bit;
    GenericApp_DstAddress.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_DstAddress.addr.shortAddr = lookForAddr(ID);
    
    if ( AF_DataRequest( &GenericApp_DstAddress, &GenericApp_epDesc,
                           GENERICAPP_CLUSTERID,
                           //(byte)osal_strlen( theMessage[GenericApp_DstAddress.addr.shortAddr] ) + 1,
                           (byte)osal_strlen( doorOpened ) + 1,
                           //(byte *)&theMessage[GenericApp_DstAddress.addr.shortAddr],
                           (byte *)&doorOpened,
                           &GenericApp_TransID,
                           AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
      
      {
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_BLINK );  // Blink an LED
      }

    */
}
    
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
    
  if ( events & GENERICAPP_SEND_MSG_EVT )
  {
    if(keyPressSW4)
    {
      
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = 0x0000;//NLME_GetShortAddr();//0x0000; // Coordinator
      ZDP_EndDeviceBindReq( &dstAddr, 0x0000, //NLME_GetShortAddr(),
                            GenericApp_epDesc.endPoint,
                            GENERICAPP_PROFID,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            FALSE );
      
      keyPressSW4 = 0;
      
      osal_set_event(GenericApp_TaskID,GENERICAPP_SEND_MSG_EVT);
      
    }
    else
    {

    // if((char)*buff!='\0') //zakomentarisao
     {
       
       GenericApp_SendTheMessage();
        //HalLcdWriteString("Sile before send",0);
     }

     //Setup to send message again
     osal_start_timerEx( GenericApp_TaskID,
                         GENERICAPP_SEND_MSG_EVT,
                         1000); //txMsgDelay );
    
    }
    // return unprocessed events
    return (events ^ GENERICAPP_SEND_MSG_EVT);
  }

#if defined( IAR_ARMCM3_LM )
  // Receive a message from the RTOS queue
  if ( events & GENERICAPP_RTOS_MSG_EVT )
  {
    // Process message from RTOS queue
    GenericApp_ProcessRtosMessage();

    // return unprocessed events
    return (events ^ GENERICAPP_RTOS_MSG_EVT);
  }
#endif

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      GenericApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
static void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{

  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
      
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined( BLINK_LEDS )
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        HalLcdWriteString("End Device Match Desc",0);
        
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
              if ( pRsp->status == ZSuccess && pRsp->cnt )
              {
                
            GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            GenericApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            GenericApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
static void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  //uint8 i;
  //////////////////////////////////////////////////////////////////////////////
  Data_t Data;
  uint16 addr;
  //char * id;
  //////////////////////////////////////////////////////////////////////////////
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      
      {
      rxMsgCount += 1;  // Count this message
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_BLINK );  // Blink an LED


 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Data.devID = *(pkt->cmd.Data);
      Data.devID =getIDFromPkt(&pkt->cmd);

      Data.data =getDataFromPkt(&pkt->cmd); 
      //id = (char*)&Data.data;
      addr = pkt->macSrcAddr;
      
      updateLookUpTable(Data, addr);

      
 ///////////////////////////////////////////////////////////////////////////////
 
     
      }
      break; //break first switch
      
  default:
    
   //HalLcdWriteString("Podatak nije primljen.",0);
        
    break;

  }


}

/*********************************************************************
 * @fn      GenericApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none

 *
 * @return  none
 */
static void GenericApp_SendTheMessage( void )
{
  uint8 i;
 // char * id;
  //uint32 data;
  //BS
  //char doorOpened[] = {'S','E','N','D','\0'};
  
   // Successfully requested to be sent.
    //HalLcdWriteString("Podatak je poslan.",0);
  /* 
  GenericApp_DstAddress.addrMode = (afAddrMode_t)Addr16Bit;
    GenericApp_DstAddress.endPoint = GENERICAPP_ENDPOINT;
    GenericApp_DstAddress.addr.shortAddr = lutData[0].devAddress;
*/
          //GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  //GenericApp_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  //GenericApp_DstAddr.addr.shortAddr = 0xFFFF; //0;// NLME_GetShortAddr();//1;*/
    
    //HalLcdWriteString("Provjera slanja",0);
  
    //HalUARTWrite(HAL_UART_PORT_1, "Sinisa", 7);
    
   for(i = 0; i < 20; i++)
  {
    //HalLcdWriteString("Sinisa-------------petlja-----",0);
    if(0 != lutData[i].devID)
    {
      sendDataToPC(&lutData[i]);
      
     //GenericApp_DstAddr.addr.shortAddr = lutData[0].devAddress;
      //GenericApp_DstAddr.addr.shortAddr = 0xFFFF; 
      /*
      GenericApp_DstAddress.addr.shortAddr = lookForAddr(lutData[i].devID);
    
    AF_DataRequest( &GenericApp_DstAddress, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       5,                                                       
                       (byte *)&doorOpened,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
      */
      //id = (char*)&lutData[i].devID;
     //id[2] = '\0';
        //HalLcdWriteString("-----------------------------Borislav  Send----------------------",0);
        //HalLcdWriteString(id,0);
        //HalLcdWriteString("-----------------------------Borislav  Send----------------------",0);
      //HalLcdWriteString("Sinisa-------------petlja-----",0);
    }
  } 
   //HalLcdWriteString("Sinisa----------------------",0); 
/*
  
  uint8 i;
  char num = '0';

  char theMessageData[MAX_NUMBER_OF_ENDDEVICES][25] = {"You are EndDevice1111","You are EndDevice2222"};
  //char *msg1;
  //char *msg2;
  //char *msg[MAX_NUMBER_OF_ENDDEVICES];
  //HalLcdRead(msg[0],2);
  //HalLcdRead(msg[1],2);
  
  //HalLcdWriteString("Sile in send",0);
  */
  /*
  char theMessage[MAX_NUMBER_OF_ENDDEVICES][25] = {"0000", "FFFF"};
  if('1' == (char)*buff)
  {
    theMessage[0][0] = (char)buff[0];
    theMessage[0][1] = (char)buff[1];
    theMessage[0][2] = '\0';
    theMessage[1][0] = '0';
    theMessage[1][1] = '0';
    theMessage[1][2] = '\0';
   // HalLcdWriteString("Buff 1",0);
  }
  else
  {
    theMessage[1][0] = (char)buff[0];
    theMessage[1][1] = (char)buff[1];
    //theMessage[1][0] = (char)*buff;
    //theMessage[1][1] = (char)*(buff+1);
    theMessage[1][2] = '\0';
    theMessage[0][0] = '0';
    theMessage[0][1] = '0';
    theMessage[0][2] = '\0';
    //HalLcdWriteString("Buff 2",0);
  }
  
  for(i = 0; i<brojac; i++)
  {
   */ 
   /*  HalLcdWriteString("Ulazak u petlju",0);
     num+=i;
     uartSend(num);
     HalLcdWriteString("",0);
     */
  /*  if(GenericApp_DstAddress[i].addr.shortAddr != 0 )
                         
    {
      if ( AF_DataRequest( &GenericApp_DstAddress[i], &GenericApp_epDesc,
                         GENERICAPP_CLUSTERID,
                         //(byte)osal_strlen( theMessageData[i] ) + 1,
                         //(byte *)&theMessageData[i],
                         (byte)osal_strlen( theMessage[i] ) + 1,
                         (byte *)&theMessage[i],
                         //(byte *)theMessageData[i],
                         //(byte)osal_strlen(msg[i]) + 1,
                         //(byte *)msg[i],
                         &GenericApp_TransID,
                         AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
      {
      // Successfully requested to be sent.
        HalLcdWriteString("Podatak je poslan.",0);
  */
        /*HalLcdWriteString("###############################",0);
     
      
          for(i = 0;i <3;i++)
          {
            uartSend(*(buff + i));
          }
      
          HalLcdWriteString("",0);
          HalLcdWriteString("--------------------------------",0);
        */
  /*    }
      else
      {
      // Error occurred in request to send.
        HalLcdWriteString("Podatak nije poslan.",0);
      }
    
    }
  }

  buff[0] = '\0';
  */
}

#if defined( IAR_ARMCM3_LM )
/*********************************************************************
 * @fn      GenericApp_ProcessRtosMessage
 *
 * @brief   Receive message from RTOS queue, send response back.
 *
 * @param   none
 *
 * @return  none
 */
static void GenericApp_ProcessRtosMessage( void )
{
  osalQueue_t inMsg;

  if ( osal_queue_receive( OsalQueue, &inMsg, 0 ) == pdPASS )
  {
    uint8 cmndId = inMsg.cmnd;
    uint32 counter = osal_build_uint32( inMsg.cbuf, 4 );

    switch ( cmndId )
    {
      case CMD_INCR:
        counter += 1;  /* Increment the incoming counter */
                       /* Intentionally fall through next case */

      case CMD_ECHO:
      {
        userQueue_t outMsg;

        outMsg.resp = RSP_CODE | cmndId;  /* Response ID */
        osal_buffer_uint32( outMsg.rbuf, counter );    /* Increment counter */
        osal_queue_send( UserQueue1, &outMsg, 0 );  /* Send back to UserTask */
        break;
      }

      default:
        break;  /* Ignore unknown command */
    }
  }
}
#endif

/**********************************************************************/

/*********************************************************************/