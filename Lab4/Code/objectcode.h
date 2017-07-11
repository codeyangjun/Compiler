#ifndef _OBJECT_CODE_H_
#define _OBJECT_CODE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercode.h"
#include "semantic.h"

typedef struct Var_t {
	int reg_no;
	Operand op;
	struct Var_t *next;
}VarDescipter;

typedef struct RegDescipter {
	char name[6];
	int old;
	struct Var_t *var;
}RegDescipter;

typedef struct StkDescipter {
	int length;
	int from;
	int old[1024];
	VarDescipter *varstack[1024];
}StkDescipter;

void writeAllObject(FILE *fp);

#endif

