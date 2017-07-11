#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercode.h"

int LabelNum = 1;
int TempVarNum = 1;
int lineNum = 1;
int VarNum = 1;

InterCode *IRList;
int IRlength;
int IRcapacity;
#define IRLIST_INIT_SIZE 10

void initIRList() {
	IRList = (InterCode*)malloc(IRLIST_INIT_SIZE * sizeof(InterCode));
	if (IRList == NULL) {
		printf("IRList Error!\n");
		return;
	}
	IRcapacity = IRLIST_INIT_SIZE;
	IRlength = 0;
}

void insertCode(InterCode ir) {
	if (IRlength >= IRcapacity) {
		IRList = (InterCode*)realloc(IRList, sizeof(InterCode)*IRcapacity * 2);
		IRcapacity = IRcapacity * 2;
	}
	IRList[IRlength] = ir;
	IRlength++;
}

void writeCode(char *filename) {
	optimize();
	FILE *fp;
	if (strcmp(filename, "stdout") == 0) {
		fp = stdout;
	}
	else {
		fp = fopen(filename, "w");
	}
	if (fp == NULL) {
		printf("open file error!\n");
		return;
	}

	for (int i = 0; i < IRlength; i++) {
		InterCode ir = IRList[i];
		if (ir == NULL) {
			continue;
		}
		switch (ir->kind) {
		case LABEL_IR:
			fputs("LABEL ", fp);
			writeOp(ir->u.singleOP.op, fp);
			fputs(" : ", fp);
			break;
		case  FUNCTION_IR:
			fputs("FUNCTION ", fp);
			writeOp(ir->u.singleOP.op, fp);
			fputs(" : ", fp);
			break;
		case ASSIGN_IR:
			writeOp(ir->u.doubleOP.left, fp);
			fputs(" := ", fp);
			writeOp(ir->u.doubleOP.right, fp);
			break;
		case PLUS_IR:
			writeOp(ir->u.tripleOP.result, fp);
			fputs(" := ", fp);
			writeOp(ir->u.tripleOP.op1, fp);
			fputs(" + ", fp);
			writeOp(ir->u.tripleOP.op2, fp);
			break;
		case MINUS_IR:
			writeOp(ir->u.tripleOP.result, fp);
			fputs(" := ", fp);
			writeOp(ir->u.tripleOP.op1, fp);
			fputs(" - ", fp);
			writeOp(ir->u.tripleOP.op2, fp);
			break;
		case STAR_IR:
			writeOp(ir->u.tripleOP.result, fp);
			fputs(" := ", fp);
			writeOp(ir->u.tripleOP.op1, fp);
			fputs(" * ", fp);
			writeOp(ir->u.tripleOP.op2, fp);
			break;
		case DIV_IR:
			writeOp(ir->u.tripleOP.result, fp);
			fputs(" := ", fp);
			writeOp(ir->u.tripleOP.op1, fp);
			fputs(" / ", fp);
			writeOp(ir->u.tripleOP.op2, fp);
			break;
		case GET_ADDR_IR:
			writeOp(ir->u.tripleOP.result, fp);
			fputs(" := &", fp);
			writeOp(ir->u.tripleOP.op1, fp);
			fputs(" + ", fp);
			writeOp(ir->u.tripleOP.op2, fp);
			break;
		case RIGHTAT_IR:
			writeOp(ir->u.doubleOP.left, fp);
			fputs(" := &", fp);
			writeOp(ir->u.doubleOP.right, fp);
			break;
		case GET_VALUE_IR:
			writeOp(ir->u.doubleOP.left, fp);
			fputs(" := *", fp);
			writeOp(ir->u.doubleOP.right, fp);
			break;
		case TO_MEMORY_IR:
			fputs("*", fp);
			writeOp(ir->u.doubleOP.left, fp);
			fputs(" := ", fp);
			writeOp(ir->u.doubleOP.right, fp);
			break;
		case GOTO_IR:
			fputs("GOTO ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case IF_GOTO_IR:
			fputs("IF ", fp);
			writeOp(ir->u.ifgotoOP.op1, fp);
			fputs(" ", fp);
			fputs(ir->u.ifgotoOP.relop, fp);
			fputs(" ", fp);
			writeOp(ir->u.ifgotoOP.op2, fp);
			fputs(" GOTO ", fp);
			writeOp(ir->u.ifgotoOP.label, fp);
			break;
		case RETURN_IR:
			fputs("RETURN ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case DEC_IR:
			fputs("DEC ", fp);
			writeOp(ir->u.decOP.op, fp);
			char str[10];
			memset(str, 0, sizeof(str));
			sprintf(str, " %d ", ir->u.decOP.size);
			fputs(str, fp);
			break;
		case ARG_IR:
			fputs("ARG ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case CALL_IR:
			writeOp(ir->u.doubleOP.left, fp);
			fputs(" := CALL ", fp);
			writeOp(ir->u.doubleOP.right, fp);
			break;
		case PARAM_IR:
			fputs("PARAM ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case READ_IR:
			fputs("READ ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case WRITE_IR:
			fputs("WRITE ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		case DEBUG_IR:
			fputs("Debug imformation: ", fp);
			writeOp(ir->u.singleOP.op, fp);
			break;
		default:
			break;
		}

		fputs("\n", fp);
	}
	fclose(fp);
}

void writeOp(Operand op, FILE *fp) {
	if (op == NULL) {
		fputs("t0	", fp);
		return;
	}
	char str[50];
	memset(str, 0, sizeof(str));

	switch (op->kind) {
		//enum {VARIABLE_OP, TEMP_VAR_OP, CONSTANT_OP, ADDRESS_OP, TEMP_ADDR_OP, LABEL_OP, FUNCTION_OP } kind;
	case VARIABLE_OP:
		if (op->u.value != NULL) {
			sprintf(str, "%s", op->u.value);
			fputs(str, fp);
		}
		break;
	case TEMP_VAR_OP:
		sprintf(str, "t%d", op->u.tvar_no);
		fputs(str, fp);
		break;
	case CONSTANT_OP:
		sprintf(str, "#%s", op->u.value);
		fputs(str, fp);
		break;
	case ADDRESS_OP:
		sprintf(str, "*%s", op->u.name->u.value);
		fputs(str, fp);
		break;
	case TEMP_ADDR_OP:
		sprintf(str, "*t%d", op->u.name->u.tvar_no);
		fputs(str, fp);
		break;
	case LABEL_OP:
		sprintf(str, "label%d", op->u.label_no);
		fputs(str, fp);
		break;
	case FUNCTION_OP:
		sprintf(str, "%s", op->u.value);
		fputs(str, fp);
		break;
	case DEBUG_OP:
		sprintf(str, "debug: %s", op->u.value);
		fputs(str, fp);
		break;
	}
}

void optimize() {

}