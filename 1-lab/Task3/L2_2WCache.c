#include "L2_2WCache.h"

L1Cache L1;
L2Cache L2;
uint8_t DRAM[DRAM_SIZE];
uint32_t time;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initCache(){
  initCache1();
  initCache2();
}

void initCache1() {
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

void initCache2() {
  for(int i = 0; i < L2_NUM_SETS; i++) {
    for(int j = 0; j < SET_SIZE; j++){
      L2.sets[i].init = 1;
      L2.sets[i].lines[j].Valid = 0;
      L2.sets[i].lines[j].Tag = 0;
      L2.sets[i].lines[j].Dirty = 0;
      L2.sets[i].lru = 0;
      for (int k = 0; k < BLOCK_SIZE; k += WORD_SIZE) {
	    L2.sets[i].lines[j].Data[k] = 0;
	  }
    } 
  }
  L2.init = 1;
}

/*********************** L1 Cache *************************/

unsigned int log2_clz(unsigned int x) {
    if (x == 0) {
        return 0;
    }
    return 31 - __builtin_clz(x);  // For a 32-bit integer
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
  
  uint32_t offset, index, tag;
  uint8_t TempBlock[BLOCK_SIZE];

	/* init cache */
  if (L1.init == 0) {
   	initCache1();
  }

  index = (address / BLOCK_SIZE) % (L1_NUM_LINES); 
  tag = address / L1_SIZE;
  offset = address % BLOCK_SIZE; 

  CacheLine *Line = &L1.lines[index];

  if (!Line->Valid || Line->Tag != tag) {         // if block not present - miss
    accessL2(address, TempBlock, MODE_READ); // get new block from DRAM      
   
    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      uint32_t Write_Back_Mem = (Line->Tag << log2_clz(L1_NUM_LINES)) + index;
      Write_Back_Mem = (Write_Back_Mem << 6) + offset;
      accessL2(Write_Back_Mem, &(Line->Data[offset]), MODE_WRITE); // then write back old block     
    }

    memcpy(&(Line->Data[offset]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(Line->Data[offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(Line->Data[offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t offset, index, tag, MemAddress;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (L2.init == 0) {
    initCache2();
  }

  index = (address / BLOCK_SIZE) % (L2_NUM_LINES); 
  tag = address / L2_SIZE;
  MemAddress = address / BLOCK_SIZE;
  offset = address % BLOCK_SIZE; 

  CacheSet *Set = &L2.sets[index];
  CacheLine *Line1 = &L2.sets[index].lines[0];
  CacheLine *Line2 = &L2.sets[index].lines[1];
  CacheLine *Target_Line;

  /* access Cache*/

  if ((!Line1->Valid && !Line2->Valid) || (Line1->Tag != tag && Line2->Tag != tag)) { // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM  
    
    int lru = Set->lru;
    Target_Line = &Set->lines[lru];

    if ((Target_Line->Valid) && (Target_Line->Dirty)) { // line has dirty block
      uint32_t Write_Back_Mem = (Target_Line->Tag << log2_clz(L2_NUM_LINES)) + index;
      accessDRAM(Write_Back_Mem, &(Target_Line->Data[offset]), MODE_WRITE); // then write back old block 
    }

    memcpy(&(Target_Line->Data[offset]), TempBlock, BLOCK_SIZE); // copy new block to cache line
    Target_Line->Valid = 1;
    Target_Line->Tag = tag;
    Target_Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (Line1->Tag == tag) {
    Target_Line = &Set->lines[0];
    Set->lru = 1;
  } else {
    Target_Line = &Set->lines[1];
    Set->lru = 0;
  } 

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(Target_Line->Data[offset]), BLOCK_SIZE);
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(Target_Line->Data[offset]), data, BLOCK_SIZE);
    time += L2_WRITE_TIME;
    Target_Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
