#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include "json.h"

struct job {
	struct job *next;
	char *name, *home;
	int uid;
};

struct job *job_head;

int foreground;

void
usage (void)
{
	printf ("usage: mttf-queue [-f]\n");
	exit (1);
}

void *
xcalloc (unsigned int a, unsigned int b)
{
	void *p;

	if ((p = calloc (a, b)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	return (p);
}

struct json *
run_script (struct json *json)
{
	int fd, size;
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
	write (fd, "\n", 1);
	close (fd);

	script = json_objref_str (json, "script");
	sprintf (cmd, "%s %s", script, filename);
	if (system (cmd) != 0) {
		fprintf (stderr, "something bad happened\n");
		return (NULL);
	}

	if ((f = fopen (filename, "r")) == NULL) {
		fprintf (stderr, "error opening %s: %m\n", filename);
		return (NULL);
	}

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
	int size, uid, pid, idx, c, fd, count;
	long nextyear, nextmonth, nextday;
	struct passwd *pp;
	struct json *queue, *newqueue, *cur, *new;
	struct tm tm;
	struct job *jp;
	char *queuename, *home, *name, queuefile[10000], *jsonstr, newqf[10000];
	time_t t;
	FILE *f;

	queuename = "mttf-queue.json";

	foreground = 0;

	while ((c = getopt (argc, argv, "f")) != EOF) {
		switch (c) {
		case 'f':
			foreground = 1;
			break;
		default:
			break;
		}
	}

	if (optind > 2)
		usage ();

	while ((pp = getpwent ()) != NULL) {
		home = pp->pw_dir;
		uid = pp->pw_uid;
		name = pp->pw_name;

		sprintf (queuefile, "%s/%s", home, queuename);

		if (access (queuefile, F_OK) == 0) {
			jp = xcalloc (1, sizeof *jp);

			jp->name = strdup (name);
			jp->home = strdup (home);
			jp->uid = uid;

			if ( ! job_head) {
				job_head = jp;
			} else {
				jp->next = job_head;
				job_head = jp;
			}
		}
	}

	endpwent ();

	for (jp = job_head; jp; jp = jp->next) {
		pid = fork ();

		if (pid == -1) {
			printf ("fork error %m\n");
			exit (1);
		} else if (pid > 0) {
			continue;
		}

		pid = fork ();
		if (pid == -1) {
			printf ("fork error %m\n");
			exit (1);
		} else if (pid > 0) {
			return (0);
		}

		fflush (NULL);

		if (!foreground) {
			setsid ();

			if ((fd = open ("/dev/null", O_WRONLY)) == -1) {
				printf ("failed to open /dev/null: %m\n");
				return (1);
			}

			if (dup2 (fd, 1) == -1) {
				printf ("dup2 failed: %m\n");
				return (1);
			}
		}

		for (idx = 3; idx < 100; idx++) {
			close (idx);
		}

		if (chdir (jp->home) == -1) {
			printf ("chdir error %m\n");
			exit (1);
		}

		if (setuid (jp->uid) == -1) {
			printf ("error switching to %s,%d: %m\n", jp->name,
				jp->uid);
			return (1);
		}

		sprintf (queuefile, "%s/%s", jp->home, queuename);

		if ((f = fopen (queuefile, "r")) == NULL) {
			fprintf (stderr, "error opening %s: %m\n", queuefile);
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

		queue = json_decode (jsonstr);

		newqueue = json_make_arr ();

		t = time (NULL);
		tm = *localtime (&t);

		count = json_array_size (queue);
		for (idx = 0; idx < count; idx++) {
			cur = json_aref (queue, idx);

			nextyear = atoi (json_objref_str (cur, "nextyear"));
			nextmonth = atoi (json_objref_str (cur, "nextmonth"));
			nextday = atoi (json_objref_str (cur, "nextday"));

			if (nextyear <= tm.tm_year + 1900
			    && nextmonth <= tm.tm_mon + 1
			    && nextday <= tm.tm_mday) {
				new = run_script (cur);

// clean up "returned"
				if (strcmp (json_objref_str (new,
							     "returned"), "1") == 0) {
					json_aset_json (newqueue,
							json_array_size (newqueue),
							new);
				}
			} else {
				json_aset_json (newqueue,
						json_array_size (newqueue),
						cur);
			}
		}

		fclose (f);

		sprintf (newqf, "%s.new", queuefile);

		if ((f = fopen (newqf, "w")) == NULL) {
			fprintf (stderr, "error opening %s: %m\n", newqf);
			return (1);
		}

		char *newjson;
		newjson = json_encode (newqueue);
		fwrite (newjson, 1, strlen (newjson), f);
		fwrite ("\n", 1, 1, f);
		fclose (f);

		if (rename (newqf, queuefile) == -1) {
			printf ("error copying in new queue: %m\n");
			return (1);
		}

		return (0);
	}

	return (0);
}
