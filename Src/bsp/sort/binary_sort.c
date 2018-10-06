#include <stdlib.h>
#include "usart/bsp_debug_usart.h"
#include "sort\binary_sort.h"


/*****************************************************
 *
 *�������ƣ�
 *�������ܣ�����һ������������
 *���������xtree ��X�ᣬvalue X������ϵ
 *
 *
******************************************************/
void x_insert(XTree *xtree, float value)
{
   XNode *xnode =(XNode *)malloc(sizeof(XNode)); //�¿���һ��x�ڵ㣬x����
   xnode -> xdata = value;
   xnode -> x_left =NULL;
   xnode -> x_right = NULL;
	
   //û�����ݣ�������
   if(xtree -> x_root == NULL)
   {
       xtree -> x_root = xnode;
   }
   else
   {
      XNode *temp = xtree -> x_root; //��ʱָ�룬ָ�����
	   
	  while( temp != NULL)
	  {
	      if(value < temp -> xdata) //��������ֵС�ģ�������ߡ�������ұ�
		  {
		     if(temp -> x_left == NULL) //�������߽ڵ��ǿ�
			 {
			    temp = temp ->x_left = xnode; //����һֵ������ߣ�ֵС
			    return ;
			 }
		    else  //����ͼ������ң�ֻ�ܷ��ڿյ��Ӹ��ڵ���
			  {
				 temp= temp -> x_left;
			  
			  }
	  
	      }
		  else //����߼������ڣ�����temp ,ֵ������߽ڵ�
		  {
		     if(temp -> x_right == NULL) //�սڵ㣬�Ͳ���
			 {
			     temp ->x_right =xnode;
				 return ;
			 }
			 else 
			 {
			    temp = temp -> x_right; //�ڵ㲻Ϊ�գ��������ҡ��ܹ��������������ֵ
			 
			 }
		  
		  }
       }

  }
}

/****************************************************************
 *
 *��������
 *�������ܣ��������
 *���������
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
 *��������
 *�������� :  �����������С��������
 *���������
 *
****************************************************************/
void x_inorder(XTree *xnode)
{


}

