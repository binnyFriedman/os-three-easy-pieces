#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

int PAGESIZE = 0;

struct Array
{
    int length;
    int *arr;
};

struct Array initializeArray(int sizeInPages)
{
    int size = (PAGESIZE / sizeof(int)) * sizeInPages;
    int array[size];
    for (int i = 0; i < size; ++i) {
        array[i] = 0;
    }
    struct Array a;
    a.length = size;
    a.arr = array;

    return a;
}

int measurePageAccess(struct Array arr, int index)
{
    clock_t start, end;
    int i = index;
    int arbitrary_measure_unit = 100;
    start = clock();
    for (; i < index + arbitrary_measure_unit && i < arr.length; i++)
    {
        arr.arr[i] = i;
    }
    end = clock();

    return end - start;
}

clock_t accessPage(int pages, struct Array arr)
{
    clock_t start, end;
    int currentPage = 1, longestPage = 1,  longestTime = 0, currentTime = 0,pageJump = PAGESIZE/ sizeof(int);
    double avgTime = 0;

    start = clock();
    //Jump by a page length
    for (int i = 0; currentPage < pages && i < arr.length; i += pageJump)
    {
        currentTime = measurePageAccess(arr, i);
        if (currentTime > longestTime)
        {
            longestTime = currentTime;
            longestPage = currentPage;
            if(i > 0){
                printf("Longest page yet %d , took %d\n", longestPage, longestTime);
            }
        }
        avgTime += currentTime;
        currentPage++;
    }
    end = clock();

    printf("Longest page  %d , took %d\n", longestPage, longestTime);
    printf("Avg page access %f\n", avgTime / pages);
    return end - start;
}

void setPageSize(){
    PAGESIZE = getpagesize();
}

void printCurrentTime(){
    struct timeval start;
    double secs = 0;
    gettimeofday(&start, NULL);
    secs = (double)(start.tv_usec) / CLOCKS_PER_SEC;
    printf("time taken %f\n",secs);
}

void runAccessTrials(int numberOfTrials,int pages,int sleepTime){
    struct Array a;
    for (int i = 0; i < numberOfTrials; ++i) {
        a = initializeArray(pages);
        printf("\nTrial number %d\n",i+1);
        accessPage(pages,a);
        sleep(sleepTime);
    }
}


int main(int argc, char **argv)
{
    int trials = 1,pages = 250,sleep=2;


    if(argc>1){
        trials = atoi(argv[1]);
        printf("%s\n", *argv);
    }
    if(argc >2){
        pages = atoi(argv[2]);
    }
    if(argc >3){
        sleep = atoi(argv[3]);
    }

    setPageSize();
    runAccessTrials(trials,pages,sleep);
    printf("total time: %lu μs\n", clock());
    return 0;
}