/* Modification 1: only look at odd numbers */
/* Modification 2: replicate the first sqrt(n) numbers in each process
                   (no broadcast) */
/* Modification 3: reduce cache miss by exchanging the loops */

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "MyMPI.h"
#define MIN(a,b) ((a)<(b) ? (a) : (b))

int main(int argc, char *argv[])
{
	int	count, first, global_count, high_value, i, id, index, 
		low_value, n, m, loc, primes_size, sec, chunk, lv,
            	p, prime, size;
	double 	elapsed_time;
	char 	*marked, *primes;

	MPI_Init(&argc, &argv);

	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time = -MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (argc != 3) {
		if (!id) printf("Command line: %s <n> <chunk>\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}
	n = atoi(argv[1]);
	m = (n-3)/2 + 1;
	chunk = atoi(argv[2]);

	/* Figure out this process' share of the array. */
	low_value = 2 * BLOCK_LOW(id, p, m) + 3;
	high_value = 2 * BLOCK_HIGH(id, p, m) + 3;
	size = BLOCK_SIZE(id, p, m);

	/* Allocate this process' share of the array */
	marked = (char *) malloc(size);
	if (marked == NULL) {
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		exit(1);
	}
	for (i=0; i<size; i++) marked[i] = 0;

	primes_size = (sqrt(n) - 3)/2 + 1;
	primes = (char *) malloc(primes_size);
	if (primes == NULL) {
		printf("Cannot allocate enough memory\n");
		free(marked);
		MPI_Finalize();
		exit(1);
	}
	for (i=0; i<primes_size; i++) primes[i] = 0;

	index = 0;
	prime = 3;
	do {
		for (i = (prime*prime-3)/2; i < primes_size; i += prime)
			primes[i] = 1;
		while (primes[++index]);
		prime = 2*index + 3;
	} while (prime*prime <= sqrt(n));

	for (sec = 0; sec < size; sec += chunk) {
		index = 0;
		prime = 3;
		lv = 2*((low_value-3)/2+sec)+3;
		do {
			if (prime*prime > lv)
				first = (prime*prime-3)/2 - (lv-3)/2;
			else {
				loc = lv % prime;
				if (!loc) first = 0;
				else {
					first = prime - loc;
					if (!((lv+first)%2))
						first = (first+prime)/2;
					else first /= 2;
				}
			}
/*printf("process=%d size=%d sec=%d low_value=%d lv=%d first=%d prime=%d\n", id, size, sec, low_value, lv, first, prime);*/
			for (i = first+sec; i < first+sec+chunk && i < size; i += prime)
				marked[i] = 1;
			while (primes[++index]);
			prime = 2*index + 3;
		} while (prime*prime <= n);
	}
	
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
	free(primes);
	MPI_Finalize();
	return 0;
}