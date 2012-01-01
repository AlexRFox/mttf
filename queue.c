#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

int
main (int argc, char **argv)
{
	int idx, size;
	FILE *f;
	char *jsonstr, *filename, *json_arg;
	struct json *inp_json, *queue;

	json_arg = NULL;

	filename = "queue.json";
	if ((f = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	fseek (f, 0, SEEK_END);
	size = ftell (f);
	fseek (f, 0, SEEK_SET);
	if ((jsonstr = malloc (size+1)) == NULL) {
		fprintf (stderr, "out of memory\n");
		return (1);
	}
	size = fread (jsonstr, 1, size, f);
	jsonstr[size] = 0;

	inp_json = json_decode (jsonstr);
	queue = json_dup (inp_json);

	idx = 0;
	while (idx < json_array_size (queue)) {
		cur = json_aref (queue, idx);
		idx++;

		
	}

//	json_print (queue);

	return (0);
}
