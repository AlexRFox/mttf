#include <stdio.h>
#include <string.h>
#include "json.h"

void
add_bday (struct json *json, char *jsonfile)
{
	FILE *f;
	char jsonstr[10000], buf[1000];
	struct json *inp_json;

	if ((f = fopen (jsonfile, "r")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return;
	}

	memset (&jsonstr, 0, sizeof jsonstr);
	while (fgets (buf, sizeof buf, f) != NULL) {
		sprintf (jsonstr, "%s%s", jsonstr, buf);
	}

	inp_json = json_dup (json_decode (jsonstr));

	json_aset_json (json, json_array_size (json), inp_json);
}

int
main (int argc, char **argv)
{
	struct json *arr;

	arr = json_make_arr ();

	add_bday (arr, "atwbday.json");
	add_bday (arr, "epwbday.json");
	add_bday (arr, "werbday.json");
	add_bday (arr, "pacebday.json");

	json_print (arr);

	return (0);
}
