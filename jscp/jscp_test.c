
#include "jscp.h"
#include <string.h>
#include <stdio.h>

void jscp_dump_node(jscp_doc_t *doc, jscp_node_t *node, int nest)
{
	int i;
	jscp_node_t *cn;

	for (i = 0; i < nest; i++) printf("  ");

	switch (jscp_type(node)) {
	case JSCP_TYPE_STR:
		printf("\"%.*s\"", jscp_str_len(node), jscp_str_val(doc, node));
		break;
	case JSCP_TYPE_INT:
		printf("%d", jscp_int_val(node));
		break;
	case JSCP_TYPE_NULL:
		printf("null");
		break;
	case JSCP_TYPE_BOOL:
		printf("%s", jscp_bool_val(node) ? "true" : "false");
		break;
	case JSCP_TYPE_ARRAY:
		printf("[\n");
		for (i = 0, cn = jscp_child_head(doc, node); cn; cn = jscp_array_next(doc, cn))
		{
			if (i > 0) printf(",\n");
			jscp_dump_node(doc, cn, nest + 1);
			i++;
		}
		printf("\n");
		for (i = 0; i < nest; i++) printf("  ");
		printf("]");
		break;
	case JSCP_TYPE_OBJECT:
		printf("{\n");
		for (i = 0, cn = jscp_child_head(doc, node); cn; cn = jscp_object_next(doc, cn))
		{
			if (i > 0) printf(",\n");
			jscp_dump_node(doc, cn, nest + 1);
			printf(":\n");
			jscp_dump_node(doc, jscp_object_val(doc, cn), nest + 1);
			i++;
		}
		printf("\n");
		for (i = 0; i < nest; i++) printf("  ");
		printf("}");
		break;
	}
}

void jscp_dump(jscp_doc_t *doc)
{
	jscp_dump_node(doc, jscp_root(doc), 0);
	printf("\n");
}

int main(void)
{
	char msg[] = "[[456,128],33,true,[],{},false, null, {\"test\":567, \"tt\":{\"test5\":567},\"test3\":1567}]";
	int r;
	jscp_doc_t doc;
	jscp_union_node_t nodes[256];
	r = jscp_parse(msg, strlen(msg), nodes, 256, &doc);

	if (r >= 0) {
		jscp_dump(&doc);
	}
	else {
		printf("err=%d\n", r);
	}

	return 0;
}
