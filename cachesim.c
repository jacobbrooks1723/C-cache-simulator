#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

typedef struct Line{
  long tag;
  char valid;
  int accesses;
  int lastTimeUsed;
}Line;

Line** cache;

void freeCache(int s){
  for(int i = 0; i < s; i++){
    free(cache[i]);
  }
  free(cache);
}

void printCache(int s, int e){
  for(int i = 0; i < s; i++){
    for(int j = 0; j < e; j++){
      printf("| %ld |", cache[i][j].tag);
    }
    printf("\n");
    for(int k = 0; k < e; k++){
      printf("______");
    }
    printf("\n");
  }
  printf("\n");
}

void userPrompt(int* s, int* e, int* b, int* m, char* rp, int* h, int* p ){
  int sIn; int eIn; int bIn; int mIn; char rpIn[4]; int hIn; int pIn;
  printf("Enter parameters S, E, B and m.\n");
  scanf("%d", &sIn); scanf("%d", &eIn); scanf("%d", &bIn); scanf("%d", &mIn);
  printf("Enter replacement policy.\n");
  getchar();
  fgets(rpIn, sizeof(rpIn), stdin);
  printf("Enter h and p.\n");
  scanf("%d", &hIn); scanf("%d", &pIn);
  *s = sIn; *e = eIn; *b = bIn; *m = mIn; strcpy(rp, rpIn); *h = hIn; *p = pIn;
}

void initializeCache(int s, int e){
    cache = malloc(sizeof(Line) * s);
    for(int i = 0; i < s; i++){
      cache[i] = malloc(sizeof(Line) * e);
    }
    for(int i = 0; i < s; i++){
      for(int j = 0; j < e; j++){
        cache[i][j].valid = 0;
        cache[i][j].accesses = 0;
        cache[i][j].lastTimeUsed = 0;
        cache[i][j].tag = 0;
      }
    }
}

int checkHit(long address, Line* l, unsigned long tagMask){
  int retValue = (l -> valid && (l -> tag == (address & tagMask)));
  if(retValue){
    l -> accesses++;
  }
  return retValue;
}

int lfuReplacementIndex(int e, int smallB, unsigned long address, unsigned long setMask){
  int minIndex = -1; 
  int minVal = INT_MAX;
  for(int i = 0; i < e; i++){
    if(cache[(address & setMask) >> smallB][i].accesses <= minVal){
      minVal = cache[(address & setMask) >> smallB][i].accesses;
      minIndex = i;
    } 
  }
  return minIndex;
}

int lruReplacementIndex(int e, int smallB, unsigned long address, unsigned long setMask, int clockCycles){
  int minIndex = -1;
  int minVal = clockCycles;
  for(int i = 0; i < e; i++){
    if(cache[(address & setMask) >> smallB][i].lastTimeUsed <= minVal){
      minVal = cache[(address & setMask) >> smallB][i].lastTimeUsed;
      minIndex = i;
    }
  }
  return minIndex;
}

int noValidTags(int e, int address, int setMask, int smallB){
  for(int i = 0; i < e; i++){
    if(cache[(address & setMask) >> smallB][i].valid){
      return 0;
    }
  }
  return 1;
}

int main(){
  int s; int e; int b; int m; char rp[4]; int h; int p;
  userPrompt(&s, &e, &b, &m, rp, &h, &p);
  initializeCache(s,e);
  int smallB = log2(b);
  int smallS = log2(s);
  unsigned long blockOffsetMask = ~(~0 << smallB);
  unsigned long setMask = ~(~0 << smallS) << smallB;  
  unsigned long tagMask = ~0 << (smallS + smallB);

  double totalAccesses = 0;
  double totalMisses = 0; 
  int clockCycles = 0;

  unsigned long address;

  while(address != -1){
    scanf("%lx", &address);
    if(address == -1){
      break;
    }
    char hit = 0;
    for(int i = 0; i < e; i++){
      if(checkHit(address, &cache[(address & setMask) >> smallB][i], tagMask)){
        hit = 1;
        clockCycles+=h;
        cache[(address & setMask) >> smallB][i].lastTimeUsed = clockCycles;
        break;
      }
    }
    if(hit){
      printf("%lx H\n", address);
    }else{
      totalMisses++;
      printf("%lx M\n", address);
      if(noValidTags(e, address, setMask, smallB)){
        clockCycles += p;
        cache[(address & setMask) >> smallB][e-1].valid = 1;
        cache[(address & setMask) >> smallB][e-1].tag = address & tagMask;
        cache[(address & setMask) >> smallB][e-1].lastTimeUsed = clockCycles;
        cache[(address & setMask) >> smallB][e-1].accesses++; 
      }else{
        int replacementIndex = -1;
        if(strcmp("LFU", rp) == 0){
          replacementIndex = lfuReplacementIndex(e, smallB, address, setMask);
        }else{
          replacementIndex = lruReplacementIndex(e, smallB, address, setMask, clockCycles);
        }
        clockCycles += p;
        cache[(address & setMask) >> smallB][replacementIndex].tag = address & tagMask;
        cache[(address & setMask) >> smallB][replacementIndex].lastTimeUsed = clockCycles;
        cache[(address & setMask) >> smallB][replacementIndex].valid = 1;
        cache[(address & setMask) >> smallB][replacementIndex].accesses++;
      }
    }
    totalAccesses++;
    printCache(s,e);
  }
  double missRate = (totalMisses / totalAccesses) * 100;
  printf("%d %d\n", (int) missRate, clockCycles);
  freeCache(s);
}
