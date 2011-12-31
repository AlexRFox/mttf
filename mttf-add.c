#include <stdio.h>
#include "json.h"

struct event {
	char *title;
};

struct json *
encode_event (struct event *evp)
{
	struct json *jp;
	struct json *arr;

	jp = json_make_obj ();
	json_objset_str (jp, "title", evp->title);
	json_objset_num (jp, "test", 123);

	arr = json_make_arr ();
	json_aset_num (arr, 0, 0);
	json_aset_num (arr, 1, 111);
	json_aset_num (arr, 3, 333);
	json_aset_num (arr, 2, 222);
	json_aset_num (arr, 1, 1111);
	json_objset_json (jp, "atest", arr);

	return (jp);
}

int
main (int argc, char **argv)
{
	char *str;
	struct event ev;
	struct json *json;

	ev.title = "alex";

	json = encode_event (&ev);
	json_print (json);
	json_free (json);

	if (0) {
		str = "{\"hello\": \"world\"}";
		printf ("%s\n", str);
		json = json_decode (str);
		json_print (json);
	}

	return (0);
}
