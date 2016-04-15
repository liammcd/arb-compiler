Liam McDermott
liam.mcdermott@mail.utoronto.ca	

Compiler for ARB shader language

Run make first.

This implementation successfully runs both Demo1 and Demo2. To generate the frag.txt file (default output file) run:
	./compiler467 Demos/Demo1/shader.frag
and
	./compiler467 Demos/Demo2/phong.frag
This will output frag.txt in the base directory. To run,
	./Demos/Demo1/shader frag.txt
and
	./Demos/Demo2/phong frag.txt
The frag.txt will need to be re-compiled before switching to the other demo.

Non-trivial math operations:
	- Division is implemented through multiplication, taking the reciprocal of the right side expression
	- MUL, ADD, SUB, POW were used to implement their respective operations

Boolean types are treated as either 1 (true) or -1 (false) to avoid confusion with the zero register.

If statements:
	The condition expression is generated and stored in a temporary register. It's inverse (!condition) is stored in another temporary register. When generating the conditional statements, a flag is passed into the codeGen_stmt(...) function to indicate that the statement is conditional, and the name of the register holding the condition is passed as well. When the conditional flag is raised, statements will check against the condition register to determine if an assignment should be a new expression or remain the same (as if the statement was never executed) using the CMP instruction. The statements in the optional else will be passed the inverse condition, ensuring that they will update assignments if the if branch does not and vice-versa.
	
Constants:
	Constants are defined with the PARAM keyword. When generating declarations, the is_const flag in the symbol table is checked. The type of node of the expression is checked. If it is a constructor, process the arguments and output inside braces { arg0, arg1, ... }. Literals are output directly in the 'PARAM = [lit]' line.

Declaration nodes are generated either as constant PARAMs if specified, or TEMP registers. 
Assignments will CMP if in a conditional statement or MOV the assignment expression directly to the variable's register.
Constructor nodes will MOV each argument supplied to the corresponding element of the vector (.x, .y, .z, .w).
VAR nodes will return the name of the variable along with the offset if it is being indexed (.x, .y, .z, .w).
INT, FLOAT, and BOOL nodes will MOV the literal value to a temporary register and return the name of the register (assignment will then MOV the value from the temporary register to the variable register).
UNARY and BINARY expression nodes use ADD, SUB, SLT, SGE to determine the result of the operation and CMP will return either the true register or false register depending on the preceding instructions.
Function nodes will copy the arguments to temporary registers and issue the corresponding function instruction passing in the necessary arguments from the temporary registers.

AST Allocation:
The AST is created bottom-up during the parsing phase.
A symbol table is pushed to the stack when the 'declarations' matches empty (this signifies the end of declarations). When a declaration node is inserted in the ast a symbol table entry is inserted in the symbol table at the top of the stack. A declaration list is also pushed to the declaration list stack when 'declarations' matches empy, and when 'declarations' matches 'declarations declaration' the declaration is added to the list at the top of the stack. The same occurs for the 'statements' rule to create a statement list. When a scope node is allocated, the lists at the top of the statements, declarations, and symbol table are popped and recorded in the node.

Semantic Checking:
This submission only supports variable semantic checking. Test file "tests/test2" tests multiple declarations, assignment with no declaration, and assignment of const variable. The AST is traversed recursively to check substatements and subexpressions. The symbol table is checked for duplicated entries when a scope node is encountered.

Challenges faced:

The use of structs in c was a difficult means of representing the AST nodes; use of c++ with classes and inheritance would have been preferable. The management of multiple stacks of lists (for statements, declarations, and symbol tables) complicated programming as well. Some ambiguity in the instruction set was also encountered -> i.e. which register will be copied during a CMP instruction. Handling if/else statements was also a challenge as the ARB instruction set does not include any branch instructions.
