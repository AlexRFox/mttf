#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include "json.h"

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
		fprintf (stderr, "error opening file\n");
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
	int size;
	long nextyear, nextmonth, nextday;
	struct passwd *pp;
	struct json *queue, *newqueue, *cur, *new;
	struct tm tm;
	char *queuename, *home, *username, queuefile[10000], *jsonstr, *p;
	int uid, pid, idx;
	time_t t;
	FILE *f;

	queuename = "queue.json";

	while ((pp = getpwent ()) != NULL) {
		home = pp->pw_dir;
		uid = pp->pw_uid;
		username = pp->pw_name;

		sprintf (queuefile, "%s/%s", home, queuename);

		if (access (queuefile, F_OK) == 0) {
			pid = fork ();

			if (pid == -1) {
				printf ("fork error %m\n");
				exit (1);
			} else if (pid == 0) {
				break;
			} else {
				continue;
			}
		}
	}

	endpwent ();

	if (pid > 0) {
		return (0);
	}

	pid = fork ();

	if (pid == -1) {
		printf ("fork error %m\n");
		exit (1);
	} else if (pid > 0) {
		return (0);
	}

	setsid ();

	for (idx = 3; idx < 100; idx++) {
		close (idx);
	}

	if (chdir (home) == -1) {
		printf ("chdir error %m\n");
		exit (1);
	}

	if (setuid (uid) == -1) {
		printf ("error switching to %s,%d: %m\n", username, uid);
		return (1);
	}

	if ((f = fopen (queuefile, "r")) == NULL) {
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

	queue = json_decode (jsonstr);

	newqueue = json_make_arr ();

	t = time (NULL);
	tm = *localtime (&t);

	idx = 0;
	while (idx < json_array_size (queue)) {
		cur = json_aref (queue, idx);
		new = NULL;
		idx++;

		nextyear = strtol (json_objref_str (cur, "nextyear"), &p, 0);
		nextmonth = strtol (json_objref_str (cur, "nextmonth"), &p, 0);
		nextday = strtol (json_objref_str (cur, "nextday"), &p, 0);

		if (nextyear <= tm.tm_year + 1900
		    && nextmonth <= tm.tm_mon + 1
		    && nextday <= tm.tm_mday) {
			new = run_script (cur);

			if (strcmp (json_objref_str (new,
						     "returned"), "1") == 0) {
				json_aset_json (newqueue,
						json_array_size (newqueue),
						new);
			}
		} else {
			json_aset_json (newqueue,
					json_array_size (newqueue), cur);
		}			
	}

	fclose (f);

	if ((f = fopen (queuefile, "w")) == NULL) {
		fprintf (stderr, "error opening file\n");
		return (1);
	}

	char *newjson;
	newjson = json_encode (newqueue);
	fwrite (newjson, 1, strlen (newjson), f);
	fwrite ("\n", 1, 1, f);
	fclose (f);

	return (0);
}
