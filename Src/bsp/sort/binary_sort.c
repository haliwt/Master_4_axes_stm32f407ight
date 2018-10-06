#include <stdlib.h>
#include "usart/bsp_debug_usart.h"
#include "sort\binary_sort.h"


/*****************************************************
 *
 *函数名称：
 *函数功能：建立一个二叉树排序
 *输入参数：xtree ，X轴，value X轴坐标系
 *
 *
******************************************************/
void x_insert(XTree *xtree, float value)
{
   XNode *xnode =(XNode *)malloc(sizeof(XNode)); //新开辟一个x节点，x坐标
   xnode -> xdata = value;
   xnode -> x_left =NULL;
   xnode -> x_right = NULL;
	
   //没有数据，空坐标
   if(xtree -> x_root == NULL)
   {
       xtree -> x_root = xnode;
   }
   else
   {
      XNode *temp = xtree -> x_root; //临时指针，指向根。
	   
	  while( temp != NULL)
	  {
	      if(value < temp -> xdata) //二叉树，值小的，放在左边。大的在右边
		  {
		     if(temp -> x_left == NULL) //如果是左边节点是空
			 {
			    temp = temp ->x_left = xnode; //插入一值，在左边，值小
			    return ;
			 }
		    else  //否则就继续查找，只能放在空的子根节点下
			  {
				 temp= temp -> x_left;
			  
			  }
	  
	      }
		  else //往左边继续放在，大于temp ,值放在左边节点
		  {
		     if(temp -> x_right == NULL) //空节点，就插入
			 {
			     temp ->x_right =xnode;
				 return ;
			 }
			 else 
			 {
			    temp = temp -> x_right; //节点不为空，继续查找。能够放在新新坐标的值
			 
			 }
		  
		  }
       }

  }
}

/****************************************************************
 *
 *函数名：
 *函数功能：先序遍历
 *输入参数：
 *
****************************************************************/
void x_preorder(XNode *xnode)
{
    if(xnode != NULL)
	{
	   printf("%f\n",xnode -> xdata); 
	   x_preorder(xnode -> x_left);
	   x_preorder(xnode -> x_right); 
	}
}

/****************************************************************
 *
 *函数名：
 *函数功能 :  中序遍历，从小到大排列
 *输入参数：
 *
****************************************************************/
void x_inorder(XTree *xnode)
{


}

