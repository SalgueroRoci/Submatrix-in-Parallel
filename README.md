# Submatrix-in-Parallel

Identifying the submatrix of 2x2 with the largest sum of values: </br>
Programmer: Rocio Salguero </br>
Published: 12/1/17 </br>

### Problem: 
    Find the submatrix z x z (default 2 x 2 here) in an n x n matrix in parallel 

### Compiling
//Docker https://hub.docker.com/r/nlknguyen/alpine-mpich/ </br>
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich </br>

compile: mpicc project2.c -o \<name\> </br>
run: mpirun -n \<number of processes\> ./\<name> \<filename\> </br>

row/col start is 0. Relative matrix [indexrow][indexcol] </br>

# Parameters:  </br>
n > z > 0 </br>

2D -> 1D \[row]\[col] -> \[row * n + col] </br>
</br>
n - Matrix square size </br>
p - Number of processes </br>
z - Submatrix size  </br>

## Splitting up the number of rows each process will deal with </br>
(z - 1) - number of elements in overlap </br>
(p - 1) - number of overlaps </br>
(n + ((z - 1) * (p - 1)) - Amount of elements to divide (amt) </br>

floor( amt / p ) >= z therefore p <= n - z + 1 </br>

Split up array:  </br>
  r - number or processes to get certain amount </br>
  r = amt % p  </br>
  
  Ranks 0 - (r - 1) -> rowchunk = ceil( amt / p ) </br>
        r - (p - 1) -> rowchunk = floor( amt / p ) </br>

### MPI Programming Difficulties: </br>
  - MPI handles 1D arrays only </br>
  - Splitting up the array (How many elements exactly?) to all the proccess </br>
  - Sending a struct using MPI </br>
  
# Pseudocode
```c
if ( rank == 0 ) { 
        read_file(file_name); 
        //get n; 
        A[n * n] //store in 1D matrix[n * n] 
} 

B_Cast(n, 0) //broadcast the n value to all other proces from rank 0 
amt = n + ((z - 1) * (p - 1))  

get subsize_z, rowchunk 
subsize_z = 2; 
amt_to_split = size_n + ((subsize_z -1) * (proc_size - 1)); 
r = amt_to_split % proc_size; 

if (rank < r) { 
    rowchunk = ceil( (float)amt_to_split / (float)proc_size);       
} 
else if (rank >= r) { 
    rowchunk = floor((float)amt_to_split / (float) proc_size); 
} 
    
if ( rank == 0 )  { 
    startRow = 0; 
    rowchunk = ceil( amt / p ) 
    prevend = startRow + rowchunk - 1; 
    
    for ( proc = 0 < r : proc++ ) { 
        startRow = prevend - ( z - 2 ) 
        prevend = startRow + rowchunk - 1 
        send( startRow, proc ) //send the start row value to the process 
        send ( A[ startRow * n ] , rowchunk * n , proc ) 
    }  
    rowchunk = floor( amt / p ) 
    for ( proc = r < p; proc++ ) { 
        startRow = prevend - ( z - 2 )  
        prevend = startRow + rowchunk - 1 
        send( startRow, proc ) //send the start row value to the process 
        send ( A[ startRow * n ] , rowchunk * n , proc ) 
    }  
    //reset for process 0  
    startRow = 0 ;  
    rowchunk = ceil( amt / p )  
} //end of rank 0    
else { 
      recv(startRow, 0 ) 
      recv( A[0], rowchunk * n, 0 )  
}

//calculate sum / all procces calculate sum 
start = startRow * n; 
endRow = startRow + rowchunk - 1; 
maxSum = -9999; maxRow = 0 ; maxCol = 0; 
for( row = startRow; row < endRow - z + 2; row++ ) 
    for ( col = 0; col < n - z + 1 ; col++) 
            //sum of matrix 
            sum = 0 ; 
            for( k = row; k < row + z' k++) 
                for( j = col; j < col + z; j++) 
                   sum += A[n * k + j - start] 

                    if( sum > maxSum ) 
                        maxSum = sum; 
                        maxRow = row; 
                        maxCol = col; 
              
Gather( max, 0 ) // gather from all processes maxSum,maxRow, maxCol from each on process 0

if ( rank == 0 ) { 
      for ( i = 0 ; i < p) 
          //find max among max 
     //print max 
} 
``` 
