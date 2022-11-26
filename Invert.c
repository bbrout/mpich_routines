//*******************************
// function to invert a matrix
//*******************************

void 	Invert(int N, double A[N][N], double AINV[N][N]){

	double I[N][N],ratio;
	int i,j,k,irow,jrow,dest,source;
	int rowsleft;
	
	int rowcount = 0;  // begin with first row      	
	int loops;
	

	int pivot; 
	double divisor;

	int nwhile;
	//*******************************
// function to invert a matrix
//*******************************

void 	Invert(int N, double A[N][N], double AINV[N][N]){

	double I[N][N],ratio;
	int i,j,k,irow,jrow,dest,source;
	int rowsleft;
	
	int rowcount = 0;  // begin with first row      	
	int loops;
	

	int pivot; 
	double divisor;

	int nwhile;
	
	loops = nslaves;
	if (N < nslaves) loops=N;
	
	// turn AINV into Identity matrix
	
	Identity(N,AINV);
	
	// copy A into I initializing I

	MatCopy(N,A,I);

	//turn the Identity matrix into the inverse of XTX
	
       /* Applying Gauss Jordan Elimination */

  	for(i=0;i<N;i++){ // i is the pivot row. Go through entire matrix, except for the pivot row, for each row of the matrix
  	
  		loops = nslaves; // use all slave processors if possible
		if (N < nslaves) loops=N; // if N is small enough, don't need all the slave processors
		int nwhile = 0;
		rowsleft=N;
		nwhile=0;

         	if(ProcessID == 0){  //check if pivot is singular abort routine
	 		if(I[i][i] == 0.0){
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f \n",i,i,i,I[i][i]);
                      	      	exit(0);
    				}
    		}
         
         	// send pivot row number to slaves

            	if(ProcessID == 0){
	         	pivot = i; // identify the pivot row
	         	divisor = I[i][i];
	         	if (divisor == 0.0){ // doing this again for debug purposes
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f\n",i,i,i,I[i][i]);
				printf("Press any key\n");
				getchar();
                      	      	exit(0);
	         	}
        		//send stuff through tag = 3
			for (irow=1;irow<=loops;irow++){ //send pivot and augmented pivot row to all at once (changes with each value of i)
               		dest=irow;
    				MPI_Send(&pivot,1, MPI_INT,dest,3, MPI_COMM_WORLD);
      				MPI_Send(&I[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    				MPI_Send(&AINV[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    			}
    		}
 				// Receive pivot and augmented pivot row
 		if (ProcessID > 0 && ProcessID <= loops){
 			source=0;
			MPI_Recv(&pivot, 1, MPI_DOUBLE,source,3, MPI_COMM_WORLD,&status);
			MPI_Recv(&I[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
		}
		
		j=0; 		// Each flop increases the j counter by loops within the while loop

		while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
			if (ProcessID==0){
      	      			//Send rows and ratio to slaves
				srand ( time(NULL) );
	    			// Each message's tag is 1
	    			for (int idest=0; idest < loops; idest++){
					dest = idest+1;
					jrow = j + idest;
					//check for pivot row
					if (jrow == pivot){ //skip and move on
						ratio = 1.0; // set ratio as harmless
						}
					else{	
						ratio = I[jrow][pivot]/divisor; // if pivot = jrow, we have a problem.
						//check if we have a singularity
						if (divisor == 0.0){ //let everyone know. Done again for debug purposes
							printf("divisor is zero at i = %d pivot = %d jrow = %d\n",i,pivot,jrow);
							printf("Press any key\n");
							getchar();
							exit(0);
						}
					}

					MPI_Send(&jrow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&ratio, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
	    				MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
				} // end of Processor 0 send loop				
	     			for (int isource = 1;isource <= loops; isource++){
	     				source=isource; // cannot use loop parameter isource in MPI calls (I think) 
	      				MPI_Recv(&jrow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
	      				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
					MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				} // end of Processor 0 receive loop
			} // end of Processor 0 processes which involves as many slaves as possible in this while loop
      	      		if (ProcessID > 0 && ProcessID <= loops){ 
	              		//slave Receives row and ratio
	              		source=0;

				MPI_Recv(&jrow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&ratio, 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				if (jrow == pivot){ //skip it and move on
					// continue  -- this for old school
				}
				else {
	                       	for(k=0;k<N;k++){
      		                            	I[jrow][k]      = I[jrow][k]    - ratio*I[pivot][k];
      		                             	AINV[jrow][k]   = AINV[jrow][k] - ratio*AINV[pivot][k];
      	                               }
      				}

      	           		//slave sends rows to master including pivot, which should be unchanged (easier this way).
 				dest = 0;

				MPI_Send(&jrow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
		      		MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
      	               } // end of all processor tasks for this while loop. Set counters for more tasks in the while loop or to escape.
	
      	               j+=loops;
			nwhile++; // keeping track of this for debuggging purposes if necessary
		  	//All processors access this line
	  		//How many rows are left to process?
	  		rowsleft=rowsleft - loops;
	  		if (ProcessID > rowsleft) rowsleft = 0; // Processor no longer needed and can leave the while loop
	  		if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  		if (rowsleft == 0) loops=0;
      	 	} // end of the while loop

      	 } // all rows have been processed and ready to be normalized
  
  	/* Row Operation to Make Principle Diagonal to 1 */

	// Again set the counters for the while loop to do its job

	loops = nslaves;
	if (N < nslaves) loops=N;
	nwhile = 0;
	rowsleft=N;
	i=0;

	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows have been divided out 

      		if (ProcessID==0){
        	      	//Send rows to slaves
			srand ( time(NULL) );
		    	// Each message's tag is 1
		    	for (int idest=0; idest < loops; idest++){
				dest = idest+1;
				int irow = i + idest;

				MPI_Send(&irow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
		      		MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		      	} // Processor 0 has sent rows to slaves
		     	for (int isource = 0;isource < loops; isource++){ //Processor 0 must wait for all slaves to be done and get results
		     		source=isource+1; // cannot use loop parameter in MPI calls 
		     		int irow;

		      		MPI_Recv(&irow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		      		MPI_Recv(&I[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);

			} // Processor 0 has received results
		} // Processor 0 has sent and received rows

        	if (ProcessID > 0 && ProcessID <= loops){ 
			//slave Receives row and ratio
			source=0;
			
			MPI_Recv(&irow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&I[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);

			// do it properly. There is a bug in many programs at this point. Don't change value of I[irow][irow] while doing this loop
			double divisor = I[irow][irow];
	              	for(k=0;k<N;k++){ 
		                AINV[irow][k] = AINV[irow][k]/divisor;
	                        I[irow][k]     = I[irow][k]/divisor;
        		}                              

       		//slave sends rows to master
    			dest = 0;
			MPI_Send(&irow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
			MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
			MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
        	}
		//All processors are finished for this section. set counters for while loop and free slaves not needed
        	i+=loops;
		rowcount+=loops;
		nwhile++;
	  	//How many rows are left to process?
	  	rowsleft=rowsleft-loops;
	  	if (ProcessID > rowsleft) rowsleft = 0; // processor no longer needed and can leave the while loop
	  	if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  	if (rowsleft == 0) loops=0;
	} // End of while loop and all rows have been processed

} // end of function Invert

	loops = nslaves;
	if (N < nslaves) loops=N;
	
	// turn AINV into Identity matrix
	
	Identity(N,AINV);
	//*******************************
// function to invert a matrix
//*******************************

void 	Invert(int N, double A[N][N], double AINV[N][N]){

	double I[N][N],ratio;
	int i,j,k,irow,jrow,dest,source;
	int rowsleft;
	
	int rowcount = 0;  // begin with first row      	
	int loops;
	

	int pivot; 
	double divisor;

	int nwhile;
	
	loops = nslaves;
	if (N < nslaves) loops=N;
	
	// turn AINV into Identity matrix
	
	Identity(N,AINV);
	
	// copy A into I initializing I

	MatCopy(N,A,I);

	//turn the Identity matrix into the inverse of XTX
	
       /* Applying Gauss Jordan Elimination */

  	for(i=0;i<N;i++){ // i is the pivot row. Go through entire matrix, except for the pivot row, for each row of the matrix
  	
  		loops = nslaves; // use all slave processors if possible
		if (N < nslaves) loops=N; // if N is small enough, don't need all the slave processors
		int nwhile = 0;
		rowsleft=N;
		nwhile=0;

         	if(ProcessID == 0){  //check if pivot is singular abort routine
	 		if(I[i][i] == 0.0){
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f \n",i,i,i,I[i][i]);
                      	      	exit(0);
    				}
    		}
         
         	// send pivot row number to slaves

            	if(ProcessID == 0){
	         	pivot = i; // identify the pivot row
	         	divisor = I[i][i];
	         	if (divisor == 0.0){ // doing this again for debug purposes
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f\n",i,i,i,I[i][i]);
				printf("Press any key\n");
				getchar();
                      	      	exit(0);
	         	}
        		//send stuff through tag = 3
			for (irow=1;irow<=loops;irow++){ //send pivot and augmented pivot row to all at once (changes with each value of i)
               		dest=irow;
    				MPI_Send(&pivot,1, MPI_INT,dest,3, MPI_COMM_WORLD);
      				MPI_Send(&I[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    				MPI_Send(&AINV[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    			}
    		}
 				// Receive pivot and augmented pivot row
 		if (ProcessID > 0 && ProcessID <= loops){
 			source=0;
			MPI_Recv(&pivot, 1, MPI_DOUBLE,source,3, MPI_COMM_WORLD,&status);
			MPI_Recv(&I[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
		}
		
		j=0; 		// Each flop increases the j counter by loops within the while loop

		while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
			if (ProcessID==0){
      	      			//Send rows and ratio to slaves
				srand ( time(NULL) );
	    			// Each message's tag is 1
	    			for (int idest=0; idest < loops; idest++){
					dest = idest+1;
					jrow = j + idest;
					//check for pivot row
					if (jrow == pivot){ //skip and move on
						ratio = 1.0; // set ratio as harmless
						}
					else{	
						ratio = I[jrow][pivot]/divisor; // if pivot = jrow, we have a problem.
						//check if we have a singularity
						if (divisor == 0.0){ //let everyone know. Done again for debug purposes
							printf("divisor is zero at i = %d pivot = %d jrow = %d\n",i,pivot,jrow);
							printf("Press any key\n");
							getchar();
							exit(0);
						}
					}

					MPI_Send(&jrow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&ratio, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
	    				MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
				} // end of Processor 0 send loop				
	     			for (int isource = 1;isource <= loops; isource++){
	     				source=isource; // cannot use loop parameter isource in MPI calls (I think) 
	      				MPI_Recv(&jrow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
	      				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
					MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				} // end of Processor 0 receive loop
			} // end of Processor 0 processes which involves as many slaves as possible in this while loop
      	      		if (ProcessID > 0 && ProcessID <= loops){ 
	              		//slave Receives row and ratio
	              		source=0;

				MPI_Recv(&jrow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&ratio, 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				if (jrow == pivot){ //skip it and move on
					// continue  -- this for old school
				}
				else {
	                       	for(k=0;k<N;k++){
      		                            	I[jrow][k]      = I[jrow][k]    - ratio*I[pivot][k];
      		                             	AINV[jrow][k]   = AINV[jrow][k] - ratio*AINV[pivot][k];
      	                               }
      				}

      	           		//slave sends rows to master including pivot, which should be unchanged (easier this way).
 				dest = 0;

				MPI_Send(&jrow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
		      		MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
      	               } // end of all processor tasks for this while loop. Set counters for more tasks in the while loop or to escape.
	
      	               j+=loops;
			nwhile++; // keeping track of this for debuggging purposes if necessary
		  	//All processors access this line
	  		//How many rows are left to process?
	  		rowsleft=rowsleft - loops;
	  		if (ProcessID > rowsleft) rowsleft = 0; // Processor no longer needed and can leave the while loop
	  		if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  		if (rowsleft == 0) loops=0;
      	 	} // end of the while loop

      	 } // all rows have been processed and ready to be normalized
  
  	/* Row Operation to Make Principle Diagonal to 1 */

	// Again set the counters for the while loop to do its job

	loops = nslaves;
	if (N < nslaves) loops=N;
	nwhile = 0;
	rowsleft=N;
	i=0;

	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows have been divided out 

      		if (ProcessID==0){
        	      	//Send rows to slaves
			srand ( time(NULL) );
		    	// Each message's tag is 1
		    	for (int idest=0; idest < loops; idest++){
				dest = idest+1;
				int irow = i + idest;

				MPI_Send(&irow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
		      		MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		      	} // Processor 0 has sent rows to slaves
		     	for (int isource = 0;isource < loops; isource++){ //Processor 0 must wait for all slaves to be done and get results
		     		source=isource+1; // cannot use loop parameter in MPI calls 
		     		int irow;

		      		MPI_Recv(&irow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		      		MPI_Recv(&I[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);

			} // Processor 0 has received results
		} // Processor 0 has sent and received rows

        	if (ProcessID > 0 && ProcessID <= loops){ 
			//slave Receives row and ratio
			source=0;
			
			MPI_Recv(&irow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&I[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);

			// do it properly. There is a bug in many programs at this point. Don't change value of I[irow][irow] while doing this loop
			double divisor = I[irow][irow];
	              	for(k=0;k<N;k++){ 
		                AINV[irow][k] = AINV[irow][k]/divisor;
	                        I[irow][k]     = I[irow][k]/divisor;
        		}                              

       		//slave sends rows to master
    			dest = 0;
			MPI_Send(&irow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
			MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
			MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
        	}
		//All processors are finished for this section. set counters for while loop and free slaves not needed
        	i+=loops;
		rowcount+=loops;
		nwhile++;
	  	//How many rows are left to process?
	  	rowsleft=rowsleft-loops;
	  	if (ProcessID > rowsleft) rowsleft = 0; // processor no longer needed and can leave the while loop
	  	if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  	if (rowsleft == 0) loops=0;
	} // End of while loop and all rows have been processed

} // end of function Invert

	// copy A into I initializing I

	MatCopy(N,A,I);

	//turn the Identity matrix into the inverse of XTX
	
       /* Applying Gauss Jordan Elimination */

  	for(i=0;i<N;i++){ // i is the pivot row. Go through entire matrix, except for the pivot row, for each row of the matrix
  	
  		loops = nslaves; // use all slave processors if possible
		if (N < nslaves) loops=N; // if N is small enough, don't need all the slave processors
		int nwhile = 0;
		rowsleft=N;
		nwhile=0;

         	if(ProcessID == 0){  //check if pivot is singular abort routine
	 		if(I[i][i] == 0.0){
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f \n",i,i,i,I[i][i]);
                      	      	exit(0);
    				}
    		}
         
         	// send pivot row number to slaves

            	if(ProcessID == 0){
	         	pivot = i; // identify the pivot row
	         	divisor = I[i][i];
	         	if (divisor == 0.0){ // doing this again for debug purposes
                      	      	printf("Mathematical Error!");
                      	      	printf("Row = %d value of I[%d][%d] is %f\n",i,i,i,I[i][i]);
				printf("Press any key\n");
				getchar();
                      	      	exit(0);
	         	}
        		//send stuff through tag = 3
			for (irow=1;irow<=loops;irow++){ //send pivot and augmented pivot row to all at once (changes with each value of i)
               		dest=irow;
    				MPI_Send(&pivot,1, MPI_INT,dest,3, MPI_COMM_WORLD);
      				MPI_Send(&I[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    				MPI_Send(&AINV[pivot][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
    			}
    		}
 				// Receive pivot and augmented pivot row
 		if (ProcessID > 0 && ProcessID <= loops){
 			source=0;
			MPI_Recv(&pivot, 1, MPI_DOUBLE,source,3, MPI_COMM_WORLD,&status);
			MPI_Recv(&I[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[pivot][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
		}
		
		j=0; 		// Each flop increases the j counter by loops within the while loop

		while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows of matrix_a have been multiplied out (rowcount starts counting at 0 not 1) 
			if (ProcessID==0){
      	      			//Send rows and ratio to slaves
				srand ( time(NULL) );
	    			// Each message's tag is 1
	    			for (int idest=0; idest < loops; idest++){
					dest = idest+1;
					jrow = j + idest;
					//check for pivot row
					if (jrow == pivot){ //skip and move on
						ratio = 1.0; // set ratio as harmless
						}
					else{	
						ratio = I[jrow][pivot]/divisor; // if pivot = jrow, we have a problem.
						//check if we have a singularity
						if (divisor == 0.0){ //let everyone know. Done again for debug purposes
							printf("divisor is zero at i = %d pivot = %d jrow = %d\n",i,pivot,jrow);
							printf("Press any key\n");
							getchar();
							exit(0);
						}
					}

					MPI_Send(&jrow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&ratio, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
	      				MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
	    				MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
				} // end of Processor 0 send loop				
	     			for (int isource = 1;isource <= loops; isource++){
	     				source=isource; // cannot use loop parameter isource in MPI calls (I think) 
	      				MPI_Recv(&jrow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
	      				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
					MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				} // end of Processor 0 receive loop
			} // end of Processor 0 processes which involves as many slaves as possible in this while loop
      	      		if (ProcessID > 0 && ProcessID <= loops){ 
	              		//slave Receives row and ratio
	              		source=0;

				MPI_Recv(&jrow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&ratio, 1, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
				MPI_Recv(&I[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[jrow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
				if (jrow == pivot){ //skip it and move on
					// continue  -- this for old school
				}
				else {
	                       	for(k=0;k<N;k++){
      		                            	I[jrow][k]      = I[jrow][k]    - ratio*I[pivot][k];
      		                             	AINV[jrow][k]   = AINV[jrow][k] - ratio*AINV[pivot][k];
      	                               }
      				}

      	           		//slave sends rows to master including pivot, which should be unchanged (easier this way).
 				dest = 0;

				MPI_Send(&jrow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
		      		MPI_Send(&I[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[jrow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
      	               } // end of all processor tasks for this while loop. Set counters for more tasks in the while loop or to escape.
	
      	               j+=loops;
			nwhile++; // keeping track of this for debuggging purposes if necessary
		  	//All processors access this line
	  		//How many rows are left to process?
	  		rowsleft=rowsleft - loops;
	  		if (ProcessID > rowsleft) rowsleft = 0; // Processor no longer needed and can leave the while loop
	  		if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  		if (rowsleft == 0) loops=0;
      	 	} // end of the while loop

      	 } // all rows have been processed and ready to be normalized
  
  	/* Row Operation to Make Principle Diagonal to 1 */

	// Again set the counters for the while loop to do its job

	loops = nslaves;
	if (N < nslaves) loops=N;
	nwhile = 0;
	rowsleft=N;
	i=0;

	while (rowsleft > 0 && ProcessID <= loops ){  // do this until all rows have been divided out 

      		if (ProcessID==0){
        	      	//Send rows to slaves
			srand ( time(NULL) );
		    	// Each message's tag is 1
		    	for (int idest=0; idest < loops; idest++){
				dest = idest+1;
				int irow = i + idest;

				MPI_Send(&irow, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
		      		MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		    		MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
		      	} // Processor 0 has sent rows to slaves
		     	for (int isource = 0;isource < loops; isource++){ //Processor 0 must wait for all slaves to be done and get results
		     		source=isource+1; // cannot use loop parameter in MPI calls 
		     		int irow;

		      		MPI_Recv(&irow, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		      		MPI_Recv(&I[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
				MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);

			} // Processor 0 has received results
		} // Processor 0 has sent and received rows

        	if (ProcessID > 0 && ProcessID <= loops){ 
			//slave Receives row and ratio
			source=0;
			
			MPI_Recv(&irow, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&I[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);
			MPI_Recv(&AINV[irow][0], N, MPI_DOUBLE,source,1, MPI_COMM_WORLD, &status);

			// do it properly. There is a bug in many programs at this point. Don't change value of I[irow][irow] while doing this loop
			double divisor = I[irow][irow];
	              	for(k=0;k<N;k++){ 
		                AINV[irow][k] = AINV[irow][k]/divisor;
	                        I[irow][k]     = I[irow][k]/divisor;
        		}                              

       		//slave sends rows to master
    			dest = 0;
			MPI_Send(&irow, 1, MPI_INT, dest, 2, MPI_COMM_WORLD);
			MPI_Send(&I[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
			MPI_Send(&AINV[irow][0], N, MPI_DOUBLE,dest,2, MPI_COMM_WORLD);
        	}
		//All processors are finished for this section. set counters for while loop and free slaves not needed
        	i+=loops;
		rowcount+=loops;
		nwhile++;
	  	//How many rows are left to process?
	  	rowsleft=rowsleft-loops;
	  	if (ProcessID > rowsleft) rowsleft = 0; // processor no longer needed and can leave the while loop
	  	if (ProcessID <= rowsleft && rowsleft < nslaves) loops=rowsleft;
	  	if (rowsleft == 0) loops=0;
	} // End of while loop and all rows have been processed

} // end of function Invert

