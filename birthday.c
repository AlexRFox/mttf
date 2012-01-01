#include <stdio.h>
#include <string.h>
#include "json.h"

struct bday {
	char *who, *date;
};

void
init (struct json *json)
{
	struct json *arr;

	arr = json_objref (json, "args");

	json_objset_num (json, "init", 0);
	json_objset_str (json, "nextrun", json_aref_str (arr, 1));
}

void
alert (void)
{
	printf ("do something alerty- possibly send user an email\n");
}

void
rerun (struct json *json)
{
	int year, month, day;
	char nextrun[100];
	struct json *arr;

	arr = json_objref (json, "args");

	sscanf (json_objref_str (json, "nextrun"), "%d-%d-%d", &year, &month, &day);

	snprintf (nextrun, sizeof nextrun, "%d-%02d-%02d", year + 1, month, day);

	json_objset_str (json, "nextrun", nextrun);
}

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
	struct json *inp_json, *outp_json;

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
	
	inp_json = json_decode (jsonstr);

	outp_json = json_clone (inp_json);

	if (strcmp (json_objref_str (inp_json, "init"), "1") == 0) {
		init (outp_json);
	} else {
		alert ();
		rerun (outp_json);
	}

	json_print (outp_json);

	return (0);
}
