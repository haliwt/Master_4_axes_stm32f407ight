#include <stdlib.h>
#include "usart/bsp_debug_usart.h"
#include "sort\binary_sort.h"
#include "queue/queue.h"

/******************************************
 *
 *�������ܣ��������� data,�������� *T
 *
 *
*******************************************/
AVLtree *find(myType data, AVLtree *T)
{
	if(T == NULL)
		return NULL;
	if(data < T->element)
		return find(data, T->lchild);
	else if(data > T->element)
		return find(data, T->rchild);
	else
		return T;
}
/******************************************
 *
 *�������ܣ�������Сֵ,�������� *T
 *
 *
*******************************************/
AVLtree *findMin(AVLtree *T)
{
	if(T == NULL)
		return NULL;
	else if(T->lchild == NULL)
		return T;
	else
		return findMin(T->lchild);
}
/******************************************
 *
 *�������ܣ��������ֵ,�������� *T
 *
 *
*******************************************/
AVLtree *findMax(AVLtree *T)
{
	if(T != NULL)
		while(T->rchild != NULL)
			T = T->rchild;
	return T;

}

int getHeight(AVLtree *T)
{
	return GET_HEIGHT(T);
}
/***************************************************
 *
 *�������ܣ�LL ��ת��������һ������ת��
 *�������ṹ���� T
 *����ֵ: ��
 *
 *
****************************************************/
static AVLtree *left_left_rotation(AVLtree *T)
{
	AVLtree *TT;

	TT = T->lchild;
	T->lchild = TT->rchild;
	TT->rchild = T;

	T->height = MAX(getHeight(T->lchild), getHeight(T->rchild)) + 1; //WT.EDIT
	TT->height = MAX(getHeight(TT->lchild), T->height) + 1;  //WT.EDIT

	return TT;
}
/***************************************************
 *
 *�������ܣ�RR ����ת
 *�������: ��תǰ�����ڵ�T, ��ת��ĸ��ڵ�TT.
 *����ֵ�����ڵ�
 *
****************************************************/
static AVLtree *right_right_rotation(AVLtree *T)
{
	AVLtree *TT;

	TT = T->rchild;
	T->rchild = TT->lchild;
	TT->lchild = T;

	T->height = MAX(getHeight(T->lchild), getHeight(T->rchild)) + 1; //WT.EDTI
	TT->height = MAX(getHeight(TT->lchild), T->height) + 1;

	return TT;
}
/***************************************************
 *
 *��������
 *�������ܣ�LR ,������������--������������ת��������ת
 *���������
 *����ֵ�������
 *
****************************************************/
static AVLtree *left_right_rotation(AVLtree *T)//*doubleRotateWithLeft(AVLtree *T)
{
	T->lchild = right_right_rotation(T->lchild);

	return left_left_rotation(T);
}
/***************************************************
 *
 *????:
 *????:RL -????,?????
 *????:???????
 *???:???
 *
****************************************************/
static AVLtree *right_left_rotation(AVLtree *T)//doubleRotateWithRight(AVLtree *T)
{
	T->rchild = left_left_rotation(T->rchild);

	return right_right_rotation(T);
}
/*************************************************************
 *
 *�������ƣ�
 *�������ܣ�����
 *????:AVL? T ,???????,data
 *???:???
 *
**************************************************************/
AVLtree *insert(myType data, AVLtree *T)
{
	if(T == NULL) {
		T = (AVLtree *)malloc(sizeof(struct treeNode));
		T->element = data;
		T->lchild = NULL;
		T->rchild = NULL;
		T->height = 0;
	}
	else if (data < T->element)
	 {
		T->lchild = insert(data, T->lchild);
		if(GET_HEIGHT(T->lchild) - GET_HEIGHT(T->rchild) == 2)
        { //WT.EDIT

			if(data < T->lchild->element)
				T = left_left_rotation(T);
			else
				T = left_right_rotation(T);//doubleRotateWithLeft(T);
        }
	}
	else if (data > T->element)
    {
		T->rchild = insert(data, T->rchild);
		if(GET_HEIGHT(T->rchild) - GET_HEIGHT(T->lchild) == 2)
        {  //WT.EDIT
          if(data > T->rchild->element)
				T = right_right_rotation(T);
			else
				T = right_left_rotation(T);//doubleRotateWithRight(T);
        }

	}

	T->height = MAX(getHeight(T->lchild), getHeight(T->rchild)) + 1; //WT.EDIT

	return T;
}
/****************************************
 *
 *��������
 *�������ܣ�ɾ��AVL���еĽ�㣬
 *���������Ҫɾ�������� data ,������T
 *����ֵ�����ڵ�
 *
*****************************************/
AVLtree *erase(myType data, AVLtree *T)
{
	AVLtree *tmpNode;

	if(T == NULL) {
		printf("NOT FOUNT\n");
	} else if (data < T->element) {
		T->lchild = erase(data, T->lchild);
		if(getHeight(T->rchild) - getHeight(T->lchild) == 2) {
			AVLtree *tmp = T->rchild;
			if(getHeight(tmp->lchild) > getHeight(tmp->rchild))
				T = right_left_rotation(T);//doubleRotateWithRight(T);
			else
				T = right_right_rotation(T);
		}
	} else if (data > T->element) {
		T->rchild = erase(data, T->rchild);
		if(getHeight(T->lchild) - getHeight(T->rchild) == 2) {
			AVLtree *tmp = T->lchild;
			if(getHeight(tmp->rchild) > getHeight(tmp->lchild))
				T = left_right_rotation(T);//doubleRotateWithLeft(T);
			else
				T = left_left_rotation(T);
		}
	//found element to delete, two children
	} else if (T->lchild && T->rchild){
		if(getHeight(T->rchild) > getHeight(T->lchild)) {
			tmpNode = findMin(T->rchild);//??????????root
			T->element = tmpNode->element;
			T->rchild = erase(T->element, T->rchild);
		} else {
			tmpNode = findMax(T->lchild);//??????????root
			T->element = tmpNode->element;
			T->lchild = erase(T->element, T->lchild);
		}
	} else {
		//one or zero children
		tmpNode = T;
		T = (T->lchild == NULL ? T->rchild : T->lchild);
		free(tmpNode);
	}

	return T;
}
/******************************************
 *
 *��������
 *�������ƣ���α���
 *
 *
*******************************************/
void levelOrder(AVLtree *T)
{
	QUEUE *q = createQueue(100);

	while(T != NULL) {
		printf("%d ", T->element);
		if(T->lchild != NULL)
			enQueue(T->lchild, q);
		if(T->rchild != NULL)
			enQueue(T->rchild, q);

		if(!isEmpty(q))
			T = frontAndDequeue(q);
		else
			T = NULL;
	}

	disposeQueue(q);
}
/**************************************************
 *
 *��������
 *�������ܣ��������
 *????:??????
 *
 *
***************************************************/
void preOrder(AVLtree *T)
{
	if(T != NULL) {
		printf("%d ", T->element);
		preOrder(T->lchild);
		preOrder(T->rchild);
	}
}
/**************************************************
 *
 *��������
 *�������ܣ����������������---��---������
 *????:??????
 *???:?
 *
***************************************************/
void inOrder(AVLtree *T)
{
	if(T != NULL) {
		inOrder(T->lchild);
		printf("%d ", T->element);
		inOrder(T->rchild);
	}
}
/**************************************************
 *
 *��������
 *�������ܣ��������
 *????:??????
 *???:?
 *
***************************************************/
void postOrder(AVLtree *T)
{
	if(T != NULL) {
		postOrder(T->lchild);
		postOrder(T->rchild);
		printf("%d ", T->element);
	}
}
/*
void createTree(AVLtree **T)
{
	myType data;
	scanf("%d", &data);

	if(data == -1) {
		*T = NULL;
	} else {
		*T = (AVLtree *)malloc(sizeof(struct treeNode));
		(*T)->element = data;
		printf("???%d??????:", data);
		createTree(&((*T)->lchild));
		printf("???%d??????:", data);
		createTree(&((*T)->rchild));
	}
}
*/
/******************************************************
 *
 *�������ܣ���ӡAVL����ƽ�������
 *����:tree AVL��, key --�����ֵ
 *    direction --- 0. ���ڵ�
 *                  -1,����������
 *                   1,�������Һ���
 *
*********************************************************/
void printf_avltree(AVLtree *tree,myType key,int direction)
{
   if(tree !=NULL)
   {
       if(direction == 0) //tree ????
           printf("%2d is root, %d \n",tree ->element,key);
       else // tree ?????
           printf("%2d is %2d's %6s child\n",tree->element,key,direction==1?"right":"left");
           printf_avltree(tree->lchild,tree->element,-1);
           printf_avltree(tree->rchild,tree->element,1);
   }

}

