#ifndef __INNERCODE_H__
#define __INNERCODE_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _IS_DEBUG_ 1

//变量 n
//临时变量 t1, t2
//常量 #1 #2
//变量地址
//临时变量地址
//标号Label
//函数定义

//单条中间代码的数据结构
typedef struct Operand_t* Operand;
typedef struct Operand_t {
	//		0			1		2			3		4		5		6
	enum {VARIABLE_OP, TEMP_VAR_OP, CONSTANT_OP, ADDRESS_OP, TEMP_ADDR_OP, LABEL_OP, FUNCTION_OP, DEBUG_OP } kind;
	union {
		int tvar_no;		//TEMP_VAR_OP
		int label_no;		//LABEL
		char value[32];		//VARIABLE_OP  CONSTANT_OP  FUNCTION_OP DEBUG_OP
		Operand name;		//ADDRESS_OP  TEMP_ADDR_OP
	}u;
	struct Operand_t *nextArgs;
	struct Operand_t *prevArgs;
}Operand_t;


typedef struct InterCode_t* InterCode;
typedef struct InterCode_t {
	//见实验指导书64页表3-1 各个对应的19个IR 可以实现枚举体和结构体
	enum {
			LABEL_IR, FUNCTION_IR, ASSIGN_IR, PLUS_IR, MINUS_IR, STAR_IR, DIV_IR, GET_ADDR_IR, GET_VALUE_IR, 
			TO_MEMORY_IR, GOTO_IR, IF_GOTO_IR, RETURN_IR, DEC_IR, ARG_IR, CALL_IR, PARAM_IR, READ_IR, WRITE_IR, DEBUG_IR, RIGHTAT_IR
	}kind;

	union{
		//1操作数/码(8): LABEL_IR FUNCTION_IR GOTO_IR RETURN_IR ARG_IR PARAM_IR READ_IR WRITE_IR DEBUG_IR
		struct{
			Operand op;
		}singleOP;
		
		//2-操作数/码(5): ASSIGN_IR GET_VALUE_IR TO_MEMORY_IR CALL_IR
		struct{
			Operand left;
			Operand right;
		}doubleOP;
		
		//3-操作数/码(4): PLUS_IR MINUS_IR STAR_IR DIV_IR GET_ADDR_IR
		struct{
			Operand result;
			Operand op1;
			Operand op2;
		}tripleOP;
		
		//特殊(1): IF_GOTO_IR
		struct{
			Operand op1;
			Operand op2;
			Operand label;
			char relop[32];
		}ifgotoOP;
		
		//特殊(1): DEC_IR
		struct{
			Operand op;
			int size;
		}decOP;
	}u;
}InterCode_t;

void initIRList();
void insertCode(InterCode ir);
void writeCode(char *filename);
void writeOp(Operand op,FILE *file);

void optimize();

#endif