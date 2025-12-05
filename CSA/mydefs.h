#pragma once

#define BIGSTR 100000

bool getVal(block* b, int idx, int* val);
int getValIndex(block* b, int idx);
bool addNewBlock(csa* c, int idx, int val);
bool addValToBlock(block* b, int idx, int val);
int offsets(const void* b1, const void* b2);
void printBlock(block* b, char* s);
