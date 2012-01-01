#include <stdio.h>
#include <string.h>
#include "json.h"

int
main (int argc, char **argv)
{
	FILE *f;
	char *filename, jsonstr[10000], buf[1000];
	struct json *arr, *inp_json;

	arr = json_make_arr ();

	filename = "atwbday.json";
	if ((f = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	memset (&jsonstr, 0, sizeof jsonstr);
	while (fgets (buf, sizeof buf, f) != NULL) {
		sprintf (jsonstr, "%s%s", jsonstr, buf);
	}

	inp_json = json_clone (json_decode (jsonstr));

	json_aset_json (arr, 0, inp_json);

	json_print (inp_json);
	json_print (arr);

	return (0);
}
