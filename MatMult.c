//***********************************
// Function to multiply two matrices
//***********************************
void	MatMult(int arows,int acols,double MATA[arows][acols],int brows,int bcols,double MATB[brows][bcols], double MATC[arows][bcols]){

	int rowcount = 0;  // begin with first row      	
	int loops, i,j,k;
	int source,dest;
	int na,nb,nc;
	
	if(acols != brows){
		printf("ERROR: Incompatitble matricies in MatMult\n");
		printf("Processor %d on %s arows %d acols %d brows %d, bcols %d\n",ProcessID,processor_name, arows,acols,brows,bcols);
		exit(0);
	}
	loops = nslaves;
	if (ncoef < nslaves) loops=ncoef;
	
	int nwhile = 0;
	int rowsleft;
	rowsleft=arows;
	source = 0;
	
	na = arows*acols;
	nb = brows*bcols;
	nc = arows*bcols;

	if (ProcessID == 0){
		for (i=1;i<=loops;i++){
			dest = i; // cannot use loop parameter in MPI calls
			//printf("Master Sending to processor %d\n",dest);
			MPI_Send(&MATB[0][0], nb, MPI_DOUBLE, dest, 3, MPI_COMM_WORLD);		
			//printf("Master sent to processor %d\n",dest);
		}
	}
	if (ProcessID > 0 && ProcessID <= loops){
			MPI_Recv(&MATB[0][0], nb, MPI_DOUBLE, source, 3, MPI_COMM_WORLD,&status);
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
    				//printf("rowcount sent dest %d rowcount %d Loops %d\n",dest,rowcount,loops);
    				rowcount++;
    				//printf("Master process has changed rowcount to %d\n",rowcount);
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
      				MPI_Recv(&MATC[rowcount][0], bcols, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
			}
			rowcount++; //rowcount was reset by last slave
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
	    		MPI_Recv(&MATA[rowcount][0], acols, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
			//printf("					Processor %d received rowcount %d and starting task\n",ProcessID,rowcount);
	    		for (j = 0; j < bcols; j++) {	
	        		// Set initial value of the row summataion
	        		double sum = 0.0;
	        		// Matrix A's element(i, j) will be multiplied
	        		//  with Matrix B's element(j, k)
	        		for (k = 0;k < acols; k++){
	        			sum = sum + MATA[rowcount][k] * MATB[k][j];
	        		}
				MATC[rowcount][j] = sum;
	      		}
	    		// Calculated result will be sent back to Root process 
	    		// (process 0) with message tag 2
	   		// Calculated row will be sent to Root
	   		// by sending the starting point of the calculated
	    		// value in matrix C
	    		dest = 0; // sending to the master processor 
	    		MPI_Send(&rowcount, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
	    		// Resulting matrix with calculated rows will be sent to root process
	    		MPI_Send(&MATC[rowcount][0], bcols, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
	    		//printf("					Processor %d sent to master rowcount %d rowsdone= %d\n",ProcessID,rowcount,rowsdone);
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

