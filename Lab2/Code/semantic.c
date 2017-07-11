#include "node.h"
#include "semantic.h" 


FieldList hashTable[HASH_SIZE];

unsigned int hash_pjw(char *name){
	unsigned int val = 0, i;
	for(;*name;++name){
		val = (val << 2) + *name;
		if(i=val & ~0x3fff){
			val = (val ^ (i>>12)) & 0x3fff;
		}
		return val % HASH_SIZE;
	}
}


void initHashtable(){
	for(int i=0; i<HASH_SIZE; i++){
		hashTable[i] = NULL;
	}
}



int insertSymbol(FieldList f){
	if(f==NULL)
		return 0;
	if(f->name==NULL)
		return 0;
	f->collision = 0;
	unsigned int key;
	if(f->type->kind==3)
	     key= hash_pjw(1+f->name);
	else key=hash_pjw(f->name);
	if(hashTable[key] == NULL){
		hashTable[key] = f;
		return 1;
	}

	while(1){
		key = (++key) % HASH_SIZE;
		f->collision = f->collision + 1;
		if(hashTable[key] == NULL){
			hashTable[key] = f;
			return 1;
		}
	}
	return 0;
}

FieldList lookupSymbol(char *name,int function){
	if(name == NULL){
		return NULL;
	}
	unsigned int key;
	if(function)
	    key=hash_pjw(1+name);
	else key=hash_pjw(name);
	FieldList p=hashTable[key];
	while(p!=NULL){
		if(strcmp(name,p->name)==0){
			if((function==1)&&(p->type->kind==FUNCTION))
				return p;
			if((function==0)&&(p->type->kind!=FUNCTION))
				return p;
		}
		key=(++key)%HASH_SIZE;
		p=hashTable[key];
	}
	return NULL;
}

void AllSymbol(){
	for(int i=0;i<HASH_SIZE;i++)
		if(hashTable[i]!=NULL)
			printf("name:%s,kind:%d\n",hashTable[i]->name,hashTable[i]->type->kind);
}

int TypeEqual(TypePtr type1,TypePtr type2){
	if((type1==NULL)||(type2==NULL))
		return 0;
	if(type1->kind!=type2->kind)
		return 0;
	else switch(type1->kind){
		case BASIC:{
			if(type1->u.basic_==type2->u.basic_)
				return 1;
			else return 0;
		}break;
		case ARRAY:{
			if(TypeEqual(type1->u.array_.elem,type2->u.array_.elem)==1)
				return 1;
			else return 0;
		}break;
		case STRUCTURE:{
			FieldList field1=type1->u.structure_;
			FieldList field2=type2->u.structure_;
			if((field1!=NULL)&&(field2!=NULL)){
				while((field1!=NULL)&&(field2!=NULL)){
					if(TypeEqual(field1->type,field2->type)==0){
						return 0;
					}
					field1=field1->tail;
					field2=field2->tail;
				}
				if((field1==NULL)&&(field2==NULL))
					return 1;
			}
			return 0;
		}break;
		case FUNCTION:{
			if(type1->u.function_.paramNum!=type2->u.function_.paramNum)
				return 0;
			FieldList param1=type1->u.function_.params;
			FieldList param2=type2->u.function_.params;
			for(int i=0;i<type1->u.function_.paramNum;i++){
				if(TypeEqual(param1->type,param2->type)==0)
					return 0;
				param1=param1->tail;
				param2=param2->tail;
			}
			return 1;
		}break;
		default:{
			return 0;
		}break;
	}
}

FieldList VarDec(Node *root,TypePtr basictype){
    Node *temp=root;
    int i=0;
    while(strcmp(temp->child[0]->name,"ID")!=0){
        temp=temp->child[0];
        i++;
    }
    char *s=temp->child[0]->text;

    FieldList field=(FieldList )malloc(sizeof(FieldList_));
    field->name=s;

    if(strcmp(root->child[0]->name,"ID")==0){
        field->type=basictype;
        return field;
    }
    
    switch(i){//allow 2 row array
        case 1:{
            TypePtr var1=(TypePtr)malloc(sizeof(Type_));
            var1->kind=ARRAY;
            var1->u.array_.size=atoi(root->child[2]->text);
            var1->u.array_.elem=basictype;
            field->type=var1;
            return field;
        }break;
        case 2:{
            TypePtr var1=(TypePtr)malloc(sizeof(Type_));
            var1->kind=ARRAY;
            var1->u.array_.size=atoi(root->child[2]->text);
            var1->u.array_.elem=basictype;
            TypePtr var2=(TypePtr)malloc(sizeof(Type_));
            var2->kind=ARRAY;
            var2->u.array_.size=atoi(root->child[0]->child[2]->text);
            var2->u.array_.elem=var1;
            field->type=var2;
            return field;
        }break;
        default:printf("error in VarDec");break;
    }
}

