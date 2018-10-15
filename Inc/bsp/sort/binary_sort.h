#ifndef __BINARY_SORT_H__
#define __BINARY_SORT_H__
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b)	(a > b ? a : b)
#define GET_HEIGHT(T)	(T == NULL ? -1 : T->height)

typedef int myType;

typedef struct treeNode
{
	myType element;
	struct treeNode *lchild;
	struct treeNode *rchild;
	int height;
}AVLtree;


void preOrder(AVLtree *T);
void inOrder(AVLtree *T);
void postOrder(AVLtree *T);
void levelOrder(AVLtree *T);
AVLtree *find(myType data, AVLtree *T);
AVLtree *findMin(AVLtree *T);
AVLtree *findMax(AVLtree *T);
AVLtree *insert(myType data, AVLtree *T);
AVLtree *erase(myType data, AVLtree *T);
void createTree(AVLtree **T);
int getHeight(AVLtree *T);

void printf_avltree(AVLtree *tree,myType key,int direction);



#endif 
