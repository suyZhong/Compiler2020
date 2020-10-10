#ifndef __SYNTAXTREE_H__
#define __SYNTAXTREE_H__

#include <stdio.h>

#define SYNTAX_TREE_NODE_NAME_MAX 30

struct _SyntaxTreeNode {
	struct _SyntaxTreeNode * parent;
	struct _SyntaxTreeNode * children[1000];
	int children_num;

	char name[SYNTAX_TREE_NODE_NAME_MAX];
};
typedef struct _SyntaxTreeNode SyntaxTreeNode;

SyntaxTreeNode * newSyntaxTreeNodeNoName();
SyntaxTreeNode * newSyntaxTreeNode(const char * name);
SyntaxTreeNode * newSyntaxTreeNodeFromNum(const int num);
int SyntaxTreeNode_AddChild(SyntaxTreeNode * parent, SyntaxTreeNode * child);
void deleteSyntaxTreeNodeNoRecur(SyntaxTreeNode * node);
void deleteSyntaxTreeNode(SyntaxTreeNode * node, int recursive);

struct _SyntaxTree {
	SyntaxTreeNode * root;
};
typedef struct _SyntaxTree SyntaxTree;

SyntaxTree * newSyntaxTree();
void deleteSyntaxTree(SyntaxTree * tree);
void printSyntaxTree(FILE * fout, SyntaxTree * tree);

#endif /* SyntaxTree.h */
