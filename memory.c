#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long int currentMemory;
long int peakMemory;

void printMemoryUsage() {
	//printf("Memory usage: %ld B (peak: %ld B)\n", currentMemory, peakMemory);
}

void * allocateMemory(size_t nbr_of_elements, size_t size) {
	size_t totalSize = size*nbr_of_elements + sizeof(size_t);
	void *ptr = malloc(totalSize);
	if (!ptr) {
		printf("Failed to allocate memory\n");
		return NULL;
	}
	*((size_t *)ptr) = size;
	currentMemory += size;
	printf("Allocated %ld bytes\n", size);
	if (currentMemory > peakMemory) {
		peakMemory = currentMemory;
	}
	return (char *)ptr + sizeof(size_t);
}

void freeMemory(void *ptr) {
	if (!ptr) return;
	void *realPtr = (char *)ptr - sizeof(size_t);
	size_t size = *((size_t *)realPtr);
	free(realPtr);
	currentMemory -= size;
}