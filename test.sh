make clean;

NAME="lcsmpi10$1"
TESTE=$2

mpicc -o $NAME lcs-mpi.c

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do 
    scp -r tests  ist169905@lab13p$i:/tmp;
		scp -r tests  ist169905@lab14p$i:/tmp;
    scp $NAME ist169905@lab13p$i:/tmp;
		scp $NAME ist169905@lab14p$i:/tmp;
 done 

mpirun --host ist169905@lab13p1,ist169905@lab13p2,ist169905@lab13p3,ist169905@lab13p4,ist169905@lab13p5,ist169905@lab13p6,ist169905@lab13p7,ist169905@lab13p8,ist169905@lab13p9,ist169905@lab13p10,ist169905@lab14p9,ist169905@lab14p13,ist169905@lab14p16,ist169905@lab14p17,ist169905@lab14p18,ist169905@lab14p2 /tmp/$NAME /tmp/tests/ex$2.in
 
