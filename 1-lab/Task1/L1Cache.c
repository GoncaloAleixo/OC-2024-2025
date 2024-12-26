#include "L1Cache.h"

//global variables
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L1;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {
	if (address >= DRAM_SIZE - WORD_SIZE + 1)
		exit(-1);

	// read/fetch a block from memory RAM
	if (mode == MODE_READ) {
		memcpy(data, &(DRAM[address]), BLOCK_SIZE);
		time += DRAM_READ_TIME;
	}
	// write a block in memory RAM
	else if (mode == MODE_WRITE) {
		memcpy(&(DRAM[address]), data, BLOCK_SIZE);
		time += DRAM_WRITE_TIME;
	}
}

/*********************** L1 cache *************************/

unsigned int log2_clz(unsigned int x) {
    if (x == 0) {
        return 0;
    }
    return 31 - __builtin_clz(x);  // For a 32-bit integer
}

void initCache() {
	for (int i = 0; i < L1_NUM_LINES; i++) {
		L1.lines[i].Valid = 0;
		L1.lines[i].Dirty = 0;
		L1.lines[i].Tag = 0;
		for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
			L1.lines[i].Data[j] = 0;
		}
	}
	L1.init = 1;
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

	uint32_t offset, index, tag, MemAddress;
	uint8_t TempBlock[BLOCK_SIZE];

	/* init cache */
	if (L1.init == 0) {
		initCache();
	}

	index = (address / BLOCK_SIZE) % (L1_NUM_LINES); 
    tag = address / L1_SIZE;
    MemAddress = (address / BLOCK_SIZE) * BLOCK_SIZE;
    offset = address % BLOCK_SIZE; 

	CacheLine *Line = &L1.lines[index];

	//MISS
	if (!Line->Valid || Line->Tag != tag) {
		accessDRAM(MemAddress, TempBlock, MODE_READ); 

		if ((Line->Valid) && (Line->Dirty)) {	
			uint32_t Write_Back_Mem = (Line->Tag << log2_clz(L1_NUM_LINES)) + index;
			accessDRAM(Write_Back_Mem, &(Line->Data[offset]), MODE_WRITE); 
		}

		memcpy(&(Line->Data[offset]), TempBlock, BLOCK_SIZE);
		Line->Valid = 1;
		Line->Tag = tag;
		Line->Dirty = 0;
	}

	//HIT
	if (mode == MODE_READ) {    
		memcpy(data, &(Line->Data[offset]), WORD_SIZE);
		time += L1_READ_TIME;
	}
	else if (mode == MODE_WRITE) { 
		memcpy(&(Line->Data[offset]), data, WORD_SIZE);
		time += L1_WRITE_TIME;
		Line->Dirty = 1;
	}
}

void read(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
	accessL1(address, data, MODE_WRITE);
}