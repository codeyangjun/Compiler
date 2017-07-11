#include "node.h"
#include "semantic.h"
#include "intercode.h"

extern int semanticError;

extern int LabelNum;
extern int TempVarNum;
extern int VarNum;

FieldList hashTable[HASH_SIZE];

unsigned int hash_pjw(char *name) {
	unsigned int val = 0, i;
	for (; *name; ++name) {
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) {
			val = (val ^ (i >> 12)) & 0x3fff;
		}
		return val % HASH_SIZE;
	}
}

void initHashtable() {
	for (int i = 0; i < HASH_SIZE; i++) {
		hashTable[i] = NULL;
	}
	TempVarNum = 1;
	//插入'int read()'函数
	{
		FieldList field1 = (FieldList)malloc(sizeof(FieldList_t));
		field1->name = "read";
		TypePtr typ1 = (TypePtr)malloc(sizeof(Type_t));
		typ1->kind = FUNCTION_S;
		TypePtr typ1_return = (TypePtr)malloc(sizeof(Type_t));
		typ1_return->kind = BASIC;
		typ1_return->u.basic_ = INT_TYPE;
		typ1->u.function_.funcType = typ1_return;
		typ1->u.function_.paramNum = 0;
		typ1->u.function_.params = NULL;
		field1->type = typ1;
		insertSymbol(field1);
	}
	//插入'int write(int write_param_name)'函数
	{

		FieldList field2 = (FieldList)malloc(sizeof(FieldList_t));
		field2->name = "write";
		TypePtr typ2 = (TypePtr)malloc(sizeof(Type_t));
		typ2->kind = FUNCTION_S;
		TypePtr typ2_return = (TypePtr)malloc(sizeof(Type_t));
		typ2_return->kind = BASIC;
		typ2_return->u.basic_ = INT_TYPE;
		typ2->u.function_.funcType = typ2_return;
		typ2->u.function_.paramNum = 1;
		FieldList write_param = (FieldList)malloc(sizeof(FieldList_t));
		write_param->name = "write_param_name_is_n";
		TypePtr write_param_typ = (TypePtr)malloc(sizeof(Type_t));
		write_param_typ->kind = BASIC;
		write_param_typ->u.basic_ = INT_TYPE;
		write_param->type = write_param_typ;
		typ2->u.function_.params = write_param;
		field2->type = typ2;
		insertSymbol(field2);
	}
}

int insertSymbol(FieldList f) {
	if (f == NULL) {
		return 0;
	}
	if (f->name == NULL) {
		return 0;
	}
	f->collision = 0;
	unsigned int key;
	if (f->type->kind == 3) {
		key = hash_pjw(1 + f->name);
	}
	else {
		key = hash_pjw(f->name);
	}
	if (hashTable[key] == NULL) {
		hashTable[key] = f;
		return 1;
	}

	while (1) {
		key = (++key) % HASH_SIZE;
		f->collision = f->collision + 1;
		if (hashTable[key] == NULL) {
			hashTable[key] = f;
			return 1;
		}
	}
	return 0;
}

FieldList lookupSymbol(char *name, int function) {
	if (name == NULL) {
		return NULL;
	}
	unsigned int key;
	if (function) {
		key = hash_pjw(1 + name);
	}
	else {
		key = hash_pjw(name);
	}
	FieldList p = hashTable[key];
	while (p != NULL) {
		if (strcmp(name, p->name) == 0) {
			if ((function == 1) && (p->type->kind == FUNCTION_S)) {
				return p;
			}
			if ((function == 0) && (p->type->kind != FUNCTION_S)) {
				return p;
			}
		}
		key = (++key) % HASH_SIZE;
		p = hashTable[key];
	}
	return NULL;
}

void AllSymbol() {
	//printf("---------- Symbol Table -----------\n");
	for (int i = 0; i < HASH_SIZE; i++) {
		if (hashTable[i] != NULL) {
			Kind t = hashTable[i]->type->kind;
			printf("symbol: %s\t", hashTable[i]->name);
			switch (t) {
			case BASIC:
				if (hashTable[i]->type->u.basic_ == INT_TYPE) {
					printf("int");
				}
				else {
					printf("float");
				}
				break;
			case ARRAY: {
				int j = 1;
				int size = 4;
				TypePtr typ = hashTable[i]->type;
				while (typ->u.array_.elem->kind != BASIC) {
					j++;
					size = typ->u.array_.size * size;
					typ = typ->u.array_.elem;
				}
				size = typ->u.array_.size * size;
				if (BASIC == typ->u.array_.elem->kind) {
					if (typ->u.array_.elem->kind == INT_TYPE) {
						printf("int array\t%d-order %d-byte", j, size);
					}
					else {
						printf("float array\t%d-order %d-byte", j, size);
					}
				}
			}
						break;
			case STRUCTURE:
				printf("structure\t");
				break;
			case FUNCTION_S: {
				TypePtr typ = hashTable[i]->type;
				if (typ->u.function_.funcType->u.basic_ == INT_TYPE) {
					printf("int-function\t%d-params", typ->u.function_.paramNum);
				}
				else {
					printf("float-function\t%d-params", typ->u.function_.paramNum);
				}
				int j = 0;
				FieldList params = typ->u.function_.params;
				while (params != NULL) {
					Kind t = params->type->kind;
					switch (t) {
					case BASIC:
						if (params->type->u.basic_ == INT_TYPE) {
							printf("  int ");
						}
						else {
							printf("  float ");
						}
						break;
					case ARRAY: {
						int j = 1;
						int size = 4;
						TypePtr typ = params->type;
						while (typ->u.array_.elem->kind != BASIC) {
							j++;
							size = typ->u.array_.size * size;
							typ = typ->u.array_.elem;
						}
						size = typ->u.array_.size * size;
						if (BASIC == typ->u.array_.elem->kind) {
							if (typ->u.array_.elem->kind == INT_TYPE) {
								printf("int array\t%d-order %d-byte", j, size);
							}
							else {
								printf("float array\t%d-order %d-byte", j, size);
							}
						}
					}
					}
					printf("%s", params->name);
					params = params->tail;
				}
			}
							 break;
			default:
				break;
			}
			printf("\n");
		}
	}
	printf("\n");
}

int TypeEqual(TypePtr type1, TypePtr type2) {
	if ((type1 == NULL) || (type2 == NULL))
		return 0;
	if (type1->kind != type2->kind)
		return 0;
	else switch (type1->kind) {
	case BASIC: {
		if (type1->u.basic_ == type2->u.basic_) {
			return 1;
		}
		else {
			return 0;
		}
	}break;
	case ARRAY: {
		if (TypeEqual(type1->u.array_.elem, type2->u.array_.elem) == 1) {
			return 1;
		}
		else {
			return 0;
		}
	}break;
	case STRUCTURE: {
		FieldList field1 = type1->u.structure_;
		FieldList field2 = type2->u.structure_;
		if ((field1 != NULL) && (field2 != NULL)) {
			while ((field1 != NULL) && (field2 != NULL)) {
				if (TypeEqual(field1->type, field2->type) == 0) {
					return 0;
				}
				field1 = field1->tail;
				field2 = field2->tail;
			}
			if ((field1 == NULL) && (field2 == NULL)) {
				return 1;
			}
		}
		return 0;
	}break;
	case FUNCTION_S: {
		if (type1->u.function_.paramNum != type2->u.function_.paramNum) {
			return 0;
		}
		FieldList param1 = type1->u.function_.params;
		FieldList param2 = type2->u.function_.params;
		for (int i = 0; i < type1->u.function_.paramNum; i++) {
			if (TypeEqual(param1->type, param2->type) == 0) {
				return 0;
			}
			param1 = param1->tail;
			param2 = param2->tail;
		}
		return 1;
	}break;
	default: {
		return 0;
	}break;
	}
}

