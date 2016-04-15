/*********************************************
 * Liam McDermott - 998083942
 * liam.mcdermott@mail.utoronto.ca
 * CSC467F - 2015 - Phase 4
 *********************************************/

#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"
#include "common.h"
#include "symbol.h"

#define MAX_TEMP_REG	10
#define MAX_BUFFER		32
#define MAPPED			13

/* Forward declarations */
void codeGen_stmt(node *ast, int we_conditional, char *the_condition);
void codeGen_expr(node *ast, char *buffer);
void codeGen_expr_lit(node *ast, char *out);
void codeGen_args(node *ast, char *arg0, char *arg1, char *arg2, char *arg3, int depth);
void codeGen_args_lit(node *ast, char *arg0, char *arg1, char *arg2, char *arg3, int depth);

char temp_reg[MAX_TEMP_REG][MAX_IDENTIFIER];
char temp_used[MAX_TEMP_REG];
char temp_decl[MAX_TEMP_REG];

const char *reg_zero = "reg_zero";
const char *reg_true = "reg_true";
const char *reg_false = "reg_false";

static node *cur_scope = NULL;
static node *prev_scope = NULL;
 
const char *mapped[MAPPED] = {
	"gl_FragColor",
	"gl_FragDepth",
	"gl_FragCoord",
	"gl_TexCoord",
	"gl_Color",
	"gl_Secondary",
	"gl_FogFragCoord",
	"gl_Light_Half",
	"gl_Light_Ambient",
	"gl_Material_Shininess",
	"env1",
	"env2",
	"env3"
};
// Translated mapped variables
const char *mapped_trans[MAPPED] = {
	"result.color",
	"result.depth",
	"fragment.position",
	"fragment.texcoord",
	"fragment.color",
	"fragment.color.secondary",
	"fragment.fogcoord",
	"state.light[0].half",
	"state.lightmodel.ambient",
	"state.material.shininess",
	"program.env[1]",
	"program.env[2]",
	"program.env[3]"
};

/*****************
 * Setup temorary registers
 *****************/
void temp_reg_init() {
	fprintf(outputFile, "PARAM %s = 0;\n", reg_zero);
	fprintf(outputFile, "PARAM %s = 1;\n", reg_true);
	fprintf(outputFile, "PARAM %s = -1;\n", reg_false);

	int i;
	for (i = 0; i < MAX_TEMP_REG; i++) {
		sprintf(temp_reg[i], "reg%d", i);
		temp_used[i] = 0;
		temp_decl[i] = 0;
	}
}

/*****************
 * Return a free temorary register
 *****************/
char *temp_reg_get() {
	int i;
	char *the_reg;
	for (i = 0; i < MAX_TEMP_REG; i++) {
		if (!temp_used[i]) {
			temp_used[i] = 1; // Used
			the_reg = temp_reg[i];
			break;
		} 
	}
	if (i == MAX_TEMP_REG)
		return NULL;
	else {
		if (!temp_decl[i]) { // Temporary reg hasn't been used yet
			temp_decl[i] = 1;
			fprintf(outputFile, "TEMP %s;\n", the_reg);
		}
		return the_reg;
	}
}

/*****************
 * Free a temorary register
 *****************/
void temp_reg_free(char *reg) {
	int i;
	for (i = 0; i < MAX_TEMP_REG; i++) {
		if (!strcmp(temp_reg[i], reg)) {
			temp_used[i] = 0;
			break;
		} 
	}
}

/************************
 * Top-level codeGen
 * Generates program scope statement and ARB assembly header and footer
 ************************/
void codeGen(node *ast) {
	
	fprintf(outputFile, "!!ARBfp1.0\n");
	// Init temporary registers
	temp_reg_init();	
	codeGen_stmt(ast->program.scope, 0, NULL);
	fprintf(outputFile, "END\n");	
}

/************************************
 * Generate code for statements
 *
 * node *ast -> the statement node
 * int we_condition -> 0 or 1, set if the statement belongs to a conditional statement (if/else)
 * char *the_condition -> register where the condition is held
 ************************************/
