%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "SyntaxTree.h"

#include "lexical_analyzer.h"

// external functions from lex
extern int yylex();
extern int yyparse();
extern int yyrestart();
extern FILE * yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char * yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree.
SyntaxTree * gt;
	
SyntaxTreeNode * addparent(SyntaxTreeNode * parent, char * parent_name, int num, ...){
	va_list ap;
	va_start(ap, num);
	parent = newSyntaxTreeNode(parent_name);
	SyntaxTreeNode	* child;
	for(int i = 0; i < num; i++){
		child = va_arg(ap, SyntaxTreeNode*);
		SyntaxTreeNode_AddChild(parent, child);
	}
	va_end(ap);
	return parent;
}

SyntaxTreeNode * epsilon(SyntaxTreeNode * parent, char * parent_name){
	parent = newSyntaxTreeNode(parent_name);
	SyntaxTreeNode * child = newSyntaxTreeNode("epsilon");
	SyntaxTreeNode_AddChild(parent, child);
	return parent;
}

void yyerror(const char * s);
%}

%union {
     struct _SyntaxTreeNode * node;
     char * name;
}

%token <node> ERROR 
%token <node> ADD 
%token <node> SUB 
%token <node> MUL 
%token <node> DIV 
%token <node> LT 
%token <node> LTE 
%token <node> GT 
%token <node> GTE 
%token <node> EQ 
%token <node> NEQ 
%token <node> ASSIN 
%token <node> SEMICOLON 
%token <node> COMMA 
%token <node> LPARENTHESE 
%token <node> RPARENTHESE 
%token <node> LBRACKET 
%token <node> RBRACKET 
%token <node> LBRACE 
%token <node> RBRACE 
%token <node> ELSE 
%token <node> IF 
%token <node> INT 
%token <node> RETURN 
%token <node> VOID 
%token <node> WHILE 
%token <node> IDENTIFIER 
%token <node> INTEGER
%token <node> FLOAT
%token <node> FLOATPOINT
%token <node> ARRAY 
//%token <node> EOL
//%token <node> BLANK
//%token <node> COMMENT 
%type <node> program declaration-list declaration var-declaration type-specifier fun-declaration params param-list param compound-stmt local-declarations statement-list statement expression-stmt selection-stmt iteration-stmt return-stmt expression var simple-expression relop additive-expression addop term mulop factor call args arg-list



/* compulsory starting symbol */
%start program

%%
/*************** TODO: Your rules here *****************/
program : 	declaration-list {$$ = addparent($$, "program", 1, $1); gt->root = $$;}
		;

declaration-list 	: 	declaration-list declaration {$$ = addparent($$, "declaration-list", 2, $1, $2);}
					|	declaration {$$ = addparent($$, "declaration-list", 1, $1);}
					;

declaration : 	var-declaration {$$ = addparent($$, "declaration", 1, $1);}
			| 	fun-declaration {$$ = addparent($$, "declaration", 1, $1);}
			;

var-declaration : 	type-specifier IDENTIFIER SEMICOLON {$$ = addparent($$, "var-declaration", 3, $1, $2, $3);}
                | 	type-specifier IDENTIFIER LBRACKET INTEGER RBRACKET SEMICOLON {$$ = addparent($$, "var-declaration", 6, $1, $2, $3, $4, $5, $6);}
                ;

type-specifier 	: 	INT {$$ = addparent($$, "type-specifier", 1, $1);}
| FLOAT { $$ = addparent($$, "type-specifier", 1, $1); }
				| 	VOID {$$ = addparent($$, "type-specifier", 1, $1);}
				;

fun-declaration : 	type-specifier IDENTIFIER LPARENTHESE params RPARENTHESE compound-stmt {$$ = addparent($$, "fun-declaration", 6, $1, $2, $3, $4, $5, $6);}
				;

params 	: 	param-list {$$ = addparent($$, "params", 1, $1);}
		|	VOID {$$ = addparent($$, "params", 1, $1);}
		;
		
param-list 	: 	param-list COMMA param {$$ = addparent($$, "param-list", 3, $1, $2, $3);}
			| 	param {$$ = addparent($$, "param-list", 1, $1);}
			;

param 	: 	type-specifier IDENTIFIER {$$ = addparent($$, "param", 2, $1, $2);}
		| 	type-specifier IDENTIFIER ARRAY {$$ = addparent($$, "param", 3, $1, $2, $3);}
		;

compound-stmt 	: 	LBRACE local-declarations statement-list RBRACE {$$ = addparent($$, "compound-stmt", 4, $1, $2, $3, $4);}
				;

local-declarations 	: 	local-declarations var-declaration {$$ = addparent($$, "local-declarations", 2, $1, $2);}
					| 	{$$ = epsilon($$, "local-declarations");} 
					;
			
statement-list 	: 	statement-list statement {$$ = addparent($$, "statement-list", 2, $1, $2);}
				| 	{$$ = epsilon($$, "statement-list");}
				;
				
