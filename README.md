# Submatrix-in-Parallel

Identifying the submatrix of 2x2 with the largest sum of values:
Programmer: Rocio Salguero 
Published: 12/1/17

Problem: Find the submatrix z x z (default 2 x 2 here) in an n x n matrix 
parameters: n > z > 0
p - number of processes
p <= (n + (z - 1)*(p - 1)) / p => p <= (n - z + 1)

2D -> 1D [row][col] -> [row * n + col]

//Docker https://hub.docker.com/r/nlknguyen/alpine-mpich/
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich

compile: mpicc project2.c -o <name>
run: mpirun -n <number of processes> ./<name> <filename>

row/col start is 0. Relative matrix [indexrow][indexcol]
