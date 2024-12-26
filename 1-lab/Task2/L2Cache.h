#ifndef L1CACHE_H
#define L1CACHE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

#define L1_NUM_LINES (L1_SIZE / BLOCK_SIZE)
#define L2_NUM_LINES (L2_SIZE / BLOCK_SIZE)

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void initCache1();
void initCache2();

unsigned int log2_clz(unsigned int x);
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
	uint8_t Valid;	//bits de validade e tag
	uint8_t Dirty;
	uint32_t Tag;
	uint8_t Data[BLOCK_SIZE];
} CacheLine;

typedef struct L1Cache {
	uint32_t init;
	CacheLine lines[L1_NUM_LINES];	// Num of Lines = L1_SIZE / BLOCK_SIZE
} L1Cache;

typedef struct L2Cache {
	uint32_t init;
	CacheLine lines[L2_NUM_LINES];	// Num of Lines = L2_SIZE / BLOCK_SIZE
} L2Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);


#endif