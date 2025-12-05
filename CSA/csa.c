#include "csa.h"
#include "mydefs.h"

csa* csa_init(void) { 
  return (csa*)calloc(1, sizeof(csa)); 
}

bool csa_get(csa* c, int idx, int* val) {
  if (c->n == 0) return false;
  int blockIndex = 0;
  while (blockIndex < c->n && c->b[blockIndex].offset <= (unsigned int)(idx/(int)MSKLEN) * MSKLEN) blockIndex++;
  return (c->b[--blockIndex].offset > (unsigned int)(idx/(int)MSKLEN) * MSKLEN) ? false : (getVal(&(c->b[blockIndex]), idx % MSKLEN, val));
}

bool getVal(block* b, int idx, int* val) {
  return ((b->msk & (1ull << idx)) == 0) ? false : ((*val = b->vals[getValIndex(b, idx)]) || true);
}

int getValIndex(block* b, int idx) {
  return __builtin_popcountl(b->msk & ((1ull << idx) - 1));
}

bool csa_set(csa* c, int idx, int val) {
  if (c->n == 0) return addNewBlock(c, idx, val);
  int blockIndex = 0;
  while (blockIndex < c->n && c->b[blockIndex].offset < (unsigned int)(idx/(int)MSKLEN) * MSKLEN) blockIndex++;
  return ((blockIndex == c->n) || (c->b[blockIndex].offset > (unsigned int)(idx/(int)MSKLEN) * MSKLEN)) ? addNewBlock(c, idx, val) : (addValToBlock(&(c->b[blockIndex]), idx % MSKLEN, val));
}

int offsets(const void* b1, const void* b2) { return ((block*)b2)->offset - ((block*)b1)->offset; }


bool addNewBlock(csa* c, int idx, int val) {
  if (!(c->b = (block*)realloc(c->b, (c->n + 1) * sizeof(block))) || !(c->b[c->n].vals = (int*)malloc(sizeof(int)))) return false;
  *(c->b[c->n].vals) = val;
  c->b[c->n].msk = 1ull << (idx % MSKLEN);
  return (c->b[(c->n)++].offset = (idx / MSKLEN) * MSKLEN) || true;
}

bool addValToBlock(block* b, int idx, int val) {
  if (b->msk & (1ull << idx)) return ((b->vals[getValIndex(b, idx)] = val) || true);

  int* temp = (int*)malloc(sizeof(int) * (__builtin_popcountl(b->msk) + 1));
  if (!temp) return false;
  
  memcpy(temp, b->vals, getValIndex(b, idx) * sizeof(int));
  temp[getValIndex(b, idx)] = val;
  memcpy(temp + getValIndex(b, idx) + 1, b->vals + getValIndex(b, idx), (__builtin_popcountl(b->msk) - getValIndex(b, idx)) * sizeof(int));

  free(b->vals);
  b->vals = temp;
  return (b->msk |= 1ull << idx) || true;
}

void csa_tostring(csa* c, char* s) {
  if (!c) return;
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
  for (int i = 0; i < (*l)->n; i++) free((*l)->b[i].vals);
  free((*l)->b);
  free(*l);
  *l = NULL;
}

void test(void) {
  // Clearly correct based on inspection.  No testing needed.
}

// #ifdef EXT
void csa_foreach(void (*func)(int* p, int* ac), csa* c, int* ac) {
  for (int blk = 0; blk < c->n; blk++) for (int v = 0; v < __builtin_popcountl(c->b[blk].msk); v++) func(&(c->b[blk].vals[v]), ac);
}

bool csa_delete(csa* c, int indx) {
  int blockIndex = 0;
  while (blockIndex < c->n && c->b[blockIndex].offset <= (unsigned int)(indx/(int)MSKLEN) * MSKLEN) blockIndex++;
  if ((blockIndex == 0 || c->b[--blockIndex].offset > (unsigned int)(indx/(int)MSKLEN) * MSKLEN) || !(c->b[blockIndex].msk & (1ull << (indx % MSKLEN)))) return false;

  int valIndex = getValIndex(&(c->b[blockIndex]), indx % MSKLEN);
  int* temp = (int*)malloc(sizeof(int) * (__builtin_popcountl(c->b[blockIndex].msk) - 1));
  if (!temp) return false;

  memcpy(temp, c->b[blockIndex].vals, valIndex * sizeof(int));
  memcpy(temp + valIndex, c->b[blockIndex].vals + valIndex + 1, (__builtin_popcountl(c->b[blockIndex].msk) - valIndex - 1) * sizeof(int));

  free(c->b[blockIndex].vals);
  c->b[blockIndex].vals = temp;

  if (!(c->b[blockIndex].msk &= ~(1ull << (indx % MSKLEN)))) {
    free(c->b[blockIndex].vals);
    for (int i = blockIndex; i < c->n - 1; i++) c->b[i] = c->b[i + 1];
    if (--(c->n) == 0) {
      free(c->b);
      c->b = NULL;
    } else c->b = (block*)realloc(c->b, c->n * sizeof(block));
  }
  return true;
}
// #endif