TypePtr Specifier(Node *root){
    TypePtr spe=(TypePtr)malloc(sizeof(Type_));
    if(strcmp(root->child[0]->name,"TYPE")==0){//TYPE
        spe->kind=BASIC;
        if(strcmp(root->child[0]->text,"int")==0)
            spe->u.basic_=INT_TYPE;
        else spe->u.basic_=FLOAT_TYPE;
        return spe;
    }
    else{
        spe->kind=STRUCTURE;
        if(root->child[0]->childsum==2){//STRUCT Tag
            char *s=root->child[0]->child[1]->child[0]->text;
            FieldList field=lookupSymbol(s,0);
            if(field==NULL){
                printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",root->lineno,s);
                spe->u.structure_=NULL;
                return spe;
            }
            else if(field->type!=NULL)
                return field->type;
            spe->u.structure_=NULL;
            return spe;
        }
        else{//STRUCT OptTag LC DefList RC
            Node* DefList=root->child[0]->child[3];
            spe->u.structure_=NULL;
            //DefList in STRUCT is different from that outside
            while(DefList!=NULL){//Def DefList
                Node *Def=DefList->child[0];
                TypePtr basictype=Specifier(Def->child[0]);
                
                Node* DecList=Def->child[1];
                while(DecList->childsum==3){//Dec COMMA DecList
                    FieldList field=VarDec(DecList->child[0]->child[0],basictype);
                    if(DecList->child[0]->childsum!=1)
                        printf("Error type 15 at Line %d: Variable %s in struct is initialized.\n",Def->lineno,field->name);
                    FieldList temp=spe->u.structure_;
                    while(temp!=NULL){
                        if(strcmp(temp->name,field->name)==0){
                            printf("Error type 15 at Line %d: Redefined field \"%s\".\n",Def->lineno,field->name);
                            break;
                        }
                        temp=temp->tail;
                    }
                    if(temp==NULL){
			if(lookupSymbol(field->name,0)!=NULL)
			    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",Def->lineno,field->name);
			else{
			    insertSymbol(field);
                            field->tail=spe->u.structure_;
                            spe->u.structure_=field;
			}
                    }
                    DecList=DecList->child[2];
                }
                FieldList field=VarDec(DecList->child[0]->child[0],basictype);
                if(DecList->child[0]->childsum!=1)
                    printf("Error type 15 at Line %d: Variable \"%s\" in struct is initialized.\n",Def->lineno,field->name);
                FieldList temp=spe->u.structure_;
                while(temp!=NULL){
                    if(strcmp(temp->name,field->name)==0){
                        printf("Error type 15 at Line %d: Redefined field \"%s\".\n",Def->lineno,field->name);
                        break;
                    }
                    temp=temp->tail;
                }
                if(temp==NULL){
		    if(lookupSymbol(field->name,0)!=NULL)
		        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",Def->lineno,field->name);
		    else{
			insertSymbol(field);
                        field->tail=spe->u.structure_;
                        spe->u.structure_=field;
		    }
                }
                DefList=DefList->child[1];
            }
            if(root->child[0]->child[1]!=NULL){//OptTag exist
                FieldList field=(FieldList )malloc(sizeof(FieldList_));
                field->type=spe;
                char *s=root->child[0]->child[1]->child[0]->text;//get the name of OptTag
                field->name=s;
                if(lookupSymbol(field->name,0)!=NULL)
                    printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",root->lineno,field->name);
                else insertSymbol(field);
            }
            return spe;
        }
    }
}