void ExtDefList(Node *root) {
	Node* ExtDefList = root;
	while (ExtDefList->childsum != 0) {//ExtDef ExtDefList
		Node* ExtDef = ExtDefList->child[0];
		TypePtr basictype = Specifier(ExtDef->child[0]);
		//lab3中无全局变量
		if (strcmp(ExtDef->child[1]->name, "ExtDecList") == 0) {//Specifier ExtDecList SEMI
			Node* temp = ExtDef->child[1];//ExtDecList
			FieldList field;
			int from = 1;
			while (temp->childsum == 3) {
				field = VarDec(temp->child[0], basictype, from);
				if (lookupSymbol(field->name, 0) != NULL) {
					semanticError++;
					//printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, field->name);
				}
				else {
					insertSymbol(field);
				}
				temp = temp->child[2];
			}
			field = VarDec(temp->child[0], basictype, from);
			if (lookupSymbol(field->name, 0) != NULL) {
				semanticError++;
				//printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, field->name);
			}
			else {
				insertSymbol(field);
			}
		}
		else if (strcmp(ExtDef->child[1]->name, "FunDec") == 0) {//Specifier FunDec CompSt
			Node *funcNode = ExtDef->child[1];
			FieldList field = (FieldList)malloc(sizeof(FieldList_t));
			memset(field, 0, sizeof(FieldList_t));

			field->name = funcNode->child[0]->text;
			TypePtr typ = (TypePtr)malloc(sizeof(Type_t));
			memset(typ, 0, sizeof(Type_t));

			typ->kind = FUNCTION_S;
			typ->u.function_.funcType = basictype;
			typ->u.function_.paramNum = 0;
			typ->u.function_.params = NULL;

			Operand funcOp = (Operand)malloc(sizeof(struct Operand_t));
			memset(funcOp, 0, sizeof(Operand_t));
			funcOp->kind = FUNCTION_OP;
			strcpy(funcOp->u.value, field->name);
			InterCode funcIR = (InterCode)malloc(sizeof(InterCode_t));
			memset(funcIR, 0, sizeof(InterCode_t));
			funcIR->kind = FUNCTION_IR;
			funcIR->u.singleOP.op = funcOp;
			insertCode(funcIR);

			int from = 4;

			if (strcmp(ExtDef->child[1]->child[2]->name, "VarList") == 0) {//ID LP VarList RP
				//ExtDef->child[1]->child[2] = ExtDef
				Node *VarList = ExtDef->child[1]->child[2];
				while (VarList->childsum != 1) {//ParamDec COMMA VarList
					TypePtr tempType = Specifier(VarList->child[0]->child[0]);
					FieldList tempField = VarDec(VarList->child[0]->child[1], tempType, from);

					if (lookupSymbol(tempField->name, 0) != NULL) {
						semanticError++;
						//printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, tempField->name);
					}
					else {
						tempField->is_in_params = 1;
						insertSymbol(tempField);
					}
					typ->u.function_.paramNum++;
					tempField->tail = typ->u.function_.params;
					typ->u.function_.params = tempField;
					Operand paramOp = NULL;
					if (tempField->type->kind == BASIC) {

						paramOp = (Operand)malloc(sizeof(struct Operand_t));
						memset(paramOp, 0, sizeof(Operand_t));
						paramOp->kind = VARIABLE_OP;
						strcpy(paramOp->u.value, typ->u.function_.params->name);
						InterCode paramIR = (InterCode)malloc(sizeof(InterCode_t));
						memset(paramIR, 0, sizeof(InterCode_t));
						paramIR->kind = PARAM_IR;
						paramIR->u.singleOP.op = paramOp;
						insertCode(paramIR);
					}
					else if (tempField->type->kind == ARRAY) {
						Operand tempOp = (Operand)malloc(sizeof(Operand_t));
						memset(tempOp, 0, sizeof(Operand_t));
						tempOp->kind = VARIABLE_OP;
						strcpy(tempOp->u.value, typ->u.function_.params->name);

						InterCode paramIR = (InterCode)malloc(sizeof(InterCode_t));
						memset(paramIR, 0, sizeof(InterCode_t));
						paramIR->kind = PARAM_IR;
						paramIR->u.singleOP.op = tempOp;
						insertCode(paramIR);
					}


					VarList = VarList->child[2];
				}//ParamDec
				TypePtr tempType = Specifier(VarList->child[0]->child[0]);
				FieldList tempField = VarDec(VarList->child[0]->child[1], tempType, from);
				if (lookupSymbol(tempField->name, 0) != NULL) {
					semanticError++;
					//printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDef->lineno, tempField->name);
				}
				else {
					tempField->is_in_params = 1;
					insertSymbol(tempField);
				}
				typ->u.function_.paramNum++;
				tempField->tail = typ->u.function_.params;
				typ->u.function_.params = tempField;

				Operand paramOp = (Operand)malloc(sizeof(Operand_t));
				memset(paramOp, 0, sizeof(Operand_t));
				paramOp->kind = VARIABLE_OP;
				strcpy(paramOp->u.value, typ->u.function_.params->name);
				//paramOp->u.value = typ->u.function_.params->name;
				InterCode paramIR = (InterCode)malloc(sizeof(InterCode_t));
				memset(paramIR, 0, sizeof(InterCode_t));
				paramIR->kind = PARAM_IR;
				paramIR->u.singleOP.op = paramOp;
				insertCode(paramIR);

			}

			field->type = typ;
			if (lookupSymbol(field->name, 1) != NULL) {
				semanticError++;
				printf("Error type 4 at Line %d: Redefined function \"%s\".\n", ExtDef->lineno, field->name);
			}
			else {
				field->is_in_params = 1;
				insertSymbol(field);
			}

			//CompSt->LC DefList StmtList RC
			CompSt(ExtDef->child[2], basictype);
		}
		else {//Specifier SIMI
			//do nothing
		}

		if (ExtDefList->child[1] == NULL)//ExtDef
			return;
		ExtDefList = ExtDefList->child[1];
	}
}

TypePtr Specifier(Node *root) {
	TypePtr spe = (TypePtr)malloc(sizeof(Type_t));
	if (strcmp(root->child[0]->name, "TYPE") == 0) {//TYPE
		spe->kind = BASIC;
		if (strcmp(root->child[0]->text, "int") == 0) {
			spe->u.basic_ = INT_TYPE;
		}
		else spe->u.basic_ = FLOAT_TYPE;
		return spe;
	}
	else {
		spe->kind = STRUCTURE;
		if (root->child[0]->childsum == 2) {//STRUCT Tag
			char *s = root->child[0]->child[1]->child[0]->text;
			FieldList field = lookupSymbol(s, 0);
			if (field == NULL) {
				semanticError++;
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", root->lineno, s);
				spe->u.structure_ = NULL;
				return spe;
			}
			else if (field->type != NULL) {
				return field->type;
			}
			spe->u.structure_ = NULL;
			return spe;
		}
		else {//STRUCT OptTag LC DefList RC
			int from = 3;
			Node* DefList = root->child[0]->child[3];
			spe->u.structure_ = NULL;
			//DefList in STRUCT is different from that outside
			while (DefList != NULL) {//Def DefList
				Node *Def = DefList->child[0];
				TypePtr basictype = Specifier(Def->child[0]);

				Node* DecList = Def->child[1];
				while (DecList->childsum == 3) {//Dec COMMA DecList
					FieldList field = VarDec(DecList->child[0]->child[0], basictype, from);
					if (DecList->child[0]->childsum != 1) {
						semanticError++;
						printf("Error type 15 at Line %d: Variable %s in struct is initialized.\n", Def->lineno, field->name);
					}
					FieldList temp = spe->u.structure_;
					while (temp != NULL) {
						if (strcmp(temp->name, field->name) == 0) {
							semanticError++;
							printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, field->name);
							break;
						}
						temp = temp->tail;
					}
					if (temp == NULL) {
						if (lookupSymbol(field->name, 0) != NULL) {
							semanticError++;
							printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, field->name);
						}
						else {
							insertSymbol(field);
							field->tail = spe->u.structure_;
							spe->u.structure_ = field;
						}
					}
					DecList = DecList->child[2];
				}
				FieldList field = VarDec(DecList->child[0]->child[0], basictype, from);
				if (DecList->child[0]->childsum != 1) {
					semanticError++;
					printf("Error type 15 at Line %d: Variable \"%s\" in struct is initialized.\n", Def->lineno, field->name);
				}
				FieldList temp = spe->u.structure_;
				while (temp != NULL) {
					if (strcmp(temp->name, field->name) == 0) {
						semanticError++;
						printf("Error type 15 at Line %d: Redefined field \"%s\".\n", Def->lineno, field->name);
						break;
					}
					temp = temp->tail;
				}
				if (temp == NULL) {
					if (lookupSymbol(field->name, 0) != NULL) {
						semanticError++;
						printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Def->lineno, field->name);
					}
					else {
						insertSymbol(field);
						field->tail = spe->u.structure_;
						spe->u.structure_ = field;
					}
				}
				DefList = DefList->child[1];
			}
			if (root->child[0]->child[1] != NULL) {//OptTag exist
				FieldList field = (FieldList)malloc(sizeof(FieldList_t));
				memset(field, 0, sizeof(FieldList_t));
				field->type = spe;
				char *s = root->child[0]->child[1]->child[0]->text;//get the name of OptTag
				field->name = s;
				if (lookupSymbol(field->name, 0) != NULL) {
					semanticError++;
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", root->lineno, field->name);
				}
				else {
					insertSymbol(field);
				}
			}
			return spe;
		}
	}
}

