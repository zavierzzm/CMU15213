/*
 * cachelab.c - Cache Lab helper functions
 */

#include "cachelab.h"

trans_func_t func_list[MAX_TRANS_FUNCS];
int func_counter = 0;
int VERBOSE = 0;

/*
 * readParameters - This function read a line of parameters from the cmd
 */
int readParameters(int argc, char **argv, options_t* option)
{
    int i;
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            option->help = 1;
        }
        else if (strcmp(argv[i], "-v") == 0) {
            option->verbose = 1;
        }
        else if (i + 1 < argc && strcmp(argv[i], "-s") == 0) {
            option->setBits = atoi(argv[++i]);
        }
        else if (i + 1 < argc && strcmp(argv[i], "-E") == 0) {
            option->associativity = atoi(argv[++i]);
        }
        else if (i + 1 < argc && strcmp(argv[i], "-b") == 0) {
            option->blockBits = atoi(argv[++i]);
        }
        else if (i + 1 < argc && strcmp(argv[i], "-t") == 0) {
            option->tracefile = argv[++i];
        }
        else {
            return RET_FAIL;
        }
    }
    return RET_SUCC;
}

/*
 * processTrace - This function process the trace file
 */
int processTrace(options_t *option, results_t *result)
{
    FILE* fd;
    lines_t* memory;
    char trace[BUFF_SIZE];
    instructions_t instruction = {0, 0};

    if (option->help == 1) {
        helpInfo();
        return RET_SUCC;
    }

    VERBOSE = option->verbose;

    if (option->tracefile) {
        fd = fopen(option->tracefile, "r");
        if (!fd) {
            fprintf(stderr, "Cannot open file %s\n", option->tracefile);
            return RET_FAIL;
        }
    }
    else {
        fprintf(stderr, "No trace file\n");
        return RET_FAIL;
    }

    memory = allocMemory(option->setBits, option->associativity, option->blockBits);
    if (!memory) {
        fprintf(stderr, "Fail to allocate cache memory\n");
        return RET_FAIL;
    }

    while (fgets(trace, BUFF_SIZE, fd)) {
        preProcessInstruction(trace, &instruction);
        processInstruction(&instruction, result, memory, option->setBits, option->associativity, option->blockBits);
    }
    fclose(fd);
    free(memory);
    return RET_SUCC;
}

/* 
 * preProcessInstruction - Preprocess string instruction
 */ 
void preProcessInstruction(const char* trace, instructions_t* instruction)
{
    if (trace[0] != ' ') instruction->type = INS_INS;
    else {
        switch (trace[1]) {
            case 'L': instruction->type = INS_LOAD; break;
            case 'S': instruction->type = INS_STORE; break;
            case 'M': instruction->type = INS_MODIFY; break;
        }
        sscanf(trace, " %*[^ ] %lx,%d", &(instruction->address), &(instruction->pos));
    }
}

/* 
 * processInstruction - Preprocess instructions_t instruction and update results
 */ 
void processInstruction(instructions_t* instruction, results_t* result, lines_t* memory, int s, int e, int b)
{
    if (instruction->type == INS_INS) return;

    switch (instruction->type) {
    case INS_MODIFY: 
        printf("M %lx,%d", instruction->address, instruction->pos);
        touchAddress(instruction->address, result, memory, s, e, b);
        touchAddress(instruction->address, result, memory, s, e, b);
        break;
    case INS_LOAD:
        printf("L %lx,%d", instruction->address, instruction->pos);
        touchAddress(instruction->address, result, memory, s, e, b); 
        break;
    case INS_STORE:
        printf("S %lx,%d", instruction->address, instruction->pos);
        touchAddress(instruction->address, result, memory, s, e, b); 
        break;
    }
    printVerbose(" \n");
}

/* 
 * touchAddress - Touch a given address once and perform the operations
 */ 
void touchAddress(unsigned long address, results_t* result, lines_t* memory, int s, int e, int b)
{
    int i;
    lines_t* line;
    unsigned long tag = address >> (s + b);
    long mod = 1 << s;
    long set = (address >> b) % mod;
    // if (set == 3) printf(" set %ld tag %lu ", set, tag);
    memory += e * set;
    for (i = 0; i < e; ++i) {
        line = memory + i;
        if (line->dirty && line->tag == tag) {
            printVerbose(" hit");
            ++(result->hit);
            for (; i > 0; --i) {
                swap(memory + i, memory + i - 1);
            }
            return;
        }
    }

    printVerbose(" miss");
    ++(result->miss);
    for (i = 0; i < e; ++i) {
        line = memory + i;
        if (!(line->dirty)) {
            line->dirty = 1;
            line->tag = tag;
            for (; i > 0; --i) {
                swap(memory + i, memory + i - 1);
            }
            return;
        }
    }
    line = memory + e - 1;
    line->tag = tag;
    ++(result->evict);
    for (i = e - 1; i > 0; --i) {
        swap(memory + i, memory + i - 1);
    }
    printVerbose(" eviction");
}

void swap(lines_t* a, lines_t* b)
{
    lines_t c;
    c = *a;
    *a = *b;
    *b = c;
}

/* 
 * allocMemory - This function allocate memory for cache
 */ 
void* allocMemory(int s, int e, int b)
{
    void *memory;
    if (s <= 0 || e <= 0 || b <= 0) return NULL;
    memory = malloc(sizeof(lines_t) * (1 << s) * e);
    if (memory) {
        memset(memory, 0, sizeof(lines_t) * s * e);
    }
    return memory;
}
/*
 * printVerbose - This function print the verbose infomation
 */
void printVerbose(const char *s, ...) 
{
    va_list ap;
    va_start(ap, s);
    if (VERBOSE) {
        printf(s, va_arg(ap, unsigned long), va_arg(ap, int));
    }
    va_end(ap);
}


/*
 * helpInfo - This function print the usage info
 */
void helpInfo()
{
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("\t-h: Optional help flag that prints usage info\n");
    printf("\t-v: Optional verbose flag that displays trace info\n");
    printf("\t-s <s>: Number of set index bits\n");
    printf("\t-E <E>: Associativity\n");
    printf("\t-b <b>: Number of block bits\n");
    printf("\t-t <tracefile>: Name of the valgrind trace to replay\n");
}

/* 
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded. 
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/* 
 * initMatrix - Initialize the given matrix 
 */
void initMatrix(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
            B[j][i]=rand();
        }
    }
}

void randMatrix(int M, int N, int A[N][M]) {
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
        }
    }
}

/* 
 * correctTrans - baseline transpose function used to evaluate correctness 
 */
void correctTrans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}



/* 
 * registerTransFunction - Add the given trans function into your list
 *     of functions to be tested
 */
void registerTransFunction(void (*trans)(int M, int N, int[N][M], int[M][N]), 
                           char* desc)
{
    func_list[func_counter].func_ptr = trans;
    func_list[func_counter].description = desc;
    func_list[func_counter].correct = 0;
    func_list[func_counter].num_hits = 0;
    func_list[func_counter].num_misses = 0;
    func_list[func_counter].num_evictions =0;
    func_counter++;
}
