#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

//#include <stdint.h>
//#include <stdlib.h>
#include "hal_types.h"

#define CAPACITY 256

typedef struct circularBuffer 
{
		uint8 buffer[CAPACITY];
		uint8 *head;
		uint8 *tail;
} CircularBuffer_t;

uint8 circularInit(CircularBuffer_t * );

uint8 circularSize(CircularBuffer_t * );
uint8 circularIsFull(CircularBuffer_t * );
uint8 circularIsEmpty(CircularBuffer_t * );

void circularPut(CircularBuffer_t * , uint8 );
uint8 circularGet(CircularBuffer_t * , uint8 * );

void circularEmptyBuffer(CircularBuffer_t * );
void circularDump(CircularBuffer_t * );

int circular2array(CircularBuffer_t * , uint8 * );
uint8 array2circular(CircularBuffer_t *c_buff, uint8 *uc_array, uint8 size);

#endif //__CIRCULAR_BUFFER__
