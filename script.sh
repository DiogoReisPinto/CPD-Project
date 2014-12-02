for i in 1 2 3 4; do 
    scp -r tests  ist169905@lab13p$i:/tmp;
    scp lcs-mpi010 ist169905@lab13p$i:/tmp;
 done 

mpirun --host ist169905@lab13p1,ist169905@lab13p2,ist169905@lab13p3,ist169905@lab13p4 /tmp/lcs-mpi010 /tmp/tests/ex48k.30k.in