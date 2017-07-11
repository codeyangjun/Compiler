#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "intercode.h"

#define HASH_SIZE 65536  

#define INT_TYPE 1
#define FLOAT_TYPE 2

typedef enum Kind_ {
	BASIC, ARRAY, STRUCTURE, FUNCTION_S
}Kind;

typedef struct Type_t *TypePtr;
typedef struct FieldList_t *FieldList;

typedef struct Type_t {

	Kind kind;
	union{
		//basic type
		int basic_;

		//array type
		struct {
			int size;
			TypePtr elem;
		}array_;

		//structure type
		FieldList structure_;

		//function type
		struct{
			FieldList params;//parameters
			TypePtr funcType;
			int paramNum;//number of parameters
		}function_;
	}u;
}Type_t;

typedef struct FieldList_t {
	char *name;
	TypePtr type;
	FieldList tail;
	int collision;
	int is_in_params;
}FieldList_t;

void traverseTree(Node *root);

TypePtr Specifier(Node *root);
void ExtDefList(Node *root);

FieldList VarDec(Node *root, TypePtr basictype, int from);
void CompSt(Node *root,TypePtr funcType);

void Stmt(Node *root,TypePtr funcType);
TypePtr Exp(Node* root, Operand op);

int getSize(TypePtr typ, int t);

void DefList(Node *root);
FieldList Def(Node* root, int from);
FieldList DecList(Node *root, TypePtr type, int from);
FieldList Dec(Node *root, TypePtr type, int from);

unsigned int hash_pjw(char *name);
void initHashtable();
int insertSymbol(FieldList f);
int TypeEqual(TypePtr type1,TypePtr type2);
FieldList lookupSymbol(char *name,int function);//function 1,varible 0
void AllSymbol();

int Args(Node* root, Operand argsList);
TypePtr Cond(Node* root, Operand left, Operand right);
#endif
