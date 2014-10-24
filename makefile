PF = kinst-ompp
CC = gcc
CFLAGS  = -Wall -fopenmp -lm
TARGET = lcs-omp
all: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
clean:
	$(RM) $(TARGET)

profile: $(TARGET).c
	$(PF) $(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c