void CompSt(Node *root, TypePtr funcType) {
	Node *CompSt = root;
	DefList(CompSt->child[1]);
	Node *StmtList = CompSt->child[2];
	while (StmtList != NULL) {
		Node *Stmt_ = StmtList->child[0];
		Stmt(Stmt_, funcType);
		StmtList = StmtList->child[1];
	}
}

/*Local Definitions*/
void DefList(Node* root) {
	if (root == NULL || root->childsum <= 1) {
		return;
	}
	FieldList f_def = Def(root->child[0], 1);
	DefList(root->child[1]);
}

FieldList Def(Node* root, int from) {
	if (root == NULL || root->childsum < 3) {
		printf("Def为空或者Def->childsum < 3");
		return NULL;
	}
	TypePtr type = Specifier(root->child[0]);
	//Node* declist = root->child[1];
	FieldList f = DecList(root->child[1], type, from);
	return f;
}

FieldList DecList(Node *root, TypePtr type, int from) {
	if (root == NULL || root->childsum < 1) {
		printf("DecList为空或者DecList->childsum < 1");
		return NULL;
	}
	FieldList f = Dec(root->child[0], type, from);
	if (root->childsum == 3) {
		f = DecList(root->child[2], type, from);
	}
	return f;
}

FieldList Dec(Node *root, TypePtr type, int from) {
	if (root == NULL || root->childsum < 1) {
		printf("Dec为空或者Dec->childsum < 1");
		return NULL;
	}
	FieldList f = VarDec(root->child[0], type, from);
	if (f == NULL) {
		return NULL;
	}

	if (f->type->kind == ARRAY && from == 1) {
		////DEC t_no sizeof(left)
		Operand op = (Operand)malloc(sizeof(Operand_t));
		memset(op, 0, sizeof(Operand_t));
		//op->kind = TEMP_VAR_OP;
		//op->u.tvar_no = TempVarNum++;
		op->kind = VARIABLE_OP;
		strcpy(op->u.value, f->name);

		InterCode decIR = (InterCode)malloc(sizeof(InterCode_t));
		memset(decIR, 0, sizeof(InterCode_t));
		decIR->kind = DEC_IR;
		decIR->u.decOP.op = op;
		decIR->u.decOP.size = getSize(f->type, 0);
		insertCode(decIR);

	}
	if (root->childsum == 3) {
		Operand place = (Operand)malloc(sizeof(Operand_t));
		TypePtr typ;
		memset(place, 0, sizeof(Operand_t));
		if (strcmp(root->child[2]->child[0]->name, "INT") == 0) {
			typ = Exp(root->child[2], NULL);
			place->kind = CONSTANT_OP;
			sprintf(place->u.value, "%s", root->child[2]->child[0]->text);
		}
		else {
			place->kind = TEMP_VAR_OP;
			typ = Exp(root->child[2], place);
		}

		if (place->kind != VARIABLE_OP || strcpy(place->u.value, f->name) != 0) {
			Operand leftOp = (Operand)malloc(sizeof(Operand_t));
			memset(leftOp, 0, sizeof(Operand_t));
			leftOp->kind = VARIABLE_OP;
			strcpy(leftOp->u.value, f->name);

			InterCode addrIR = (InterCode)malloc(sizeof(InterCode_t));
			memset(addrIR, 0, sizeof(InterCode_t));
			addrIR->kind = ASSIGN_IR;
			addrIR->u.doubleOP.left = leftOp;
			addrIR->u.doubleOP.right = place;
			insertCode(addrIR);
		}
	}
	return f;
}

int getSize(TypePtr typ, int t) {
	if (typ == NULL) {
		return 0;
	}
	if (t == 0) {
		if (typ->kind == BASIC) {
			return 4;
		}
		int size = 4;
		//TypePtr typ = f->type;
		while (typ->u.array_.elem->kind != BASIC) {
			size = typ->u.array_.size * size;
			typ = typ->u.array_.elem;
		}
		size = typ->u.array_.size * size;
		if (BASIC == typ->u.array_.elem->kind) {
			return size;
		}
	}
	else if (t == 2) {
		if (typ->u.array_.elem->kind == BASIC) {
			return 4;
		}
		int size = 4;
		while (typ->u.array_.elem->kind != BASIC) {
			size = typ->u.array_.size * size;
			if (typ->u.array_.elem->u.array_.elem->kind == BASIC) {
				return size;
			}
			typ = typ->u.array_.elem;
		}
		size = typ->u.array_.size * size;
		return size;
	}
	return 1;
}

FieldList VarDec(Node *root, TypePtr type, int from) {
	if (root == NULL || root->childsum < 1) {
		printf("VarDec为空或者VarDec->childsum < 1");
		return NULL;
	}
	//1 BASIC, 2 ARRAY, 3 STRUCTURE, 4 PARAM
	if (strcmp(root->child[0]->name, "ID") == 0) {	// VarDec -> ID
		FieldList field = (FieldList)malloc(sizeof(FieldList_t));
		memset(field, 0, sizeof(FieldList_t));
		field->name = root->child[0]->text;
		field->type = type;
		if (lookupSymbol(field->name, 0) != NULL) {
			semanticError++;
			//printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", root->lineno, field->name);
			return NULL;
		}
		else {
			if (from == 4) {
				field->is_in_params = 1;
			}
			insertSymbol(field);
			return field;
		}
	}
	else if (root->childsum == 4) {	// VarDec -> VarDec LB INT RB
		FieldList field = VarDec(root->child[0], type, from);
		if (field == NULL) {
			return NULL;
		}
		TypePtr basictype = field->type;
		TypePtr arrayType = (TypePtr)malloc(sizeof(Type_t));
		memset(arrayType, 0, sizeof(Type_t));

		arrayType->kind = ARRAY;
		arrayType->u.array_.size = atoi(root->child[2]->text);
		arrayType->u.array_.elem = basictype;
		field->type = arrayType;
		return field;
	}
}

