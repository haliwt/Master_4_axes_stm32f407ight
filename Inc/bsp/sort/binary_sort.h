#ifndef __BINARY_SORT_H__
#define __BINARY_SORT_H__

//定义一个X轴链表
typedef struct xnode
{
    float xdata;
	struct xnode *x_left;
	struct xnode *x_right;
	
}XNode ;

//封装一个树
typedef struct xtree
{
   XNode *x_root;

}XTree;



//定义源函数
void x_insert(XTree *xtree, float value); //二叉树
void x_preorder(XNode *xnode);  //先序遍历 
void x_inorder(XNode *xnode);  //中序遍历




#endif 
