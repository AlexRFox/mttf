#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "json.h"

void
json_adel_json (struct json *json, int idx)
{
	int i;

	for (i = idx+1; i < json_array_size (json); i++) {
		char *str;
		struct json *obj;

		switch (json_get_type (json_aref (json, i))) {
		case JSON_STRING:
			str = json_aref_str (json, i);
			json_aset_str (json, i-1, str);
			break;
		case JSON_ARRAY:
		case JSON_OBJECT:
			obj = json_aref (json, i);
			json_aset_json (json, i-1, obj);
			break;
		}
	}

	json_aset_json (json, json_array_size (json)-1, NULL);
}

struct json *
json_apop_json (struct json *json, int idx)
{
	struct json *jsonval;

	jsonval = json_aref (json, idx);

	json_adel_json (json, idx);

	return (jsonval);
}

struct json *
run_script (struct json *json)
{
	int fd;
	char filename[1000], *jsonstr, cmd[1000], *script;
	FILE *f;

	strcpy (filename, "/tmp/queue.XXXXXX");
	if ((fd = mkstemp (filename)) < 0) {
		fprintf (stderr, "can't create temporary file %s",
			 filename);
		return (NULL);
	}

	jsonstr = json_encode (json);

	write (fd, jsonstr, strlen (jsonstr));
	close (fd);

	script = json_objref_str (json, "script");
	sprintf (cmd, "%s %s", script, filename);
	if (system (cmd) != 0) {
		fprintf (stderr, "something bad happened\n");
		return (NULL);
	}

	if ((f = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (NULL);
	}

	int size;
	fseek (f, 0, SEEK_END);
	size = ftell (f);
	fseek (f, 0, SEEK_SET);
	if ((jsonstr = malloc (size+1)) == NULL) {
		fprintf (stderr, "out of memory\n");
		return (NULL);
	}
	size = fread (jsonstr, 1, size, f);
	jsonstr[size] = 0;

	fclose (f);
	remove (filename);

	return (json_decode (jsonstr));
}

int
main (int argc, char **argv)
{
	int idx, size, cur_idx;
	char *jsonstr, *filename;
	struct json *inp_json, *queue, *cur, *new;
	struct tm tm;
	time_t t;
	FILE *f;

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

	t = time (NULL);
	tm = *localtime (&t);

	idx = 0;
	while (idx < json_array_size (queue)) {
		cur = json_aref (queue, idx);
		cur_idx = idx;
		idx++;

		if (atoi (json_objref_str (cur, "nextyear"))
		    <= tm.tm_year+1900
		    && atoi (json_objref_str (cur, "nextmonth"))
		    <= tm.tm_mon+1
		    && atoi (json_objref_str (cur, "nextday"))
		    <= tm.tm_mday) {
			idx = 0;

			new = run_script (cur);

			json_aset_json (queue, json_array_size (queue),
					new);
			json_adel_json (queue, cur_idx);
		}
	}

	fclose (f);

	if ((f = fopen (filename, "w")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	char *newjson;
	newjson = json_encode (queue);
	fwrite (newjson, 1, strlen (newjson), f);
	fclose (f);

	return (0);
}