void Stmt(Node *root, TypePtr funcType) {
	Node *Stmt_ = root;
	if (root == NULL || funcType == NULL) {
		return;
	}
	if (strcmp(Stmt_->child[0]->name, "RETURN") == 0) {//RETURN Exp SEMI
		Operand returnOp = (Operand)malloc(sizeof(Operand_t));
		memset(returnOp, 0, sizeof(Operand_t));
		//returnOp->kind = TEMP_VAR_OP;
		if(strcmp(root->child[1]->child[0]->name, "INT") ==0){
			returnOp->kind = CONSTANT_OP;
			strcpy(returnOp->u.value, root->child[1]->child[0]->text);
		}
		else{
			TypePtr returnType = Exp(Stmt_->child[1], returnOp);
		}
		InterCode returnIR = (InterCode)malloc(sizeof(InterCode_t));
		memset(returnIR, 0, sizeof(InterCode_t));
		returnIR->kind = RETURN_IR;
		returnIR->u.singleOP.op = returnOp;
		insertCode(returnIR);
	}
	else if (strcmp(Stmt_->child[0]->name, "Exp") == 0) {//Exp
		Exp(Stmt_->child[0], NULL);
	}
	else if (strcmp(Stmt_->child[0]->name, "CompSt") == 0) {//CompSt
		CompSt(Stmt_->child[0], funcType);
	}
	else if (strcmp(Stmt_->child[0]->name, "WHILE") == 0) {//WHILE LP Exp RP Stmt
		//LABEL label1
		Operand labelOp1 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp1, 0, sizeof(Operand_t));
		labelOp1->kind = LABEL_OP;
		labelOp1->u.label_no = LabelNum++;
		InterCode labelIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR1, 0, sizeof(InterCode_t));
		labelIR1->kind = LABEL_IR;
		labelIR1->u.singleOP.op = labelOp1;
		insertCode(labelIR1);

		Operand labelOp2 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp2, 0, sizeof(Operand_t));
		labelOp2->kind = LABEL_OP;
		labelOp2->u.label_no = LabelNum++;
		Operand labelOp3 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp3, 0, sizeof(Operand_t));
		labelOp3->kind = LABEL_OP;
		labelOp3->u.label_no = LabelNum++;
		
		//CODE code1
		TypePtr typ1 = Cond(root->child[2], labelOp2, labelOp3);
		if (!((typ1->kind == BASIC) && (typ1->u.basic_ == INT_TYPE))) {
			semanticError++;
			printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt_->lineno);
		}

		//LABEL label2
		InterCode labelIR2 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR2, 0, sizeof(InterCode_t));
		labelIR2->kind = LABEL_IR;
		labelIR2->u.singleOP.op = labelOp2;
		insertCode(labelIR2);

		//CODE codeIR2
		Stmt(Stmt_->child[4], funcType);

		//GOTO label1
		InterCode gotLabelIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(gotLabelIR1, 0, sizeof(InterCode_t));
		gotLabelIR1->kind = GOTO_IR;
		gotLabelIR1->u.singleOP.op = labelOp1;
		insertCode(gotLabelIR1);

		//LABEL label3
		InterCode labelIR3 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR3, 0, sizeof(InterCode_t));
		labelIR3->kind = LABEL_IR;
		labelIR3->u.singleOP.op = labelOp3;
		insertCode(labelIR3);
	}
	else if (Stmt_->childsum < 6) {//IF LP Exp RP Stmt
		Operand labelOp1 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp1, 0, sizeof(Operand_t));
		labelOp1->kind = LABEL_OP;
		labelOp1->u.label_no = LabelNum++;

		Operand labelOp2 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp2, 0, sizeof(Operand_t));
		labelOp2->kind = LABEL_OP;
		labelOp2->u.label_no = LabelNum++;

		//CODE code1
		TypePtr typ = Cond(root->child[2], labelOp1, labelOp2);
		if (typ != NULL) {
			if (!((typ->kind == BASIC) && (typ->u.basic_ == INT_TYPE))) {
				semanticError++;
				printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt_->lineno);
			}
		}

		//LABEL label1
		InterCode labelIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR1, 0, sizeof(InterCode_t));
		labelIR1->kind = LABEL_IR;
		labelIR1->u.singleOP.op = labelOp1;
		insertCode(labelIR1);

		//CODE codeIR2
		Stmt(root->child[4], funcType);

		//LABEL label2
		InterCode labelIR2 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR2, 0, sizeof(InterCode_t));
		labelIR2->kind = LABEL_IR;
		labelIR2->u.singleOP.op = labelOp2;
		insertCode(labelIR2);
	}
	else {//IF LP Exp RP Stmt ELSE Stmt

		Operand labelOp1 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp1, 0, sizeof(Operand_t));
		labelOp1->kind = LABEL_OP;
		labelOp1->u.label_no = LabelNum++;

		Operand labelOp2 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp2, 0, sizeof(Operand_t));
		labelOp2->kind = LABEL_OP;
		labelOp2->u.label_no = LabelNum++;

		//CODE code1
		TypePtr typ = Cond(root->child[2], labelOp1, labelOp2);
		if (!((typ->kind == BASIC) && (typ->u.basic_ == INT_TYPE))) {
			semanticError++;
			printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n", Stmt_->lineno);
		}

		//LABEL label1
		InterCode labelIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR1, 0, sizeof(InterCode_t));
		labelIR1->kind = LABEL_IR;
		labelIR1->u.singleOP.op = labelOp1;
		insertCode(labelIR1);

		//CODE codeIR2
		Stmt(Stmt_->child[4], funcType);

		//GOTO label3
		Operand labelOp3 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp3, 0, sizeof(Operand_t));
		labelOp3->kind = LABEL_OP;
		labelOp3->u.label_no = LabelNum++;
		InterCode gotolabel3IR = (InterCode)malloc(sizeof(InterCode_t));
		memset(gotolabel3IR, 0, sizeof(InterCode_t));
		gotolabel3IR->kind = GOTO_IR;
		gotolabel3IR->u.singleOP.op = labelOp3;
		insertCode(gotolabel3IR);

		//LABEL label2
		InterCode labelIR2 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR2, 0, sizeof(InterCode_t));
		labelIR2->kind = LABEL_IR;
		labelIR2->u.singleOP.op = labelOp2;
		insertCode(labelIR2);

		//CODE ifgotoIR
		Stmt(Stmt_->child[6], funcType);

		//LABEL label3
		InterCode labelIR3 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR3, 0, sizeof(InterCode_t));
		labelIR3->kind = LABEL_IR;
		labelIR3->u.singleOP.op = labelOp3;
		insertCode(labelIR3);
	}
}

