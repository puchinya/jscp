
#include "jscp.h"

typedef struct {
	char	*s;
	int		len;
} jscp_substr_t;

static void jscp_skip_spaces(jscp_substr_t *ss);
static jscp_union_node_t *jscp_new_node(jscp_doc_t *doc);

static void jscp_skip_spaces(jscp_substr_t *ss)
{
	while (ss->len > 0) {
		int ch = *ss->s;
		switch (ch) {
		case ' ':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			ss->s++;
			ss->len--;
			break;
		default:
			return;
		}
	}
}

static jscp_union_node_t *jscp_new_node(jscp_doc_t *doc)
{
	if (doc->node_mem_pos >= doc->node_mem_len)
		return 0;
	return &doc->node_mem[doc->node_mem_pos++];
}

static int jscp_read_ch(jscp_substr_t *ss)
{
	int ch;
	if (ss->len == 0) return -1;
	ch = *ss->s;
	ss->s++;
	ss->len--;
	return ch;
}

static void jscp_unget_ch(jscp_substr_t *ss)
{
	ss->s--;
	ss->len++;
}

#define JSCP_TYPEMASK_PARSING		0x10
#define JSCP_TYPEMASK_PARSING_KEY	0x20

static jscp_union_node_t *jscp_up_parent(jscp_doc_t *doc)
{
	int i;
	i = doc->node_mem_pos - 1;
	for (; i >= 0; i--){
		if (doc->node_mem[i].n.type & 0xF0)
			return &doc->node_mem[i];
	}
	return 0;
}

static jscp_union_node_t *jscp_last_node(jscp_doc_t *doc, jscp_union_node_t *parent)
{
	jscp_union_node_t *n;
	int i = parent->a.child_node_idx;
	if (i == JSCP_INVALID_IDX) return 0;

	for (;;) {
		n = &doc->node_mem[i];
		if (n->n.next_node_idx == JSCP_INVALID_IDX)
			return n;
		i = n->n.next_node_idx;
	}
}

int32_t jscp_strtoi(const char *s, int len)
{
	int32_t r = 0;
	int sign = 0;

	if (*s == '-') {
		sign = 1;

		s++; len--;
	}
	for (;;){
		if (len == 0) break;
		r = r * 10 + (*s - '0');

		s++; len--;
	}

	if (sign) r = -r;

	return r;
}

