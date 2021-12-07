#ifndef HW3_MEM_SIM_H
#define HW3_MEM_SIM_H
#define PAGE_SIZE 4
#define NUM_OF_PAGES 20
#define MEMORY_SIZE 24

typedef struct
{
    int valid; //indicates if this page is mapped to a frame
    int frame; //the number of a frame in case it is page-mapped
    int dirty; //indicates if this page has changed

} page_descriptor;



void init_system(page_descriptor[], char[], int*, int*);

char load(int, page_descriptor[], char[], int, int);

void store(int, char, page_descriptor[], char[], int, int);

void print_memory(char[]);

#endif //HW3_MEM_SIM_H