TypePtr Exp(Node* root, Operand place) {
	if (root == NULL) {
		return NULL;
	}
	else if ((strcmp(root->child[0]->name, "ID") == 0) && (root->childsum == 1)) {//ID
		FieldList field = lookupSymbol(root->child[0]->text, 0);
		if (field == NULL) {
			semanticError++;
			printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", root->lineno, root->child[0]->text);
			return NULL;
		}
		if (place != NULL) {
			place->kind = VARIABLE_OP;
			strcpy(place->u.value, root->child[0]->text);
		}
		return field->type;
	}
	else if (strcmp(root->child[0]->name, "INT") == 0) {//INT
		TypePtr typ = (TypePtr)malloc(sizeof(Type_t));
		memset(typ, 0, sizeof(Type_t));
		typ->kind = BASIC;
		typ->u.basic_ = INT_TYPE;
		if (place != NULL) {
			InterCode intIR = (InterCode)malloc(sizeof(InterCode_t));
			memset(intIR, 0, sizeof(intIR));
			intIR->kind = ASSIGN_IR;
			
			Operand intOp = (Operand)malloc(sizeof(Operand_t));
			memset(intOp, 0, sizeof(Operand_t));
			intOp->kind = CONSTANT_OP;
			strcpy(intOp->u.value, root->child[0]->text);

			intIR->u.doubleOP.right = intOp;
			place->kind = TEMP_VAR_OP;
			place->u.tvar_no = TempVarNum++;

			intIR->u.doubleOP.left = place;
			insertCode(intIR);
		}

		return typ;
	}
	else if (strcmp(root->child[0]->name, "FLOAT") == 0) {//FLOAT
		TypePtr typ = (TypePtr)malloc(sizeof(Type_t));
		typ->kind = BASIC;
		typ->u.basic_ = FLOAT_TYPE;

		return typ;
	}
	else if ((strcmp(root->child[0]->name, "LP") == 0) || (strcmp(root->child[0]->name, "MINUS") == 0) || (strcmp(root->child[0]->name, "NOT") == 0)) {
		if (strcmp(root->child[0]->name, "LP") == 0) {
			return Exp(root->child[1], place);
		}
		else if (strcmp(root->child[0]->name, "MINUS") == 0) { // Exp: Minus Exp
			if(strcmp(root->child[1]->child[0]->name, "INT")==0 && place!=NULL){
				int n = atoi(root->child[1]->child[0]->text);
				if(n >= 0){
					TypePtr typ = Exp(root->child[1], NULL);
					place->kind = CONSTANT_OP;
					sprintf(place->u.value,"-%s", root->child[1]->child[0]->text);
					return typ;
				}
			}
			Operand expOp = (Operand)malloc(sizeof(Operand_t));
			memset(expOp, 0, sizeof(Operand_t));
			TypePtr type = Exp(root->child[1], expOp);
			if (type == NULL) {
				return NULL;
			}

			// place := #0 - Exp
			if (place != NULL) {
				Operand zeroOp = (Operand)malloc(sizeof(Operand_t));
				memset(zeroOp, 0, sizeof(Operand_t));
				zeroOp->kind = CONSTANT_OP;
				strcpy(zeroOp->u.value, "0");
				InterCode minusIR = (InterCode)malloc(sizeof(InterCode_t));
				memset(minusIR, 0, sizeof(InterCode_t));
				minusIR->kind = MINUS_IR;
				minusIR->u.tripleOP.result = place;
				minusIR->u.tripleOP.op1 = zeroOp;
				minusIR->u.tripleOP.op2 = expOp;
				place->kind = TEMP_VAR_OP;
				place->u.tvar_no = TempVarNum++;
				insertCode(minusIR);
			}
			return type;
		}
		else if (strcmp(root->child[0]->name, "NOT") == 0) {
			//CODE code0
			InterCode code0 = (InterCode)malloc(sizeof(InterCode_t));
			memset(code0, 0, sizeof(InterCode_t));
			code0->kind = ASSIGN_IR;
			code0->u.doubleOP.left = place;
			Operand zeroOp = (Operand)malloc(sizeof(Operand_t));
			memset(zeroOp, 0, sizeof(Operand_t));
			zeroOp->kind = CONSTANT_OP;
			strcpy(zeroOp->u.value, "0");
			code0->u.doubleOP.right = zeroOp;
			if (place != NULL) {
				place->kind = TEMP_VAR_OP;
				insertCode(code0);	//code0
			}


			Operand labelOp1 = (Operand)malloc(sizeof(Operand_t));
			memset(labelOp1, 0, sizeof(Operand_t));
			labelOp1->kind = LABEL_OP;
			labelOp1->u.label_no = LabelNum++;
			Operand labelOp2 = (Operand)malloc(sizeof(Operand_t));
			memset(labelOp2, 0, sizeof(Operand_t));
			labelOp2->kind = LABEL_OP;
			labelOp2->u.label_no = LabelNum++;

			//CODE code1
			TypePtr typ = Cond(root, labelOp1, labelOp2);

			//LABEL label1
			InterCode labelIR1 = (InterCode)malloc(sizeof(InterCode_t));
			memset(labelIR1, 0, sizeof(InterCode_t));
			labelIR1->kind = LABEL_IR;
			labelIR1->u.singleOP.op = labelOp1;
			insertCode(labelIR1);

			//CODE codeIR2
			//place := #1
			if (place != NULL) {
				Operand oneOp = (Operand)malloc(sizeof(Operand_t));
				memset(oneOp, 0, sizeof(Operand_t));
				oneOp->kind = CONSTANT_OP;
				strcpy(oneOp->u.value, "1");
				InterCode codeIR2 = (InterCode)malloc(sizeof(InterCode_t));
				memset(codeIR2, 0, sizeof(InterCode_t));
				codeIR2->kind = ASSIGN_IR;
				codeIR2->u.doubleOP.left = place;
				codeIR2->u.doubleOP.right = oneOp;
				insertCode(codeIR2);
			}
	
			//LABEL label3
			InterCode labelIR3 = (InterCode)malloc(sizeof(InterCode_t));
			memset(labelIR3, 0, sizeof(InterCode_t));
			labelIR3->kind = LABEL_IR;
			labelIR3->u.singleOP.op = labelOp2;
			insertCode(labelIR3);
			return typ;
		}
		return Exp(root->child[1], place);
	}
	else if ((strcmp(root->child[1]->name, "PLUS") == 0) || (strcmp(root->child[1]->name, "MINUS") == 0) || (strcmp(root->child[1]->name, "STAR") == 0) || (strcmp(root->child[1]->name, "DIV") == 0)) {
		Operand tempOp1 = (Operand)malloc(sizeof(Operand_t));
		memset(tempOp1, 0, sizeof(Operand_t));
		TypePtr typ1;
		int flag1 = -1;
		if(strcmp(root->child[0]->child[0]->name, "INT")==0){
			flag1 = 1;
			typ1 = Exp(root->child[0], NULL);
			tempOp1->kind = CONSTANT_OP;
			sprintf(tempOp1->u.value,"%s", root->child[0]->child[0]->text);
		}
		else{
			tempOp1->kind = TEMP_VAR_OP;
			typ1 = Exp(root->child[0], tempOp1);
		}

		//Exp: Exp PLUS Exp
		Operand tempOp2 = (Operand)malloc(sizeof(Operand_t));
		memset(tempOp2, 0, sizeof(Operand_t));
		TypePtr typ2;
		int flag2 = -1;
		if(strcmp(root->child[2]->child[0]->name, "INT")==0){
			flag2 = 1;
			typ2 = Exp(root->child[2], NULL);
			tempOp2->kind = CONSTANT_OP;
			sprintf(tempOp2->u.value,"%s", root->child[2]->child[0]->text);
		}
		else{
			tempOp2->kind = TEMP_VAR_OP;
			typ2 = Exp(root->child[2], tempOp2);
		}
		if (TypeEqual(typ1, typ2) == 0) {
			if ((typ1 != NULL) && (typ2 != NULL)) {
				semanticError++;
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->lineno);
			}
			printf("Error type 7 at Line %d: Type mismatched for operands.\n", root->lineno);
			return NULL;
		}
		else {
			if (place == NULL) {
				return NULL;
			}
			InterCode codeIR = (InterCode)malloc(sizeof(InterCode_t));
			memset(codeIR, 0, sizeof(InterCode_t));
			if (strcmp(root->child[1]->name, "PLUS") == 0) {
				codeIR->kind = PLUS_IR;
			}
			else if (strcmp(root->child[1]->name, "MINUS") == 0) {
				codeIR->kind = MINUS_IR;
			}
			else if (strcmp(root->child[1]->name, "STAR") == 0) {
				codeIR->kind = STAR_IR;
			}
			else if (strcmp(root->child[1]->name, "DIV") == 0) {
				codeIR->kind = DIV_IR;
			}

			if (place != NULL) {
				place->kind = TEMP_VAR_OP;
				place->u.tvar_no = TempVarNum++;
				if (flag1 == 1 && flag2 == 1) {
					int num1 = atoi(root->child[0]->child[0]->text);
					int num2 = atoi(root->child[2]->child[0]->text);
					int num = 0;
					switch (codeIR->kind) {
					case PLUS_IR:
						num = num1 + num2;
						break;
					case MINUS_IR:
						num = num1 - num2;
						break;
					case STAR_IR:
						num = num1 * num2;
						break;
					case DIV_IR:
						num = num1 / num2;
						break;
					default:
						break;
					}
					memset(codeIR, 0, sizeof(InterCode_t));
					free(tempOp1);
					tempOp1 = NULL;
					memset(tempOp2, 0, sizeof(Operand_t));
					tempOp2->kind = CONSTANT_OP;
					sprintf(tempOp2->u.value, "%d", num);
					codeIR->kind = ASSIGN_IR;
					codeIR->u.doubleOP.left = place;
					codeIR->u.doubleOP.right = tempOp2;
				}
				else {
					codeIR->u.tripleOP.op1 = tempOp1;
					codeIR->u.tripleOP.op2 = tempOp2;
					codeIR->u.tripleOP.result = place;
				}
				insertCode(codeIR);
			}
			return typ1;
		}
	}
	else if ((strcmp(root->child[1]->name, "AND") == 0) || (strcmp(root->child[1]->name, "OR") == 0) || (strcmp(root->child[1]->name, "RELOP") == 0)) {
		Operand labelOp1 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp1, 0, sizeof(Operand_t));
		labelOp1->kind = LABEL_OP;
		labelOp1->u.label_no = LabelNum++;

		Operand labelOp2 = (Operand)malloc(sizeof(Operand_t));
		memset(labelOp2, 0, sizeof(Operand_t));
		labelOp2->kind = LABEL_OP;
		labelOp2->u.label_no = LabelNum++;

		if (place != NULL) {
			//CODE code0
			InterCode codeIR0 = (InterCode)malloc(sizeof(InterCode_t));
			memset(codeIR0, 0, sizeof(InterCode_t));
			codeIR0->kind = ASSIGN_IR;
			codeIR0->u.doubleOP.left = place;
			Operand zeroOp = (Operand)malloc(sizeof(Operand_t));
			zeroOp->kind = CONSTANT_OP;
			strcpy(zeroOp->u.value, "0");
			codeIR0->u.doubleOP.right = zeroOp;
			insertCode(codeIR0);
		}

		//CODE code1
		TypePtr typ = Cond(root, labelOp1, labelOp2);

		//LABEL label1
		InterCode labelIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR1, 0, sizeof(InterCode_t));
		labelIR1->kind = LABEL_IR;
		labelIR1->u.singleOP.op = labelOp1;
		insertCode(labelIR1);

		//place := #1
		if (place != NULL) {
			InterCode codeIR2 = (InterCode)malloc(sizeof(InterCode_t));
			memset(codeIR2, 0, sizeof(InterCode_t));
			codeIR2->kind = ASSIGN_IR;
			codeIR2->u.doubleOP.left = place;
			Operand oneOp = (Operand)malloc(sizeof(Operand_t));
			memset(oneOp, 0, sizeof(Operand_t));
			oneOp->kind = CONSTANT_OP;
			strcpy(oneOp->u.value, "1");
			codeIR2->u.doubleOP.right = oneOp;
			insertCode(codeIR2);
		}

		//LABEL label2
		InterCode labelIR2 = (InterCode)malloc(sizeof(InterCode_t));
		memset(labelIR2, 0, sizeof(InterCode_t));
		labelIR2->kind = LABEL_IR;
		labelIR2->u.singleOP.op = labelOp2;
		insertCode(labelIR2);

		return typ;
	}
	else if (strcmp(root->child[1]->name, "ASSIGNOP") == 0) {
		InterCode assignIR1 = (InterCode)malloc(sizeof(InterCode_t));
		memset(assignIR1, 0, sizeof(InterCode_t));
		assignIR1->kind = ASSIGN_IR;
		
		if (root->child[0]->childsum == 1) {
			if (!(strcmp(root->child[0]->child[0]->name, "ID") == 0)) {
				semanticError++;
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
				return NULL;
			}
		}
		else if (root->child[0]->childsum == 3) {
			if (!((strcmp(root->child[0]->child[0]->name, "Exp") == 0) && (strcmp(root->child[0]->child[1]->name, "DOT") == 0) && (strcmp(root->child[0]->child[2]->name, "ID") == 0))) {
				semanticError++;
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
				return NULL;
			}
		}
		else if (root->child[0]->childsum == 4) {
			if (!((strcmp(root->child[0]->child[0]->name, "Exp") == 0) && (strcmp(root->child[0]->child[1]->name, "LB") == 0) && (strcmp(root->child[0]->child[2]->name, "Exp") == 0) && (strcmp(root->child[0]->child[3]->name, "RB") == 0))) {
				semanticError++;
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", root->lineno);
				return NULL;
			}
		}

		Operand leftOp = (Operand)malloc(sizeof(Operand_t));
		memset(leftOp, 0, sizeof(Operand_t));
		leftOp->kind = TEMP_VAR_OP;
		TypePtr typ1 = Exp(root->child[0], leftOp);

		Operand rightOp = (Operand)malloc(sizeof(Operand_t));
		memset(rightOp, 0, sizeof(Operand_t));
		TypePtr typ2;
		if(strcmp(root->child[2]->child[0]->name, "INT")==0){
				typ2 = Exp(root->child[2], NULL);
				rightOp->kind = CONSTANT_OP;
				sprintf(rightOp->u.value,"%s", root->child[2]->child[0]->text);
		}
		else{
			rightOp->kind = TEMP_VAR_OP;
			typ2 = Exp(root->child[2], rightOp);
		}
		if (TypeEqual(typ1, typ2) == 0) {
			if ((typ1 != NULL) && (typ2 != NULL)) {
				semanticError++;
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->lineno);
			}
			printf("Error type 5 at Line %d: Type mismatched for assignment.\n", root->lineno);
			return NULL;
		}

		assignIR1->u.doubleOP.left = leftOp;
		assignIR1->u.doubleOP.right = rightOp;
		insertCode(assignIR1);
		if (place != NULL) {
			InterCode assignIR2 = malloc(sizeof(InterCode_t));
			memset(assignIR2, 0, sizeof(InterCode_t));
			assignIR2->kind = ASSIGN_IR;
			assignIR2->u.doubleOP.left = place;
			assignIR2->u.doubleOP.right = rightOp;
			insertCode(assignIR2);
		}
		return typ1;
	}
	else if (strcmp(root->child[0]->name, "ID") == 0) {//ID LP RP
		FieldList fie = lookupSymbol(root->child[0]->text, 1);
		if (fie == NULL) {
			FieldList fie2 = lookupSymbol(root->child[0]->text, 0);
			if (fie2 != NULL) {
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", root->lineno, root->child[0]->text);
			}
			else {
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", root->lineno, root->child[0]->text);
			}
			semanticError++;
			return NULL;
		}
		TypePtr definedType = fie->type;
		if (strcmp(root->child[2]->name, "RP") == 0) {
			if (strcmp("read", root->child[0]->text) == 0) {
				if (place != NULL) {
					InterCode readIR = (InterCode)malloc(sizeof(InterCode_t));
					memset(readIR, 0, sizeof(InterCode_t));
					readIR->kind = READ_IR;
					place->kind = TEMP_VAR_OP;
					place->u.tvar_no = TempVarNum++;
					readIR->u.singleOP.op = place;
					insertCode(readIR);
				}
			}
			else {
				Operand funcOp = (Operand)malloc(sizeof(Operand_t));
				memset(funcOp, 0, sizeof(Operand_t));
				funcOp->kind = FUNCTION_OP;
				strcpy(funcOp->u.value, root->child[0]->text);
				InterCode callIR = (InterCode)malloc(sizeof(InterCode_t));
				memset(callIR, 0, sizeof(InterCode_t));


				if (place == NULL) {
					place = (Operand)malloc(sizeof(Operand_t));
				}
				callIR->kind = CALL_IR;
				callIR->u.doubleOP.left = place;
				callIR->u.doubleOP.right = funcOp;
				place->kind = TEMP_VAR_OP;
				place->u.tvar_no = TempVarNum++;
				insertCode(callIR);
			}
		}
		else if (strcmp(root->child[2]->name, "RP") != 0) {//root: ID LP Args RP
			if (strcmp("write", root->child[0]->text) == 0) {
				Operand orgOp = (Operand)malloc(sizeof(Operand_t));
				memset(orgOp, 0, sizeof(Operand_t));
				TypePtr type;
				Node *exp = root->child[2]->child[0];
				if (strcmp(exp->child[0]->name, "INT") == 0) {
					type = Exp(exp, NULL);
					orgOp->kind = CONSTANT_OP;
					sprintf(orgOp->u.value, "%s", exp->child[0]->text);
				}
				else {
					orgOp->kind = TEMP_VAR_OP;
					type = Exp(exp, orgOp);
				}
				InterCode writeIR = (InterCode)malloc(sizeof(InterCode_t));
				memset(writeIR, 0, sizeof(InterCode_t));
				writeIR->kind = WRITE_IR;
				writeIR->u.singleOP.op = orgOp;
				if (writeIR != NULL) {
					insertCode(writeIR);
				}
			}
			else {
				Node* argsNode = root->child[2];
				//FieldList tempField = (FieldList)malloc(sizeof(FieldList_t));
				//memset(tempField, 0, sizeof(FieldList_t));
				Operand* argsList = (Operand*)malloc(sizeof(Operand) * 30);
				
				int i = 0;
				Operand argOp = (Operand)malloc(sizeof(Operand_t));
				memset(argOp, 0, sizeof(Operand_t));
				TypePtr type = NULL;
				if (strcmp(argsNode->child[0]->child[0]->name, "INT") == 0) {
					type = Exp(argsNode->child[0], NULL);
					argOp->kind = CONSTANT_OP;
					sprintf(argOp->u.value, "%s", argsNode->child[0]->child[0]->text);
				}
				else {
					type = Exp(argsNode->child[0], argOp);
				}
				if (type->kind == ARRAY && argOp->kind == VARIABLE_OP) {
					char s[32];
					sprintf(s, "&%s", argOp->u.value);
					strcpy(argOp->u.value, s);
				}
				argsList[i] = argOp;
				i++;				
				while (argsNode->childsum == 3) {//Args: Exp COMMA Args
					argsNode = argsNode->child[2];
					Operand argOp = (Operand)malloc(sizeof(Operand_t));
					memset(argOp, 0, sizeof(argOp));
					//Node *expNode = argsNode->child[0];
					TypePtr type1 = NULL;// = Exp(argsNode->child[0], argOp);

					if (strcmp(argsNode->child[0]->child[0]->name, "INT") == 0) {
						type1 = Exp(argsNode->child[0], NULL);
						argOp->kind = CONSTANT_OP;
						sprintf(argOp->u.value, "%s", argsNode->child[0]->child[0]->text);
					}
					else {
						argOp->kind = TEMP_VAR_OP;
						type1 = Exp(argsNode->child[0], argOp);
					}
					if (type1->kind == ARRAY && argOp->kind == VARIABLE_OP) {
						char s[32];
						sprintf(s, "&%s", argOp->u.value);
						strcpy(argOp->u.value, s);
					}
					argsList[i++] = argOp;				
				}//Exp
				
				while (i>0) {//Exp COMMA Args
					InterCode argIR = (InterCode)malloc(sizeof(InterCode_t));
					memset(argIR, 0, sizeof(InterCode_t));
					argIR->kind = ARG_IR;
					argIR->u.singleOP.op = argsList[--i];
					insertCode(argIR);
				}//Exp
				free(argsList);
				argsList = NULL;
				Operand funcOp = (Operand)malloc(sizeof(Operand_t));
				memset(funcOp, 0, sizeof(Operand_t));
				funcOp->kind = FUNCTION_OP;
				strcpy(funcOp->u.value, root->child[0]->text);

				InterCode callIR = (InterCode)malloc(sizeof(InterCode_t));
				memset(callIR, 0, sizeof(Operand_t));
				callIR->kind = CALL_IR;
				if (place == NULL) {
					place = (Operand)malloc(sizeof(Operand_t));
					memset(place, 0, sizeof(Operand_t));
				}
				place->kind = TEMP_VAR_OP;
				place->u.tvar_no = TempVarNum++;
				callIR->u.doubleOP.left = place;
				callIR->u.doubleOP.right = funcOp;
				insertCode(callIR);
			}
		}
		return definedType->u.function_.funcType;
	}
	else if (strcmp(root->child[1]->name, "DOT") == 0) {//Exp DOT ID
		TypePtr typ1 = Exp(root->child[0], place);
		if (typ1->kind != STRUCTURE) {
			Node* temp = root->child[0];
			char *s;
			switch (temp->childsum) {
			case 1: {
				if (strcmp(temp->child[0]->name, "ID") == 0)
					s = temp->child[0]->text;
			}break;
			case 3: {
				if (strcmp(temp->child[2]->name, "ID") == 0)
					s = temp->child[0]->text;
			}break;
			case 4: {
				if (strcmp(temp->child[0]->name, "Exp") == 0)
					if (strcmp(temp->child[0]->child[0]->name, "ID") == 0)
						s = temp->child[0]->child[0]->text;
			}break;
			default:s = "error"; break;
			}
			if (lookupSymbol(s, 0) != NULL) {
				semanticError++;
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", root->lineno);
			}
			return NULL;
		}
		char *s = root->child[2]->text;
		FieldList temp = typ1->u.structure_;
		while (temp != NULL) {
			if (strcmp(temp->name, s) == 0) {
				return temp->type;
			}
			temp = temp->tail;
		}
		semanticError++;
		printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", root->lineno, root->child[2]->text);
		return NULL;
	}
	else if (strcmp(root->child[1]->name, "LB") == 0) {//Exp LB Exp RB
		Operand baseOp = (Operand)malloc(sizeof(Operand_t));
		memset(baseOp, 0, sizeof(Operand_t));

		TypePtr typ1 = Exp(root->child[0], baseOp);
		if (typ1 == NULL || typ1->kind != ARRAY) {
			printf("Error type 10 at Line %d: it is not an array.\n", root->lineno);
			return NULL;
		}
		TypePtr typ2;
		Operand subscripOp = (Operand)malloc(sizeof(Operand_t));
		memset(subscripOp, 0, sizeof(Operand_t));
		if(strcmp(root->child[2]->child[0]->name, "INT")==0){
				typ2 = Exp(root->child[2], NULL);
				subscripOp->kind = CONSTANT_OP;
				sprintf(subscripOp->u.value,"%s", root->child[2]->child[0]->text);
		}
		else{
			typ2 = Exp(root->child[2], subscripOp);
		}

		Operand offsetOp = malloc(sizeof(Operand_t));
		memset(offsetOp, 0, sizeof(Operand_t));
		offsetOp->kind = TEMP_VAR_OP;
		offsetOp->u.tvar_no = TempVarNum++;
		if(strcmp(root->child[2]->child[0]->name, "INT")==0 && atoi(root->child[2]->child[0]->text)==0){
			free(subscripOp);
			offsetOp->kind = CONSTANT_OP;
			strcpy(offsetOp->u.value, "0");
		}
		else{
			Operand widthOp = malloc(sizeof(Operand_t));
			memset(widthOp, 0, sizeof(Operand_t));
			widthOp->kind = CONSTANT_OP;
			int elem_width = getSize(typ1, 2);
			sprintf(widthOp->u.value, "%d", elem_width);

			InterCode offsetIR = (InterCode)malloc(sizeof(InterCode_t));
			memset(offsetIR, 0, sizeof(InterCode_t));
			offsetIR->kind = STAR_IR;
			offsetIR->u.tripleOP.result = offsetOp;
			offsetIR->u.tripleOP.op2 = widthOp;
			offsetIR->u.tripleOP.op1 = subscripOp;
			insertCode(offsetIR);
		}
		InterCode baseIR = (InterCode)malloc(sizeof(InterCode_t));
		memset(baseIR, 0, sizeof(InterCode_t));
		baseIR->kind = GET_ADDR_IR;

		baseIR->u.tripleOP.op1 = baseOp;
		baseIR->u.tripleOP.op2 = offsetOp;
		if (typ1->u.array_.elem->kind == BASIC) {
			baseIR->kind = GET_ADDR_IR;
			Operand temAddrOp = (Operand)malloc(sizeof(Operand_t));
			temAddrOp->kind = TEMP_VAR_OP;
			temAddrOp->u.tvar_no = TempVarNum++;

			baseIR->u.tripleOP.result = temAddrOp;
			place->kind = TEMP_ADDR_OP;
			place->u.name = temAddrOp;
		}
		else {
			baseIR->u.tripleOP.result = place;
			place->kind = TEMP_VAR_OP;
			place->u.tvar_no = TempVarNum++;
		}
		if (strcmp(root->child[0]->child[0]->name, "ID") == 0) {
			FieldList f = lookupSymbol(baseOp->u.value, 0);
			if (f->is_in_params == 1) {
				baseIR->kind = PLUS_IR;
			}
		}
		else {
			baseIR->kind = PLUS_IR;
		}
		insertCode(baseIR);
		return 	typ1->u.array_.elem;
	}
}