void ExtDefList(Node *root){
    Node* ExtDefList=root;
    while(ExtDefList->childsum!=0){//ExtDef ExtDefList
        Node* ExtDef=ExtDefList->child[0];
        TypePtr basictype=Specifier(ExtDef->child[0]);
        
        if(strcmp(ExtDef->child[1]->name,"ExtDecList")==0){//Specifier ExtDecList SEMI
            Node* temp=ExtDef->child[1];//ExtDecList
            FieldList field;
            while(temp->childsum==3){
                field=VarDec(temp->child[0],basictype);
                if(lookupSymbol(field->name,0)!=NULL)
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",ExtDef->lineno,field->name);
                else insertSymbol(field);
                temp=temp->child[2];
            }
            field=VarDec(temp->child[0],basictype);
            if(lookupSymbol(field->name,0)!=NULL)
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",ExtDef->lineno,field->name);
            else insertSymbol(field);
        }
        else if(strcmp(ExtDef->child[1]->name,"FunDec")==0){//Specifier FunDec CompSt
            FieldList field=(FieldList )malloc(sizeof(FieldList_));
            field->name=ExtDef->child[1]->child[0]->text;
            TypePtr typ=(TypePtr)malloc(sizeof(Type_));
            typ->kind=FUNCTION;
            typ->u.function_.funcType=basictype;
            //ID LP RP already done
            typ->u.function_.paramNum=0;
            typ->u.function_.params=NULL;

            if(strcmp(ExtDef->child[1]->child[2]->name,"VarList")==0){//ID LP VarList RP
                Node *VarList=ExtDef->child[1]->child[2];
                while(VarList->childsum!=1){//ParamDec COMMA VarList
                    TypePtr tempType=Specifier(VarList->child[0]->child[0]);
                    FieldList tempField=VarDec(VarList->child[0]->child[1],tempType);
                    if(lookupSymbol(tempField->name,0)!=NULL)
                        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",ExtDef->lineno,tempField->name);
                    else insertSymbol(tempField);
                    typ->u.function_.paramNum++;
                    tempField->tail=typ->u.function_.params;
                    typ->u.function_.params=tempField;

                    VarList=VarList->child[2];
                }//ParamDec
                TypePtr tempType=Specifier(VarList->child[0]->child[0]);
                FieldList tempField=VarDec(VarList->child[0]->child[1],tempType);
                if(lookupSymbol(tempField->name,0)!=NULL)
                    printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",ExtDef->lineno,tempField->name);
                else insertSymbol(tempField);
                typ->u.function_.paramNum++;
                tempField->tail=typ->u.function_.params;
                typ->u.function_.params=tempField;
            }
            field->type=typ;
            if(lookupSymbol(field->name,1)!=NULL)
                printf("Error type 4 at Line %d: Redefined function \"%s\".\n",ExtDef->lineno,field->name);
            else insertSymbol(field);

            //CompSt->LC DefList StmtList RC
            CompSt(ExtDef->child[2],basictype);
        }
        else{//Specifier SIMI
            //do nothing
        }

        if(ExtDefList->child[1]==NULL)//ExtDef
            return;
        ExtDefList=ExtDefList->child[1];
    }
}

void CompSt(Node *root,TypePtr funcType){
    Node *CompSt=root;
    DefList(CompSt->child[1]);
    Node *StmtList=CompSt->child[2];
    while(StmtList!=NULL){
        Node *Stmt_=StmtList->child[0];
        Stmt(Stmt_,funcType);
        StmtList=StmtList->child[1];
    }
}

void DefList(Node *root){
    Node* DefList=root;
    while(DefList!=NULL){//Def DefList
        Node* Def=DefList->child[0];
        TypePtr basictype=Specifier(Def->child[0]);
        Node *DecList=Def->child[1];
        while(DecList->childsum==3){//Dec COMMA DecList
            FieldList field=VarDec(DecList->child[0]->child[0],basictype);
            if(lookupSymbol(field->name,0)!=NULL)
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",DecList->lineno,field->name);
            else insertSymbol(field);
            DecList=DecList->child[2];
        }
        FieldList field=VarDec(DecList->child[0]->child[0],basictype);
        if(lookupSymbol(field->name,0)!=NULL)
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",DecList->lineno,field->name);
        else insertSymbol(field);
        if(DefList->child[1]==NULL)//Def
            return;
        DefList=DefList->child[1];
    }
}

void Stmt(Node *root,TypePtr funcType){
    Node *Stmt_=root;
    if(strcmp(Stmt_->child[0]->name,"RETURN")==0){//RETURN Exp SEMI
        TypePtr returnType=Exp(Stmt_->child[1]);
        if(TypeEqual(funcType,returnType)==0)
            printf("Error type 8 at Line %d: Type mismatched for return.\n",Stmt_->lineno);
    }
    else if(strcmp(Stmt_->child[0]->name,"Exp")==0){//Exp
        Exp(Stmt_->child[0]);
    }
    else if(strcmp(Stmt_->child[0]->name,"CompSt")==0){//CompSt
        CompSt(Stmt_->child[0],funcType);
    }
    else if(strcmp(Stmt_->child[0]->name,"WHILE")==0){//WHILE LP Exp RP Stmt
        TypePtr typ=Exp(Stmt_->child[2]);
        if(!((typ->kind==BASIC)&&(typ->u.basic_==INT_TYPE)))
            printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n",Stmt_->lineno);
        Stmt(Stmt_->child[4],funcType);
    }
    else if(Stmt_->childsum<6){//IF LP Exp RP Stmt
        TypePtr typ=Exp(Stmt_->child[2]);
        if(typ!=NULL)
            if(!((typ->kind==BASIC)&&(typ->u.basic_==INT_TYPE)))
                printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n",Stmt_->lineno);

        Stmt(Stmt_->child[4],funcType);
    }
    else{//IF LP Exp RP Stmt ELSE Stmt
        TypePtr typ=Exp(Stmt_->child[2]);
        if(!((typ->kind==BASIC)&&(typ->u.basic_==INT_TYPE)))
            printf("Error type 5 at Line %d: Only type INT could be used for judgement.\n",Stmt_->lineno);
        Stmt(Stmt_->child[4],funcType);
        Stmt(Stmt_->child[6],funcType);
    }
}

