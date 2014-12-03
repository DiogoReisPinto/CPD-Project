make clean;

NAME="lcsmpi10$1"

mpicc -o $NAME lcs-mpi.c

for i in 1 2 3 4 5 6 7 8 9 10; do 
    scp -r tests  ist169905@lab13p$i:/tmp;
    scp $NAME ist169905@lab13p$i:/tmp;
 done 

mpirun --host ist169905@lab8p8,ist169905@lab8p2,ist169905@lab8p7,ist169905@lab8p6 /tmp/$NAME /tmp/tests/ex3k.8k.in