#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "json.h"

struct event {
	char *script, *title, *date, *repeat, *ical_recur, *descript;
	int run_year, run_month, run_day;
};

struct event ev;

struct json *
encode_event (struct event *evp)
{
	struct json *jp;

	jp = json_make_obj ();
	json_objset_str (jp, "script", evp->script);
	json_objset_str (jp, "title", evp->title);
	if (evp->descript)
		json_objset_str (jp, "descript", evp->descript);
	json_objset_str (jp, "date", evp->date);
	if (evp->ical_recur)
		json_objset_str (jp, "recurrence", evp->ical_recur);

	

	return (jp);
}

void
usage (void)
{
	printf ("bad usage\n");
	exit (1);
}

void
parse_recur (void)
{
	int val, plural;
	char *unit_name, noval_unit, val_unit, *inp_unit;

	plural = 0;
	unit_name = "";
	ev.descript = "";
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
	int c;
	struct json *json;
	struct tm tm;

	while ((c = getopt (argc, argv, "d:")) != EOF) {
		switch (c) {
		case 'd':
			if ((ev.descript = calloc (1, sizeof optarg)) == NULL) {
				fprintf (stderr, "memory error\n");
				exit (1);
			}
			ev.descript = optarg;
			break;
		default:
			break;
		}
	}

	if (optind + 3 > argc)
		usage ();

	ev.script = argv[optind++];
	ev.title = argv[optind++];
	ev.date = argv[optind++];

	ev.repeat = "";

	if (optind < argc) {
		ev.repeat = argv[optind++];
		parse_recur ();
	}

	if (optind != argc)
		usage ();

	time_t t;
	t = time (NULL);
	tm = *localtime (&t);

	printf ("%d\n", tm.tm_year + 1900);
	printf ("%d\n", tm.tm_mon + 1);
	printf ("%d\n", tm.tm_mday);

	sscanf (ev.date, "%d-%d-%d", &ev.run_year, &ev.run_month, &ev.run_day);

	json = encode_event (&ev);
	json_print (json);
	json_free (json);

	return (0);
}
