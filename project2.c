/*
Programmer: Rocio Salguero 
Published: 12/1/17

Problem: Find the submatrix z x z (default 2 x 2 here) in an n x n matrix 
parameters: n > z > 0
p - number of processes
p <= (n + (z - 1)*(p - 1)) / p => p <= (n - z + 1)

2D -> 1D [row][col] -> [row * n + col]

C Program to Illustrate Reading of Data from a File 
http://www.sanfoundry.com/c-program-illustrate-reading-datafile/

Tutorials used: 
MPI - Sending segments of an array
https://stackoverflow.com/questions/15483360/mpi-sending-segments-of-an-array

How to send a integer array via MPI_Send?
https://stackoverflow.com/questions/46437458/how-to-send-a-integer-array-via-mpi-send

How to send part of an array MPI: 
https://www.nics.tennessee.edu/files/pdf/MPI_WORKSHOP/array.c.soln

struct serialization in C and transfer over MPI
https://stackoverflow.com/questions/9864510/struct-serialization-in-c-and-transfer-over-mpi

Program that computes the average of an array of elements in parallel using
MPI_Scatter and MPI_Gather
https://github.com/wesleykendall/mpitutorial/blob/gh-pages/tutorials/mpi-scatter-gather-and-allgather/code/avg.c

Proper way to receive a data structure with MPI_Gather
https://stackoverflow.com/questions/14673886/proper-way-to-receive-a-data-structure-with-mpi-gather

MPI_reduce() with custom Datatype
https://stackoverflow.com/questions/13434879/mpi-reduce-with-custom-datatype-containing-dynamically-allocated-arays-segme

MPI Data Types
https://www.rc.colorado.edu/sites/default/files/Datatypes.pdf

//Docker https://hub.docker.com/r/nlknguyen/alpine-mpich/
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich

compile: mpicc project2.c -o <name>
run: mpirun -n <number of processes> ./<name> <filename>

*/ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <mpi.h>

typedef struct SubMax {
   int maxSum;
   int maxRow; 
   int maxCol; 
} SubMax;

void CreateMyTypeMPI(SubMax *mt, MPI_Datatype *MyTypeMPI)
{
     /* create a type for struct car */
    const int nitems =3;
    int          blocklengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint     offsets[3];
    MPI_Aint intex;
    MPI_Type_extent(MPI_INT, &intex);

    offsets[0] = 0;
    offsets[1] = intex;
    offsets[2] = intex+intex;

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, MyTypeMPI);
    MPI_Type_commit(MyTypeMPI);
}

