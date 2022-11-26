//********************************************
// function to copy one matrix into another
//********************************************

void	MatCopy(int N,double A[N][N],double B[N][N]){

	int source = 0;
	int rowcount = 0;  // begin with first row of matrix A      	
	int loops = nslaves;
	if (ncoef < nslaves) loops=ncoef;
	int nwhile = 0;
	int rowsleft;
	rowsleft = N;
	int i,j;
	// all processors should receive loops and initialization, including the master
	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
		// Root (Master) process
		if (ProcessID == 0) {
			srand ( time(NULL) );
	    		// Calculation details are assigned to slave tasks. Process 1 onwards;
    			// Each message's tag is 1
    			for (i=1; i <= loops; i++){
    				int dest = i;
 	    			// Acknowledging the rowcount of the Matrix XTXINV
				MPI_Send(&rowcount, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      				// Send rows of the Matrix A and all of Martrix B
      				MPI_Send(&A[rowcount][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
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
      				// in coiefficient vector according to their rowcount
      				MPI_Recv(&B[rowcount][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
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
	    		MPI_Recv(&A[rowcount][0], N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
	    		for (j = 0; j < N; j++) {	//columns of A and B
	        		B[rowcount][j] = A[rowcount][j];
	        	}
	        	// let's see if it did it
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	   		// by sending the starting point of the calculated
	    		// value in matrix C
	    		int dest = 0; // sending to the master processor 
	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
	    		// Resulting matrix with calculated rows will be sent to root process
	    		MPI_Send(&B[rowcount], N, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
	  	}
	  	//All processors access this line
	  	//How many rows are left to process?
    		nwhile++;
	  	rowsleft=rowsleft-loops;
	  	if (ProcessID > rowsleft) rowsleft = 0; //Processor not needed and can leave the while loop
	  	if (ProcessID <= rowsleft && rowsleft < nslaves){
	  	 	loops=rowsleft;
	  	 }
	} //end of while loop
}

