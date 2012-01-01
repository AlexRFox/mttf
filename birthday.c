#include <stdio.h>
#include <string.h>
#include "json.h"

void
usage (void)
{
	printf ("usage: birthday json_obj_input_file\n");
}

int
main (int argc, char **argv)
{
	FILE *f;
	char *filename, buf[1000], jsonstr[10000];
	struct json *json;

	if (argc != 2)
		usage ();

	filename = argv[1];

	if ((f = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	memset (&jsonstr, 0, sizeof jsonstr);
	while (fgets (buf, sizeof buf, f) != NULL) {
		sprintf (jsonstr, "%s%s", jsonstr, buf);
	}

	json = json_decode (jsonstr);

	if (strcmp (json_objref_str (json, "init"), "1") == 0) {
		printf ("init\n");
	} else {
		printf ("running\n");
	}

	printf ("%s\n", json_objref_str (json, "script"));
	
	struct json *arr;
	arr = json_objref (json, "args");
	printf ("%s\n", json_aref_str (arr, 0));
	

	return (0);
}
