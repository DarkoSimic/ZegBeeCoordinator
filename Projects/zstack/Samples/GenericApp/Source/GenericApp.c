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
   
   
/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 coin = 1;
char prevData = 0;


uint8 keyPressSW4 = 1;

uint8 dataBuffer[10];
   
int counter = 0;


uint16 shortAddressOfEndDevice[MAX_NUMBER_OF_ENDDEVICES] = {0,0};
uint16 SAddr[MAX_NUMBER_OF_ENDDEVICES] = {0,0};

uint8 index = 0;

int brojac = 0;


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


/*********************************************************************
 * LOCAL VARIABLES
 */
byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.

devStates_t GenericApp_NwkState;

byte GenericApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GenericApp_DstAddr;
afAddrType_t GenericApp_DstAddr1;
afAddrType_t GenericApp_DstAddr2;
// Number of recieved messages
static uint16 rxMsgCount;

// Time interval between sending messages
static uint32 txMsgDelay = GENERICAPP_SEND_MSG_TIMEOUT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
static void GenericApp_HandleKeys( byte shift, byte keys );
static void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
static void GenericApp_SendTheMessage( void );

static void GenericApp_EndPointList(uint16);




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

  GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_DstAddr.addr.shortAddr = 0xFFFF; //0;// NLME_GetShortAddr();//1;
  
  GenericApp_DstAddr1.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_DstAddr1.endPoint = GENERICAPP_ENDPOINT;
  //GenericApp_DstAddr1.addr.shortAddr =  0x8740; //0;// NLME_GetShortAddr();//1;
  
  GenericApp_DstAddr2.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_DstAddr2.endPoint = GENERICAPP_ENDPOINT;
 // GenericApp_DstAddr2.addr.shortAddr = 0xE2CB;  //0;// NLME_GetShortAddr();//1;

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

    char shAddr[5];
    shAddr[4] = '\0';
    
    uint8 i;
    uint16 shortAdrress;
  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    

      
     /* shortAddressOfEndDevice[0] = MSGpkt->macSrcAddr;
      GenericApp_DstAddr1.addr.shortAddr = shortAddressOfEndDevice[0];
    
    
    
     //if(MSGpkt->macSrcAddr != shortAddressOfEndDevice[0] )
     {
           shortAddressOfEndDevice[1] = MSGpkt->macSrcAddr;
           GenericApp_DstAddr2.addr.shortAddr = 0x903C;//shortAddressOfEndDevice[1];
         
           
     }      
      */
    
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          
          GenericApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          
          break;

        case KEY_CHANGE:
           
           
           dstAddr.addrMode = AddrBroadcast;
           dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
           ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        GENERICAPP_PROFID,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        FALSE );
      
           
          // GenericApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
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
          
          /*
         shortAddressOfEndDevice[0] = MSGpkt->macSrcAddr;
         if(MSGpkt->macSrcAddr != shortAddressOfEndDevice[0])
         {
           shortAddressOfEndDevice[1] = MSGpkt->macSrcAddr;
         }
                
         
         
         GenericApp_DstAddr1.addr.shortAddr = shortAddressOfEndDevice[0];
         GenericApp_DstAddr2.addr.shortAddr = shortAddressOfEndDevice[1];
         */
         /*if(0==brojac)
         {
          shortAddressOfEndDevice[0] = MSGpkt->macSrcAddr;
          GenericApp_DstAddr1.addr.shortAddr = shortAddressOfEndDevice[0];
    
         }
    */
     //if(MSGpkt->macSrcAddr != shortAddressOfEndDevice[0] )
     /*
           shortAddressOfEndDevice[1] = MSGpkt->macSrcAddr;
           GenericApp_DstAddr2.addr.shortAddr = 0x903C;//shortAddressOfEndDevice[1];
         
           
     }    */  
      
          if(0==brojac)
          {
           shortAddressOfEndDevice[brojac] = MSGpkt->macSrcAddr;
           brojac++;
          }
          else if(shortAddressOfEndDevice[brojac-1]!=MSGpkt->macSrcAddr)
          {
            shortAddressOfEndDevice[brojac] = MSGpkt->macSrcAddr;
            brojac++;
          }
          
          GenericApp_DstAddr1.addr.shortAddr = shortAddressOfEndDevice[0];
          if(1<=brojac)
          GenericApp_DstAddr2.addr.shortAddr = shortAddressOfEndDevice[1];
          
          //uartSend(GenericApp_DstAddr1.addr.shortAddr);
          //uartSend(GenericApp_DstAddr2.addr.shortAddr);
          

          //SAddr[0]=shortAddressOfEndDevice[0];
          //SAddr[1]=shortAddressOfEndDevice[1];
          SAddr[0]=GenericApp_DstAddr1.addr.shortAddr;
          SAddr[1]=GenericApp_DstAddr2.addr.shortAddr;
          
         // HalLcdWriteString("--------------------##########################------------",0);    
     // HalLcdWriteString("Short address1:",0);
     
      for(i = 0;i<4;i++)
      {
        shAddr[3-i] =  SAddr[1] % 16  + '0';
        SAddr[1] /= 16;
      }
      
    //  HalLcdWriteString(shAddr,0);
    //  HalLcdWriteString("--------------------------------",0);
      
    //  HalLcdWriteString("Short address0:",0);
     
      for(i = 0;i<4;i++)
      {
        shAddr[3-i] =  SAddr[0] % 16  + '0';
        SAddr[0] /= 16;
      }
      
     // HalLcdWriteString(shAddr,0);
     // HalLcdWriteString("--------------------------------",0);
          
          
          
         // HalLcdWriteString("AF_INCOMING_MSG_CMD:",0);
          
         GenericApp_MessageMSGCB( MSGpkt );
         
          
         
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
      
    // if(MSGpkt->macSrcAddr != shortAddressOfEndDevice[0])
  //   {
    //       shortAddressOfEndDevice[1] = MSGpkt->macSrcAddr;
   //  }       
      
     
     // GenericApp_DstAddr2.addr.shortAddr = shortAddressOfEndDevice[1];   
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in GenericApp_Init()).
  if ( events & GENERICAPP_SEND_MSG_EVT )
  {
    
    
    
    if(keyPressSW4)
    {
     /*
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;
      ZDP_MatchDescReq( &dstAddr,NWK_BROADCAST_SHORTADDR_DEVALL,
                        GENERICAPP_PROFID,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        FALSE );
     
   
     */ 
      
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = NLME_GetShortAddr();//0x0000; // Coordinator
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
     
      
    // Send "the" message
    GenericApp_SendTheMessage();
     
    
     //Setup to send message again
     osal_start_timerEx( GenericApp_TaskID,
                         GENERICAPP_SEND_MSG_EVT,
                         100); //txMsgDelay );
    
    // osal_set_event(GenericApp_TaskID,GENERICAPP_SEND_MSG_EVT);
    
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
  char shAddr[5];
  uint16 SAddr[2];
    shAddr[4] = '\0';
    uint8 i = 0;
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
         HalLcdWriteString("End Device bind",0);
         SAddr[1] = inMsg->macSrcAddr;
          for(i = 0;i<4;i++)
      {
        shAddr[3-i] =  SAddr[1] % 16  + '0';
        SAddr[1] /= 16;
      }
      
      HalLcdWriteString(shAddr,0);
      
         
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
        SAddr[1] = inMsg->macSrcAddr;
          for(i = 0;i<4;i++)
      {
        shAddr[3-i] =  SAddr[1] % 16  + '0';
        SAddr[1] /= 16;
      }
      
      HalLcdWriteString(shAddr,0);
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
 * @fn      GenericApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void GenericApp_HandleKeys( uint8 shift, uint8 keys )
{
  zAddrType_t dstAddr;

  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
#if defined( SWITCH1_BIND )
      // We can use SW1 to simulate SW2 for devices that only have one switch,
      keys |= HAL_KEY_SW_2;
#elif defined( SWITCH1_MATCH )
      // or use SW1 to simulate SW4 for devices that only have one switch
      keys |= HAL_KEY_SW_4;
#else
      // Normally, SW1 changes the rate that messages are sent
      if ( txMsgDelay > 100 )
      {
        // Cut the message TX delay in half
        txMsgDelay /= 2;
      }
      else
      {
        // Reset to the default
        txMsgDelay = GENERICAPP_SEND_MSG_TIMEOUT;
      }
#endif
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request for the mandatory endpoint
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = 0x0000; // Coordinator
      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                            GenericApp_epDesc.endPoint,
                            GENERICAPP_PROFID,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      // Initiate a Match Description Request (Service Discovery)
      dstAddr.addrMode = AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        GENERICAPP_PROFID,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GenericApp_ClusterList,
                        FALSE );
    }
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
   /*
  uint8 i;
 
 
  
  
  
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      rxMsgCount += 1;  // Count this message
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_BLINK );  // Blink an LED

/*
#if defined( LCD_SUPPORTED )
      HalLcdWriteString( (char*)pkt->cmd.Data, HAL_LCD_LINE_1 );
      HalLcdWriteStringValue( "Rcvd:", rxMsgCount, 10, HAL_LCD_LINE_2 );
#elif defined( WIN32 )
      //WPRINTSTR( pkt->cmd.Data );
       
      while(*(pkt->cmd.Data + i) != '\0')
      { 
        uartSend(*(pkt->cmd.Data + i++));
      }
#endif
      break;*//*
      if(prevData != *pkt->cmd.Data)
      {
        coin = 1;
      }
      
      if(*pkt->cmd.Data== '0')
      {
      
       
       if(coin)
       {
        HalLcdWriteString ( "Door closed", 0);
        coin=0;
       }
       }
      else
      {
       if(coin) 
       {
         HalLcdWriteString (  "Door opened", 0);
         coin=0;
       }
      }
      prevData = *pkt->cmd.Data;
     /* HalLcdWriteString("-------------",0);
      for(i = 0;i < pkt->cmd.DataLength;i++)
      {
        uartSend(*(pkt->cmd.Data + i));
      }
      HalLcdWriteString("",0);
      HalLcdWriteString("-------------",0);*//*
      //HalLcdWriteString("Podatak je primljen.",0);
      break;
      
  default:
   // HalLcdWriteString("Podatak nije primljen.",0);
        
    break;
      
  }
  
  
  */
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  uint8 i;
  uint8 base = 1;
  char shAddr[5];
  shAddr[4]= '\0';
  uint16 hexStr = 0;
  
  
  
  
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      rxMsgCount += 1;  // Count this message
     // HalLedSet ( HAL_LED_4, HAL_LED_MODE_BLINK );  // Blink an LED

/*
#if defined( LCD_SUPPORTED )
      HalLcdWriteString( (char*)pkt->cmd.Data, HAL_LCD_LINE_1 );
      HalLcdWriteStringValue( "Rcvd:", rxMsgCount, 10, HAL_LCD_LINE_2 );
#elif defined( WIN32 )
      //WPRINTSTR( pkt->cmd.Data );
       
#endif
      break;*/
      
      HalLcdWriteString("--------------------------------",0);
      HalLcdWriteString("Received data:",0);
      
      for(i = 0;i < pkt->cmd.DataLength;i++)
      {
        uartSend(*(pkt->cmd.Data + i));
      }
      
      HalLcdWriteString("",0);
      HalLcdWriteString("--------------------------------",0);
      //HalLcdWriteString("Short address:",0);
     
      for(i = 0;i<4;i++)
      {
        shAddr[3-i] =  shortAddressOfEndDevice[1] % 16  + '0';
        shortAddressOfEndDevice[1] /= 16;
      }
      
      //HalLcdWriteString(shAddr,0);
      //HalLcdWriteString("--------------------------------",0);
      
      break;
      
  default:
    HalLcdWriteString("Podatak nije primljen.",0);
        
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
  uint16 shortAdrress;
  char shAddr[4];
  uint8 i;
  
 
  char theMessageData[] = "You are Coord from EndDevice1";
  char theMessageData1[] = "You are EndDevice1";
  char theMessageData2[] = "You are EndDevice2";
  char doorOpened[] = {'1'};
  char doorClosed[] = {'0'};
  char theOpticalData[5];
  uint16 optDat;
  
  
  optDat = HalAdcRead(HAL_ADC_CHANNEL_1,HAL_ADC_RESOLUTION_12);

  
     // shortAdrress = NLME_GetShortAddr();
      
      for(i = 0;i<4;i++)
      {
        theOpticalData[3-i] =  optDat % 10  + '0';
        optDat /= 10;
      }
      
      theOpticalData[4] = '\0';
       HalLcdWriteString("###################################",0);
  

        HalLcdWriteString(theOpticalData,0);
        HalLcdWriteString("###################################",0);
  

  //&GenericApp_DstAddr
  /*
  GenericApp_DstAddr.addr.shortAddr = 0x0000;
  
  if ( !magneticSwitch_DoorDetection() )
  {
    // Successfully requested to be sent.
    //HalLcdWriteString("Podatak je poslan.",0);
    AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       2,                                                       //(byte)osal_strlen( theMessageData ) + 1,
                       (byte *)&doorOpened,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
    
    HalLcdWriteString("Vrata su otvorena.",0);
    
  }
  else
  {
    
    // Error occurred in request to send.
    // HalLcdWriteString("Podatak nije poslan.",0);
    AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       2,                                                       //(byte)osal_strlen( theMessageData ) + 1,
                       (byte *)&doorClosed,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
  
    HalLcdWriteString("Vrata su zatvorena.",0);
    
  }
  */
  
  GenericApp_DstAddr.addr.shortAddr = 0x0000;
  
  
  
    if ( AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                         GENERICAPP_CLUSTERID,
                         (byte)osal_strlen( theOpticalData ) + 1,
                         (byte *)&theOpticalData,
                         &GenericApp_TransID,
                         AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
      // Successfully requested to be sent.
      HalLcdWriteString("Podatak je poslan.",0);
      //HalLcdWriteString(theMessageData,0);
    }
    else
    {
      // Error occurred in request to send.
      HalLcdWriteString("Podatak nije poslan.",0);
    }
 
 
  

  /*
  if(GenericApp_DstAddr1.addr.shortAddr != 0 )
  {
    if ( AF_DataRequest( &GenericApp_DstAddr1, &GenericApp_epDesc,
                         GENERICAPP_CLUSTERID,
                         (byte)osal_strlen( theMessageData1 ) + 1,
                         (byte *)&theMessageData1,
                         &GenericApp_TransID,
                         AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
      // Successfully requested to be sent.
      HalLcdWriteString("Podatak je poslan.",0);
    }
    else
    {
      // Error occurred in request to send.
      HalLcdWriteString("Podatak nije poslan.",0);
    }
    
  }
  
  if(GenericApp_DstAddr2.addr.shortAddr != 0)
  {
    if ( AF_DataRequest( &GenericApp_DstAddr2, &GenericApp_epDesc,
                         GENERICAPP_CLUSTERID,
                         (byte)osal_strlen( theMessageData2 ) + 1,
                         (byte *)&theMessageData2,
                         &GenericApp_TransID,
                         AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
      // Successfully requested to be sent.
      HalLcdWriteString("Podatak je poslan.",0);
    }
    else
    {
      // Error occurred in request to send.
      HalLcdWriteString("Podatak nije poslan.",0);
    }
  } 
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

/*********************************************************************
 */
void magneticSwitchInit(void)
{
 // Set GPIO function for P1_3  
 P1SEL &= 0xFC;
 
 // Set inputs on P0_0
 P1DIR &= 0xFC;
 
 // Set pulldown for port 0 pins
 P2INP |= 0x40;
  
}

/*********************************************************************
 * @fn      magneticSwitch_DoorDetection()
 *
 * @brief   Detects if door are closed/opened
 *
 * @param   none
 *
 * @return  1 if closed, 0 if opened
 */
uint8 magneticSwitch_DoorDetection()
{
    
 if(TRUE == DOOR_CLOSED_DETECTION) 
        { 
          return 0;
 }
        else
        {
          return 1;
        }
 
}  



/*********************************************************************
 */