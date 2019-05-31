#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "MyMPI.h"
#define MIN(a,b) ((a)<(b) ? (a) : (b))

int main(int argc, char *argv[])
{
	int count, first, global_count, high_value, i, id, index, low_value, n,
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

	/* Figure out this process' share of the array. */
	low_value = 2 + BLOCK_LOW(id, p, n-1);
	high_value = 2 + BLOCK_HIGH(id, p, n-1);
	size = BLOCK_SIZE(id, p, n-1);

	/* Bail out if all primes used for sieving are not all held in process 0 */
	proc0_size = (n-1)/p;
	if ((2 + proc0_size - 1) < (int) sqrt((double) n)) {
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
	prime = 2;
	do {
		if (prime*prime > low_value)
			first = prime*prime - low_value;
		else if (!(low_value % prime)) first = 0;
		     else first = prime - (low_value % prime);
		for (i=first; i <size; i+=prime)
			marked[i] = 1;
		if (!id) {
			while (marked[++index]);
			prime = index + 2;
		}
		MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
	} while (prime*prime <= n);

	count = 0;
	for (i=0; i<size; i++)
		if (!marked[i]) count++;
	MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	elapsed_time += MPI_Wtime();

	if (!id) {
		printf("%d primes are less than or equal to %d\n", global_count, n);
		printf("Total elapsed time: %10.6f\n", elapsed_time);
	}
	free(marked);
	MPI_Finalize();
	return 0;
}