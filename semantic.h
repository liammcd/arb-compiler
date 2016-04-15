#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"


int semantic_check( node *ast);
int expr_check( node *ast);
int stmt_check( node *ast);
int check_bool( node *ast);

#endif
