//************************************************************************
// Function to find RMS between two vectors of same length
//************************************************************************

double VecRMS(int N,double U[N],double V[N]){

	int i,j,dest,source=0;
	int rowsleft;
	rowsleft=N;
	int rowcount = 0;  // begin with first row      	
	int loops;
	loops=nslaves;
	if (N < nslaves) loops=N;
	int nwhile = 0;
	double sum=0.0,slave_sum=0.0,RMS;
	
	// all processors should receive loops and initialization, including the master
	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows are done 
		// Root (Master) process
		if (ProcessID == 0) {
			srand ( time(NULL) );
	    		// Calculation details are assigned to slave tasks. Process 1 onwards;
    			// Each message's tag is 1
    			for (i=1; i <= loops; i++){
    				int dest = i;
 	    			// Acknowledging the rowcount
				MPI_Send(&rowcount, 1, MPI_INT, dest, 1, MPI_COMM_WORLD); //sending rowcount so you can get it back later or change this.
      				// Send rows
      				MPI_Send(&U[rowcount], 1, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
      				MPI_Send(&V[rowcount], 1, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    				rowcount++;
      			}
			// Root process waits untill the each slave process
			// has sent their calculated result with message tag 2
			// after slaves receive and do calculation in slave loop 
     			for (int i = 1;i <= loops; i++){
     				source=i; // cannot use loop parameter in MPI calls (different addresses in each slave machine)
//      				MPI_Recv(&rowcount, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status); // don't need rowcount sent back
      				MPI_Recv(&slave_sum, 1, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
      				sum = sum + slave_sum; //Add it up while you are here
			}
//			rowcount++; //rowcount was reset by last slave *** BUG *** only works if last slave sent was the last rowcount. Fix: commented out sending back rowcount -- not needed
		}
	  // Slave Processes 
  		if (ProcessID > 0  ) {
  	   		// Source process ID is defined
    			source = 0;
	    		// Slave process waits for the message buffers with tag 1, that Root process sent
	    		// Each process will receive and execute this separately on their processes
	    		// The slave process receives the row value sent by root process
	    		MPI_Recv(&rowcount, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
	    		// The slave process receives row 
	    		MPI_Recv(&U[rowcount], 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
	    		MPI_Recv(&V[rowcount], 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
	    		// Do the task
	    		for (j = 0; j < N; j++) {	//columns of A and B
	    			double diff = U[rowcount]-V[rowcount];
	        		slave_sum= diff*diff;
	        	}
	        	// let's see if it did it
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	    		int dest = 0; // sending to the master processor 
//	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD); // no need to send this back
	    		// Resulting matrix with calculated rows will be sent to root process
	    		MPI_Send(&slave_sum, 1, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
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
	// now to do a simple calculation to get the RMS
	if (ProcessID == 0){
		RMS = sqrt(sum/N);
	}
	return(RMS);
}

