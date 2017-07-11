#include "objectcode.h"
#include "intercode.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern InterCode *IRList;
extern int IRlength;
int argsnum = 0;

#define REG_NUM 32
#define CAN_USE 20
RegDescipter Regs[REG_NUM];
VarDescipter *VarList = NULL;
StkDescipter Stack;

void storeAllVar(FILE *fp) {
	VarDescipter *var = VarList;
	while (var != NULL) {
		if (var->reg_no >= 0) {
			Stack.varstack[Stack.length] = var;
			fputs("  addi $sp, $sp, -4\n", fp);
			char str[32];
			memset(str, 0, 32);
			sprintf(str, "  sw %s, 0($sp)\n", Regs[var->reg_no].name);
			fputs(str, fp);
			Regs[var->reg_no].var = NULL;
			Stack.old[Stack.length] = Regs[var->reg_no].old;
			Regs[var->reg_no].old = 0;
			Stack.length++;
		}
		var = var->next;
	}
	VarList = NULL;
}

void loadAllVar(FILE *fp) {
	VarDescipter *var = VarList;
	while (var != NULL) {
		VarList = var->next;
		free(var);
		var = VarList;
	}
	for (int i = Stack.length - 1; i >= 0; i--) {
		VarDescipter* var = Stack.varstack[i];
		if (var == NULL) {
			continue;
		}
		var->next = VarList;
		VarList = var;
		char str[32];
		memset(str, 0, 32);
		sprintf(str, "  lw %s, 0($sp)\n", Regs[var->reg_no].name);
		fputs(str, fp);
		fputs("  addi $sp, $sp, 4\n", fp);
		Regs[var->reg_no].var = var;
		Regs[var->reg_no].old = Stack.old[i];
	}
	Stack.length = 0;
}

int getReg(Operand op, FILE *fp) {
	char name[4];
	memset(name, 0, sizeof(name));

	VarDescipter* var = VarList;
	while (var != NULL) {
		if (op->kind == TEMP_VAR_OP && var->op->u.tvar_no == op->u.tvar_no) {
			sprintf(name, "t%d", op->u.tvar_no);
			break;
		}
		else if (op->kind == VARIABLE_OP && strcmp(var->op->u.value, op->u.value) == 0) {
			sprintf(name, "%s", op->u.value);
			break;
		}
		var = var->next;
	}

	if (var == NULL) {
		var = (VarDescipter*)malloc(sizeof(VarDescipter));
		memset(var, 0, sizeof(VarDescipter));
		var->op = op;
		var->reg_no = -1;
		if (op->kind == TEMP_VAR_OP || op->kind == VARIABLE_OP) {
			if (var == NULL) {
				printf("Insert Variable error!\n");
				exit(1);
			}
			var->next = VarList;
			VarList = var;
		}
	}
	if (var->reg_no == -1) {
		int reg_no = -1;
		for (int i = 0; i < CAN_USE; i++) {
			if (Regs[i].var != NULL) {
				Regs[i].old++;
			}
		}
		for (int i = 0; i < CAN_USE; i++) {
			if (Regs[i].var == NULL) {
				reg_no = i;
				break;
			}
		}
		if (reg_no == -1) {
			int maxOld = -100;
			for (int i = 0; i < CAN_USE; i++) {
				if (Regs[i].old > maxOld) {
					maxOld = Regs[i].old;
					reg_no = i;
				}
			}
		}
		var->reg_no = reg_no;
		Regs[reg_no].var = var;
		if (var->op->kind == CONSTANT_OP) {
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  li %s, %s\n", Regs[var->reg_no].name, var->op->u.value);
			fputs(str, fp);
		}
	}
	Regs[var->reg_no].old = 0;
	return var->reg_no;
}