int Args(Node* root, Operand argsList) {
	if (root == NULL) {
		return 0;
	}
	//Args -> Exp
	Operand argOp = (Operand)malloc(sizeof(Operand_t));
	memset(argOp, 0, sizeof(Operand_t));

	TypePtr type;
	if (strcmp(root->child[0]->child[0]->name, "INT") == 0) {
		type = Exp(root->child[0], NULL);
		argOp->kind = CONSTANT_OP;
		sprintf(argOp->u.value, "%s", root->child[0]->child[0]->text);
	}
	else {
		argOp->kind = TEMP_VAR_OP;
		type = Exp(root->child[0], argOp);
	}
	if (type->kind == ARRAY && argOp->kind == VARIABLE_OP) {
		char *s;
		sprintf(s, "&%s", argOp->u.value);
		strcpy(argOp->u.value, s);
	}
	argOp->nextArgs = argsList->nextArgs;
	argsList->nextArgs = argOp;
	// Args -> Exp COMMA Args
	if (root->childsum != 3) {
		return 1;
	}
	else {
		return Args(root->child[2], argsList);
	}

}

TypePtr Cond(Node* root, Operand trueLabelOp, Operand falseLabelOp) {
	if (strcmp(root->child[0]->name, "Exp") == 0) {
		Node *tmp = root->child[1];
		if (strcmp(root->child[1]->name, "RELOP") == 0) {
			//CODE code1
			Operand tempVarOp1 = malloc(sizeof(Operand_t));
			memset(tempVarOp1, 0, sizeof(Operand_t));
			TypePtr typ1;
			if(strcmp(root->child[0]->child[0]->name, "INT")==0){
				typ1 = Exp(root->child[0], NULL);
				tempVarOp1->kind = CONSTANT_OP;
				sprintf(tempVarOp1->u.value,"%s", root->child[0]->child[0]->text);
			}
			else{
				tempVarOp1->kind = TEMP_VAR_OP;
				typ1 = Exp(root->child[0], tempVarOp1);
			}

			//CODE code2
			Operand tempVarOp2 = malloc(sizeof(Operand_t));
			memset(tempVarOp2, 0, sizeof(Operand_t));
			TypePtr typ2;
			if (strcmp(root->child[2]->child[0]->name, "INT") == 0) {
				typ2 = Exp(root->child[2], NULL);
				tempVarOp2->kind = CONSTANT_OP;
				sprintf(tempVarOp2->u.value, "%s", root->child[2]->child[0]->text);
			}
			else {
				tempVarOp2->kind = TEMP_VAR_OP;
				typ2 = Exp(root->child[2], tempVarOp2);
			}

			if (typ1 == NULL || typ2 == NULL) {
				return NULL;
			}

			//CODE code3 goto label_true
			InterCode ifgotoIR = malloc(sizeof(InterCode_t));
			memset(ifgotoIR, 0, sizeof(InterCode_t));
			ifgotoIR->kind = IF_GOTO_IR;
			ifgotoIR->u.ifgotoOP.op1 = tempVarOp1;
			ifgotoIR->u.ifgotoOP.op2 = tempVarOp2;
			ifgotoIR->u.ifgotoOP.label = trueLabelOp;
			strcpy(ifgotoIR->u.ifgotoOP.relop, root->child[1]->text);
			insertCode(ifgotoIR);

			//LABEL label3 goto label_false
			InterCode gotoIR = malloc(sizeof(InterCode_t));
			memset(gotoIR, 0, sizeof(InterCode_t));
			gotoIR->kind = GOTO_IR;
			gotoIR->u.singleOP.op = falseLabelOp;
			insertCode(gotoIR);
			return typ2;
		}
		else if (strcmp(root->child[1]->name, "AND") == 0) {
			Operand labelop1 = malloc(sizeof(Operand_t));
			memset(labelop1, 0, sizeof(Operand_t));
			labelop1->kind = LABEL_OP;
			labelop1->u.label_no = LabelNum++;

			//CODE code1
			TypePtr typ1 = Cond(root->child[0], labelop1, falseLabelOp);

			//LABEL label1
			InterCode labelIR1 = malloc(sizeof(InterCode_t));
			memset(labelIR1, 0, sizeof(InterCode_t));
			labelIR1->kind = LABEL_IR;
			labelIR1->u.singleOP.op = labelop1;
			insertCode(labelIR1);

			//CODE code2
			TypePtr typ2 = Cond(root->child[2], trueLabelOp, falseLabelOp);	//codeIR2
			return typ1;
		}
		else if (strcmp(root->child[1]->name, "OR") == 0) {
			Operand labelop1 = malloc(sizeof(Operand_t));
			memset(labelop1, 0, sizeof(Operand_t));
			labelop1->kind = LABEL_OP;
			labelop1->u.label_no = LabelNum++;

			//CODE code1
			TypePtr typ1 = Cond(root->child[0], trueLabelOp, labelop1);	//code1

			//LABEL label1
			InterCode labelIR1 = malloc(sizeof(InterCode_t));
			memset(labelIR1, 0, sizeof(InterCode_t));
			labelIR1->kind = LABEL_IR;
			labelIR1->u.singleOP.op = labelop1;
			insertCode(labelIR1);

			//CODE code2
			TypePtr typ2 = Cond(root->child[2], trueLabelOp, falseLabelOp);
			return typ1;
		}
	}
	else if (strcmp(root->child[0]->name, "NOT") == 0) {
		return Cond(root->child[1], falseLabelOp, trueLabelOp);
	}
	else {//other case
		//CODE code1
		Operand tempOp = (Operand)malloc(sizeof(Operand_t));
		memset(tempOp, 0, sizeof(Operand_t));
		//tempOp->kind = TEMP_VAR_OP;
		//TypePtr type = Exp(root, tempOp);
		TypePtr type;
		if (strcmp(root->child[0]->name, "INT") == 0) {
			type = Exp(root, NULL);
			tempOp->kind = CONSTANT_OP;
			sprintf(tempOp->u.value, "%s", root->child[0]->text);
		}
		else {
			tempOp->kind = TEMP_VAR_OP;
			type = Exp(root, tempOp);
		}

		//CODE code2
		InterCode ifgotoIR = (InterCode)malloc(sizeof(InterCode_t));
		memset(ifgotoIR, 0, sizeof(InterCode_t));
		ifgotoIR->kind = IF_GOTO_IR;
		ifgotoIR->u.ifgotoOP.op1 = tempOp;
		strcpy(ifgotoIR->u.ifgotoOP.relop, "!=");
		Operand zeroOp = (Operand)malloc(sizeof(Operand_t));
		memset(zeroOp, 0, sizeof(Operand_t));
		zeroOp->kind = CONSTANT_OP;
		strcpy(zeroOp->u.value, "0");
		ifgotoIR->u.ifgotoOP.op2 = zeroOp;
		ifgotoIR->u.ifgotoOP.label = trueLabelOp;
		insertCode(ifgotoIR);

		//GOTO label_false
		InterCode gotoIR = malloc(sizeof(InterCode_t));
		memset(gotoIR, 0, sizeof(InterCode_t));
		gotoIR->kind = GOTO_IR;
		gotoIR->u.singleOP.op = falseLabelOp;
		insertCode(gotoIR);
		return type;
	}
	return NULL;
}