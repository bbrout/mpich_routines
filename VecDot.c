//**************************************
// Function to dot products two vectors
//**************************************

double VecDot(int N,double VECA[N],double VECB[N]){

	double slave_sum,sum;
	
	int rowcount = 0;  // begin with first row      	
	int loops, i,j;
	int source,dest;


	loops = nslaves;
	if (N < nslaves) loops=N;
	
	int nwhile = 0;
	int rowsleft;
	rowsleft = N;
	
	source = 0;

	// Send vector to all slaves tag = 3
	if (ProcessID == 0){
		for (i=1;i<=loops;i++){
			dest = i; // cannot use loop parameter in MPI calls
			MPI_Send(&VECA[0], N, MPI_DOUBLE, dest, 3, MPI_COMM_WORLD);		
			MPI_Send(&VECB[0], N, MPI_DOUBLE, dest,3, MPI_COMM_WORLD);		
		}
	}
	if (ProcessID > 0 && ProcessID <=loops){
			source=0;
			MPI_Recv(&VECA[0], N, MPI_DOUBLE, source, 3, MPI_COMM_WORLD,&status);
			MPI_Recv(&VECB[0], N, MPI_DOUBLE, source, 3, MPI_COMM_WORLD,&status);
	}
	sum = 0.0;
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
    				rowcount++;
      			}
			// Root process waits untill the each slave process
			// has sent their calculated result with message tag 2
			// after slaves receive and do calculation in slave loop 
     			for (int i = 1;i <= loops; i++){
     				source=i; // cannot use loop parameter in MPI calls (different addresses in each slave machine)
      				MPI_Recv(&slave_sum, 1, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
      				sum+=slave_sum;
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
	    		// The slave process receives the index number which was assigned by Root 

			slave_sum = VECA[rowcount]*VECB[rowcount];

	    		dest = 0; // sending to the master processor tag = 2
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
	return(sum);
}

