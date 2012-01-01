#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "json.h"

struct event {
	char *script, *date, *repeat, *ical_recur, *args;
};

struct event ev;

int numargs;
char **args;

struct json *
encode_event (struct event *evp)
{
	int idx, i;
	char nextrun[32];
	time_t t;
	struct json *jp, *arr;
	struct tm tm;

	jp = json_make_obj ();
	json_objset_str (jp, "script", evp->script);
	if (evp->ical_recur)
		json_objset_str (jp, "recurrence", evp->ical_recur);

	t = time (NULL);
	tm = *localtime (&t);

	sprintf (nextrun, "%d-%.2d-%.2d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	json_objset_str (jp, "nextrun", nextrun);
	json_objset_str (jp, "args", evp->args);

	arr = json_make_arr ();
	for (idx = optind; idx < numargs; idx++) {
		i = idx - optind;
		json_aset_str (arr, i, args[idx]);
	}
	json_objset_json (jp, "args", arr);

	json_objset_num (jp, "init", 1);

	return (jp);
}

void
usage (void)
{
	printf ("usage: -s scriptname [-r recurrence_pattern] [arguments_to_script]\n");
	exit (1);
}

void
parse_recur (void)
{
	int val, plural;
	char *unit_name, noval_unit, val_unit, *inp_unit;

	plural = 0;
	unit_name = "";
	ev.ical_recur = "";

	sscanf (ev.repeat, "%c", &noval_unit);
	
	sscanf (ev.repeat, "%d%c", &val, &val_unit);

	if (noval_unit == 'y' || noval_unit == 'm' || noval_unit == 'd') {
		inp_unit = &noval_unit;
		val = 1;
	} else if (val_unit == 'y' || val_unit == 'm' || val_unit == 'd')
		inp_unit = &val_unit;

	if (*inp_unit == 'y') {
		unit_name = "YEAR";
	} else if (*inp_unit == 'm') {
		unit_name = "MONTH";
	} else if (*inp_unit == 'd') {
		unit_name = "DAY";
	}
	
	if (val < 1) {
		printf ("invalid recurrence, ignoring\n");
		return;
	}

	if ((ev.ical_recur = calloc (1, sizeof "RRULE:FREQ=;INTERVAL="
				     + sizeof unit_name + sizeof val)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	sprintf (ev.ical_recur, "RRULE:FREQ=%s;INTERVAL=%d", unit_name, val);

	return;
}

int
main (int argc, char **argv)
{
	int c, script;
	struct json *json;

	script = 0;

	// might want to memset 0 ev to show it is empty here 
	while ((c = getopt (argc, argv, "s:r:")) != EOF) {
		switch (c) {
		case 's':
			ev.script = strdup (optarg);
			script = 1;
			break;
		case 'r':
			ev.repeat = strdup (optarg);
			parse_recur ();
		default:
			break;
		}
	}

	if (script == 0) // could test ev.script instead, reducing complexity by eliminating a variable
		usage ();

	if ((ev.args = calloc (argc, sizeof *argv)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

//	ev.args = *argv;
	for (i = 0; i < argc; i++)
		ev.args[i] = strdup (argv[i]);

	numargs = argc;
	args = argv; // uesless assignment to args

	json = encode_event (&ev);
	json_print (json);
	json_free (json);

	return (0);
}
