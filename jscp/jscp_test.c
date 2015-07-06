
#include "jscp.h"
#include <string.h>
#include <stdio.h>

void jscp_dump_node(jscp_doc_t *doc, jscp_node_t *node, int nest)
{
	int i;
	jscp_node_t *cn;
	for (i = 0; i < nest; i++) printf("  ");
	switch (jscp_get_type(node)) {
	case JSCP_TYPE_STR:
		printf("\"%s\"", jscp_get_str_val(doc, node));
		break;
	case JSCP_TYPE_INT:
		printf("%d", jscp_get_int_val(node));
		break;
	case JSCP_TYPE_NULL:
		printf("null");
		break;
	case JSCP_TYPE_BOOL:
		printf("%s", jscp_get_bool_val(node) ? "true" : "false");
		break;
	case JSCP_TYPE_ARRAY:
		cn = jscp_get_child_head(doc, node);
		printf("[\n");
		for (i = 0; i < jscp_get_array_len(node); i++){
			if (i > 0) printf(",\n");
			jscp_dump_node(doc, cn, nest + 1);
			cn = jscp_get_next(doc, cn);
		}
		printf("\n");
		for (i = 0; i < nest; i++) printf("  ");
		printf("]");
		break;
	case JSCP_TYPE_OBJECT:
		cn = jscp_get_child_head(doc, node);
		printf("[\n");
		for (i = 0; i < jscp_get_object_len(node); i++){
			if (i > 0) printf(",\n");
			jscp_dump_node(doc, cn, nest + 1);
			cn = jscp_get_next(doc, cn);
			printf(":");
			jscp_dump_node(doc, cn, 0);
			cn = jscp_get_next(doc, cn);
		}
		printf("\n");
		for (i = 0; i < nest; i++) printf("  ");
		printf("]");
		break;
	}
}

void jscp_dump(jscp_doc_t *doc)
{
	jscp_dump_node(doc, jscp_get_root(doc), 0);
}

int main(void)
{
	char msg[] = "[[456,128],33,true,false, null, {\"test\":567, \"tt\":{\"test5\":567},\"test3\":1567}]";
	int i;
	jscp_doc_t doc;
	jscp_union_node_t nodes[256];
	jscp_parse(msg, strlen(msg), nodes, 256, &doc);

	jscp_dump(&doc);

	return 0;
}
