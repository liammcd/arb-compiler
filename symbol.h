#ifndef _SYMBOL_H
#define _SYMBOL_H

struct ste_;
typedef struct ste_ ste;

struct sym_tbl_list_;
typedef struct sym_tbl_list_ sym_tbl_list;

typedef enum {
	INT = (1 << 0),
	FLOAT = (1 << 1)
} var_type;

/* Symbol table entry*/
struct ste_ {
	//var_type type;
	char *type;
	char *name;
	int as_vec;
	int is_const;
	ste *next;
};

struct sym_tbl_list_ {
	ste *e;
	sym_tbl_list *next;
};

void sym_tbl_insert(char *type, char *name, int is_const);
ste *sym_tbl_lookup(ste *sym_tbl_scope, char *name);
void sym_tbl_push();
ste *sym_tbl_pop();
void *sym_tbl_free(ste *sym_tbl_scope);

#endif

