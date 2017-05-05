#ifndef __DLL_H__
#define __DLL_H__

//#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "hal_types.h"

//*****************************************************************************
//
// Defining supported devices IDs
//
//*****************************************************************************
//#define TEMP_SENSOR_INSIDE		0x5432

#define TEMP_SENSOR_INSIDE		0x3254

#define TEMP_SENSOR_OUTSIDE		0x1112

//#define PRESSURE_SENSOR_INSIDE		0x5032

#define PRESSURE_SENSOR_INSIDE		0x3250

#define PRESSURE_SENSOR_OUTSIDE		0x1114

//#define HUMIDITY_SENSOR_INSIDE		0x4832

#define HUMIDITY_SENSOR_INSIDE		0x3248

#define HUMIDITY_SENSOR_OUTSIDE		0x1116
#define DOOR				0x1117
#define WINDOW				0x1118
#define MOTION_SENSOR			0x1119

//*****************************************************************************
//
// Values that determines start and stop of each frame as well as start and
// stop of each field of received data
//
//*****************************************************************************
#define START_DELIMITER		0x3e // '>'
#define STOP_DELIMITER		0x3c // '<'			//0x0a // '\n'
#define START_FIELD		0x23 // '#'
#define STOP_FIELD		0x2c // ','
#define EMISSION_START		0x01

//
// Maximum DLL layer payload size
//
#define MAX_BUFFER_LENGHT	256


//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************
//#pragma data_alignment = 1
//#pragma pack(push)
//#pragma pack(1)    //__packed 
//_Pragma(#1(data_alignment=1))
typedef struct sData
{
	uint16 devID;
	uint8 packNum;
        uint32 data;
} Data_t;

//#pragma pack(pop)

//#pragma pack(push)
//#pragma pack(1)

typedef struct
{
	Data_t appData;
	uint8 timeStamp;
	uint8 checkSum;
} DLLPacket_t;
//#pragma pack(pop)


typedef enum eDLLState
{
	IDLE,
	RECEPTION,
	EMISSION
} DLLState_t;

typedef void (*CallBack_t)(Data_t *);

//*****************************************************************************
//
// API Function prototypes
//
//*****************************************************************************
void dllInit(void);
void CallBackRegister(CallBack_t cb);
uint16 getSignalAddress(const uint16 ID);
//uint16 processFrameRx(uint8 *frame);
uint16 processFrame(uint8 *buff);
void dllDataRequest(Data_t *aData);

#endif //__DLL_H__
