%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}

/*****************声明和选项设置  end*****************/

%}

%x COMMENT

%%

\+ 	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
\-	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return SUB;}
\*	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return MUL;}
\/	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return DIV;}
\<	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LT;}
\<=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return LTE;}
\>	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return GT;}
\>=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return GTE;}
==	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return EQ;}
!=	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return NEQ;}
=	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return ASSIN;}
;	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return SEMICOLON;}
,	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return COMMA;}
\(	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LPARENTHESE;}
\)	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RPARENTHESE;}
\[	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACKET;}
\]	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACKET;}
\{	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACE;}
\}	{pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACE;}
else	{pos_start = pos_end; pos_end += 4; pass_node(yytext); return ELSE;}
if	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return IF;}
int	{pos_start = pos_end; pos_end += 3; pass_node(yytext); return INT;}
float   {pos_start = pos_end; pos_end += 5; pass_node(yytext); return FLOAT;}
return 	{pos_start = pos_end; pos_end += 6; pass_node(yytext); return RETURN;}
void 	{pos_start = pos_end; pos_end += 4; pass_node(yytext); return VOID;}
while 	{pos_start = pos_end; pos_end += 5; pass_node(yytext); return WHILE;}
[a-zA-Z]+	{pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return IDENTIFIER;}
[0-9]+	{pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER;}
[0-9]+\.[0-9]*|[0-9]*\.[0-9]+ { pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT; }
\[\]	{pos_start = pos_end; pos_end += 2; pass_node(yytext); return ARRAY;}

\n 	{lines++; pos_start = 1; pos_end = 1;}
[ \t] 	{pos_start = pos_end; pos_end += 1;}

"/*"             { pos_start = pos_end; pos_end += 2; BEGIN(COMMENT); }
<COMMENT>"*/"    { pos_start = pos_end; pos_end += 2; BEGIN(INITIAL); }
<COMMENT>.  { pos_start = pos_end; pos_start += 1; }
<COMMENT>\n { pos_start = 1; pos_end = 1; lines++; }

. { pos_start = pos_end; pos_end++; return ERROR; }

/*运算*/
ADD     \+
SUB     \-
MUL     \*
DIV     \/
LT      <
LTE     <=
GT      >
GTE     >=
EQ      ==
NEQ     !=
ASSIN   =
/*符号*/
SEMICOLON    ;
COMMA        ,
LPARENTHESE  \(
RPARENTHESE  \)
LBRACKET     \[
RBRACKET     \]
LBRACE       \{
RBRACE       \}
/*关键字*/
ELSE   else
IF     if
INT    int
FLOAT  float
RETURN return
VOID   void
WHILE  while
/*ID和NUM*/
IDENTIFIER [a-zA-Z][a-zA-Z]*
digit      [0-9]
INTEGER   {digit}+
FLOATPOINT  ({digit}+\.|{digit}*\.{digit}+)
ARRAY      \[]
/*other*/
SPACE [ ]
EOL [\n|\r]
TAB [\t]
COMMENT \/\*([^\*]|\r|\n)*(\*\**([^\*\/]|\r|\n)([^\*]|\r|\n)*)*\**\*\/

%%
{SUB} {
    pos_start = pos_end;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (SUB);
}
{ADD} {
    pos_start = pos_end;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (ADD);
}
{MUL} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (MUL);
}
{DIV} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (DIV);
}
{LT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (LT);
}
{LTE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (LTE);
}
{GT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (GT);
}
{GTE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (GTE);
}
{EQ} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (EQ);
}
{NEQ} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (NEQ);
}
{ASSIN} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (ASSIN);
}
{SEMICOLON} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (SEMICOLON);
}
{COMMA} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (COMMA);
}
{LPARENTHESE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (LPARENTHESE);
}
{RPARENTHESE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (RPARENTHESE);
}
{LBRACKET} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (LBRACKET);
}
{RBRACKET} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (RBRACKET);
}
{LBRACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (LBRACE);
}
{RBRACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    pass_node(yytext);
    return (RBRACE);
}
{ELSE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 4;
    pass_node(yytext);
    return (ELSE);
}
{IF} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (IF);
}
{INT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 3;
    pass_node(yytext);
    return (INT);
}
{FLOAT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 5;
    pass_node(yytext);
    return (FLOAT);
}
{RETURN} {
    pos_start = pos_end ;
    pos_end   = pos_start + 6;
    pass_node(yytext);
    return (RETURN);
}
{VOID} {
    pos_start = pos_end ;
    pos_end   = pos_start + 4;
    pass_node(yytext);
    return (VOID);
}
{WHILE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 5;
    pass_node(yytext);
    return (WHILE);
}
{IDENTIFIER} {
    pos_start = pos_end;
    pos_end   = pos_start + strlen(yytext);
    pass_node(yytext);
    return (IDENTIFIER);
}
{SPACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    }
{TAB} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    }
{EOL} {
    lines += 1;
    pos_start = 1;
    pos_end   = 1;
    pass_node(yytext);
}
{INTEGER} {
    pos_start = pos_end;
    pos_end   = pos_start + yyleng;
    pass_node(yytext);
    return (INTEGER);
}
{FLOATPOINT} {
    pos_start = pos_end;
    pos_end   = pos_start + yyleng;
    pass_node(yytext);
    return (FLOATPOINT);
}
{ARRAY} {
    pos_start = pos_end;
    pos_end   = pos_start + 2;
    pass_node(yytext);
    return (ARRAY);
}
{COMMENT} {
    comment(yytext,&lines,&pos_start,&pos_end);
    pass_node(yytext);
}

%%

void comment(char *yytext, int *lines, int *pos_start, int *pos_end){
    for(int i = 0; i < yyleng; i++){
        if(yytext[i] == '\n'){
            *lines = (*lines) +1;
            *pos_start = 1;
            *pos_end   = 1;
        }
        else{
            (*pos_start)++;
            (*pos_end)++;
        }
    }
}
