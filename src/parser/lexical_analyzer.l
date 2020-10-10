%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_analyzer.h"
#include "SyntaxTree.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = newSyntaxTreeNode(text);
}

/*****************声明和选项设置  end*****************/

%}

%x COMMENT

%%

\+ 	{pos_end += 1; pass_node(yytext); return ADD;}
\-	{pos_end += 1; pass_node(yytext); return SUB;}
\*	{pos_end += 1; pass_node(yytext); return MUL;}
\/	{pos_end += 1; pass_node(yytext); return DIV;}
\<	{pos_end += 1; pass_node(yytext); return LT;}
\<=	{pos_end += 2; pass_node(yytext); return LTE;}
\>	{pos_end += 1; pass_node(yytext); return GT;}
\>=	{pos_end += 2; pass_node(yytext); return GTE;}
==	{pos_end += 2; pass_node(yytext); return EQ;}
!=	{pos_end += 2; pass_node(yytext); return NEQ;}
=	{pos_end += 1; pass_node(yytext); return ASSIN;}
;	{pos_end += 1; pass_node(yytext); return SEMICOLON;}
,	{pos_end += 1; pass_node(yytext); return COMMA;}
\(	{pos_end += 1; pass_node(yytext); return LPARENTHESE;}
\)	{pos_end += 1; pass_node(yytext); return RPARENTHESE;}
\[	{pos_end += 1; pass_node(yytext); return LBRACKET;}
\]	{pos_end += 1; pass_node(yytext); return RBRACKET;}
\{	{pos_end += 1; pass_node(yytext); return LBRACE;}
\}	{pos_end += 1; pass_node(yytext); return RBRACE;}
else	{pos_end += 4; pass_node(yytext); return ELSE;}
if	{pos_end += 2; pass_node(yytext); return IF;}
int	{pos_end += 3; pass_node(yytext); return INT;}
float   {pos_end += 5; pass_node(yytext); return FLOAT;}
return 	{pos_end += 6; pass_node(yytext); return RETURN;}
void 	{pos_end += 4; pass_node(yytext); return VOID;}
while 	{pos_end += 5; pass_node(yytext); return WHILE;}
[a-zA-Z]+	{pos_end += strlen(yytext); pass_node(yytext); return IDENTIFIER;}
[0-9]+	{pos_end += strlen(yytext); pass_node(yytext); return INTEGER;}
[0-9]+\.[0-9]*|[0-9]*\.[0-9]+ { pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT; }
\[\]	{pos_end += 2; pass_node(yytext); return ARRAY;}

\n 	{lines++; pos_start = 1; pos_end = 1;}
[ \t] 	{pos_start += 1; pos_end += 1;}

"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>(.|\n)  { }

%%