int main( int argc, char *argv[] )
{
        
    int rank, proc_size, size_n, subsize_z, r, amt_to_split, rowchunk,
        rowstart, startindex, maxproc;
    int *subMatrix , *mainmatrix = NULL;
    SubMax submax;
    MPI_Datatype max_type;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &proc_size );
    
    //check if number of arguements is correct
    if(argc < 2) {
        if (rank == 0) 
            printf("Usage: %s <filename>\n", argv[0]);
        MPI_Barrier(MPI_COMM_WORLD);
        exit(0);
    }

    //if rank zero get n and matrix
    if(rank == 0) {        
        /*  open the file for reading */
        FILE *fptr;             
        int MAXBUFFER = 100; 
        char buffer[MAXBUFFER];
        
        fptr = fopen(argv[1], "r");
        if (fptr == NULL)
        {
            printf("Cannot open file \n");
            exit(0);
        }
        
        //get the size n 
        int i = 0;
        char ch;  
        while ( i < MAXBUFFER && !isspace(ch = fgetc(fptr)) && (ch != EOF) ) {
            buffer[i++] = ch; 
        }
        size_n = atoi(buffer); 

        //create matrix 
        mainmatrix = (int *)malloc(sizeof(int) * size_n * size_n);

        printf("proc zero got n = %d\n", size_n);

        //Store Matrix from file 
        int j = 0; 
        while (ch != EOF )
        {
            i = 0;
            while(i < MAXBUFFER && !isspace(ch = fgetc(fptr)) && (ch != EOF)){
                buffer[i++] = ch;
            }
            buffer[i] = '\0';

            if (ch == EOF) break;
            if(i > 0) {
                int n = atoi(buffer); 
                mainmatrix[j] = n; 
                j++;
            }           
        }
        fclose(fptr);
    }

    //broadcast the matrix size n x n 
    MPI_Bcast(&size_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    //check if too many procsess to do later
    if(proc_size > size_n - subsize_z + 1) {
        // Clean up
        if (rank == 0) {
            printf("Too many proccess. Please use <= %d\n", size_n - subsize_z + 1);
            free(mainmatrix);
        }
        free(subMatrix);
        MPI_Finalize();
        return 0;
    }

    //get subsize_z, rowchunk 
    subsize_z = 2; 
    amt_to_split = size_n + ((subsize_z -1) * (proc_size - 1));
    r = amt_to_split % proc_size; 
    if (rank < r) {
        rowchunk = ceil( (float)amt_to_split / (float)proc_size);        
    }
    else if (rank >= r) {
        rowchunk = floor((float)amt_to_split / (float) proc_size);
    }
    
    //start distributing array. Each process needs starting index 
    if(rank == 0) {
        rowstart = 0; 
        rowchunk = ceil( (float)amt_to_split / (float)proc_size);
        int prevend = rowstart + rowchunk -1;
        int proc; 
        for (proc = 1; proc < r ; proc++) {
            rowstart = prevend - (subsize_z-2); 
            prevend = rowstart + rowchunk -1;
            //printf("sending to  %d row: %d\n", proc, rowchunk); 
            MPI_Send(&rowstart, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
            MPI_Send(&mainmatrix[rowstart * size_n], size_n*rowchunk, MPI_INT, proc, 0, MPI_COMM_WORLD);
        }

        rowchunk = floor((float)amt_to_split / (float) proc_size);
        for(proc = r; proc < proc_size; proc++) {
            if ( proc == 0 ) continue; 
            rowstart = prevend - (subsize_z-2); 
            prevend = rowstart + rowchunk -1;
            //printf("sending to  %d row: %d\n", proc, rowchunk); 
            MPI_Send(&rowstart, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
            MPI_Send(&mainmatrix[rowstart * size_n], size_n*rowchunk, MPI_INT, proc, 0, MPI_COMM_WORLD);
        }

        //reset proc zero 
        rowstart = 0;
        rowchunk = ceil( (float)amt_to_split / (float)proc_size);

        //get own chunk
        subMatrix = (int *)malloc(sizeof(int) * size_n * rowchunk); 
        for(int i = 0; i < size_n * rowchunk; i++) {
            subMatrix[i] = mainmatrix[i]; 
        }
    }
    else {
        subMatrix = (int *)malloc(sizeof(int) * size_n * rowchunk); 
        MPI_Recv(&rowstart, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("rank %d got rowstart %d - ", rank, rowstart);
        MPI_Recv(&subMatrix[0], size_n*rowchunk , MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("recieved %d elements from mainmatrix[%d] to mainmatrix[%d]\n", rank, rowstart*size_n, size_n*rowchunk+startindex-1); 
    }
    
    //printf("Im proc %d and start is %d and chuck is %d\n", rank, rowstart, rowchunk);

    //subMatrix, rowstart, rowchunk, startindex, and rank have value
    startindex = rowstart*size_n;

    //start calculating maxsum 
    int endRow = rowstart + rowchunk - 1; 
    submax.maxSum = -9999; submax.maxRow = -1; submax.maxCol = -1; 
    for(int row = rowstart; row < endRow - subsize_z + 2 ; row++) {
        for(int col = 0; col < size_n - subsize_z + 1 ; col++) {
            //printf("rank %d matrix[%d][%d] 1D: %d\n", rank, row, col, size_n * row + col);
            
            //sum the sub matrix 
            int sum = 0; 
                for(int k = row; k < row + subsize_z; k++) {
                    for(int j = col; j < col + subsize_z; j++) {
                        sum += subMatrix[size_n * k + j - startindex];
                    }
                }

                if(sum > submax.maxSum) {
                    submax.maxSum = sum; 
                    submax.maxRow = row; 
                    submax.maxCol = col;
                }            
        }
    }
    CreateMyTypeMPI(&submax, &max_type);

    SubMax *sub_maxs = NULL;
    if (rank == 0) {
        sub_maxs = malloc(sizeof(SubMax) * proc_size);
    }
    
    MPI_Gather(&submax, 1, max_type, sub_maxs, 1, max_type, 0, MPI_COMM_WORLD);
    //figuire out how to gather info to rank 
    SubMax final; 
    final.maxRow = -1; final.maxCol = -1; final.maxSum = -99999; 
    if(rank == 0) {
        for(int i = 0; i < proc_size; i++) {
            if(sub_maxs[i].maxSum > final.maxSum) {
                final.maxSum = sub_maxs[i].maxSum; 
                final.maxRow = sub_maxs[i].maxRow; 
                final.maxCol = sub_maxs[i].maxCol; 
            }
        }
        printf("Max submatrix %d x %d is %d at matrix[%d][%d]\n", 
            subsize_z, subsize_z, final.maxSum, final.maxRow, final.maxCol);
    }    
    
    // Clean up
    if (rank == 0) {
        free(mainmatrix);
    }
    free(subMatrix);
    free(sub_maxs);
    MPI_Finalize();
    return 0;
}