int jscp_parse(char *text, int text_len, jscp_union_node_t *node_mem, int node_mem_len, jscp_doc_t *doc)
{
	int error_code = 0;
	int text_pos = 0;
	int ch;
	jscp_substr_t ss;
	jscp_union_node_t *parent_node = 0;
	jscp_union_node_t *prev_node = 0;

	doc->text = text;
	doc->text_len = text_len;
	doc->node_mem = node_mem;
	doc->node_mem_len = node_mem_len;
	doc->root_node = 0;
	doc->node_mem_pos = 0;

	ss.s = text;
	ss.len = text_len;

	for (;;) {
		jscp_union_node_t *new_node = 0;

		jscp_skip_spaces(&ss);

		ch = jscp_read_ch(&ss);
		if (ch < 0) break;
		
		switch (ch)
		{
		case '\0':
			goto last;
			break;
		case '[':
			{
				new_node = jscp_new_node(doc);
				if (!new_node) {
					error_code = JSCP_E_NOMEM;
					goto error;
				}
				new_node->a.type = JSCP_TYPE_ARRAY | JSCP_TYPEMASK_PARSING;
				new_node->a.next_node_idx = JSCP_INVALID_IDX;
				new_node->a.len = 0;
				new_node->a.child_node_idx = JSCP_INVALID_IDX;
			}
			break;
		case '{':
			{
				new_node = jscp_new_node(doc);
				if (!new_node) {
					error_code = JSCP_E_NOMEM;
					goto error;
				}
				new_node->a.type = JSCP_TYPE_OBJECT | JSCP_TYPEMASK_PARSING;
				new_node->a.next_node_idx = JSCP_INVALID_IDX;
				new_node->a.len = 0;
				new_node->a.child_node_idx = JSCP_INVALID_IDX;
			}
			break;
		case ']':
			if (!parent_node) {
				error_code = JSCP_E_SYNTAX_UNMATCH_ARRAY_BRACKET;
				goto error;
			}
			parent_node->a.type = JSCP_TYPE_ARRAY;
			parent_node = jscp_up_parent(doc);
			if (parent_node)
				prev_node = jscp_last_node(doc, parent_node);
			break;
		case '}':
			if (!parent_node) {
				error_code = JSCP_E_SYNTAX_UNMATCH_OBJECT_BRACKET;
				goto error;
			}
			parent_node->a.type = JSCP_TYPE_OBJECT;
			parent_node = jscp_up_parent(doc);
			if (parent_node)
				prev_node = jscp_last_node(doc, parent_node);
			break;
		case ',':
			if (!parent_node) {
				error_code = JSCP_E_SYNTAX;
				goto error;
			}
			break;
		case ':':
			if (!parent_node) {
				error_code = JSCP_E_SYNTAX;
				goto error;
			}
			break;
		case 't':
			{
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'r') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'u') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'e') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}

				new_node = jscp_new_node(doc);
				new_node->b.type = JSCP_TYPE_BOOL;
				new_node->b.next_node_idx = JSCP_INVALID_IDX;
				new_node->b.value = JSCP_TRUE;
			}
			break;
		case 'f':
			{
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'a') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'l') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 's') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}
				ch = jscp_read_ch(&ss);
				if (ch < 0 || ch != 'e') {
					error_code = JSCP_E_SYNTAX;
					goto error;
				}

				new_node = jscp_new_node(doc);
				new_node->b.type = JSCP_TYPE_BOOL;
				new_node->b.next_node_idx = JSCP_INVALID_IDX;
				new_node->b.value = JSCP_FALSE;
			}
			break;
		case 'n':
			{
					ch = jscp_read_ch(&ss);
					if (ch < 0 || ch != 'u') {
						error_code = JSCP_E_SYNTAX;
						goto error;
					}
					ch = jscp_read_ch(&ss);
					if (ch < 0 || ch != 'l') {
						error_code = JSCP_E_SYNTAX;
						goto error;
					}
					ch = jscp_read_ch(&ss);
					if (ch < 0 || ch != 'l') {
						error_code = JSCP_E_SYNTAX;
						goto error;
					}

					new_node = jscp_new_node(doc);
					new_node->n.type = JSCP_TYPE_NULL;
					new_node->n.next_node_idx = JSCP_INVALID_IDX;
			}
			break;
		case '-':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				char *text_start, *text_end;

				text_start = ss.s - 1;
						
				for (;;) {
					ch = jscp_read_ch(&ss);
					if (ch < 0) break;
					if (ch >= '0' && ch <= '9')
						continue;
							
					jscp_unget_ch(&ss);

					break;
				}

				text_end = ss.s;

				if (*text_start == '-' && text_end - text_start == 1) {
					error_code = JSCP_E_SYNTAX_INVALID_INTEGER_FORMAT;
					goto error;
				}

				new_node = jscp_new_node(doc);
				new_node->i.type = JSCP_TYPE_INT;
				new_node->i.next_node_idx = JSCP_INVALID_IDX;
				new_node->i.value = jscp_strtoi(text_start, text_end - text_start);
			}
			break;
		case '"':
			{
				int esc;
				char *text_start, *text_end;

				text_start = ss.s;

				esc = 0;
				for (;;) {
					ch = jscp_read_ch(&ss);
					if (ch < 0){
						error_code = JSCP_E_SYNTAX_STRING_LITERAL_NOT_CLOSED;
						goto error;
					}
					if (esc) {
						esc = 0;
					} else {
						if (ch == '\\') {
							esc = 1;
						}
						else if (ch == '"'){
							*(ss.s - 1) = '\0';
							break;
						}
					}
				}

				text_end = ss.s;

				new_node = jscp_new_node(doc);
				new_node->s.type = JSCP_TYPE_STR;
				new_node->s.next_node_idx = JSCP_INVALID_IDX;
				new_node->s.text_idx = text_start - text;
				new_node->s.text_len = text_end - text_start;
			}
			break;
		default:
			break;
		}

		if (new_node) {
			if (parent_node) {
				int type = parent_node->n.type & 0xF;
				if (type == JSCP_TYPE_ARRAY || type == JSCP_TYPE_OBJECT) {
					parent_node->a.len++;
					if (parent_node->a.child_node_idx == JSCP_INVALID_IDX) {
						parent_node->a.child_node_idx = doc->node_mem_pos - 1;
						prev_node = new_node;
					}
					else {
						prev_node->n.next_node_idx = doc->node_mem_pos - 1;
						prev_node = new_node;
					}
				}
			}
			{
				int type = new_node->n.type & 0xF;
				if (type == JSCP_TYPE_ARRAY || type == JSCP_TYPE_OBJECT) {
					parent_node = new_node;
					prev_node = 0;
				}
			}
			if (!doc->root_node) {
				doc->root_node = (jscp_node_t *)new_node;
			}
		}
	}
last:

	return 0;
error:
	return -1;
}
