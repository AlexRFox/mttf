CFLAGS = -g -Wall
LIBS = -lm

all: mttf-add birthday debug_arr_builder json_read mttf-queue

MTTF_ADD_OBJS = mttf-add.o json.o
mttf-add: $(MTTF_ADD_OBJS)
	$(CC) $(CFLAGS) -o mttf-add $(MTTF_ADD_OBJS) $(LIBS)

BIRTHDAY_OBJS = birthday.o json.o
birthday: $(BIRTHDAY_OBJS)
	$(CC) $(CFLAGS) -o birthday $(BIRTHDAY_OBJS) $(LIBS)

DEBUG_ARR_BUILDER_OBJS = debug_arr_builder.o json.o
debug_arr_builder: $(DEBUG_ARR_BUILDER_OBJS)
	$(CC) $(CFLAGS) -o debug_arr_builder $(DEBUG_ARR_BUILDER_OBJS) $(LIBS)

JSON_READ_OBJS = json_read.o json.o
json_read: $(JSON_READ_OBJS)
	$(CC) $(CFLAGS) -o json_read $(JSON_READ_OBJS) $(LIBS)

MTTF_QUEUE_OBJS = mttf-queue.o json.o
mttf-queue: $(MTTF_QUEUE_OBJS)
	$(CC) $(CFLAGS) -o mttf-queue $(MTTF_QUEUE_OBJS) $(LIBS)

clean:
	rm -f *~ *.o mttf-add birthday json_read mttf-queue
