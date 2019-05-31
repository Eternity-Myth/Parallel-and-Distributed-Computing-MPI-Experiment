/* Modification 1: only look at odd numbers */

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "MyMPI.h"
#define MIN(a,b) ((a)<(b) ? (a) : (b))

int main(int argc, char *argv[])
{
	int count, first, global_count, high_value, i, id, index, low_value, n, m, loc,
            p, proc0_size, prime, size;
	double elapsed_time;
	char *marked;

	MPI_Init(&argc, &argv);

	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time = -MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (argc != 2) {
		if (!id) printf("Command line: %s <n>\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}
	n = atoi(argv[1]);
	m = (n-3)/2 + 1;

	/* Figure out this process' share of the array. */
	low_value = 2 * BLOCK_LOW(id, p, m) + 3;
	high_value = 2 * BLOCK_HIGH(id, p, m) + 3;
	size = BLOCK_SIZE(id, p, m);
/*
printf("Process %d: low_val=%d hi_val=%d size=%d\n", id, low_value, high_value, size);
*/


	/* Bail out if all primes used for sieving are not all held in process 0 */
	proc0_size = m/p;
	if ((2*(proc0_size-1) + 3) < (int) sqrt((double) n)) {
		if (!id) printf("Too many processes\n");
		MPI_Finalize();
		exit(1);
	}

	/* Allocate this process' share of the array */
	marked = (char *) malloc(size);
	if (marked == NULL) {
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		exit(1);
	}

	for (i=0; i<size; i++) marked[i] = 0;
	if (!id) index = 0;
	prime = 3;
	do {
		if (prime*prime > low_value)
			first = (prime*prime-3)/2 - (low_value-3)/2;
		else {
			loc = low_value % prime;
			if (!loc) first = 0;
			else {
				first = prime - loc;
				if (!((low_value+first)%2))
					first = (first+prime)/2;
				else first /= 2;
			}
		}
/*
printf("P%d: prime=%d first=%d\n", id, prime, first);
*/
		for (i=first; i <size; i+=prime)
			marked[i] = 1;
		if (!id) {
			while (marked[++index]);
			prime = 2*index + 3;
		}
		MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
	} while (prime*prime <= n);

	count = 0;
	for (i=0; i<size; i++)
		if (!marked[i]) count++;
	MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	elapsed_time += MPI_Wtime();

	if (!id) {
		printf("%d primes are less than or equal to %d\n", global_count+1, n);
		printf("Total elapsed time: %10.6f\n", elapsed_time);
	}
	free(marked);
	MPI_Finalize();
	return 0;
}