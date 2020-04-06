#include "alloc.h"

/* Code to allocate page of 4KB size with mmap() call and
* initialization of other necessary data structures.
* return 0 on success and 1 on failure (e.g if mmap() fails)
*/
int init()
{// Write your code below
	addr = mmap((void*)NULL, (size_t)PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANON|MAP_PRIVATE,0,0);
	if (addr == MAP_FAILED) return -1;
	else return 0;
	for(int i=0;i<PAGESIZE/MINALLOC;i++)  free_list[i] = allocated[i] = 0;
}

/* optional cleanup with munmap() call
* return 0 on success and 1 on failure (if munmap() fails)
*/
int cleanup()
{
	for(int i=0;i<PAGESIZE/MINALLOC;i++)  free_list[i] = 0;
	return munmap(addr,PAGESIZE);
}

/* Function to allocate memory of given size
* argument: bufSize - size of the buffer
* return value: on success - returns pointer to starting address of allocated memory
*               on failure (not able to allocate) - returns NULL
*/
char *alloc(int bufSize)
{// write your code below
	int num = bufSize/8;
	if(bufSize%8 != 0) return NULL;
	else{ 
		int i,j = 0;
		for(i;i<(int)PAGESIZE/MINALLOC;i++){
			if(free_list[i] == 0){
				for(j=0;j<num;j++){
					if(free_list[i+j] == 0) continue;
					else i= i+j; break;
				}
				if(j == num) {
					for (int k = i; k<i+num; k++) free_list[k] = 1;
					allocated[i] = num;
					return addr+i*8;
				}
			}
		}
	}
	return NULL;
}


/* Function to free the memory
* argument: takes the starting address of an allocated buffer
*/
void dealloc(char *memAddr)
{
	// write your code below
	int i = (int)(memAddr - addr)/8;
	int num = allocated[i];
	allocated[i] = 0;
	for(int j = i;j<i+num;j++){
		free_list[j] = 0;
	}
}


// int main(int argc, char* argv[]){
// 	printf("%i\n",init());
// 	printf("%i\n",cleanup());
// }