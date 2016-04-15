#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"

sym_tbl_list *sym_tbl = NULL;

void sym_tbl_insert(char *type, char *name, int is_const){
	ste *entry = (ste *) malloc (sizeof(ste));
	//entry->kind = kind;
	entry->type = strdup(type);
	entry->name = strdup(name);
	entry->is_const = is_const;
	entry->next = sym_tbl->e;
	sym_tbl->e = entry;
}

ste *sym_tbl_lookup(ste *sym_tbl_scope, char *name){
	ste *tmp = sym_tbl_scope;

	while (tmp){
		if (!strcmp(name, tmp->name)) {
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

/*******************
 * sym_tbl_pop()
 * Return the current symbol table TODO 
 * Set
 *******************/
ste *sym_tbl_pop(){
	ste *tmp = sym_tbl->e;
	sym_tbl_list *tmp2 = sym_tbl;
	sym_tbl = sym_tbl->next;
	free(tmp2);
	return tmp;
}

void sym_tbl_push() {
	sym_tbl_list *new_list = (sym_tbl_list *) malloc (sizeof(sym_tbl_list));
	new_list->e = NULL;
	new_list->next = sym_tbl;
	sym_tbl = new_list;
}
