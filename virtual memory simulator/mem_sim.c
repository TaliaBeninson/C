/*
 * program to simulate the way a computer works with its memory
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem_sim.h"

int placeToRemove=0;

void init_system(page_descriptor page_table[], char main_memory[], int * logicalmem_fd, int * swapfile_fd)
{
    const char zero ='0';
    (*logicalmem_fd) = open("program.txt", O_RDONLY);//open file to read logical addresses
    if((*logicalmem_fd) == -1)//if failed opening file print error
    {
        perror("error opening file1\n");
        exit(1);
    }

    (*swapfile_fd) = open("swap.txt", O_RDWR |O_CREAT|O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);//create file for Transient space if needed
    if((*swapfile_fd) == -1)//if failed opening file print error
    {
        close(*logicalmem_fd);
        perror("error opening file2\n");
        exit(1);
    }

    int i;
    for(i=0; i < MEMORY_SIZE; i++) //initialize main memory array with zero
    {
        main_memory[i] = zero;
    }

    for(i=0; i < NUM_OF_PAGES; i++) //initialize the page tables valid and dirty with zero
    {
        page_table[i].dirty = 0;
        page_table[i].valid = 0;
        page_table[i].frame = -1;

    }
    char zeroArray[NUM_OF_PAGES * PAGE_SIZE];
    for (i = 0; i < NUM_OF_PAGES * PAGE_SIZE; i++) //initialize Transient space with 80 characters of zero
    {
        zeroArray[i]=zero;
    }
    if(write(*swapfile_fd,zeroArray,PAGE_SIZE*NUM_OF_PAGES)<0)
    {
        perror("error writing to file\n");
        exit(1);
    }
}

int getLogicalAddress(page_descriptor page_table[])
{
    //function to return the logical address the address from main memory is located
    int v, logicAddress;
    for (v = 0; v < NUM_OF_PAGES; v++)//run through all pages in table array
    {
        if (page_table[v].frame == placeToRemove&&page_table[v].valid==1)//check which page is in frame of the index to be removed
        {
            logicAddress = (v * PAGE_SIZE);//update the variable to hold its place in array
            break;
        }
    }
    return logicAddress;
}

int getEmptyNextSpace(page_descriptor page_table[])
{
    //function to check which place is empty in main memory
    int i,k,temp;
    for(i=0; i<MEMORY_SIZE; i=i+PAGE_SIZE)//run in loop through the array of main memory
    {
        temp=i;
        for(k=0; k<NUM_OF_PAGES;k++)
        {
            if (page_table[k].frame == (i/PAGE_SIZE))//if finds an empty place return its location
            {
                temp=-1;
                break;
            }
        }
        if(temp!=-1)
        {
            return temp;
        }
    }
    return MEMORY_SIZE;
}

void getPage(int logicalmem_fd,char buffer[],int page)
{
    //function to get the page from logical memory to an array
    if(lseek(logicalmem_fd,page*PAGE_SIZE, SEEK_SET)<0)//if lseek failed return from function
    {
        perror(("failed reading logical memory\n"));
        return;
    }
    if(read(logicalmem_fd,buffer,PAGE_SIZE)!=PAGE_SIZE)//if read failed return from function
    {
        perror("failed reading logical memory to buffer\n");
        return;
    }
}

void getPageToRemove(page_descriptor page_table[], char main_memory[] , int swapfile_fd,int logicAddress,char temp[])
{
    //function to get the page that needs to be removed from main memory if it has been changed and put in different memory
    int x=0;
    int v;
    if(page_table[logicAddress/PAGE_SIZE].dirty==1)//if the page has been changed save a copy of the page
    {
        for(v=(placeToRemove*PAGE_SIZE); v<((placeToRemove*PAGE_SIZE)+PAGE_SIZE);v++)//save a copy in temp of page
        {
            temp[x]=main_memory[v];
            x++;
        }
        if(lseek(swapfile_fd,logicAddress, SEEK_SET)<0)//if lseek failed return from function
        {
            printf(("failed reading logical memory\n"));
            return;
        }
        if(write(swapfile_fd,temp,PAGE_SIZE)!=PAGE_SIZE)//if write failed return from function
        {
            printf(("failed reading logical memory to buffer\n"));
            return;
        }
    }
}

char load (int address, page_descriptor page_table[], char main_memory[] , int logicalmem_fd, int swapfile_fd)
{
    //function that receives from user an  address and returns it. if the page that the address is in already exists in main memory return it
    //otherwise bring the page to main memory and return address. if the main memory is full make space.
    int page,offset;
    if(address>79 || address<0)//if the address is invalid return from function
    {
        printf("invalid address\n");
        return '\0';
    }
    else
    {
        page = address/PAGE_SIZE;
        offset = address%PAGE_SIZE;
        if(page_table[page].valid==1)//if the page that the address is n already exists in main memory return the address
        {
            return main_memory[((PAGE_SIZE*page_table[page].frame)+offset)];
        }
        else
        {
            char buffer[PAGE_SIZE];
            int j;
            int index=getEmptyNextSpace(page_table);//get the next empty space in main memory
            getPage(logicalmem_fd,buffer,page);//get the page the address is located in
            if(index<MEMORY_SIZE)//if still have room in main memory
            {
                int k=0;
                for(j=index; j<(index+4);j++)//enter the page in main memory
                {
                    main_memory[j]=buffer[k];
                    k++;
                }
                page_table[page].valid=1;//update that the page exists
                page_table[page].frame=index/PAGE_SIZE;//update the frame the page is located in the main memory

                return main_memory[((page_table[page].frame*PAGE_SIZE)+offset)];//return the address
            }
            else if(index==MEMORY_SIZE)//if there is no more room in main memory
            {
                if(placeToRemove == 6)
                {
                    placeToRemove=0;
                }
                int logicAddress=getLogicalAddress(page_table);//get the location of the address that needs to be removed location in logical memory
                char temp[PAGE_SIZE];
                getPageToRemove(page_table,main_memory,swapfile_fd,logicAddress,temp);
                int m;
                page_table[logicAddress/PAGE_SIZE].valid=0;//update that the page no longer exists in main memory
                page_table[logicAddress/PAGE_SIZE].frame=-1;//update that the page no longer exists in main memory
                for(m=(placeToRemove*PAGE_SIZE); m<((placeToRemove*PAGE_SIZE)+PAGE_SIZE); m++)
                {
                    main_memory[m]='0';
                }
                load (address, page_table, main_memory , logicalmem_fd, swapfile_fd);//send back to function to put in main memory
                placeToRemove++;
            }
            return main_memory[((page_table[page].frame*PAGE_SIZE)+offset)];
        }
    }
}


void store(int address, char val, page_descriptor page_table[], char main_memory[] , int logicalmem_fd, int swapfile_fd)
{
    //function that receives from user an  address and a character and updats the address to the character. if the page that the address is in already exists in main memory return it
    //otherwise bring the page to main memory and and updats the address to the character. if the main memory is full make space.
    int page,offset;

    page = address/PAGE_SIZE;
    offset = address%PAGE_SIZE;

    char value = load(address,page_table,main_memory,logicalmem_fd,swapfile_fd);
    if(value=='\0')
    {
        return;
    }
    else
    {
        main_memory[((page_table[page].frame*PAGE_SIZE)+offset)]=val;
        page_table[page].dirty=1;
    }
}

void print_memory(char main_memory[])
{
    //function to print main memory
    int i,j=1;
    for(i=0; i<MEMORY_SIZE; i++)
    {
        printf("%c",main_memory[i]);
        if(j%4==0)
        {
            printf("\n");
        }
        j++;
    }
}

