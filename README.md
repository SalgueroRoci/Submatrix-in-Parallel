# Submatrix-in-Parallel

Identifying the submatrix of 2x2 with the largest sum of values: </br>
Programmer: Rocio Salguero </br>
Published: 12/1/17 </br>

Problem: Find the submatrix z x z (default 2 x 2 here) in an n x n matrix in parallel </br>

//Docker https://hub.docker.com/r/nlknguyen/alpine-mpich/ </br>
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich </br>

compile: mpicc project2.c -o <name> </br>
run: mpirun -n <number of processes> ./<name> <filename> </br>

row/col start is 0. Relative matrix [indexrow][indexcol] </br>

MPI Programming Difficulties: 
  - MPI handles 1D arrays only </br>
  - Splitting up the array (How many elements exactly?) to all the proccess </br>
  - Sending a struct using MPI </br>
  
# parameters:  </br>
n > z > 0 </br>

2D -> 1D [row][col] -> [row * n + col] </br>
</br>
n - Matrix square size </br>
p - number of processes </br>
z - submatrix size  </br>

# Splitting up the number of rows each process will deal with </br>
(z - 1) - number of elements in overlap </br>
(p - 1) - number of overlaps </br>
(n + ((z - 1) * (p - 1)) - Amount of elements to divide (amt) </br>

floor( amt / p ) >= z therefore p <= n - z + 1 </br>

Split up array:  </br>
  r - number or processes to get certain amount </br>
  r = amt % p  </br>
  
  Ranks 0 - (r - 1) -> rowchunk = ceil( amt / p ) </br>
        r - (p - 1) -> rowchunk = floor( amt / p ) </br>
        