void irToObject(InterCode ir, int ith, FILE *fp) {
	if (ir == NULL) {
		return;
	}
	if (ir->kind == LABEL_IR) {
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "label%d:\n", ir->u.singleOP.op->u.label_no);
		fputs(str, fp);
	}
	else if (ir->kind == ASSIGN_IR) {
		Operand leftOp = ir->u.doubleOP.left;
		Operand rightOp = ir->u.doubleOP.right;
		if (rightOp->kind == CONSTANT_OP) {
			int reg_no1 = getReg(leftOp, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  li %s, %s\n", Regs[reg_no1].name, rightOp->u.value);
			fputs(str, fp);
		}
		else if (rightOp->kind == TEMP_VAR_OP || rightOp->kind == VARIABLE_OP) {
			int reg_no2 = getReg(rightOp, fp);
			int reg_no1 = getReg(leftOp, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move %s, %s\n", Regs[reg_no1].name, Regs[reg_no2].name);
			fputs(str, fp);
		}
	}
	else if (ir->kind == PLUS_IR) {
		Operand result = ir->u.tripleOP.result;
		Operand op1 = ir->u.tripleOP.op1;
		Operand op2 = ir->u.tripleOP.op2;

		if (op2->kind == CONSTANT_OP) {
			int reg_no2 = getReg(op1, fp);
			int reg_no1 = getReg(result, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  addi %s, %s, %s\n", Regs[reg_no1].name, Regs[reg_no2].name, op2->u.value);
			fputs(str, fp);
		}
		else {
			int reg_no2 = getReg(op1, fp);
			int reg_no3 = getReg(op2, fp);
			int reg_no1 = getReg(result, fp);
			char str[32];
			sprintf(str, "  add %s, %s, %s\n", Regs[reg_no1].name, Regs[reg_no2].name, Regs[reg_no3].name);
			fputs(str, fp);
		}

	}
	else if (ir->kind == MINUS_IR) {
		Operand result = ir->u.tripleOP.result;
		Operand op1 = ir->u.tripleOP.op1;
		Operand op2 = ir->u.tripleOP.op2;

		if (op2->kind == CONSTANT_OP) {
			int reg_no2 = getReg(op1, fp);
			int reg_no1 = getReg(result, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  addi %s, %s, -%s\n", Regs[reg_no1].name, Regs[reg_no2].name, op2->u.value);
			fputs(str, fp);
		}
		else {
			int reg_no2 = getReg(op1, fp);
			int reg_no3 = getReg(op2, fp);
			int reg_no1 = getReg(result, fp);
			char str[32];
			sprintf(str, "  sub %s, %s, %s\n", Regs[reg_no1].name, Regs[reg_no2].name, Regs[reg_no3].name);
			fputs(str, fp);
		}
	}
	else if (ir->kind == STAR_IR) {
		Operand result = ir->u.tripleOP.result;
		Operand op1 = ir->u.tripleOP.op1;
		Operand op2 = ir->u.tripleOP.op2;
		int reg_no2 = getReg(op1, fp);
		int reg_no3 = getReg(op2, fp);
		int reg_no1 = getReg(result, fp);
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  mul %s, %s, %s\n", Regs[reg_no1].name, Regs[reg_no2].name, Regs[reg_no3].name);
		fputs(str, fp);

	}
	else if (ir->kind == DIV_IR) {
		Operand result = ir->u.tripleOP.result;
		Operand op1 = ir->u.tripleOP.op1;
		Operand op2 = ir->u.tripleOP.op2;
		int reg_no2 = getReg(op1, fp);
		int reg_no3 = getReg(op2, fp);
		int reg_no1 = getReg(result, fp);
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  div %s, %s\n", Regs[reg_no2].name, Regs[reg_no3].name);
		fputs(str, fp);
		memset(str, 0, sizeof(str));
		sprintf(str, "  mflo %s\n", Regs[reg_no1].name);
		fputs(str, fp);
	}
	else if (ir->kind == GOTO_IR) {
		Operand op = ir->u.singleOP.op;
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  j label%d\n", ir->u.singleOP.op->u.label_no);
		fputs(str, fp);
	}
	else if (ir->kind == IF_GOTO_IR) {
		int reg_no1 = getReg(ir->u.ifgotoOP.op1, fp);
		int reg_no2 = getReg(ir->u.ifgotoOP.op2, fp);
		char rels[4];
		memset(rels, 0, sizeof(rels));
		strcpy(rels, ir->u.ifgotoOP.relop);
		if (strcmp(rels, "==") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "beq");
		}
		else if (strcmp(rels, "!=") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "bne");
		}
		else if (strcmp(rels, ">") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "bgt");
		}
		else if (strcmp(rels, "<") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "blt");
		}
		else if (strcmp(rels, ">=") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "bge");
		}
		else if (strcmp(rels, "<=") == 0) {
			memset(rels, 0, sizeof(rels));
			strcpy(rels, "ble");
		}
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  %s %s, %s, label%d\n", rels, Regs[reg_no1].name, Regs[reg_no2].name, ir->u.ifgotoOP.label->u.label_no);
		fputs(str, fp);
	}
	else if (ir->kind == RETURN_IR) {
		Operand op = ir->u.singleOP.op;
		if (op->kind == CONSTANT_OP && strcmp(op->u.value, "0") == 0) {
			fputs("  move $v0, $0\n", fp);
		}
		else {
			int reg_no = getReg(op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $v0, %s\n", Regs[reg_no].name);
			fputs(str, fp);
		}
		fputs("  jr $ra\n", fp);
	}
	else if (ir->kind == ARG_IR) {
		if (argsnum >= 4) {
			argsnum = 4;
		}
		else {
			argsnum++;
		}
		getReg(ir->u.singleOP.op, fp);
	}
	else if (ir->kind == CALL_IR) {
		char str[32];
		memset(str, 0, sizeof(str));
		int off = -4 * argsnum - 4;
		sprintf(str, "  addi $sp, $sp, %d\n", off);
		fputs(str, fp);
		fputs("  sw $ra, 0($sp)\n", fp);
		if (argsnum == 1) {
			fputs("  sw $a0, 4($sp)\n", fp);
			InterCode pre1 = IRList[ith - 1];
			int reg_no = getReg(pre1->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a0 %s\n", Regs[reg_no].name);
			fputs(str, fp);

			storeAllVar(fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  jal %s\n", ir->u.doubleOP.right->u.value);
			fputs(str, fp);
			loadAllVar(fp);
			fputs("  lw $a0, 4($sp)\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 8\n", fp);
		}
		else if (argsnum == 2) {
			fputs("  sw $a0, 4($sp)\n", fp);
			fputs("  sw $a1, 8($sp)\n", fp);
			InterCode pre1 = IRList[ith - 1];
			int reg_no1 = getReg(pre1->u.singleOP.op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a0 %s\n", Regs[reg_no1].name);
			fputs(str, fp);

			InterCode pre2 = IRList[ith - 2];
			int reg_no2 = getReg(pre2->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a1 %s\n", Regs[reg_no2].name);
			fputs(str, fp);

			storeAllVar(fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  jal %s\n", ir->u.doubleOP.right->u.value);
			fputs(str, fp);
			loadAllVar(fp);
			fputs("  lw $a1, 8($sp)\n", fp);
			fputs("  lw $a0, 4($sp)\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 12\n", fp);
		}
		else if (argsnum == 3) {
			fputs("  sw $a0, 4($sp)\n", fp);
			fputs("  sw $a1, 8($sp)\n", fp);
			fputs("  sw $a2, 12($sp)\n", fp);

			InterCode pre1 = IRList[ith - 1];
			int reg_no1 = getReg(pre1->u.singleOP.op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a0 %s\n", Regs[reg_no1].name);
			fputs(str, fp);

			InterCode pre2 = IRList[ith - 2];
			int reg_no2 = getReg(pre2->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a1 %s\n", Regs[reg_no2].name);
			fputs(str, fp);

			InterCode pre3 = IRList[ith - 3];
			int reg_no3 = getReg(pre2->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a2 %s\n", Regs[reg_no3].name);
			fputs(str, fp);

			storeAllVar(fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  jal %s\n", ir->u.doubleOP.right->u.value);
			fputs(str, fp);
			loadAllVar(fp);

			fputs("  lw $a2, 12($sp)\n", fp);
			fputs("  lw $a1, 8($sp)\n", fp);
			fputs("  lw $a0, 4($sp)\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 16\n", fp);
		}
		else if (argsnum == 3) {
			fputs("  sw $a0, 4($sp)\n", fp);
			fputs("  sw $a1, 8($sp)\n", fp);
			fputs("  sw $a2, 12($sp)\n", fp);
			fputs("  sw $a3, 16($sp)\n", fp);
			InterCode pre1 = IRList[ith - 1];
			int reg_no1 = getReg(pre1->u.singleOP.op, fp);
			char str[32];
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a0 %s\n", Regs[reg_no1].name);
			fputs(str, fp);

			InterCode pre2 = IRList[ith - 2];
			int reg_no2 = getReg(pre2->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a1 %s\n", Regs[reg_no2].name);
			fputs(str, fp);

			InterCode pre3 = IRList[ith - 3];
			int reg_no3 = getReg(pre3->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a2 %s\n", Regs[reg_no3].name);
			fputs(str, fp);

			InterCode pre4 = IRList[ith - 4];
			int reg_no4 = getReg(pre4->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move $a3 %s\n", Regs[reg_no4].name);
			fputs(str, fp);

			storeAllVar(fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  jal %s\n", ir->u.doubleOP.right->u.value);
			fputs(str, fp);
			loadAllVar(fp);

			fputs("  lw $a3, 16($sp)\n", fp);
			fputs("  lw $a2, 12($sp)\n", fp);
			fputs("  lw $a1, 8($sp)\n", fp);
			fputs("  lw $a0, 4($sp)\n", fp);
			fputs("  lw $ra, 0($sp)\n", fp);
			fputs("  addi $sp, $sp, 20\n", fp);
		}
		argsnum = 0;
		//取v0寄存器中的返回值
		int reg_no = getReg(ir->u.doubleOP.left, fp);
		memset(str, 0, sizeof(str));
		sprintf(str, "  move %s, $v0\n", Regs[reg_no].name);
		fputs(str, fp);
	}
	else if (ir->kind == FUNCTION_IR) {
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "\n%s:\n", ir->u.singleOP.op->u.value);
		fputs(str, fp);

		VarDescipter *var = VarList;
		while (var != NULL) {
			VarList = var->next;
			free(var);
			var = VarList;
		}

		for (int i = 0; i < CAN_USE; i++) {
			Regs[i].old = 0;
			Regs[i].var = NULL;
		}
		int i = 0;
		while (IRList[ith + i + 1]->kind == PARAM_IR) {
			int reg_no = getReg(IRList[ith + i + 1]->u.singleOP.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "  move %s, $a%d\n", Regs[reg_no].name, i);
			fputs(str, fp);
			i++;
		}
	}
	else if (ir->kind == READ_IR) {
		fputs("  addi $sp, $sp, -4\n", fp);
		fputs("  sw $ra, 0($sp)\n", fp);
		fputs("  jal read\n", fp);
		int reg_no = getReg(ir->u.singleOP.op, fp);
		fputs("  lw $ra, 0($sp)\n", fp);
		fputs("  addi $sp, $sp, 4\n", fp);
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  move %s, $v0\n", Regs[reg_no].name);
		fputs(str, fp);
	}
	else if (ir->kind == WRITE_IR) {
		int reg_no = getReg(ir->u.singleOP.op, fp);
		char str[32];
		memset(str, 0, sizeof(str));
		sprintf(str, "  move $a0, %s\n", Regs[reg_no].name);
		fputs(str, fp);
		fputs("  addi $sp, $sp, -4\n", fp);
		fputs("  sw $ra, 0($sp)\n", fp);
		fputs("  jal write\n", fp);
		fputs("  lw $ra, 0($sp)\n", fp);
		fputs("  addi $sp, $sp, 4\n", fp);
	}
}

void writeAllObject(FILE *fp) {

	//memset(Regs, 0, sizeof(Regs));
	for (int i = 0; i < CAN_USE; i++) {
		Regs[i].var = NULL;
		Regs[i].old = 0;
	}
	//0-19可使用
	strcpy(Regs[0].name, "$t0");
	strcpy(Regs[1].name, "$t1");
	strcpy(Regs[2].name, "$t2");
	strcpy(Regs[3].name, "$t3");
	strcpy(Regs[4].name, "$t4");
	strcpy(Regs[5].name, "$t5");
	strcpy(Regs[6].name, "$t6");
	strcpy(Regs[7].name, "$t7");
	strcpy(Regs[8].name, "$t8");
	strcpy(Regs[9].name, "$t9");
	strcpy(Regs[10].name, "$s0");
	strcpy(Regs[11].name, "$s1");
	strcpy(Regs[12].name, "$s2");
	strcpy(Regs[13].name, "$s3");
	strcpy(Regs[14].name, "$s4");
	strcpy(Regs[15].name, "$s5");
	strcpy(Regs[16].name, "$s6");
	strcpy(Regs[17].name, "$s7");
	strcpy(Regs[18].name, "$v1");

	strcpy(Regs[19].name, "$fp");


	//保留使用,无数组和函数调用情况下
	strcpy(Regs[20].name, "$a0");
	strcpy(Regs[21].name, "$a1");
	strcpy(Regs[22].name, "$a2");
	strcpy(Regs[23].name, "$a3");
	strcpy(Regs[24].name, "$v0");	//$v0 存放函数返回值


	//25-31 不能使用
	strcpy(Regs[25].name, "$ra");	//$ra 保存函数返回地址
	strcpy(Regs[26].name, "$gp");	//$gp 静态数据段指针
	strcpy(Regs[27].name, "$sp");	//$sp 栈指针
	strcpy(Regs[28].name, "$k0");	//$k0 中断处理保留
	strcpy(Regs[29].name, "$k1");	//$k1 中断处理保留
	strcpy(Regs[30].name, "$at");	//$at 汇编器保留
	strcpy(Regs[31].name, "$zero");	//$zero 常数0寄存器

	fputs(".data\n", fp);
	fputs("_prompt: .asciiz \"Enter an integer:\"\n", fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp);
	fputs(".globl main\n", fp);
	fputs(".text\n\n", fp);

	fputs("read:\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _prompt\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 5\n", fp);
	fputs("  syscall\n", fp);
	fputs("  jr $ra\n\n", fp);

	fputs("write:\n", fp);
	fputs("  li $v0, 1\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _ret\n", fp);
	fputs("  syscall\n", fp);
	fputs("  move $v0, $0\n", fp);
	fputs("  jr $ra\n\n", fp);

	for (int i = 0; i < IRlength; i++) {
		if (IRList[i] != NULL) {
			irToObject(IRList[i], i, fp);
		}
	}
}
