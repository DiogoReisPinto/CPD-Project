make clean;

NAME="lcsmpi10$1"
TESTE=$2

mpicc -fopenmp -o  $NAME lcs-mpi.c

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do 
    scp $NAME ist169905@lab13p$i:/tmp;
		scp $NAME ist169905@lab14p$i:/tmp;
 done 

mpirun --host ist169905@lab14p1,ist169905@lab14p2 /tmp/$NAME /tmp/tests/ex$2.in
 
