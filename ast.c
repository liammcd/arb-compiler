/*********************************************
 * Liam McDermott - 998083942
 * liam.mcdermott@mail.utoronto.ca
 * CSC467F - 2015 - Phase 4
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

list *stm_lists = NULL;
list *decl_lists = NULL;

static node *cur_scope = NULL;
static node *prev_scope = NULL;

void ast_print_args (node *ast);

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  node *tmp;
  node_list *tmp_list;
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {

  case PROGRAM_NODE:
	ast->program.scope = va_arg(args, node *);
  	break;

  case SCOPE_NODE:
	ast->scope.declarations = decl_list_pop();
	ast->scope.statements = stmt_list_pop();
	ast_add_mapped(ast);
	ast->scope.sym_tbl = sym_tbl_pop();
	break;

  case EXPRESSION_NODE:
	ast->expr.value = va_arg(args, int);
	break;

  case UNARY_EXPRESION_NODE:
	ast->unary_expr.op = va_arg(args, int);
	ast->unary_expr.right = va_arg(args, node*);
	break;

  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
    break;

  case INT_NODE:
	ast->int_lit.value = va_arg(args, int);
	break;

  case FLOAT_NODE:
	ast->float_lit.value = va_arg(args, double);
	break;

  case BOOL_NODE:
	ast->bool_lit.value = va_arg(args, int);
	break;

  case IDENT_NODE:

	break;

  case VAR_NODE:
	ast->var.name = strdup( va_arg(args, char *));
	ast->var.offset = va_arg(args, int);
	break;

  case CONSTRUCTOR_NODE:
	ast->constr.type = va_arg(args, node*);
	ast->constr.args = va_arg(args, node*);
	break;

  case IF_STATEMENT_NODE:
		ast->if_stmt.cond = va_arg(args, node*);
		ast->if_stmt.if_body = va_arg(args, node*);
		ast->if_stmt.else_body = va_arg(args, node*);
	break;


  case WHILE_STATEMENT_NODE:

	break;

  case ASSIGNMENT_NODE:
		ast->assmt.var = va_arg(args, node*);
		ast->assmt.exp = va_arg(args, node*);
	break;

  case NESTED_SCOPE_NODE:
		ast->nested_scope.scope = va_arg(args, node*);
	break;

  case DECLARATION_NODE:
		ast->decl.type = va_arg(args, node*);
		ast->decl.name = strdup( va_arg(args, char*) );
		ast->decl.expr = va_arg(args, node*);
		ast->decl.is_const = va_arg(args, int);
		tmp = ast->decl.type;
		sym_tbl_insert( tmp->type.value, ast->decl.name, ast->decl.is_const );
	break;

  case TYPE_NODE:
		ast->type.value = strdup( va_arg(args, char*) );
		ast->type.as_vec = va_arg(args, int);
	break;
	
  case FUNCTION_NODE:
		ast->func.name = (int) va_arg(args, int);
		ast->func.args = va_arg(args, node*);
	break;

  case ARGS_NODE:
	// Add expression to args list
	//tmp_list = (node_list *) malloc (sizeof(node_list));
	//tmp_list->e = va_arg(args, node*);
	//tmp_list->next = ast->args.exprs;
	//ast->args.exprs = tmp_list;
	ast->args.args = va_arg(args, node*);
	ast->args.expr = va_arg(args, node*);
	break;
	
  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {
	
}

void ast_print(node * ast) {
	ast_print_stmt(ast->program.scope);
	printf("\n");
}

void ast_print_stmt(node * ast) {

	node *tmp, *tmp2;
	node_list *tmp_list;
	ste *tmp3;
	switch (ast->kind) {

			case IF_STATEMENT_NODE:
				printf("( IF ");
				ast_print_expr(ast->if_stmt.cond);
				ast_print_stmt(ast->if_stmt.if_body);
				// Optional else
				if (ast->if_stmt.else_body) ast_print_stmt(ast->if_stmt.else_body);
				printf(") ");
				break;
			case ASSIGNMENT_NODE:
				printf("( ASSIGN ");
				//type
				tmp = ast->assmt.var;
				tmp3 = sym_tbl_lookup(cur_scope->scope.sym_tbl, tmp->var.name);
				if (tmp3)
					printf("%s ", tmp3->type);
				ast_print_expr(ast->assmt.var);
				ast_print_expr(ast->assmt.exp);
				printf(") ");
				break;
			case NESTED_SCOPE_NODE:
				ast_print_stmt(ast->nested_scope.scope);
				break;
			case DECLARATION_NODE:
				printf("( DECLARATION ");
				printf("%s ", ast->decl.name);
				ast_print_expr(ast->decl.type);
				if (ast->decl.expr) ast_print_expr(ast->decl.expr);
				printf(") ");
				break;
			case SCOPE_NODE:
				printf("( SCOPE ");
				tmp = ast;
				node_list *tmp_list;
				prev_scope = cur_scope;
				cur_scope = ast;

				tmp_list = tmp->scope.declarations;
				printf("( DECLARATIONS ");
				while (tmp_list) {
					tmp2 = tmp_list->e;
					ast_print_stmt(tmp2);
					tmp_list = tmp_list->next;
				}
				printf(") ");
				
				printf("( STATEMENTS ");
				tmp_list = tmp->scope.statements;
				while (tmp_list) {
					tmp2 = tmp_list->e;
					ast_print_stmt(tmp2);
					tmp_list = tmp_list->next;
				}
				printf(") ");
				cur_scope = prev_scope;
				printf(") ");
				break;
			default:break;
	}		
}

void ast_print_expr(node * ast) {
	
	node *tmp, *tmp2;
	node_list *tmp_list;
	ste *tmp3;
	switch (ast->kind) {

			case UNARY_EXPRESION_NODE:
				printf("( UNARY ");
				// print type
				tmp = ast->binary_expr.left;
				if (tmp->kind == VAR_NODE) {
					tmp3 = sym_tbl_lookup(cur_scope->scope.sym_tbl, tmp->var.name);
					if (tmp3)
						printf("%s ", tmp3->type);
				}
				else if (tmp->kind == INT_NODE) {
					printf("int ");
				}
				else if (tmp->kind == FLOAT_NODE) {
					printf("bool ");
				}
				printf("%c ", ast->unary_expr.op);
				ast_print_expr(ast->unary_expr.right);
				printf(") ");
				break;
			case BINARY_EXPRESSION_NODE:
				printf("( BINARY ");
				// print type -> look in symbol table
				tmp = ast->binary_expr.left;
				if (tmp->kind == VAR_NODE) {
					tmp3 = sym_tbl_lookup(cur_scope->scope.sym_tbl, tmp->var.name);
					if (tmp3)
						printf("%s ", tmp3->type);
				}
				else if (tmp->kind == INT_NODE) {
					printf("int ");
				}
				else if (tmp->kind == FLOAT_NODE) {
					printf("bool ");
				}
				printf("%c ", ast->binary_expr.op);
				ast_print_expr(ast->binary_expr.left);
				ast_print_expr(ast->binary_expr.right);
				printf(") ");
				break;
			case INT_NODE:
				printf("%d ", ast->int_lit.value);
				break;
			case FLOAT_NODE:
				printf("%f ", ast->float_lit.value);
				break;
			case IDENT_NODE:
			
				break;
			case VAR_NODE:
				printf("%s ", ast->var.name);
				break;
			case FUNCTION_NODE:
				printf("( CALL ");
				switch (ast->func.name) {
					case 0:
						printf("DP3 ");
						break;
					case 1:
						printf("LIT ");
						break;
					case 2:
						printf("RSQ ");
						break;
					default:
						break;
				}
				ast_print_args(ast->func.args);
				printf(") ");
				break;
			case CONSTRUCTOR_NODE:
			
				break;
			case EXPRESSION_NODE:
				if (ast->expr.value) printf("true ");
				else printf("false ");
				break;
			case TYPE_NODE:
				switch(ast->type.as_vec) {
					case 0:
						printf("%s ", ast->type.value);
						break;
					case 1:
						printf("%s2 ", ast->type.value);
						break;
					case 2:
						printf("%s3 ", ast->type.value);
						break;
					case 3:
						printf("%s4 ", ast->type.value);
						break;
					default:
						break;
				}
				break;
			default:break;
		}		
}

void ast_print_args (node *ast) {
	// ast is not null
	if (ast) {
		if (ast->args.args) {
			// print more args
			ast_print_args(ast->args.args);
		}
		ast_print_expr(ast->args.expr);
	}
}

/**********************************
 *
 * List functions to facilitate tables
 *
 **********************************/
