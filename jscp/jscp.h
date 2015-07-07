
#ifndef __JSCP_H
#define __JSCP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSCP_TYPE_NULL		0
#define JSCP_TYPE_INT		1
#define JSCP_TYPE_STR		2
#define JSCP_TYPE_BOOL		3
#define JSCP_TYPE_ARRAY		4
#define JSCP_TYPE_OBJECT	5

#define JSCP_INVALID_IDX	-1

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
} jscp_node_t;

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
	int32_t		value;
} jscp_int_node_t;

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
	uint16_t	text_idx;
	uint16_t	text_len;
} jscp_str_node_t;

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
	int32_t		value;
} jscp_bool_node_t;

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
	int16_t		len;
	int16_t		child_node_idx;
} jscp_array_node_t;

typedef struct {
	int16_t		next_node_idx;
	int16_t		type;
	int16_t		len;
	int16_t		child_node_idx;
} jscp_object_node_t;

typedef union {
	jscp_node_t			n;
	jscp_int_node_t		i;
	jscp_str_node_t		s;
	jscp_bool_node_t	b;
	jscp_array_node_t	a;
	jscp_object_node_t	o;
} jscp_union_node_t;

typedef struct {
	char				*text;
	int					text_len;
	jscp_node_t			*root_node;
	jscp_union_node_t	*node_mem;
	int					node_mem_len;
	int					node_mem_pos;
} jscp_doc_t;

typedef int jscp_bool_t;
#define JSCP_TRUE	1
#define JSCP_FALSE	0

#define JSCP_E_NOMEM								-1
#define JSCP_E_SYNTAX_UNMATCH_ARRAY_BRACKET			-2
#define JSCP_E_SYNTAX_UNMATCH_OBJECT_BRACKET		-3
#define JSCP_E_SYNTAX_STRING_LITERAL_NOT_CLOSED		-4
#define JSCP_E_SYNTAX_INVALID_INTEGER_FORMAT		-5
#define JSCP_E_SYNTAX_ARRAY_COMMA_NOT_FOUND			-6
#define JSCP_E_SYNTAX_OBJECT_KEY_NOT_STRING			-7
#define JSCP_E_SYNTAX_OBJECT_COLON_NOT_FOUND		-8
#define JSCP_E_SYNTAX_OBJECT_COMMA_NOT_FOUND		-9
#define JSCP_E_SYNTAX								-10

int jscp_parse(char *text, int text_len, jscp_union_node_t *node_mem, int node_mem_len, jscp_doc_t *doc);

static jscp_node_t *jscp_get_root(jscp_doc_t *doc) {
	return doc->root_node;
}
static int jscp_get_type(jscp_node_t *node) { return node->type; }

static jscp_node_t *jscp_get_child_head(jscp_doc_t *doc, jscp_node_t *node)
{
	return (jscp_node_t *)&doc->node_mem[((jscp_array_node_t *)node)->child_node_idx];
}

static jscp_node_t *jscp_get_next(jscp_doc_t *doc, jscp_node_t *node)
{
	return (jscp_node_t *)&doc->node_mem[node->next_node_idx];
}

static int32_t jscp_get_int_val(jscp_node_t *node)
{
	return ((jscp_int_node_t *)node)->value;
}

static const char *jscp_get_str_val(jscp_doc_t *doc, jscp_node_t *node)
{
	return doc->text + ((jscp_str_node_t *)node)->text_idx;
}

static int jscp_get_str_len(jscp_node_t *node)
{
	return ((jscp_str_node_t *)node)->text_len;
}

static jscp_bool_t jscp_get_bool_val(jscp_node_t *node)
{
	return ((jscp_bool_node_t *)node)->value;
}

static int jscp_get_array_len(jscp_node_t *node) {
	return ((jscp_array_node_t *)node)->len;
}

static int jscp_get_object_len(jscp_node_t *node) {
	return ((jscp_object_node_t *)node)->len >> 1;
}

#ifdef __cplusplus
}
#endif

#endif
