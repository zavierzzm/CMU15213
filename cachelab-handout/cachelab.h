/* 
 * cachelab.h - Prototypes for Cache Lab helper functions
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define MAX_TRANS_FUNCS 100
#define RET_SUCC 0
#define RET_FAIL 1
#define BUFF_SIZE 100
#define INS_INS 0
#define INS_LOAD 1
#define INS_STORE 2
#define INS_MODIFY 3

typedef struct trans_func{
  void (*func_ptr)(int M,int N,int[N][M],int[M][N]);
  char* description;
  char correct;
  unsigned int num_hits;
  unsigned int num_misses;
  unsigned int num_evictions;
} trans_func_t;

typedef struct options{
	int help;
	int verbose;
	int setBits;
	int associativity;
	int blockBits;
	char* tracefile;
} options_t;

typedef struct lines{
	int dirty;
	unsigned long tag;
} lines_t;

typedef struct resultss{
	int hit;
	int miss;
	int evict;
} results_t;

typedef struct instructions{
	int type;
	unsigned long address;
	int pos;
} instructions_t;

void swap(lines_t* a, lines_t* b);

/* 
 * preProcessInstruction - Preprocess string instruction
 */ 
void preProcessInstruction(const char* trace, instructions_t* instruction);

/* 
 * processInstruction - Preprocess instructions_t instruction and update results
 */ 
void processInstruction(instructions_t* instruction, results_t* result, lines_t* memory, int s, int e, int b);

/* 
 * touchAddress - Touch a given address once and perform the operations
 */ 
void touchAddress(unsigned long address, results_t* result, lines_t* memory, int s, int e, int b);

/*
 * readParameters - This function read a line of parameters from the cmd
 */
int readParameters(int argc,
				   char **argv,
				   options_t *option);

/*
 * processTrace - This function process the trace file
 */
int processTrace(options_t *option, results_t* result);

/*
 * helpInfo - This function print the usage info
 */
void helpInfo();

/*
 * printVerbose - This function print the verbose infomation
 */
void printVerbose(const char *s, ...);

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */
/* 
 * allocMemory - This function allocate memory for cache
 */ 
void* allocMemory(int s, int e, int b);

/* Fill the matrix with data */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
void registerTransFunction(
    void (*trans)(int M,int N,int[N][M],int[M][N]), char* desc);

#endif /* CACHELAB_TOOLS_H */
