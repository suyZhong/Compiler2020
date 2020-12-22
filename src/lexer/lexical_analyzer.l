%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/

%}

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
IDENTIFIER [a-zA-Z][a-zA-Z_0-9]*
INTEGER   {digit}+
digit      [0-9]
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
    return (SUB);
}
{ADD} {
    pos_start = pos_end;
    pos_end   = pos_start + 1;
    return (ADD);
}
{MUL} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (MUL);
}
{DIV} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (DIV);
}
{LT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (LT);
}
{LTE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    return (LTE);
}
{GT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (GT);
}
{GTE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    return (GTE);
}
{EQ} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    return (EQ);
}
{NEQ} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    return (NEQ);
}
{ASSIN} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (ASSIN);
}
{SEMICOLON} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (SEMICOLON);
}
{COMMA} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (COMMA);
}
{LPARENTHESE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (LPARENTHESE);
}
{RPARENTHESE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (RPARENTHESE);
}
{LBRACKET} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (LBRACKET);
}
{RBRACKET} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (RBRACKET);
}
{LBRACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (LBRACE);
}
{RBRACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (RBRACE);
}
{ELSE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 4;
    return (ELSE);
}
{IF} {
    pos_start = pos_end ;
    pos_end   = pos_start + 2;
    return (IF);
}
{INT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 3;
    return (INT);
}
{FLOAT} {
    pos_start = pos_end ;
    pos_end   = pos_start + 5;
    return (FLOAT);
}
{RETURN} {
    pos_start = pos_end ;
    pos_end   = pos_start + 6;
    return (RETURN);
}
{VOID} {
    pos_start = pos_end ;
    pos_end   = pos_start + 4;
    return (VOID);
}
{WHILE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 5;
    return (WHILE);
}
{IDENTIFIER} {
    pos_start = pos_end;
    pos_end   = pos_start + strlen(yytext);
    return (IDENTIFIER);
}

{SPACE} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (BLANK);
    }
{TAB} {
    pos_start = pos_end ;
    pos_end   = pos_start + 1;
    return (BLANK);
    }
{EOL} {
    lines += 1;
    pos_start = 1;
    pos_end   = 1;
    return (EOL);
}
{INTEGER} {
    pos_start = pos_end;
    pos_end   = pos_start + yyleng;
    return (INTEGER);
}
{FLOATPOINT} {
    pos_start = pos_end;
    pos_end   = pos_start + yyleng;
    return (FLOATPOINT);
}
{ARRAY} {
    pos_start = pos_end;
    pos_end   = pos_start + 2;
    return (ARRAY);
}
{COMMENT} {
    comment(yytext,&lines,&pos_start,&pos_end);
    return (COMMENT);
}
. {return ERROR;}




 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/


int count_num_enter(char *string) {
    int ret = 0;
    int index = 0;
    while(string[index] != '\0') {
        if(string[index] == '\n')
            ret++;
        index++;
    }
    return ret;
}



/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

    while(token = yylex()){
        switch(token){
            case COMMENT:
                break;
            case BLANK:
                break;
            case EOL:
                break;
            case ERROR:
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                token_stream[index].pos_end = pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}

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

/****************C代码 end*************/
