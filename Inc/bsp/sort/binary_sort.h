#ifndef __BINARY_SORT_H__
#define __BINARY_SORT_H__

//����һ��X������
typedef struct xnode
{
    float xdata;
	struct xnode *x_left;
	struct xnode *x_right;
	
}XNode ;

//��װһ����
typedef struct xtree
{
   XNode *x_root;

}XTree;



//����Դ����
void x_insert(XTree *xtree, float value); //������
void x_preorder(XNode *xnode);  //������� 
void x_inorder(XNode *xnode);  //�������




#endif 
