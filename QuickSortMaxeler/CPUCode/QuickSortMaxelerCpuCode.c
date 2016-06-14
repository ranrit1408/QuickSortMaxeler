#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Maxfiles.h"
#include <MaxSLiCInterface.h>
#define DATA_SIZE 256
#define SWAP(a,b) {int temp; temp = a; a = b; b = temp;}

double data[DATA_SIZE];
void quick_sort(double *, int);
void quick_sort_sub(double *, int, int);
void quick_sort(double *data, int size)
{
    quick_sort_sub(data, 0, size - 1);
}

void quick_sort_sub(double *data, int left, int right)
{
    int left_index = left;
    int right_index = right;
    int pivot = data[(left + right) / 2];

    while (left_index <= right_index) {
        for ( ; data[left_index] < pivot; left_index++)
        	;

        for ( ; data[right_index] > pivot; right_index--)
            ;

        if (left_index <= right_index) {
            SWAP(data[left_index], data[right_index]);
            left_index++;
            right_index--;
        }
    }

    if (right_index > left) {
        quick_sort_sub(data, left, right_index);
    }
    if (left_index < right) {
        quick_sort_sub(data, left_index, right);
    }
}

void printData(){
	for (int i = 0; i < DATA_SIZE; i++) {
	           printf("contents[%i]=%1.8g\n",i,data[i]);
	       }
}

void generateInputData(
	int size,
	uint8_t *inStreamAddressA,
	uint8_t *inStreamAddressB,
	double *romContents,
	double *romContentsOrdered)
{
	for (int i = 0; i < size; i++) {
		//Address for the Data Generation in this time is sequencial.
		inStreamAddressA[i] = i;
		//Set to 0 all the Address for Ordered Data.(Reset)
		inStreamAddressB[i] = 0;
		//printf("inStreamAddressA[%i]=%i   --- inStreamAddressB[%i]=%i\n",i,inStreamAddressA[i],i,inStreamAddressB[i]);
	}

	for (int i = 0; i < size; i++) {
		// Generate Random Data with the size
		romContents[i] = rand()%size+1;
		// set to 0 All the Ordered Data
		romContentsOrdered[i] = 0;

	}

}

int check(
	int size,
	float *outStreamDataA,
	float *outStreamDataB,
	double *romContents,
	double *romContentsOrdered)
{
	int status = 0;
	for (int i = 0; i < size; ++i) {
		if (fabs(outStreamDataB[i] - romContentsOrdered[i]) > 0.01) {
			fprintf(stderr, "%d => %1.8g\n", i, outStreamDataA[i]);
			fprintf(stderr, "ERROR, expected %1.8g\n", romContentsOrdered[i]);
			status = 1;
		}
		if (fabs(outStreamDataA[i] - romContents[i]) > 0.01) {
				fprintf(stderr, "%d => %1.8g\n", i, outStreamDataB[i]);
				fprintf(stderr, "ERROR, expected %1.8g\n", romContents[i]);
				status = 1;
		}
	}

	return status;
}

int main()
{
	const int size = DATA_SIZE;
	int sizeBytesFloat = size * sizeof(float);
	int sizeBytesDouble = size * sizeof(double);
	int sizeBytesInt = size * sizeof(uint8_t);
	uint8_t *inAddressA = malloc(sizeBytesInt);
	uint8_t *inAddressB = malloc(sizeBytesInt);
	double *romContents = malloc(sizeBytesDouble);
	double *romContentsOrdered = malloc(sizeBytesDouble);
	float *outDataA = malloc(sizeBytesFloat);
	float *outDataB = malloc(sizeBytesFloat);

	generateInputData(
		size,
		inAddressA, inAddressB,
		romContents, romContentsOrdered);

	//loadData in Data Array
	for (int i = 0; i < size; i++) {
			data[i] = romContents[i];
		}
	//QuickSort using C
	clock_t start = clock();
	quick_sort(data, size);
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	//printData(data);
	printf("Execution Time for %i elements using QuickSort with C implementation: %4.5f seconds\n",DATA_SIZE,seconds);

	//Load Address of the Ordered Results to be used in Kernel
	for (int i = 0; i < size; i++){
		romContentsOrdered[i]=data[i];
		for(int j =0; j< size; j++){
			if(romContents[j]==data[i]){
				inAddressB[i]=j;
			}
		}
	}

	printf("Running DFE.\n");
	start = clock();
	QuickSortMaxeler(
		size,
		inAddressA,	inAddressB,
		outDataA, outDataB,
		romContents);
	end = clock();
	seconds = (float)(end - start) / CLOCKS_PER_SEC;
	printf("Execution Time for %i elements using QuickSort using DFE Kernel implementation: %4.5f seconds\n",DATA_SIZE,seconds);


	int status = check(
		size,
		outDataA, outDataB,
		romContents, romContentsOrdered);

	if (status)
		printf("Test failed.\n");
	else
		printf("Test passed OK!\n");

	return status;
}
