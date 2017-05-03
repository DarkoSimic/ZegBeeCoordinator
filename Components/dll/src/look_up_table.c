#include "includes/look_up_table.h"
#include "includes/dll.h"

#include "hal_lcd.h"
//*****************************************************************************
//
// Extern variables
//
// treba promijeniti naziv ova dva bafera u circularBuffer.h
//
//*****************************************************************************
Data_t app_packet;

//static LookUpTable_t lutData[20];
extern LookUpTable_t lutData[20];
//extern uint8 dataRdy;
//*****************************************************************************
//
// Local variables
//
//*****************************************************************************



//*****************************************************************************
//
// Function definition
//
//*****************************************************************************

void lookUpInit(void)
{
	CallBackRegister(updateCmd);
}

void updateCmd(Data_t *pData)
{
  int i = 0;
  while(lutData[i].devID != 0 && lutData[i].devID != pData->devID && i<20)
  {
    i++;
  }
  if(lutData[i].devID == 0)
  {
      lutData[i].devID = pData->devID;
      //lutData[i].devAddress = devAddress;
  }
  if(i < 20)
  {
      lutData[i].data = pData->data;
      //dataRdy = 1;
  }
}
void updateLookUpTable(Data_t pData, uint16 devAddress)
{
  int i = 0;
 // char *id;
  //uint32 idde;
  while(lutData[i].devID != 0 && lutData[i].devID != pData.devID && i<20)
  {
    i++;
  }
  if(lutData[i].devID == 0)
  {
      lutData[i].devID = (uint16)pData.devID;
      //idde =  lutData[i].devID;
      //idde =  lutData[i].data;
      //id = (char*)&idde;
      //id[2] = '\0';
      lutData[i].devAddress = devAddress;
  }
  if(i < 20)
  {
      lutData[i].data = pData.data;
      //idde =  lutData[i].data;
      //id = (char*)&idde;
      //id[4] = '\0';
      //dataRdy = 1;
  }
    
       // HalLcdWriteString("-----------------------------Borislav----LUT------------------",0);
       // HalLcdWriteString(id,0);
       // HalLcdWriteString("-----------------------------Borislav----LUT------------------",0);
}

uint16 lookForAddr(uint16 devID)
{
  int i = 0;
  uint16 devAddress = 0;
  while(lutData[i].devID != 0 && lutData[i].devID != devID)
  {
    i++;
  }
  if(lutData[i].devID == devID)
  {
    devAddress = lutData[i].devAddress;
  }
  return devAddress;
}

int16 getIDFromPkt(afMSGCommandFormat_t *cmd)
{
	/*uint8 i = 0;
	uint16 devID = 0;
	while(*(cmd->Data + i) != START_DELIMITER && (*(cmd->Data + i) != '\0'))
	{
		i++;
	}
	if (*(cmd->Data + i) == START_DELIMITER)
	{
		devID = (*(cmd->Data + i))<<8;
		devID +=*(cmd->Data + i + 1);
	}
	return devID;*/
        uint16 devID = 0;
        //char * id;
        
        //devID = (uint16)(*(cmd->Data+1))<<8;
	//devID +=(uint16)*(cmd->Data );
        devID = (uint16)(*(cmd->Data + 1))<<8;
	devID +=(uint16)*(cmd->Data);
        //id = (char*)&devID;
        //HalLcdWriteString("-----------------------------Borislav----------------------",0);
        //HalLcdWriteString(id,0);
        //HalLcdWriteString("-----------------------------Borislav----------------------",0);
        return devID;
}

int32 getDataFromPkt(afMSGCommandFormat_t *cmd)
{
	/*uint8 i = 0;
        uint8 j = 0;
	uint32 data = 0;
	while(*(cmd->Data + i) != START_DELIMITER && (*(cmd->Data + i) != '\0'))
	{
		i++;
	}
	if (*(cmd->Data + i) == START_DELIMITER)
	{
		for(j = 0; j < 3; j++)
                {
                  data +=(*(cmd->Data + i + j + 2))<<8;
                }
                j++;
                data +=(*(cmd->Data + i + j + 2));
	}
	return data;*/
        //char * id;
        uint8 i = 0;
        uint32 data = 0x00000000;
        
        for(i = 0; i < cmd->DataLength - 2; i++)
        {
          data +=(uint32)(*(cmd->Data + cmd->DataLength - 1 - i))<<(8*(cmd->DataLength - 3 - i));
          
        }
        /*
        data =(uint32)(*(cmd->Data + 5))<<24;
        data +=(uint32)(*(cmd->Data + 4))<<16;
        data +=(uint32)(*(cmd->Data + 3))<<8;        
        data +=(uint32)(*(cmd->Data + 2));
        */
        //id = (char*)&data;
        //HalLcdWriteString("-----------------------------GetData----------------------",0);
        //HalLcdWriteString(id,0);
        //HalLcdWriteString("-----------------------------GetData----------------------",0);
        return data;
}

int32 getCmd(uint16 devID)
{
  int i = 0;
  uint32 data = 1500;
  while(lutData[i].devID != devID && i < 20) //
	//while(DataID[i].devID != 0 && DataID[i].devID != devID && i < 20)
  {
    i++;
  }
  if(lutData[i].devID == devID)
  {
    data = lutData[i].cmd;
  }
  return data;
}


void sendDataToPC(LookUpTable_t *lutData)//uint32_t data, uint16_t devID, uint8_t port);
{
  
 
  //char * id;
  //uint16 idde;
 // uint32 da;
  //uint8 num;
  
  app_packet.data = lutData->data;
  app_packet.devID = lutData->devID;
  app_packet.packNum = 0;
  
  //app_packet->data = lutData->data;
  //app_packet->devID = lutData->devID;
  ///app_packet->packNum = 0;
  
  //idde = app_packet.devID;
 // da = app_packet.data;
  //num = app_packet.packNum + 0x30;
  //id = (char*)&idde;
 // id = (char*)&da;
 // id = (char*)&num;
  //id[2] = '\0';
  //id[4] = '\0';
  //id[1] = '\0';

  // HalLcdWriteString("-----------------------------SENDDATA----------------------",0);
  // HalLcdWriteString(id,0);
  // HalLcdWriteString("-----------------------------SENDDATA----------------------",0);
  
  dllDataRequest(&app_packet);
  //HalLcdWriteString("Milos----------------------",0); 
}