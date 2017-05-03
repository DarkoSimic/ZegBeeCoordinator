#ifndef __LOOK_UP_TABLE_H__
#define __LOOK_UP_TABLE_H__

//#include <stdint.h>
#include "hal_types.h"
#include "includes/dll.h"
#include "AF.h"
//*****************************************************************************
//
// Global variables
//
//*****************************************************************************


typedef struct lookuptable
{
    uint16 devID ;
    uint16 devAddress;
    uint32 data;
    uint16 cmd;
	
} LookUpTable_t;

extern LookUpTable_t addrID[20];



void lookUpInit(void);

void updateLookUpTable(Data_t pData, uint16 devAddress);

void updateCmd(Data_t *pData);

uint16 lookForAddr(uint16 devID);

int16 getIDFromPkt(afMSGCommandFormat_t *cmd);

int32 getDataFromPkt(afMSGCommandFormat_t *cmd);

int32 getCmd(uint16 devID);

void sendDataToPC(LookUpTable_t *lutData);

#endif //__LOOK_UP_TABLE_H__