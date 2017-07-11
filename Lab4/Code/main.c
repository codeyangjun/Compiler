#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "semantic.h"
#include "intercode.h"

extern void yyrestart(FILE *);
extern int yyparse();
extern int yylineo;

Node* Root = NULL;
int errorNum = 0;
int theSameLine = 0;
int semanticError = 0;
int structError = 0;

void myerror(char *msg){
    if(theSameLine != yylineno){
        printf("Error type B at line %d: %s\n", yylineno, msg);
        theSameLine = yylineno;
    }
}

int main(int argc, char** argv){
	FILE *fp;
	if (argc == 1) {
		fp = fopen("test.cmm", "r");
	}
	else if(argc >= 2){
		fp = fopen(argv[1],"r");
	}
    if (!fp){
		printf("cant't open the test file.\n\n");
        perror(argv[1]);
        return 1;
    }

    yylineno=1;

    yyrestart(fp);
    yyparse();

	if (structError == 0 && errorNum==0) {
		initHashtable();
		initIRList();
		traverseTree(Root);
		if(argc == 1){
			writeCode("stdout");
		}
		else if (argc == 2) {
			printf("Usage:  ./parser  test.cmm  out.s\n");
		}
		else if(argc == 3){
			writeCode(argv[2]);
		}
	}
	else {
		printf("Cannot translate: Code contains variables or parameters of structure type.\n");
	}

    return 0;
}

