# mpich_routines
some C language functions for MPICH

There are a set of routines specifically designed for MPICH devices. These routines have the C course and youhave to compile them yourself and you may wish to build them into a library as a .h file. The oincentive for this work was that othere were other routines available but many crashed because there were either too many processors or not enough processors. These routines use the same algorithm throughout, that is, the main process grabs as many or all of the slaves as necessary, does the work, then loops back and continues to process matrix operations row by row until all the rows are done. 
These routines includes a Gauss-Jordan elimination routine that will loop through all processors in parallel until the entire matrix is solved.

Identity.c – Create an identity matrix

Invert.c – Gauss – Jordan Elimination matrix inversion

Matcopy.c – Copy one matrix into another

MatMult.c – Multiply two matricies together

MatVec.c – Multiply a matrix times a vector

VecDot.c – Dot product of two vectors

VecMult.c – multiply two vectors of same length component by component into another vector of the same length

VecRMS.c – Find the Root Mean Square of a vector
