#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct event {
	char *script, *title, *date, *repeat, *ical_recur;
};

struct event ev;

void
usage (void)
{
	printf ("bad usage\n");
	exit (1);
}

char *
parse_recur (void)
{
	int val;
	char unit;

	sscanf (ev.repeat, "%d%c", &val, &unit);

	printf ("%d, %c\n", val, unit);

	return (strdup (""));
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
		ev.ical_recur = parse_recur ();
	}

	if (optind != argc)
		usage ();

	printf ("script: %s\n", ev.script);
	printf ("title: %s\n", ev.title);
	printf ("date: %s\n", ev.date);
	if (*ev.ical_recur)
		printf ("recurrence: %s\n", ev.ical_recur);

	return (0);
}
