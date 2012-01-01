#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "json.h"

struct event {
	char *script, *date, *repeat, ical_recur[1000], *args;
};

struct event ev;

struct json *
encode_event (struct event *evp, char **posargs, int posargs_count)
{
	int idx;
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

	strftime (nextrun, sizeof nextrun, "%Y-%m-%d", &tm);

	json_objset_str (jp, "nextrun", nextrun);
	json_objset_str (jp, "args", evp->args);

	arr = json_make_arr ();
	for (idx = 0; idx < posargs_count; idx++) {
		json_aset_str (arr, idx, posargs[idx]);
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
	memset (&ev.ical_recur, 0, sizeof ev.ical_recur);

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

	snprintf (ev.ical_recur, sizeof ev.ical_recur,
		  "RRULE:FREQ=%s;INTERVAL=%d", unit_name, val);

	return;
}

int
main (int argc, char **argv)
{
	int c;
	struct json *json;

	memset (&ev, 0, sizeof ev);

	while ((c = getopt (argc, argv, "s:r:")) != EOF) {
		switch (c) {
		case 's':
			ev.script = strdup (optarg);
			break;
		case 'r':
			ev.repeat = strdup (optarg);
			parse_recur ();
		default:
			break;
		}
	}

	if (ev.script == 0)
		usage ();

	if ((ev.args = calloc (argc, sizeof *argv)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	ev.args = *argv;

	json = encode_event (&ev, &argv[optind], argc - optind);
	json_print (json);
	json_free (json);

	return (0);
}
