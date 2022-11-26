void Identity(int N, double I[N][N]){

	int rowcount = 0;  // begin with first row      	
	int loops, i,j;
	int source,dest;
	
	// fprintf(stdout, "Process %d of %d is on %s\n", ProcessID, numprocs, processor_name);
    	// fflush(stdout);
	
	//printf("inside identity, N %d, nslaves %d\n",N,nslaves);
	
	loops = nslaves;
	if (ncoef < nslaves) loops=ncoef;
	
	int nwhile = 0;
	int rowsdone = 0;
	// all processors should receive loops and initialization, including the master
	//printf("Before while loop. Processor %d has loops = %d nwhile = %d and rowsdone = %d\n",ProcessID,loops,nwhile,rowsdone);

	while (loops > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
		// printf("Processor %d is inside while loop\n",ProcessID);     	
		// Root (Master) process
		if (ProcessID == 0) {
			srand ( time(NULL) );
			//printf("Master loop entered while rowcount %d\t loops %d, ncoef %d, nslaves %d\n",rowcount,loops,ncoef,nslaves);
			//printf("Press any key\n");
			//getchar();
	    		// Calculation details are assigned to slave tasks. Process 1 onwards;
    			// Each message's tag is 1
    			for (i=1; i <= loops; i++){
    				dest = i;
 	    			// Acknowledging the rowcount of the Matrix A
				MPI_Send(&rowcount, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      				// Send rows of the Matrix A and all of Martrix B
      				MPI_Send(&I[rowcount][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    				//printf("rowcount sent dest %d rowcount %d Loops %d\n",dest,rowcount,loops);
    				rowcount++;
    				//printf("Master process has changed rowcount to %d\n",rowcount);
      			}
//     			printf("out of loop\n");
			// Root process waits untill the each slave process
			// has sent their calculated result with message tag 2
			// after slaves receive and do calculation in slave loop 
			//printf("Master loop leaving master send loop. Rowcount is now by master %d\t loops %d, ncoef %d, nslaves %d\n",rowcount,loops,ncoef,nslaves);
			//printf("Master loop waiting for all slaves to send\n");
			//printf("Press any key\n");
			//getchar();
     			for (int i = 1;i <= loops; i++){
     				//printf("Master waiting for slave Processor %d\n",i);
     				source=i; // cannot use loop parameter in MPI calls (different addresses in each slave machine)
    				// Receive the offset of particular slave process
      				MPI_Recv(&rowcount, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      				// Calculated rows of the each process will be stored
      				// in Matrix C according to their rowcount
      				MPI_Recv(&I[rowcount][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
 				//printf("Master loop received from %d slave processor rowcount %d loops %d, ncoef %d, nslaves %d\n",i,rowcount,loops,ncoef,nslaves);
			}
			rowcount++; //rowcount was reset by last slave
			rowsdone=rowsdone+loops;
			nwhile++;
			//printf("Master process received from %d slaves and rowsdone = %d while loop = %d\n",loops,rowsdone,nwhile);
	  		//printf("Press any key after slaves have caught up\n");
	  		//getchar();
		}
	  // Slave Processes 
  		if (ProcessID > 0 && ProcessID <= loops ) {
 			//printf("					Starting processor %d  loops = %d\n",ProcessID,loops);
  	   		// Source process ID is defined
    			source = 0;
			//printf("					Processor %d waiting for Master to send. loops = %d\n",ProcessID,loops);
//	    		//printf("slave %d loops %d\n",ProcessID,loops);
	    		// Slave process waits for the message buffers with tag 1, that Root process sent
	    		// Each process will receive and execute this separately on their processes

	    		// The slave process receives the row value sent by root process
	    		MPI_Recv(&rowcount, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
	    		// The slave process receives the sub portion of the Matrix A which assigned by Root 
	    		MPI_Recv(&I[rowcount][0], npoints, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
			//printf("					Processor %d received rowcount %d and starting task\n",ProcessID,rowcount);
	    		for (j = 0; j < N; j++) {	
	        		// Set initial value
	        		I[rowcount][j] = 0.0;
	        	}
	        	I[rowcount][rowcount] = 1.00;
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	   		// by sending the starting point of the calculated
	    		// value in matrix C
	    		//printf("					Processor %d Sending to master rowcount %d and completed task\n",ProcessID,rowcount);
	    		dest = 0; // sending to the master processor 
	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
	    		// Resulting matrix with calculated rows will be sent to root process
	    		MPI_Send(&I[rowcount][0], ncoef, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
	    		rowsdone=rowsdone+loops;
	    		nwhile++;
	    		//printf("					Processor %d sent to master rowcount %d rowsdone= %d\n",ProcessID,rowcount,rowsdone);
	  	}
	  	//All processors access this line
	  	//How many rows are left to process?
	  	int rowsleft;
	  	rowsleft=ncoef-rowsdone;
	  	if (ProcessID > rowsleft) loops = 0;
	  	//printf("At bottom of while loop Processor %d rowsleft %d rowsdone %d\n",ProcessID,rowsleft,rowsdone);
	  	if (ProcessID <= rowsleft && rowsleft < nslaves){
	  	 	//printf("Process %d says rowsleft is %d\n",ProcessID,rowsleft);
	  	 	loops=rowsleft;
	  	 	//printf("Process %d says loops is %d\n",ProcessID,loops);
	  	 }
	  	//printf("**********Process %d at bottom of while loop %d rowsdone %d rowsleft %d ncoef %d loops %d\n",ProcessID,nwhile,rowsdone,rowsleft,ncoef,loops);
	} //end of while loop
}

