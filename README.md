# Parallel-and-Distributed-Computing-MPI-Experiment

##  Introduction
UESTC-《Parallel and Distributed Computing》Course Experiment:Implements of Sieve of Eratosthenes,with modification and improvement

## Environment
Base on Windows 10,MinGW-W64 gcc version 8.1.0 and MicrosoftMPI(MSMPI)

## File Description
origin.c:The origin source code provided(corrected some bugs)

modification1.c:Only look at odd numbers

modification2.c:Based on modification1,replicate the first sqrt(n) numbers in each process(no broadcast)

modification3.c:Based on modification1 and 2,reduce cache miss by exchanging the loops
