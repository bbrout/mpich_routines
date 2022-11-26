//**********************************************
// Function to multiply a matrix times a vector
//**********************************************

void	MatVec(int N,int acols,double MATA[N][acols], double VEC[acols],double VECOUT[N]){

	int rowcount = 0;  // begin with first row  
	    	
	int loops, i,k;
	int source,dest;
	int na,nv,nvout;

	loops = nslaves;
	if (N < nslaves) loops=N;
	
	int nwhile = 0;
	int rowsleft;
	rowsleft = N;
	
	source = 0;
	
	na = N*acols;
	nv = acols;
	nvout=N;

//	printf("Processor %d entering MatVec\n",ProcessID);

	// Send vector to all slaves tag = 4
	if (ProcessID == 0){
//		printf("MatVec: Processor %d beginning send loop. Loops = %d\n",ProcessID,loops);
		for (i=1;i<=loops;i++){
			dest = i; // cannot use loop parameter in MPI calls
//			printf("MatVec: Processor %d about to send row %d length %d to processor %d\n",ProcessID,rowcount,acols,dest);
			MPI_Send(&VEC[0], nv, MPI_DOUBLE, dest, 4, MPI_COMM_WORLD);		
//			printf("MatVec: Processor %d sent vector row %d length %d\n",ProcessID,rowcount,nv);
		}
	}
	if (ProcessID > 0 && ProcessID <=loops){
			source=0;
			MPI_Recv(&VEC[0], nv, MPI_DOUBLE, source, 4, MPI_COMM_WORLD,&status);
//			printf("MatVec: Processor %d received vectorow %d length %d\n",ProcessID,rowcount,nv);
	}
	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
		// Root (Master) process
		if (ProcessID == 0) {
			srand ( time(NULL) );
	    		// Calculation details are assigned to slave tasks. Process 1 onwards;
    			// Each message's tag is 1
    			for (i=1; i <= loops; i++){
    				dest = i;
 	    			// Acknowledging the rowcount of the Matrix A
				MPI_Send(&rowcount, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      				// Send rows of the Matrix A and all of Martrix B
      				MPI_Send(&MATA[rowcount][0], acols, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
//   				printf("MatVec: Processor %d sent row %d length %d\n",ProcessID,rowcount,acols);
    				rowcount++;
      			}
			// Root process waits untill the each slave process
			// has sent their calculated result with message tag 2
			// after slaves receive and do calculation in slave loop 
     			for (int i = 1;i <= loops; i++){
     				source=i; // cannot use loop parameter in MPI calls (different addresses in each slave machine)
    				// Receive the offset of particular slave process
      				MPI_Recv(&rowcount, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      				// Calculated rows of the each process will be stored
      				// in Matrix C according to their rowcount
      				MPI_Recv(&VECOUT[rowcount], 1, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
			}
			rowcount++; //rowcount was reset by last slave
		}
	  // Slave Processes 
  		if (ProcessID > 0 && ProcessID <= loops ) {
  	   		// Source process ID is defined
    			source = 0;
	    		// Slave process waits for the message buffers with tag 1, that Root process sent
	    		// Each process will receive and execute this separately on their processes

	    		// The slave process receives the row value sent by root process
	    		MPI_Recv(&rowcount, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
	    		// The slave process receives the sub portion of the Matrix A which assigned by Root 
	    		MPI_Recv(&MATA[rowcount][0], acols, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
//			printf("MatVec: Processor %d received row %d length %d\n",ProcessID,rowcount,acols);
        		// Set initial value of the row summataion
        		VECOUT[rowcount] = 0.0;
        		// Matrix A's element(i, j) will be multiplied
        		//  with Matrix B's element(j, k)
        		for (k = 0;k < acols; k++){
        		 	VECOUT[rowcount] = VECOUT[rowcount] + MATA[rowcount][k] * VEC[k];
        		 }
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	   		// by sending the starting point of the calculated
	    		// value in matrix C
	    		dest = 0; // sending to the master processor tag = 2
	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
	    		// Resulting matrix with calculated rows will be sent to root process
	    		MPI_Send(&VECOUT[rowcount], 1, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
//	    		printf("	MatVec: Processor %d sent row %d length %d\n",ProcessID,rowcount,1);
	  	}
	  	//All processors access this line
	  	//How many rows are left to process?
    		nwhile++;
	  	rowsleft=rowsleft-loops;
	  	if (ProcessID > rowsleft) rowsleft = 0; //Processor not needed and can leave the while loop
	  	if (ProcessID <= rowsleft && rowsleft < nslaves){
	  	 	loops=rowsleft;
	  	 }
//	  	 printf("Processor %d has rowsleft = %d nwhile = %d loops = %d\n",ProcessID,rowsleft,nwhile,loops);
	  	 
	} //end of while loop
//	printf("Processor %d has left the while loop\n",ProcessID);
}

