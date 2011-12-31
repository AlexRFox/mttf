CFLAGS = -g -Wall
LIBS = -lm

all: mttf-add

MTTF_ADD_OBJS = mttf-add.o json.o
mttf-add: $(MTTF_ADD_OBJS)
	$(CC) $(CFLAGS) -o mttf-add $(MTTF_ADD_OBJS) -lm

clean:
	rm -f *~ *.o 
	rm -f mttf-add

