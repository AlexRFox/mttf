enum json_type {
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT,
};

struct json {
	enum json_type type;
	struct json *key;
	struct json *sibling_next;
	struct json *children;

	char *valstr;

	struct json *build_link;
};


struct json *json_decode (char const *str);
void json_print (struct json *p);
void json_free (struct json *root);

enum json_type json_get_type (struct json *json);
size_t json_array_size (struct json *json);
struct json *json_aref (struct json *json, size_t idx);
char *json_aref_str (struct json *json, size_t idx);
struct json *json_objref (struct json *json, char const *key);
char *json_objref_str (struct json *json, char const *key);

struct json *json_dup (struct json *oldp);


/* returns one memory block with pointers and strings, caller frees */
char **json_obj_keys (struct json *json);

/* caller must free result */
char *json_encode (struct json *json);

struct json *json_make_str (char const *str);
struct json *json_make_obj (void);
struct json *json_make_arr (void);

void json_objset_json (struct json *json, char const *key, struct json *val);
void json_objset_str (struct json *json, char const *key, char const *val);
void json_objset_num (struct json *json, char const *key, double val);

void json_aset_json (struct json *json, int idx, struct json *val);
void json_aset_str (struct json *json, int idx, char const *val);
void json_aset_num (struct json *json, int idx, double val);

struct json_encode_buf {
	char *buf;
	int avail;
	int used;
};

int json_encode_grow (struct json_encode_buf *bp, int c);

#define json_encode_put(bp,c) ((bp)->used < (bp)->avail \
			       ? (bp)->buf[(bp)->used++] = (c) \
			       : json_encode_grow ((bp), (c)))

struct json_encode_buf *json_encode_start (void);
char *json_encode_finish (struct json_encode_buf *bp);
void json_encode_put_key_num (struct json_encode_buf *bp,
			      char const *key, double num);

void json_encode_put_key_str (struct json_encode_buf *bp,
			      char const *key, char const *str);

void json_encode_str (struct json_encode_buf *bp, char const *str);
void json_encode_num (struct json_encode_buf *bp, double num);

void json_encode_arr_comma (struct json_encode_buf *bp);

void json_encode_arr_start (struct json_encode_buf *bp);
void json_encode_arr_elt_str (struct json_encode_buf *buf, char const *str);
void json_encode_arr_finish (struct json_encode_buf *bp);
