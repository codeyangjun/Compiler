#ifndef _MY_NODE_H_
#define _MY_NODE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_CHILD_NUM 7
extern int yylineno;
//抽象语法树
typedef struct Abstract_Tree{
    char name[32];  
    char text[32];
    int lineno;
    int childsum;
    struct Abstract_Tree *child[MAX_CHILD_NUM];
}Node;

Node* createNode(char* name, char* text);
void addChild(int childsum, Node* parent, ...);
void printTree(Node *root, int blank);

#endif