TypePtr Exp(Node* root){
    if(root==NULL)
        return NULL;
    else if((strcmp(root->child[0]->name,"ID")==0)&&(root->childsum==1)){//ID
        FieldList field=lookupSymbol(root->child[0]->text,0);
        if(field!=NULL)
            return field->type;
        else{
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",root->lineno,root->child[0]->text);
            return NULL;
        }
    }
    else if(strcmp(root->child[0]->name,"INT")==0){//INT
        TypePtr typ=(TypePtr)malloc(sizeof(Type_));
        typ->kind=BASIC;
        typ->u.basic_=INT_TYPE;
        return typ;
    }
    else if(strcmp(root->child[0]->name,"FLOAT")==0){//FLOAT
        TypePtr typ=(TypePtr)malloc(sizeof(Type_));
        typ->kind=BASIC;
        typ->u.basic_=FLOAT_TYPE;
        return typ;
    }
    else if((strcmp(root->child[0]->name,"LP")==0)||(strcmp(root->child[0]->name,"MINUS")==0)||(strcmp(root->child[0]->name,"NOT")==0)){
        return Exp(root->child[1]);
    }
    else if((strcmp(root->child[1]->name,"PLUS")==0)||(strcmp(root->child[1]->name,"MINUS")==0)||(strcmp(root->child[1]->name,"STAR")==0)||(strcmp(root->child[1]->name,"DIV")==0)){
        TypePtr typ1=Exp(root->child[0]);
        TypePtr typ2=Exp(root->child[2]);
        if(TypeEqual(typ1,typ2)==0){
            if((typ1!=NULL)&&(typ2!=NULL))
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            return NULL;
        }
        else return typ1;
    }
    else if((strcmp(root->child[1]->name,"AND")==0)||(strcmp(root->child[1]->name,"OR")==0)||(strcmp(root->child[1]->name,"RELOP")==0)){
        TypePtr typ1=Exp(root->child[0]);
        TypePtr typ2=Exp(root->child[2]);
        if(TypeEqual(typ1,typ2)==0){
            if((typ1!=NULL)&&(typ2!=NULL))
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->lineno);
            return NULL;
        }
        else{
	    TypePtr typ=(TypePtr)malloc(sizeof(Type_));
	    typ->kind=BASIC;
	    typ->u.basic_=INT_TYPE;
  	    return typ;
	}
    }
    else if(strcmp(root->child[1]->name,"ASSIGNOP")==0){
        if(root->child[0]->childsum==1){
            if(!(strcmp(root->child[0]->child[0]->name,"ID")==0)){
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                return NULL;
            }
        }
        else if(root->child[0]->childsum==3){
            if(!((strcmp(root->child[0]->child[0]->name,"Exp")==0)&&(strcmp(root->child[0]->child[1]->name,"DOT")==0)&&(strcmp(root->child[0]->child[2]->name,"ID")==0))){
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                return NULL;
            }
        }
        else if(root->child[0]->childsum==4){
            if(!((strcmp(root->child[0]->child[0]->name,"Exp")==0)&&(strcmp(root->child[0]->child[1]->name,"LB")==0)&&(strcmp(root->child[0]->child[2]->name,"Exp")==0)&&(strcmp(root->child[0]->child[3]->name,"RB")==0))){
                printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->lineno);
                return NULL;
            }
        }
        TypePtr typ1=Exp(root->child[0]);
        TypePtr typ2=Exp(root->child[2]);
        if(TypeEqual(typ1,typ2)==0){
            if((typ1!=NULL)&&(typ2!=NULL))
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->lineno);
            return NULL;
        }
        else return typ1;
    }
    else if(strcmp(root->child[0]->name,"ID")==0){//ID LP RP
        FieldList fie=lookupSymbol(root->child[0]->text,1);
        if(fie==NULL){
            FieldList fie2=lookupSymbol(root->child[0]->text,0);
            if(fie2!=NULL)
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n",root->lineno,root->child[0]->text);
            else printf("Error type 2 at Line %d: Undefined function \"%s\".\n",root->lineno,root->child[0]->text);
            return NULL;
        }
        TypePtr definedType=fie->type;

        TypePtr typ=(TypePtr)malloc(sizeof(Type_));
        typ->kind=FUNCTION;
        typ->u.function_.paramNum=0;
        typ->u.function_.params=NULL;
        if(strcmp(root->child[2]->name,"RP")!=0){//ID LP Args RP
            Node* temp=root->child[2];
            while(temp->childsum!=1){//Exp COMMA Args
                TypePtr tempType=Exp(temp->child[0]);
                FieldList tempField=(FieldList )malloc(sizeof(FieldList_));
                tempField->name="no";
		tempField->type=tempType;
                typ->u.function_.paramNum++;
                tempField->tail=typ->u.function_.params;
                typ->u.function_.params=tempField;

                temp=temp->child[2];
            }//Exp
            TypePtr tempType=Exp(temp->child[0]);
            FieldList tempField=(FieldList )malloc(sizeof(FieldList_));
            tempField->name="no";//just for temp compare
	    tempField->type=tempType;
            typ->u.function_.paramNum++;
            tempField->tail=typ->u.function_.params;
            typ->u.function_.params=tempField;
        }
        if(TypeEqual(typ,definedType)==0){
            printf("Error type 9 at Line %d: Params wrong in function \"%s\".\n",root->lineno,root->child[0]->text);
            return NULL;
        }
        else return definedType->u.function_.funcType;
    }
    else if(strcmp(root->child[1]->name,"DOT")==0){//Exp DOT ID
        TypePtr typ1=Exp(root->child[0]);
        if(typ1->kind!=STRUCTURE){
            Node* temp=root->child[0];
            char *s;
            switch(temp->childsum){
                case 1:{
                    if(strcmp(temp->child[0]->name,"ID")==0)
                        s=temp->child[0]->text;
                }break;
                case 3:{
                    if(strcmp(temp->child[2]->name,"ID")==0)
                        s=temp->child[0]->text;
                }break;
                case 4:{
                    if(strcmp(temp->child[0]->name,"Exp")==0)
                        if(strcmp(temp->child[0]->child[0]->name,"ID")==0)
                            s=temp->child[0]->child[0]->text;
                }break;
                default:s="error";break;
            }
            if(lookupSymbol(s,0)!=NULL)
                printf("Error type 13 at Line %d: Illegal use of \".\".\n",root->lineno);
            return NULL;
        }
        char *s=root->child[2]->text;
        FieldList temp=typ1->u.structure_;
        while(temp!=NULL){
            if(strcmp(temp->name,s)==0)
                return temp->type;

            temp=temp->tail;
        }
        
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",root->lineno,root->child[2]->text);
        return NULL;
    }
    else if(strcmp(root->child[1]->name,"LB")==0){//Exp LB Exp RB
        TypePtr typ1=Exp(root->child[0]);
        if(typ1->kind!=ARRAY){
            Node* temp=root->child[0];
            char *s;
            switch(temp->childsum){
                case 1:{
                    if(strcmp(temp->child[0]->name,"ID")==0)
                        s=temp->child[0]->text;
                }break;
                case 3:{
                    if(strcmp(temp->child[2]->name,"ID")==0)
                        s=temp->child[0]->text;
                }break;
                case 4:{
                    if(strcmp(temp->child[0]->name,"Exp")==0)
                        if(strcmp(temp->child[0]->child[0]->name,"ID")==0)
                            s=temp->child[0]->child[0]->text;
                }break;
                default:s="error";break;
            }
            if(lookupSymbol(s,0)!=NULL)
                printf("Error type 10 at Line %d: \"%s\" is not an array.\n",root->lineno,s);
            return NULL;
        }
        TypePtr temp=Exp(root->child[2]);
        if(temp->kind!=BASIC){
            printf("Error type 12 at Line %d: there is not a integer between \"[\" and \"]\".\n",root->lineno);
            return NULL;
        }
        else if(temp->u.basic_==FLOAT_TYPE){
            printf("Error type 12 at Line %d: there is not a integer between \"[\" and \"]\".\n",root->lineno);
            return NULL;
        }
        //no error
        return typ1->u.array_.elem;
    }
    else{
        printf("in\n");
        return NULL;
    }
}