void codeGen_stmt(node *ast, int we_conditional, char *the_condition) {
	node_list *tmp_list;
	ste *sym_tbl;
	char buffer[MAX_BUFFER];
	char buffer2[MAX_BUFFER];
	char *out_reg;
	char *cond_reg;
	char *cond_reg2;
	if (!ast) return;
	switch (ast->kind) {

			case IF_STATEMENT_NODE:
				cond_reg = temp_reg_get();
				cond_reg2 = temp_reg_get();
				codeGen_expr_lit(ast->if_stmt.cond, buffer);	// buffer will be 1 or -1 (true or false)
				fprintf(outputFile, "SGE %s, %s, %s;\n", cond_reg, buffer, reg_zero);
				temp_reg_free(buffer);
				//fprintf(outputFile, "CMP %s, %s, %s, %s;\n", cond_reg2, cond_reg, reg_true, reg_zero);
				fprintf(outputFile, "CMP %s, %s, %s, %s;\n", cond_reg2, cond_reg, reg_zero, reg_true);
				codeGen_stmt(ast->if_stmt.if_body, 1, cond_reg);
				
				if (ast->if_stmt.else_body)
					codeGen_stmt(ast->if_stmt.else_body, 1, cond_reg2);
				temp_reg_free(cond_reg);
				temp_reg_free(cond_reg2);
				break;

			case ASSIGNMENT_NODE:
				codeGen_expr(ast->assmt.var, buffer);	// Variable node
				codeGen_expr(ast->assmt.exp, buffer2);	// Expression node
				if (we_conditional)	// If the_condition == 0 (false) buffer = buffer else buffer = buffer2
					//fprintf(outputFile, "CMP %s, %s, %s, %s;\n", buffer, the_condition, buffer, buffer2);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", buffer, the_condition, buffer2, buffer);
				else
					fprintf(outputFile, "MOV %s, %s;\n", buffer, buffer2);
				temp_reg_free(buffer2);
				break;

			case NESTED_SCOPE_NODE:
				codeGen_stmt(ast->nested_scope.scope, we_conditional, the_condition);
				break;

			case DECLARATION_NODE:
				sym_tbl = sym_tbl_lookup(cur_scope->scope.sym_tbl, ast->decl.name);
				if (!sym_tbl) 
					sym_tbl = sym_tbl_lookup(prev_scope->scope.sym_tbl, ast->decl.name);
				if (sym_tbl->is_const) {
					// Const
					if (ast->decl.expr->kind == VAR_NODE) {
						codeGen_expr(ast->decl.expr, buffer);
						fprintf(outputFile, "PARAM %s = %s;\n", ast->decl.name, buffer);
					}
					else if (ast->decl.expr->kind == CONSTRUCTOR_NODE) {
						char buffer3[MAX_BUFFER];
						char buffer4[MAX_BUFFER];
						char result[MAX_BUFFER];
						codeGen_args_lit (ast->decl.expr->constr.args, buffer, buffer2, buffer3, buffer4, 0);

						if (ast->decl.expr->constr.type->type.as_vec == 1) {
							sprintf(result, "{%s,%s}", buffer, buffer2);
						}						
						if (ast->decl.expr->constr.type->type.as_vec == 2) {
							sprintf(result, "{%s,%s,%s}", buffer, buffer2, buffer3);
						}						
						if (ast->decl.expr->constr.type->type.as_vec == 3) {
							sprintf(result, "{%s,%s,%s,%s}", buffer, buffer2, buffer3, buffer4);
						}
						else {		
							sprintf(result, "{%s}", buffer);
						}
						fprintf(outputFile, "PARAM %s = %s;\n", ast->decl.name, result);
					}
					else {
						// Literal
						if (ast->decl.expr->kind == FLOAT_NODE) {
							sprintf(buffer, "%f", ast->decl.expr->float_lit.value);
						}
						else if (ast->decl.expr->kind == BOOL_NODE) {
							sprintf(buffer, "%d", ast->decl.expr->bool_lit.value);
						}
						else {
							sprintf(buffer, "%d", ast->decl.expr->int_lit.value);
						}
						fprintf(outputFile, "PARAM %s = %s;\n", ast->decl.name, buffer);
					}
				}
				else {
					// Not const
					fprintf(outputFile, "TEMP %s;\n", ast->decl.name);
					if (ast->decl.expr) {
						codeGen_expr(ast->decl.expr, buffer);
						fprintf(outputFile, "MOV %s, %s;\n", ast->decl.name, buffer);
						temp_reg_free(buffer);
					}
				}				
				break;

			case SCOPE_NODE:
				prev_scope = cur_scope;
				cur_scope = ast;

				tmp_list = ast->scope.declarations;
				while (tmp_list) {
					codeGen_stmt(tmp_list->e, we_conditional, the_condition);
					tmp_list = tmp_list->next;
				}
								
				tmp_list = ast->scope.statements;
				while (tmp_list) {
					codeGen_stmt(tmp_list->e, we_conditional, the_condition);
					tmp_list = tmp_list->next;
				}
				cur_scope = prev_scope;
				break;

			default:break;
	}	
}

