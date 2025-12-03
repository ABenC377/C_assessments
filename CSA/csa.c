#include "csa.h"
#include "mydefs.h"

csa* csa_init(void) { return (csa*)calloc(1, sizeof(csa)); }

bool csa_get(csa* c, int idx, int* val) {
  if (c->n == 0) return false;
  int blockIndex = 0;
  while (blockIndex < c->n && c->b[blockIndex].offset <= (unsigned int)(idx/(int)64) * 64) blockIndex++;
  if (c->b[--blockIndex].offset > (unsigned int)(idx/(int)64) * 64) return false; 
  return (*val = getVal(&(c->b[blockIndex]), idx % 64)) || true;
}

int getVal(block* b, int idx) {
  return ((b->msk & (1ull << idx)) == 0) ? 0 : b->vals[getValIndex(b, idx)];
}

int getValIndex(block* b, int idx) {
  uint64_t msk = b->msk;
  uint64_t maskCheck = (1ull << idx) - 1;
  uint64_t subMask = msk & maskCheck;
  return __builtin_popcountl(subMask);
}

bool csa_set(csa* c, int idx, int val) {
  if (c->n == 0) return addNewBlock(c, idx, val);

  int blockIndex = 0;
  unsigned int offset = (idx/(int)64) * 64;

  while (blockIndex < c->n && c->b[blockIndex].offset < offset) blockIndex++;
  
  if (blockIndex == c->n) return addNewBlock(c, idx, val);
  if (c->b[blockIndex].offset > offset) return addNewBlock(c, idx, val);

  return (addValToBlock(&(c->b[blockIndex]), idx % 64, val));
}

int offsets(const void* b1, const void* b2) { return ((block*)b2)->offset - ((block*)b1)->offset; }


bool addNewBlock(csa* c, int idx, int val) {
  c->b = (block*)realloc(c->b, (c->n + 1) * sizeof(block));
  if (!c->b) return false;
  
  c->b[c->n].vals = (int*)malloc(sizeof(int));
  if (!c->b[c->n].vals) return false;
  *(c->b[c->n].vals) = val;
  c->b[c->n].msk = 1ull << (idx % 64);
  c->b[(c->n)++].offset = (idx / 64) * 64;
  return true;
}

bool addValToBlock(block* b, int idx, int val) {
  uint64_t maskCheck = 1ull << idx;
  if (b->msk & maskCheck) return (b->vals[getValIndex(b, idx)] = val);

  int* temp = (int*)malloc(sizeof(int) * (__builtin_popcountl(b->msk) + 1));
  if (!temp) return false;
  
  int valIndex = getValIndex(b, idx);
  
  memcpy(temp, b->vals, valIndex * sizeof(int));
  
  temp[valIndex] = val;
  
  memcpy(temp + valIndex + 1, b->vals + valIndex,
         (__builtin_popcountl(b->msk) - valIndex) * sizeof(int));
  
  free(b->vals);
  b->vals = temp;

  b->msk |= maskCheck;
  return true;
}

void csa_tostring(csa* c, char* s) {
  if (!c) return;
  snprintf(s, BIGSTR, "%d block%s", c->n, (c->n == 1) ? " " : ((c->n == 0) ? "s" : "s "));
  for (int i = 0; i < c->n; i++) printBlock(&(c->b[i]), s);
}

void printBlock(block* b, char* s) {
  int startIndex = strlen(s);
  snprintf(s + startIndex, BIGSTR - startIndex, "{%d|", __builtin_popcountl(b->msk));
  bool first = true;
  for (int i = 0; i < MSKLEN; i++) {
    if (b->msk & 1ull << i) {
      if (!first) {
        startIndex = strlen(s);
        snprintf(s + startIndex, BIGSTR - startIndex, ":");
      } else {
        first = false;
      }
      startIndex = strlen(s);
      snprintf(s + startIndex, BIGSTR - startIndex, "[%d]=%d",
               b->offset + i, b->vals[getValIndex(b, i)]);  
    }
  }
  startIndex = strlen(s);
  snprintf(s + startIndex, BIGSTR - startIndex, "}");
}

void csa_free(csa** l) {
  for (int i = 0; i < (*l)->n; i++) {
    free((*l)->b[i].vals);
  }
  free((*l)->b);
  free(*l);
  *l = NULL;
}

void test(void) {
}

#ifdef EXT
void csa_foreach(void (*func)(int* p, int* ac), csa* c, int* ac)
{
}

bool csa_delete(csa* c, int indx)
{
}
#endif
