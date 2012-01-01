/* by Pace Willisson */

%union {
	struct json *value;
}

%token <value> STRING
%type <value> value 
%type <value> array array_elts
%type <value> object object_elts object_elt

%{

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "json.h"

int yydebug;

static int errflag;

int yyparse (void);

YYSTYPE yylval;

struct json *build_list;

static struct json *
make_value (enum json_type type)
{
	struct json *vp;

	if ((vp = calloc (1, sizeof *vp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}
	
	vp->type = type;

	vp->build_link = build_list;
	build_list = vp;

	return (vp);
}

static char const *json_input_orig;
static char const *json_input;

static struct json *
make_string_value (char *str)
{
	struct json *jp;

	jp = make_value (JSON_STRING);
	if ((jp->valstr = strdup (str)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}
	return (jp);
}

int
yylex (void)
{
	int c;
	int ival;
	int lastc;
	char const *p;
	int len;
	char *sval;
	char *outp;

	while ((c = *json_input) != 0) {
		if (isspace (*json_input) == 0)
			break;
		json_input++;
	}

	if (c == '"') {
		json_input++;

		lastc = 0;
		for (p = json_input; *p; p++) {
			if (lastc != '\\' && *p == '"')
				break;
			lastc = *p;
		}
		/* max possible len */
		len = p - json_input;

		if ((sval = malloc (len + 1)) == NULL) {
			fprintf (stderr, "out of memory\n");
			exit (1);
		}
			
		outp = sval;
		while ((c = *json_input) != 0) {
			json_input++;
			if (c == '"')
				break;

			if (c != '\\') {
				*outp++ = c;
			} else {
				if ((c = *json_input) == 0) {
					errflag = 1;
					break;
				}
				json_input++;

				switch (c) {
				default: *outp++ = c; break;
				case 'b': *outp++ = '\b'; break;
				case 'f': *outp++ = '\f'; break;
				case 'n': *outp++ = '\n'; break;
				case 'r': *outp++ = '\r'; break;
				case 't': *outp++ = '\t'; break;
				case 'u':
					/*
					 * input \uXXXX (6 chars)
					 * output 1,2 or 3 bytes
					 */
					p = json_input;
					if (!p[0] || !p[1] || !p[2] || !p[3]
					    || sscanf (p, "%4x", &ival) != 1) {
						errflag = 1;
						break;
					}
					json_input += 4;

					if (ival < 0x7f) {
						*outp++ = ival;
					} else if (ival < 0x7ff) {
						*outp++ = 0xc0
							|((ival>>6)&0x1f);
						*outp++ = 0x80
							|(ival & 0x3f);
					} else {
						*outp++ = 0xe0
							|((ival>>12)&0x0f);
						*outp++ = 0x80
							|((ival>>6)&0x3f);
						*outp++ = 0x80
							|(ival & 0x3f);
					}
					break;
				}
			}
		}

		if (errflag) {
			free (sval);
			return (0);
		}

		*outp = 0;

		yylval.value = make_value (JSON_STRING);
		yylval.value->valstr = sval;

		return (STRING);
	}

	if (strncasecmp (json_input, "true", 4) == 0) {
		json_input += 4;
		yylval.value = make_string_value ("true");
		return (STRING);
	}

	if (strncasecmp (json_input, "false", 5) == 0) {
		json_input += 5;
		yylval.value = make_string_value ("false");
		return (STRING);
	}

	if (strncasecmp (json_input, "null", 4) == 0) {
		json_input += 4;
		yylval.value = make_string_value ("");
		return (STRING);
	}

	if ((c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.') {
		len = 1;
		while ((c = json_input[len]) != 0) {
			if ((c >= '0' && c <= '9')
			    || c == 'e' || c == 'E'
			    || c == '-' || c == '+' || c == '.') {
				len++;
			} else {
				break;
			}
		}

		if ((sval = malloc (len + 1)) == NULL) {
			fprintf (stderr, "out of memory\n");
			exit (1);
		}

		memcpy (sval, json_input, len);
		sval[len] = 0;

		json_input += len;

		yylval.value = make_value (JSON_STRING);
		yylval.value->valstr = sval;
		return (STRING);
	}

	if (*json_input)
		json_input++;

	return (c);
}

static struct json *json_result;

void
yyerror (char const *err)
{
	printf ("error %s\n", err);
	printf ("input: %s\n", json_input_orig);
	errflag = 1;
}

%}

%%

json: value { json_result = $1; }

value: STRING { $$ = $1; }
	| array { $$ = $1; }
	| object { $$ = $1; }

array: '[' array_elts ']' { 
		$$ = make_value (JSON_ARRAY); 
		$$->children = $2;
	}

array_elts: /* empty */ { $$ = NULL; }
	| value { $$ = $1; }
	| value ',' array_elts {
		$1->sibling_next = $3;
		$$ = $1;
	}


object: '{' object_elts '}' { 
		$$ = make_value (JSON_OBJECT); 
		$$->children = $2;
	}

object_elts: /* empty */ { $$ = NULL; }
	| object_elt { $$ = $1; }
	| object_elt ',' object_elts {
		$1->sibling_next = $3;
		$$ = $1;
	}

object_elt: STRING ':' value { $$ = $3; $$->key = $1; }

%%

struct json *
json_decode (char const *input_str)
{
	struct json *jp;

	json_input_orig = input_str;
	json_input = json_input_orig;

	errflag = 0;
	build_list = NULL;
	yyparse ();
	if (errflag) {
		while ((jp = build_list) != NULL) {
			build_list = jp->build_link;
			if (jp->valstr)
				free (jp->valstr);
			free (jp);
		}
		return (NULL);
	}

	return (json_result);
}


void
json_free1 (struct json *root, int myseq, int level)
{
	struct json *val;
	static int seq;

	if (0) {
		printf ("%d json_free %d %p ", seq, level, root);
		if (root) {
			printf ("%d ", root->type);
			if (root->key)
				printf ("%s ", root->key->valstr);
			if (root->valstr)
				printf ("%s ", root->valstr);
		}
		printf ("\n");
	}

	seq++;
	level++;

	if (root == NULL)
		return;

	json_free1 (root->key, seq, level);
	json_free1 (root->children, seq, level);

	while ((val = root->sibling_next) != NULL) {
		root->sibling_next = val->sibling_next;
		val->sibling_next = NULL;
		json_free1 (val, seq, level);
	}

	if (root->valstr)
		free (root->valstr);

	free (root);
}

void
json_free (struct json *root)
{
	json_free1 (root, 0, 0);
}

static void
indent (int level)
{
	int i;

	for (i = 0; i < level * 2; i++)
		putchar (' ');
}

static void
json_print1 (struct json *p, int level)
{
	struct json *p1;

	switch (p->type) {
	case JSON_STRING:
		indent (level);
		printf ("\"%s\"", p->valstr);
		break;
	case JSON_ARRAY:
		indent (level);
		printf ("[\n");
		for (p1 = p->children; p1; p1 = p1->sibling_next) {
			json_print1 (p1, level + 1);
			if (p1->sibling_next)
				putchar (',');
			putchar ('\n');
		}
		indent (level);
		printf ("]");
		break;
	case JSON_OBJECT:
		indent (level);
		printf ("{\n");
		for (p1 = p->children; p1; p1 = p1->sibling_next) {
			indent (level+1);
			printf ("\"%s\" : ", p1->key->valstr);
			json_print1 (p1, level + 2);
			if (p1->sibling_next)
				putchar (',');
			putchar ('\n');
		}
		indent (level);
		printf ("}");
		break;
	}
}

void
json_print (struct json *p)
{
	json_print1 (p, 0);
	printf ("\n");
}

enum json_type
json_get_type (struct json *json)
{
	return (json->type);
}

size_t
json_array_size (struct json *json)
{
	struct json *jp;
	int size;

	if (json->type != JSON_ARRAY)
		return (0);

	size = 0;
	for (jp = json->children; jp; jp = jp->sibling_next)
		size++;

	return (size);
}

struct json *
json_aref (struct json *json, size_t idx)
{
	struct json *jp;

	if (json->type != JSON_ARRAY)
		return (NULL);

	for (jp = json->children; jp; jp = jp->sibling_next) {
		if (idx == 0)
			return (jp);
		idx--;
	}

	return (NULL);
}

char *
json_aref_str (struct json *json, size_t idx)
{
	struct json *jp;

	if ((jp = json_aref (json, idx)) == NULL
	    || jp->type != JSON_STRING)
		return ("");

	return (jp->valstr);
}


struct json *
json_objref (struct json *json, char const *key)
{
	struct json *jp;

	if (json->type != JSON_OBJECT)
		return (NULL);

	for (jp = json->children; jp; jp = jp->sibling_next) {
		if (strcmp (jp->key->valstr, key) == 0)
			return (jp);
	}

	return (NULL);
}

char *
json_objref_str (struct json *json, char const *key)
{
	struct json *jp;

	if ((jp = json_objref (json, key)) == NULL
	    || jp->type != JSON_STRING)
		return ("");

	return (jp->valstr);
}

/* returns one memory block with pointers and strings, caller frees */
char **
json_obj_keys (struct json *json)
{
	struct json *jp;
	char **keys;
	int pool;
	int nkeys;
	char *pool_base;
	int len;

	if (json->type != JSON_OBJECT)
		return (NULL);

	nkeys = 0;
	pool = 0;
	for (jp = json->children; jp; jp = jp->sibling_next) {
		pool += strlen (jp->key->valstr) + 1;
		nkeys++;
	}
	nkeys++; /* add one for null terminator */

	if ((keys = malloc (nkeys * sizeof (char *) + pool)) == NULL)
		return (NULL);
	pool_base = (char *)&keys[nkeys];

	nkeys = 0;
	pool = 0;
	for (jp = json->children; jp; jp = jp->sibling_next) {
		len = strlen (jp->key->valstr);
		keys[nkeys] = pool_base + pool;
		memcpy (keys[nkeys], jp->key->valstr, len);
		keys[nkeys][len] = 0;
		pool += strlen (jp->key->valstr) + 1;
		nkeys++;
	}
	keys[nkeys] = NULL;

	return (keys);
}

int
json_encode_grow (struct json_encode_buf *bp, int c)
{
	bp->avail += 10000;
	if ((bp->buf = realloc (bp->buf, bp->avail)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}
	bp->buf[bp->used++] = c;
	return (c);
}

void
json_encode_str (struct json_encode_buf *bp, char const *str)
{
	char const *p;
	int c;

	json_encode_put (bp, '"');
	if (str) {
		for (p = str; *p; p++) {
			c = *p & 0xff;
			if (c >= ' ' && c < 0x7f
			    && c != '\\'
			    && c != '"') {
				json_encode_put (bp, c);
			} else {
				switch (c) {
				default: {
					char ubuf[100], *up;
					sprintf (ubuf, "\\u%04x", c);
					for (up = ubuf; *up; up++)
						json_encode_put (bp, *up);
					break;
				}

				case '\\':
					json_encode_put (bp, '\\');
					json_encode_put (bp, '\\');
					break;
				case '"':
					json_encode_put (bp, '\\');
					json_encode_put (bp, '"');
					break;
				case '\b':
					json_encode_put (bp, '\\');
					json_encode_put (bp, 'b');
					break;
				case 'f':
					json_encode_put (bp, '\\');
					json_encode_put (bp, 'f');
					break;
				case 'n':
					json_encode_put (bp, '\\');
					json_encode_put (bp, 'n');
					break;
				case 'r':
					json_encode_put (bp, '\\');
					json_encode_put (bp, 'r');
					break;
				case 't':
					json_encode_put (bp, '\\');
					json_encode_put (bp, 't');
					break;
				}
			}
		}
	}
	json_encode_put (bp, '"');
}

void
json_encode_num (struct json_encode_buf *bp, double num)
{
	char buf[100];

	sprintf (buf, "%.16g", num);
	json_encode_str (bp, buf);
}

static void
json_encode1 (struct json_encode_buf *bp, struct json *p)
{
	struct json *p1;

	switch (p->type) {
	case JSON_STRING:
		json_encode_str (bp, p->valstr);
		break;
	case JSON_ARRAY:
		json_encode_put (bp, '[');
		for (p1 = p->children; p1; p1 = p1->sibling_next) {
			json_encode1 (bp, p1);
			if (p1->sibling_next)
				json_encode_put (bp, ',');
		}
		json_encode_put (bp, ']');
		break;
	case JSON_OBJECT:
		json_encode_put (bp, '{');
		for (p1 = p->children; p1; p1 = p1->sibling_next) {
			json_encode_str (bp, p1->key->valstr);
			if (p1->sibling_next)
				json_encode_put (bp, ',');
		}
		json_encode_put (bp, '}');
		break;
	}
}

/* caller must free result */
char *
json_encode (struct json *p)
{
	struct json_encode_buf jbuf;
	
	memset (&jbuf, 0, sizeof jbuf);

	json_encode1 (&jbuf, p);
	json_encode_put (&jbuf, 0);

	return (realloc (jbuf.buf, strlen (jbuf.buf) + 1));
}

struct json *
json_make_str (char const *str)
{
	struct json *jp;

	if ((jp = calloc (1, sizeof *jp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	jp->type = JSON_STRING;
	if ((jp->valstr = strdup (str)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	return (jp);
}

struct json *
json_make_obj (void)
{
	struct json *jp;

	if ((jp = calloc (1, sizeof *jp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	jp->type = JSON_OBJECT;
	
	return (jp);
}

struct json *
json_make_arr (void)
{
	struct json *jp;

	if ((jp = calloc (1, sizeof *jp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	jp->type = JSON_ARRAY;
	
	return (jp);
}

struct json *
json_dup (struct json *oldp)
{
	struct json *newp, *cp, **tailp;

	if (oldp == NULL)
		return (NULL);

	if ((newp = calloc (1, sizeof *newp)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}
	newp->type = oldp->type;
	newp->key = json_dup (oldp->key);
	
	tailp = &newp->children;
	for (cp = oldp->children; cp; cp = cp->sibling_next) {
		*tailp = json_dup (cp);
		tailp = &(*tailp)->sibling_next;
	}
	if (oldp->valstr) {
		if ((newp->valstr = strdup (oldp->valstr)) == NULL) {
			fprintf (stderr, "out of memory\n");
			exit (1);
		}
	}
	return (newp);
}

void
json_objset_json (struct json *jp, char const *key, struct json *val)
{
	struct json *cp, **prevp, *nextp;
	if (jp->type != JSON_OBJECT) {
		fprintf (stderr, "json_objset_str: not an object\n");
		exit (1);
	}

	prevp = &jp->children;
	for (cp = *prevp; cp; prevp = &cp->sibling_next, cp = *prevp) {
		if (cp->key
		    && cp->key->type == JSON_STRING
		    && strcmp (cp->key->valstr, key) == 0) {
			break;
		}
	}

	if (cp == NULL) {
		nextp = NULL;
	} else {
		if (cp->build_link) {
			fprintf (stderr, "json_decode result not allowed\n");
			exit (1);
		}

		nextp = cp->sibling_next;
		cp->sibling_next = NULL;
		json_free (cp);
	}
	
	cp = json_dup (val);
	cp->key = json_make_str (key);

	*prevp = cp;
	cp->sibling_next = nextp;
}

void
json_objset_str (struct json *jp, char const *key, char const *str)
{
	struct json *val;

	val = json_make_str (str);
	json_objset_json (jp, key, val);
	json_free (val);
}

void
json_objset_num (struct json *jp, char const *key, double val)
{
	char numbuf[100];

	snprintf (numbuf, sizeof numbuf, "%g", val);
	json_objset_str (jp, key, numbuf);
}

void
json_aset_json (struct json *jp, int idx, struct json *val)
{
	struct json *cp, **prevp, *nextp;
	int count;
	if (jp->type != JSON_ARRAY) {
		fprintf (stderr, "json_aset_str: not an array\n");
		exit (1);
	}

	prevp = &jp->children;
	count = 0;
	for (cp = *prevp; cp; prevp = &cp->sibling_next, cp = *prevp) {
		if (count == idx)
			break;
		count++;
	}

	while (count < idx) {
		*prevp = json_make_str ("");
		prevp = &(*prevp)->sibling_next;
		count++;
	}

	if (cp == NULL) {
		nextp = NULL;
	} else {
		if (cp->build_link) {
			fprintf (stderr, "json_decode result not allowed\n");
			exit (1);
		}

		nextp = cp->sibling_next;
		cp->sibling_next = NULL;
		json_free (cp);
	}
	
	cp = json_dup (val);

	*prevp = cp;
	cp->sibling_next = nextp;
}

void
json_aset_str (struct json *jp, int idx, char const *str)
{
	struct json *val;
	
	val = json_make_str (str);
	json_aset_json (jp, idx, val);
	json_free (val);
}

void
json_aset_num (struct json *jp, int idx, double val)
{
	char numbuf[100];

	snprintf (numbuf, sizeof numbuf, "%g", val);
	json_aset_str (jp, idx, numbuf);
}

struct json_encode_buf *
json_encode_start (void)
{
	struct json_encode_buf *bp;

	if ((bp = calloc (1, sizeof *bp)) == NULL)
		return (NULL);

	return (bp);
}

void
json_encode_put_key_str (struct json_encode_buf *bp,
			 char const *key, char const *str)
{
	json_encode_str (bp, key);
	json_encode_put (bp, ':');
	json_encode_str (bp, str);
}

void
json_encode_put_key_num (struct json_encode_buf *bp,
			 char const *key, double num)
{
	char buf[100];

	sprintf (buf, "%.16g", num);
	json_encode_put_key_str (bp, key, buf);
}


void
json_encode_arr_start (struct json_encode_buf *bp)
{
	json_encode_put (bp, '[');
}

void
json_encode_arr_comma (struct json_encode_buf *bp)
{
	if (bp->used && bp->buf[bp->used - 1] != '[')
		json_encode_put (bp, ',');
}	

void
json_encode_arr_elt_str (struct json_encode_buf *bp, char const *str)
{
	json_encode_arr_comma (bp);
	json_encode_str (bp, str);
}

void
json_encode_arr_finish (struct json_encode_buf *bp)
{
	json_encode_put (bp, ']');
}

char *
json_encode_finish (struct json_encode_buf *bp)
{
	char *ret;

	json_encode_put (bp, 0);
	
	ret = realloc (bp->buf, bp->used);
	free (bp);
	return (ret);
}