/************************************
 * Generate code for expressions 
 *
 * node *ast -> the expression node
 * char *buffer -> write output here
 ************************************/
void codeGen_expr(node *ast, char *buffer) {

	char buffer1[MAX_BUFFER];
	char buffer2[MAX_BUFFER];
	char buffer3[MAX_BUFFER];
	char buffer4[MAX_BUFFER];
	char *out_reg;

	switch (ast->kind) {
	
		case FUNCTION_NODE:

			codeGen_args(ast->func.args, buffer1, buffer2, NULL, NULL, 0); // Process arguments (at most 2)
			out_reg = temp_reg_get();
	
			if (ast->func.name == 0) {
				//dp3
				fprintf(outputFile, "DP3 %s, %s, %s;\n", out_reg, buffer1, buffer2);
			}	
			else if (ast->func.name == 1) {
				//lit
				fprintf(outputFile, "LIT %s, %s;\n", out_reg, buffer1);
			}
			else {
				//rsq
				fprintf(outputFile, "RSQ %s, %s;\n", out_reg, buffer1);
			}
			strcpy(buffer, out_reg);
			break;

		case CONSTRUCTOR_NODE:
			out_reg = temp_reg_get();
			codeGen_args (ast->constr.args, buffer1, buffer2, buffer3, buffer4, 0);
			
			fprintf(outputFile, "MOV %s.x, %s;\n", out_reg, buffer1);
			temp_reg_free(buffer1); 
			if (ast->constr.type->type.as_vec >= 1)
				fprintf(outputFile, "MOV %s.y, %s;\n", out_reg, buffer2);
				temp_reg_free(buffer2);
			if (ast->constr.type->type.as_vec >= 2)
				fprintf(outputFile, "MOV %s.z, %s;\n", out_reg, buffer3);
				temp_reg_free(buffer3);
			if (ast->constr.type->type.as_vec == 3)
				fprintf(outputFile, "MOV %s.w, %s;\n", out_reg, buffer4);
				temp_reg_free(buffer4);
			strcpy(buffer, out_reg);
			break;

		case VAR_NODE:
			int i;
			char mapped_transl[MAX_BUFFER];
			for (i = 0; i < MAPPED; i++) {
				if (!strcmp(ast->var.name, mapped[i]))
					break;
			}
			if (i != MAPPED)
				strcpy(mapped_transl, mapped_trans[i]);
			else
				strcpy(mapped_transl, ast->var.name);
			sprintf(buffer, "%s", mapped_transl);
			if (ast->var.offset == 0)
				strcat(buffer, ".x");
			else if (ast->var.offset == 1)
				strcat(buffer, ".y");
			else if (ast->var.offset == 2)
				strcat(buffer, ".z");
			else if (ast->var.offset == 3)
				strcat(buffer, ".w");
			break;

		case INT_NODE:
			out_reg = temp_reg_get();
			fprintf(outputFile, "MOV %s, %d;\n", out_reg, ast->int_lit.value);
			strcpy(buffer, out_reg);		
			break;

		case FLOAT_NODE:
			out_reg = temp_reg_get();
			fprintf(outputFile, "MOV %s, %f;\n", out_reg, ast->float_lit.value);
			strcpy(buffer, out_reg);
			break;

		case BOOL_NODE:
			out_reg = temp_reg_get();
			fprintf(outputFile, "MOV %s, %d;\n", out_reg, ast->bool_lit.value);
			strcpy(buffer, out_reg);
			break;

		case UNARY_EXPRESION_NODE:
			out_reg = temp_reg_get();
			codeGen_expr(ast->unary_expr.right, buffer1);
			switch(ast->unary_expr.op) {
				case '!':
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, buffer1, reg_false, reg_true);
					break;

				case '-':
					fprintf(outputFile, "SUB %s, %s, %s;\n", out_reg, reg_zero, buffer1);
					break;

			}		
			strcpy(buffer, out_reg);
			temp_reg_free(out_reg);
			break;

		case BINARY_EXPRESSION_NODE:
			out_reg = temp_reg_get();
			codeGen_expr(ast->binary_expr.left, buffer1);
			codeGen_expr(ast->binary_expr.right, buffer2);
			switch(ast->binary_expr.op) {
				case AND_:
					// both true -> out_reg = 2
					fprintf(outputFile, "ADD %s, %s, %s;\n", out_reg, buffer1, buffer2);
					// out_reg = 1 if both true
					fprintf(outputFile, "SGE %s, %s, %s;\n", out_reg, out_reg, reg_true);
					// if not set, false else true
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_false, reg_true); 
					break;
				case OR_:
					fprintf(outputFile, "ADD %s, %s, %s;\n", out_reg, buffer1, buffer2);
					fprintf(outputFile, "SGE %s, %s, %s;\n", out_reg, out_reg, reg_zero);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);
					break;
			
				case EQ_:
					fprintf(outputFile, "SUB %s, %s, %s;\n", out_reg, buffer1, buffer2);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_false, reg_true);					
					break;

				case NEQ_:
					fprintf(outputFile, "SUB %s, %s, %s;\n", out_reg, buffer1, buffer2);	
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);					
					break;

				case '<':
					fprintf(outputFile, "SLT %s, %s, %s;\n", out_reg, buffer1, buffer2);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);
					break;

				case LEQ_:
					fprintf(outputFile, "SGE %s, %s, %s;\n", out_reg, buffer2, buffer1);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);
					break;

				case '>':
					fprintf(outputFile, "SLT %s, %s, %s;\n", out_reg, buffer2, buffer1);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);
					break;

				case GEQ_:
					fprintf(outputFile, "SGE %s, %s, %s;\n", out_reg, buffer1, buffer2);
					fprintf(outputFile, "CMP %s, %s, %s, %s;\n", out_reg, out_reg, reg_true, reg_false);
					break;

				case '+':
					fprintf(outputFile, "ADD %s, %s, %s;\n", out_reg, buffer1, buffer2);
					break ;

				case '-':
					fprintf(outputFile, "SUB %s, %s, %s;\n", out_reg, buffer1, buffer2);
					break;

				case '*':
					fprintf(outputFile, "MUL %s, %s, %s;\n", out_reg, buffer1, buffer2);
					break;

				case '/':
					fprintf(outputFile, "RCP %s, %s;\n", out_reg, buffer2);
					fprintf(outputFile, "MUL %s, %s, %s;\n", out_reg, buffer1, out_reg);
					break;

				case '^':
					fprintf(outputFile, "POW %s, %s, %s\n", out_reg, buffer1, buffer2);
					break;
				default: break;
			}
			sprintf(buffer, out_reg);
			temp_reg_free(out_reg);
			break;

		default:break;
	}
}

