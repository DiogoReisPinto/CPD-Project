CC = mpicc
CFLAGS  = -g
TARGET = lcs-mpi
all: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
clean:
	$(RM) -r $(TARGET) lcs-mpi.dSYM

run:
	mpirun -np $(PROC) lcs-mpi $(TEST)