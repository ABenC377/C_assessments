#include "csa.h"

#define BIGSTR 100000

void sum(int* p, int* ac);
void gap(int* p, int* ac);
void dblit(int* p, int* ac);

int main(void)
{

   csa* c = NULL;
   char str[BIGSTR];
   int n = 1;

   test();

   csa_tostring(c, str);
   assert(strcmp(str, "")==0 && "Empty csa should give empty string");
   c = csa_init();
   assert(!csa_get(c, 0, &n) && "Empty csa should not get any value");
   assert(!csa_get(c, 2, &n) && "Empty csa should not get any value");
   // First Add (csa[2]=25)
   assert(csa_set(c, 2,  25) && "Should be able to add first value");
   // Add csa[3]=30
   assert(csa_set(c, 3,  30) && "Should be able to add second value");
   // Add csa[63]=100
   assert(csa_set(c, 63, 100) && "Should be able to add third value");
   // Getters
   assert(csa_get(c,  2, &n) && "Should be able to get first value");
   assert(n==25 && "First value should be 25");
   assert(csa_get(c,  3, &n) && "Should be able to get second value");
   assert(n==30 && "Second value should be 30");
   assert(csa_get(c, 63, &n) && "Should be able to get third value");
   assert(n==100 && "Third value should be 100");
   csa_tostring(c, str);
   assert(strcmp(str, "1 block {3|[2]=25:[3]=30:[63]=100}")==0 &&
          "String representation should match");

   // Use one whole block: csa[i]=i*10
   for(int i=0; i<64; i++){
    assert(csa_set(c, i, i*10) && "Should be able to set full first block");
    assert(csa_get(c, i, &n) && "Should be able to get value from full first block");
    assert(n==i*10 && "Value from full first block should match expected");
   }
   // Clean Up
   csa_free(&c);
   assert(c==NULL && "CSA should be NULL after free");

   //Let's do it all again, but with +200 higher indices
   c = csa_init();
   csa_tostring(c, str);
   assert(strcmp(str, "0 blocks")==0 && "String representation should match for empty csa");
   assert(csa_set(c, 202,  25) && "Should be able to add first value");
   assert(csa_set(c, 203,  30) && "Should be able to add second value");
   assert(csa_set(c, 263, 100) && "Should be able to add third value");
   assert(csa_get(c, 202, &n) && "Should be able to get first value");
   assert(n==25 && "First value should be 25");
   assert(csa_get(c, 203,  &n) && "Should be able to get second value");
   assert(n==30 && "Second value should be 30");
   assert(csa_get(c, 263, &n) && "Should be able to get third value");
   assert(n==100 && "Third value should be 100");
   csa_tostring(c, str);
   // 2 blocks, the first has 2 entries csa[202]=25 & csa[203]=30, and the 2nd
   // has 1 entry csa[263]=100
   assert(strcmp(str, "2 blocks {2|[202]=25:[203]=30}{1|[263]=100}")==0 &&
          "String representation should match");
   csa_free(&c);
   assert(c==NULL && "CSA should be NULL after free");

#ifdef EXT
   // EXTENSION : foreach
   c = csa_init();
   int accumulator = 0;
   assert(csa_set(c,  5, 10));
   assert(csa_set(c, 50, 20));
   csa_foreach(sum, c, &accumulator);
   assert(accumulator==30);

   // For this structure csa[50] will be stored in the adjacent cell
   // to csa[5], therefore one int apart.
   csa_foreach(gap, c, &accumulator);
   assert(accumulator==1);

   assert(csa_set(c, 65, 30));
   accumulator = 0;
   csa_foreach(sum, c, &accumulator);
   assert(accumulator==60);
   csa_foreach(dblit, c, &accumulator);
   csa_tostring(c, str);
   assert(strcmp(str, "2 blocks {2|[5]=20:[50]=40}{1|[65]=60}")==0);
   // For this structure csa[65] will not be stored in the adjacent cell
   // to csa[50], therefore not one int apart.
   csa_foreach(gap, c, &accumulator);
   assert(accumulator!=1);

   // EXTENSION : delete
   assert(!csa_delete(c,  1));
   assert( csa_delete(c, 50));
   csa_tostring(c, str);
   assert(strcmp(str, "2 blocks {1|[5]=20}{1|[65]=60}")==0);
   assert(csa_delete(c, 5));
   csa_tostring(c, str);
   assert(strcmp(str, "1 block {1|[65]=60}")==0);
   assert(csa_delete(c, 65));
   csa_tostring(c, str);
   assert(strcmp(str, "0 blocks"));
   // Don't need to do full csa_free() - only the
   // csa structure itself should be in use by now ...
   free(c);
#endif
   return EXIT_SUCCESS;
}

// p  : gets passed a pointer to each array value address in turn.
// ac : Pointer to an accumulator to store intermediate results
void sum(int* p, int* ac)
{
   *ac += *p;
}

void dblit(int* p, int* ac)
{
   // void variable not used warning
   *ac = 0;
   // integer stored in array is doubled
   *p *= 2;
}

void gap(int* p, int* ac)
{
   static int* prv = NULL;
   // The 'gap' between this pointer & last one
   // May the coding Gods forgive me ;-(
   *ac = (int)(p - prv);
   prv = p;
}