statement 	: 	expression-stmt {$$ = addparent($$, "statement", 1, $1);}
            | 	compound-stmt {$$ = addparent($$, "statement", 1, $1);}
			| 	selection-stmt {$$ = addparent($$, "statement", 1, $1);}
			| 	iteration-stmt {$$ = addparent($$, "statement", 1, $1);}
			| 	return-stmt {$$ = addparent($$, "statement", 1, $1);}
			;
			
expression-stmt : 	expression SEMICOLON {$$ = addparent($$, "expression-stmt", 2, $1, $2);}
				| 	SEMICOLON {$$ = addparent($$, "expression-stmt", 1, $1);}
				;
				
selection-stmt 	: 	IF LPARENTHESE expression RPARENTHESE statement {$$ = addparent($$, "selection-stmt", 5, $1, $2, $3, $4, $5);}
				| 	IF LPARENTHESE expression RPARENTHESE statement ELSE statement {$$ = addparent($$, "selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
				;

iteration-stmt 	: 	WHILE LPARENTHESE expression RPARENTHESE statement {$$ = addparent($$, "iteration-stmt", 5, $1, $2, $3, $4, $5);}
				;

return-stmt : 	RETURN SEMICOLON {$$ = addparent($$, "return-stmt", 1, $1);}
			| 	RETURN expression SEMICOLON {$$ = addparent($$, "return-stmt", 3, $1, $2, $3);}
			;

expression 	: 	var ASSIN expression {$$ = addparent($$, "expression", 3, $1, $2, $3);}
			| 	simple-expression {$$ = addparent($$, "expression", 1, $1);}
			;
	
var : 	IDENTIFIER {$$ = addparent($$, "var", 1, $1);}
    | 	IDENTIFIER LBRACKET expression RBRACKET {$$ = addparent($$, "var", 4, $1, $2, $3, $4);}
    ;

simple-expression 	: 	additive-expression relop additive-expression {$$ = addparent($$, "simple-expression", 3, $1, $2, $3);}
					| 	additive-expression {$$ = addparent($$, "simple-expression", 1, $1);}
					;

relop 	: 	LT {$$ = addparent($$, "relop", 1, $1);}
		| 	LTE {$$ = addparent($$, "relop", 1, $1);}
		| 	GT {$$ = addparent($$, "relop", 1, $1);}
		| 	GTE {$$ = addparent($$, "relop", 1, $1);}
		| 	EQ {$$ = addparent($$, "relop", 1, $1);}
		| 	NEQ {$$ = addparent($$, "relop", 1, $1);}
		;

additive-expression : 	additive-expression addop term  {$$ = addparent($$, "additive-expression", 3, $1, $2, $3);}
					| 	term {$$ = addparent($$, "additive-expression", 1, $1);}
					;

addop 	: 	ADD {$$ = addparent($$, "addop", 1, $1);}
		|	SUB {$$ = addparent($$, "addop", 1, $1);}
		;

term 	: 	term mulop factor {$$ = addparent($$, "term", 3, $1, $2, $3);}
		| 	factor {$$ = addparent($$, "term", 1, $1);}
		;

mulop 	: 	MUL {$$ = addparent($$, "mulop", 1, $1);}
		|	DIV {$$ = addparent($$, "mulop", 1, $1);}
		;

factor 	: 	LPARENTHESE expression RPARENTHESE {$$ = addparent($$, "factor", 3, $1, $2, $3);}
		|	var {$$ = addparent($$, "factor", 1, $1);}
		|	call {$$ = addparent($$, "factor", 1, $1);}
		| 	INTEGER {$$ = addparent($$, "factor", 1, $1);}
                | FLOATPOINT {$$ = addparent($$, "factor", 1, $1);}
		;

call 	: 	IDENTIFIER LPARENTHESE args RPARENTHESE {$$ = addparent($$, "call", 4, $1, $2, $3, $4);}
		;

args 	: 	arg-list {$$ = addparent($$, "args", 1, $1);}
		| 	{$$ = epsilon($$, "args");}
		;

arg-list 	: 	arg-list COMMA expression {$$ = addparent($$, "arg-list", 3, $1, $2, $3);}
			| 	expression {$$ = addparent($$, "arg-list", 1, $1);}
			;

%%

void yyerror(const char * s)
{
	// TODO: variables in Lab1 updates only in analyze() function in lexical_analyzer.l
	//       You need to move position updates to show error output below
	fprintf(stderr, "%s: %d %d %d syntax error for %s %s\n", s, lines, pos_start, pos_end, yytext);
}

void parse()
{
     lines = pos_start = pos_end = 1;
     gt = newSyntaxTree();
     yyin = stdin;
     yyrestart(yyin);

     yyparse();
     printSyntaxTree(stdout, gt);

     deleteSyntaxTree(gt);
     gt = NULL;
}
