//******************************************************************************
// Function to multiply two vectors, component by component, into another vector
//******************************************************************************

void VecMult(int N, double VECA[N],double VECB[N],double VECC[N]){

	int source;
	int rowcount = 0;  // begin with first row of matrix A      	
	int loops = nslaves;
	if (N < nslaves) loops=N;
	int nwhile = 0;
	int rowsleft;
	rowsleft = N;
	int i,j;


	// all processors should receive loops and initialization, including the master
	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all indexes of the vectors been multiplied out (rowcount starts counting at 0 not 1) 
		// Root (Master) process
		if (ProcessID == 0) {
			srand ( time(NULL) );
	    		// Calculation details are assigned to slave tasks. Process 1 onwards;
    			// Each message's tag is 1
    			for (i=1; i <= loops; i++){
    				int dest = i;
 	    			// Acknowledging the rowcount of the Matrix XTXINV
				MPI_Send(&rowcount, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      				MPI_Send(&VECA[rowcount], 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
      				MPI_Send(&VECB[rowcount], 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
//				printf("Processor %d sent to %d rowcount %d\n",ProcessID,dest,rowcount);
    				rowcount++;
      			}
			// Root process waits untill the each slave process
			// has sent their calculated result with message tag 2
			// after slaves receive and do calculation in slave loop 
     			for (int i = 1;i <= loops; i++){
     				source=i; // cannot use loop parameter in MPI calls (different addresses in each slave machine)
    				// Receive the index number
      				MPI_Recv(&rowcount, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      				MPI_Recv(&VECC[rowcount], 1, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
      				// Calculated rows of the each process will be stored
      				// in coiefficient vector according to their rowcount
			}
//			printf("Processor %d received from %d rowcount %d\n",ProcessID,source,rowcount);
			rowcount++; //rowcount was reset by last slave
		}
	  // Slave Processes 
  		if (ProcessID > 0 && ProcessID <= loops ) {
  	   		// Source process ID is defined
    			source = 0;
	    		// Slave process waits for the message buffers with tag 1, that Root process sent
	    		// Each process will receive and execute this separately on their processes
	    		// The slave process receives the row value sent by root process
	    		MPI_Recv(&rowcount, 1, MPI_INT, source, 1, MPI_COMM_WORLD,&status);
			MPI_Recv(&VECA[rowcount], 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD,&status);
			MPI_Recv(&VECB[rowcount], 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD,&status);
//	    		printf("Processor %d received rowcount %d\n",ProcessID,rowcount);
	    		// The slave process receives the sub portion of the Matrix A which assigned by Root 
			VECC[rowcount]=VECA[rowcount]*VECB[rowcount];
	        	// let's see if it did it
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	    		int dest = 0; // sending to the master processor 
	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
	    		MPI_Send(&VECC[rowcount], 1, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
//	    		printf("Processor %d sent rowcount %d\n",ProcessID,rowcount);
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
//	  	 printf("Processor %d has left the while loop\n",ProcessID);
}

