/*********************************************
 * Liam McDermott - 998083942
 * liam.mcdermott@mail.utoronto.ca
 * CSC467F - 2015 - Phase 4
 *********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "common.h"
#include "semantic.h"

static node *cur_scope = NULL;
static node *prev_scope = NULL;

int semantic_check( node *ast) {
  node *scope = ast->program.scope;

  return stmt_check(scope);  
  
}

int expr_check ( node *ast) {

	node *tmp;
	int error = 1;	
	ste *tmp_sym;
		
	switch (ast->kind) {	
	
		case UNARY_EXPRESION_NODE:
			switch (ast->unary_expr.op) {
				
				case '!':
					tmp = ast->unary_expr.right;
					if (!(error = check_bool(tmp))) {

					}
					break;

				case '-':
		
					break;
				default:break;
			}
			
			break;
		case BINARY_EXPRESSION_NODE:
				
			break;
		case INT_NODE:

			break;
		case FLOAT_NODE:

			break;

		case IDENT_NODE:
		
			break;
		case VAR_NODE:
			// See if variable has been declared
			tmp_sym = sym_tbl_lookup(cur_scope->scope.sym_tbl, ast->var.name);
			if (!tmp_sym) {
				// Not in sym table
				if (prev_scope) {
					tmp_sym = sym_tbl_lookup(prev_scope->scope.sym_tbl, ast->var.name);
				}
				if (!tmp_sym) {
					error = 0;
					fprintf(errorFile, "Semantic error: variable %s not declared\n", ast->var.name);
				}
			}
			break;
		case FUNCTION_NODE:

			break;
		case CONSTRUCTOR_NODE:
		
			break;
		case EXPRESSION_NODE:

			break;
		case TYPE_NODE:

			break;
		default:break;
	}	
	return error;	
}

int stmt_check ( node *ast) {
	node *tmp, *tmp2;
	node_list *tmp_list;
	ste *tmp3, *tmp4;
	int error = 1;
	switch (ast->kind) {

			case IF_STATEMENT_NODE:
				error = expr_check (ast->if_stmt.cond);
				error = stmt_check (ast->if_stmt.if_body);
				// Optional else
				if (ast->if_stmt.else_body) stmt_check(ast->if_stmt.else_body);
				break;
			case ASSIGNMENT_NODE:
				// Look in symbol table -> is_const?
				tmp = ast->assmt.var;
				tmp3 = sym_tbl_lookup(cur_scope->scope.sym_tbl, tmp->var.name);
				if (tmp3 && tmp3->is_const) {
					// Is a constant
					fprintf(errorFile, "Semantic error: attempting to assign variable %s defined as const\n", tmp->var.name);
				}
				// Check var node
				error = expr_check (ast->assmt.var);
				// Check expression
				error = expr_check (ast->assmt.exp);
				break;
			case NESTED_SCOPE_NODE:
				error = stmt_check(ast->nested_scope.scope);
				break;
			case DECLARATION_NODE:
				error = expr_check (ast->decl.type);				
				break;
			case SCOPE_NODE:
				tmp = ast;
				prev_scope = cur_scope;
				cur_scope = ast;

				// Check for multiple entries in sym_tbl
				tmp3 = tmp->scope.sym_tbl;
				while ( tmp3 ) {
					tmp4 = sym_tbl_lookup(tmp->scope.sym_tbl, tmp3->name);
					if (tmp4 && (tmp4 != tmp3)) {
						fprintf(errorFile, "Semantic error: variable %s declared more than once\n", tmp3->name);
						error = 0;
						break;
					}
					tmp3 = tmp3->next;
				}

				// Check declarations
				tmp_list = tmp->scope.declarations;
				while (tmp_list) {
					tmp2 = tmp_list->e;
					error = stmt_check(tmp2);
					tmp_list = tmp_list->next;
				}

				// Check statements
				tmp_list = tmp->scope.statements;
				while (tmp_list) {
					tmp2 = tmp_list->e;
					error = stmt_check(tmp2);
					tmp_list = tmp_list->next;
				}
				cur_scope = prev_scope;

				break;
			default:break;
	}		
	return error;
}

// Determines if an expr is boolean
int check_bool (node *ast) {
	return 1;
}
