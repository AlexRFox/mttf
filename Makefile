CFLAGS = -g -Wall
LIBS = -lm

all: mttf-add birthday queue debug_arr_builder json_read

MTTF_ADD_OBJS = mttf-add.o json.o
mttf-add: $(MTTF_ADD_OBJS)
	$(CC) $(CFLAGS) -o mttf-add $(MTTF_ADD_OBJS) -lm

BIRTHDAY_OBJS = birthday.o json.o
birthday: $(BIRTHDAY_OBJS)
	$(CC) $(CFLAGS) -o birthday $(BIRTHDAY_OBJS) -lm

QUEUE_OBJS = queue.o json.o
queue: $(QUEUE_OBJS)
	$(CC) $(CFLAGS) -o queue $(QUEUE_OBJS) -lm

DEBUG_ARR_BUILDER_OBJS = debug_arr_builder.o json.o
debug_arr_builder: $(DEBUG_ARR_BUILDER_OBJS)
	$(CC) $(CFLAGS) -o debug_arr_builder $(DEBUG_ARR_BUILDER_OBJS) -lm

JSON_READ_OBJS = json_read.o json.o
json_read: $(JSON_READ_OBJS)
	$(CC) $(CFLAGS) -o json_read $(JSON_READ_OBJS) -lm

clean:
	rm -f *~ *.o mttf-add birthday json_read queue

