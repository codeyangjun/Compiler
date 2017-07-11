%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <string.h>
    #include "syntax.tab.h"
    #include "node.h"
    extern int errorNum;
    int theSameLine2 = 0;
%}
%option yylineno

SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
SPACE [ \r\n\t]+
DECNUM ([1-9][0-9]*)|0
OCT 0[0-7]*
OCT_ERROR 0[0-9A-WYZa-wyz][0-9A-Za-z]*
HEX 0[Xx][0-9a-fA-F]*
HEX_ERROR [0][Xx][0-9A-Za-z]*
INT {DECNUM}|{OCT}|{HEX}
FLOAT [0-9]*[.][0-9]+f?
ID [A-Za-z_][A-Za-z_0-9]*
COMMENT "//"
OTHER .

%%

{SEMI}  {
    yylval.node=createNode("SEMI",yytext);
    return(SEMI);
}
{COMMA} {
    yylval.node=createNode("COMMA",yytext);
    return(COMMA);
}
{ASSIGNOP} {
    yylval.node=createNode("ASSIGNOP",yytext);
    return ASSIGNOP;
}


{RELOP}     {
    yylval.node=createNode("RELOP",yytext);
    return RELOP;
}

{PLUS}      {
    yylval.node=createNode("PLUS",yytext);
    return PLUS;
}
{MINUS}     {
    yylval.node=createNode("MINUS",yytext);
    return MINUS;
}
{STAR}      {
    yylval.node=createNode("STAR",yytext);
    return STAR;
}
{DIV}       {
    yylval.node=createNode("DIV",yytext);
    return DIV;
}
{AND}       {
    yylval.node=createNode("AND",yytext);
    return AND;
}
{OR}        {
    yylval.node=createNode("OR",yytext);
    return OR;
}
{DOT}       {
    yylval.node=createNode("DOT",yytext);
    return DOT;
}
{NOT}       {
    yylval.node=createNode("NOT",yytext);
    return NOT;
}
{TYPE}      {
    yylval.node=createNode("TYPE",yytext);
    return TYPE;
}
{LP}        {
    yylval.node=createNode("LP",yytext);
    return LP;
}
{RP}        {
    yylval.node=createNode("RP",yytext);
    return RP;
}
{LB}        {
    yylval.node=createNode("LB",yytext);
    return LB;
}
{RB}        {
    yylval.node=createNode("RB",yytext);
    return RB;
}
{LC}        {
    
    yylval.node=createNode("LC",yytext);
    return LC;
}
{RC}        {
    yylval.node=createNode("RC",yytext);
    return RC;
}
{STRUCT}    {
    yylval.node=createNode("STRUCT",yytext);
    return STRUCT;
}
{RETURN}    {
    yylval.node=createNode("RETURN",yytext);
    return RETURN;
}
{IF}        {
    yylval.node=createNode("IF",yytext);
    return IF;
}
{ELSE}      {
    yylval.node=createNode("ELSE",yytext);
    return ELSE;
}
{WHILE}     {
    yylval.node=createNode("WHILE",yytext);
    return WHILE;
}
{SPACE}     {}


{OCT}       {
    char *end = NULL;
    int l = (int)strtol(yytext, &end, 8);
    snprintf(yytext, 10,"%d",l);
    yylval.node=createNode("INT",yytext);
    return INT;
}
{OCT_ERROR} {
    errorNum++;
    printf("Error type A at Line %d: Illegal octal number '%s'\n", yylineno, yytext);
    return INT;
}
{HEX}       {
    char *end = NULL;
    int l = (int)strtol(yytext, &end, 16);
    snprintf(yytext, 10,"%d",l);
    yylval.node=createNode("INT",yytext);
    return INT;
}
{HEX_ERROR} {
    errorNum++;
    printf("Error type A at Line %d: Illegal hexadecimal number '%s'\n", yylineno, yytext);
    return INT;
}
{INT}       {
    yylval.node=createNode("INT",yytext);
    return INT;
}
{FLOAT}     {
    yylval.node=createNode("FLOAT",yytext);
    return FLOAT;
}

{ID}        {
    
    yylval.node=createNode("ID",yytext);
    return ID;
}

{OTHER} {
    errorNum++;
    if(theSameLine2 != yylineno){
        printf("Error type A at line %d: Mysterious character \"%s\".\n",yylineno,yytext);
        theSameLine2 = yylineno;
    }
    
}
{COMMENT} {
        printf("Error type A at line %d: Mysterious character \"%s\".\n",yylineno,yytext);
        theSameLine2 = yylineno;  
}


%%

int yywrap() {
    return 1;
}


  
