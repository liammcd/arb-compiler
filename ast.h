
#ifndef AST_H_
#define AST_H_

#include <stdarg.h>
#include "symbol.h"

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
extern node *ast;

struct node_list_;
typedef struct node_list_ node_list;

struct list_;
typedef struct list_ list;

typedef enum {
  UNKNOWN               = 0,

  SCOPE_NODE            = (1 << 0),
  
  EXPRESSION_NODE       = (1 << 2),
  UNARY_EXPRESION_NODE  = (1 << 2) | (1 << 3),
  BINARY_EXPRESSION_NODE= (1 << 2) | (1 << 4),
  INT_NODE              = (1 << 2) | (1 << 5), 
  FLOAT_NODE            = (1 << 2) | (1 << 6),
  IDENT_NODE            = (1 << 2) | (1 << 7),
  VAR_NODE              = (1 << 2) | (1 << 8),
  FUNCTION_NODE         = (1 << 2) | (1 << 9),
  CONSTRUCTOR_NODE      = (1 << 2) | (1 << 10),

  STATEMENT_NODE        = (1 << 1),
  IF_STATEMENT_NODE     = (1 << 1) | (1 << 11),
  WHILE_STATEMENT_NODE  = (1 << 1) | (1 << 12),
  ASSIGNMENT_NODE       = (1 << 1) | (1 << 13),
  NESTED_SCOPE_NODE     = (1 << 1) | (1 << 14),

  DECLARATION_NODE      = (1 << 15),
  TYPE_NODE = (1 << 16),
  ARGS_OPT_NODE = (1 << 17),
  PROGRAM_NODE = (1 << 18),
  ARGS_NODE = (1 << 19),
  BOOL_NODE = (1 << 20)
} node_kind;

struct node_ {

  // an example of tagging each node with a type
  node_kind kind;

  union {

	struct {
	  node *scope;
	} program;

    struct {
	  node_list *declarations;
	  node_list *statements;
	  ste *sym_tbl;
    } scope;

	struct {
		int value;
	} expr;

	struct {
		int value;
	} int_lit;

	struct {
		float value;
	} float_lit;
	
	struct {
		int value;
	} bool_lit;

    struct {
      int op;
      node *right;
    } unary_expr;

    struct {
      int op;
      node *left;
      node *right;
    } binary_expr;

	struct {
	  char *name;
	  int offset;
	} var;

	struct {
		node *cond;
		node *if_body;
		node *else_body;
	} if_stmt;

	struct {
		node *var;
		node *exp;
	} assmt;

	struct {
		node *type;
		node *expr;
		int is_const;
		char *name;
	} decl;

	struct {
		char *value;
		int as_vec;
	} type;
	
	struct {
		int name;
		node *args;
	} func;
	
	struct {
		node *scope;
	} nested_scope;

	struct {
		node *args;
		node *expr;
	} args;

	struct {
		node *type;
		node *args;
	} constr;

  };
};

struct node_list_ {
	node *e;
	node_list *next;
};

struct list_ {
	node_list *e;
	list *next;
};

node *ast_allocate(node_kind type, ...);
node *ast_allocate_list(node *stms, node *stm);
void ast_free(node *ast);
void ast_print(node * ast);
void ast_print_stmt(node * ast);
void ast_print_expr(node * ast);
node_list *ast_list_insert(node_list *list, node *stm);
void ast_stmt_list_add(node *ast);
void ast_decl_list_add(node *ast);
void ast_add_mapped(node *ast);

void stmt_list_push();
node_list *stmt_list_pop();
void decl_list_push();
node_list *decl_list_pop();

#endif /* AST_H_ */
