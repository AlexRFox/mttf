#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

struct bday {
	char *who, *date;
};

void
init (struct json *json)
{
	int year, month, day;
	struct json *arr;

	json_objset_num (json, "first", 0);

	arr = json_objref (json, "args");

	sscanf (json_aref_str (arr, 0), "%d-%d-%d", &year, &month, &day);

	json_objset_num (json, "nextyear", year);
	json_objset_num (json, "nextmonth", month);
	json_objset_num (json, "nextday", day);
}

void
alert (void)
{
	printf ("do something alerty- possibly send user an email\n");
}

void
rerun (struct json *json)
{
	json_objset_num (json, "nextyear",
			 atoi (json_objref_str (json, "nextyear")) + 1);
}

void
usage (void)
{
	printf ("usage: birthday json_obj_input_file\n");
	exit (1);
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

	outp_json = json_dup (inp_json);

	if (strcmp (json_objref_str (inp_json, "first"), "1") == 0) {
		init (outp_json);
	} else {
		alert ();
		rerun (outp_json);
	}

	fclose (f);

	if ((f = fopen (filename, "w")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	char *newjson;
	newjson = json_encode (outp_json);
	fwrite (newjson, 1, strlen (newjson), f);
	fclose (f);

	return (0);
}
