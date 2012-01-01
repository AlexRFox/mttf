#include <stdio.h>
#include <stdlib.h>
#include "json.h"

int
main (int argc, char **argv)
{
	int size;
	FILE *f;
	char *filename, *jsonstr;

	filename = argv[1];
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

	json_print (json_decode (jsonstr));

	return (0);
}