node_list *ast_list_insert(node_list *list, node *stm) {
	
	node_list *tmp = list;
	node_list *tmp_last = NULL;
	while (tmp) {
		tmp_last = tmp;
		tmp = tmp->next;
	}
	tmp = (node_list *) malloc(sizeof(node_list));	
	tmp->e = stm;
	tmp->next = NULL;
	if (tmp_last) { tmp_last->next = tmp; return list;  }
	else return tmp;

}

void ast_stmt_list_add(node *ast) {
	stm_lists->e = ast_list_insert(stm_lists->e, ast);
}

void stmt_list_push() {
	list *new_list = (list *) malloc (sizeof(list));
	new_list->e = NULL;
	new_list->next = stm_lists;
	stm_lists = new_list;
}

node_list *stmt_list_pop() {
	node_list *tmp = stm_lists->e;
	list *tmp2 = stm_lists;
	stm_lists = stm_lists->next;
	free(tmp2);
	return tmp;
}

void ast_decl_list_add(node *ast) {
	decl_lists->e = ast_list_insert(decl_lists->e, ast);
}

void decl_list_push() {
	list *new_list = (list *) malloc (sizeof(list));
	new_list->e = NULL;
	new_list->next = decl_lists;
	decl_lists = new_list;
}

node_list *decl_list_pop() {
	node_list *tmp = decl_lists->e;
	list *tmp2 = decl_lists;
	decl_lists = decl_lists->next;
	free(tmp2);
	return tmp;
}

// Add mapped variables to sym tbl
void ast_add_mapped(node *ast) {

	sym_tbl_insert( "vec", "gl_FragColor", 0 );
	sym_tbl_insert( "bool", "gl_FragDepth", 0 );
	sym_tbl_insert( "vec", "gl_FragCoord", 0 );
	sym_tbl_insert( "vec", "gl_TexCoord", 0 );
	sym_tbl_insert( "vec", "gl_Color", 0 );
	sym_tbl_insert( "vec", "gl_Secondary", 0 );
	sym_tbl_insert( "vec", "gl_FogFragCoord", 0 );
	sym_tbl_insert( "vec", "gl_Light_Half", 0 );
	sym_tbl_insert( "vec", "gl_Light_Ambient", 0 );
	sym_tbl_insert( "vec", "gl_Material_Shininess", 0 );
	sym_tbl_insert( "vec", "env1", 0 );
	sym_tbl_insert( "vec", "env2", 0 );
	sym_tbl_insert( "vec", "env3", 0 );
}
