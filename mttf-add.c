#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct event {
	char *script, *title, *date, *repeat, *ical_recur;
	int run_year, run_month, run_day;
};

struct event ev;

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

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
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

	sscanf (ev.date, "%d-%d-%d", &ev.run_year, &ev.run_month, &ev.run_day);

	printf ("script: %s\n", ev.script);
	printf ("title: %s\n", ev.title);
	printf ("run script on: %d-%.2d-%.2d\n", ev.run_year, ev.run_month, ev.run_day);

	if (*ev.ical_recur)
		printf ("recurrence: %s\n", ev.ical_recur);

	printf ("(once implemented) adding initialization to queue\n");

	return (0);
}