/**************************
 * Recursively generate arguments -> maxiumum 4
 **************************/
void codeGen_args(node *ast, char *arg0, char *arg1, char *arg2, char *arg3, int depth) {

	int next_depth = depth+1;
	if (ast) {
		if (ast->args.args) {
			// print more args
			codeGen_args(ast->args.args, arg0, arg1, arg2, arg3, next_depth);
		}
		if (depth == 0)
			codeGen_expr(ast->args.expr, arg0);
		else if (depth == 1)
			codeGen_expr(ast->args.expr, arg1);
		else if (depth == 2)
			codeGen_expr(ast->args.expr, arg2);
		else if (depth == 3)
			codeGen_expr(ast->args.expr, arg3);
	}
}

void codeGen_expr_lit(node *ast, char *out) {

	switch (ast->kind) {

		case INT_NODE:
			sprintf(out, "%d", ast->int_lit.value);		
			break;

		case FLOAT_NODE:
			sprintf(out, "%f", ast->float_lit.value);
			break;

		case BOOL_NODE:
			sprintf(out, "%d", ast->bool_lit.value);
			break;
		default:codeGen_expr(ast, out);break;

	}
}

void codeGen_args_lit(node *ast, char *arg0, char *arg1, char *arg2, char *arg3, int depth) {

	int next_depth = depth+1;
	if (ast) {
		if (ast->args.args) {
			// print more args
			codeGen_args_lit(ast->args.args, arg0, arg1, arg2, arg3, next_depth);
		}
		if (depth == 0)
			codeGen_expr_lit(ast->args.expr, arg0);
		else if (depth == 1)
			codeGen_expr_lit(ast->args.expr, arg1);
		else if (depth == 2)
			codeGen_expr_lit(ast->args.expr, arg2);
		else if (depth == 3)
			codeGen_expr_lit(ast->args.expr, arg3);
	}
}
