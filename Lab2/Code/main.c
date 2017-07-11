#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "semantic.h"

extern void yyrestart(FILE *);
extern int yyparse();
extern int yylineo;

Node* Root = NULL;
int errorNum = 0;
int theSameLine = 0;

void myerror(char *msg){
    if(theSameLine != yylineno){
        printf("Error type B at line %d: %s\n", yylineno, msg);
        theSameLine = yylineno;
    }
}

int main(int argc, char** argv){

    if (argc <= 1){
        return 1; 
    } 
    FILE* fp = fopen(argv[1],"r");
    if (!fp){
        perror(argv[1]);
        return 1;
    }   

    yylineno=1;

    yyrestart(fp);
    yyparse();

    if(errorNum == 0){
        //printf("there is no error\n\n");
        initHashtable();
        traverseTree(Root);
        //AllSymbol();
        //printTree(Root,0);
    }

    return 0;
}

