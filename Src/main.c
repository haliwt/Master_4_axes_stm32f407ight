/**
  ******************************************************************************
  * �ļ�����: main.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2017-3-30
  * ��    ��: ��Ϣ���У��жϷ�ʽ��
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F4Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "led/bsp_led.h"
#include "usart/bsp_debug_usart.h"
#include "key/bsp_key.h"
#include "stm32f4xx_hal.h"
#include "StepMotor/bsp_STEPMOTOR.h" 

__IO uint32_t set_speed  = 600;         // �ٶ� ��λΪ0.05rad/sec

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED1 = NULL;
static TaskHandle_t xHandleTaskLED2 = NULL;
static TaskHandle_t xHandleTaskLED3 = NULL;
static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;

uint8_t aRxBuffer[8];

KEY key1,key2,key3,key4,key5;

typedef struct Msg
{
	uint8_t  ucMessageID;
	uint16_t usData[8];
	uint32_t ulData[8];
}MSG_T;

MSG_T   g_tMsg; /* ����һ���ṹ��������Ϣ���� */

/* ��չ���� ------------------------------------------------------------------*/
extern __IO uint16_t CCR1_Val;

/* ˽�к���ԭ�� --------------------------------------------------------------*/
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED1(void *pvParameters);
static void vTaskLED2(void *pvParameters);
static void vTaskLED3(void *pvParameters);
static void AppTaskCreate (void);
static void AppObjCreate (void);

/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ϵͳʱ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
 
  __HAL_RCC_PWR_CLK_ENABLE();                                     //ʹ��PWRʱ��

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);  //���õ�ѹ�������ѹ����1

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;      // �ⲿ����8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                        //��HSE 
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                    //��PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;            //PLLʱ��Դѡ��HSE
  RCC_OscInitStruct.PLL.PLLM = 8;                                 //8��ƵMHz
  RCC_OscInitStruct.PLL.PLLN = 336;                               //336��Ƶ
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                     //2��Ƶ���õ�168MHz��ʱ��
  RCC_OscInitStruct.PLL.PLLQ = 7;                                 //USB/SDIO/������������ȵ���PLL��Ƶϵ��
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       // ϵͳʱ�ӣ�168MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;              // AHBʱ�ӣ� 168MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;               // APB1ʱ�ӣ�42MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;               // APB2ʱ�ӣ�84MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_RCC_EnableCSS();                                            // ʹ��CSS���ܣ�����ʹ���ⲿ�����ڲ�ʱ��ԴΪ����
  
 	// HAL_RCC_GetHCLKFreq()/1000    1ms�ж�һ��
	// HAL_RCC_GetHCLKFreq()/100000	 10us�ж�һ��
	// HAL_RCC_GetHCLKFreq()/1000000 1us�ж�һ��
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);                // ���ò�����ϵͳ�δ�ʱ��
  /* ϵͳ�δ�ʱ��ʱ��Դ */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* ϵͳ�δ�ʱ���ж����ȼ����� */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
int main(void)
{
  
  /* ��λ�������裬��ʼ��Flash�ӿں�ϵͳ�δ�ʱ�� */
  HAL_Init();
  /* ����ϵͳʱ�� */
  SystemClock_Config();
  
  MX_DEBUG_USART_Init();

  /* ��ʼ��LED */
  LED_GPIO_Init();
  /* ���Ӱ�����ʼ�� */
  KEY_GPIO_Init();
  /* ������ʱ����ʼ����100us�ж�һ�� */
   STEPMOTOR_TIMx_Init();
	/* �������� */
	AppTaskCreate();
  /* ��������ͨ�Ż��� */
	AppObjCreate();	
  /* �������ȣ���ʼִ������ */
  vTaskStartScheduler();
  
  /* ����ѭ�� */
  while (1)
  {
  }
}

/**
  * ��������: �ӿ���Ϣ����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
//static uint8_t g_uiCount[8] = 0; /* ����Ϊȫ�־�̬�������������ݸ��� */
static void vTaskTaskUserIF(void *pvParameters)
{
  uint8_t pcWriteBuffer[500];
  uint8_t g_uiCount;
    /* �������� */
  KeyCreate(&key1,GetPinStateOfKey1);
	KeyCreate(&key2,GetPinStateOfKey2);
	KeyCreate(&key3,GetPinStateOfKey3);
 
  printf("KEY1��KEY2��KEY3��Ӧ��ͬ�������\n");

    while(1)
    {
      Key_RefreshState(&key1);//ˢ�°���״̬
      Key_RefreshState(&key2);//ˢ�°���״̬
      Key_RefreshState(&key3);//ˢ�°���״̬
   
       HAL_UART_Receive(&husart_debug,aRxBuffer,8,0xffff);
	   printf("aRxBuffer[0]=%#x \n",aRxBuffer[0]);
      if(Key_AccessTimes(&key1,KEY_ACCESS_READ)!=0)//���������¹�
      {
        printf("=================================================\r\n");
        printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
        vTaskList((char *)&pcWriteBuffer);
        printf("%s\r\n", pcWriteBuffer);
       
        printf("\r\n������       ���м���         ʹ����\r\n");
        vTaskGetRunTimeStats((char *)&pcWriteBuffer);
        printf("%s\r\n", pcWriteBuffer);
        Key_AccessTimes(&key1,KEY_ACCESS_WRITE_CLEAR);
      }
      
     //if(aRxBuffer[0]==0xa6)////if(Key_AccessTimes(&key2,KEY_ACCESS_READ)!=0)//���������¹�
      {         
        printf("KEY2���£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������Ϣ\r\n");
		  
	   MSG_T   *ptMsg;
       BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      
      /* ��ʼ���ṹ��ָ�� */
      ptMsg = &g_tMsg;
      
      /* ��ʼ������ */
      ptMsg->ucMessageID=aRxBuffer[1];
      ptMsg->ulData[0]=aRxBuffer[0];
	  ptMsg->ulData[1]=aRxBuffer[1];
	  ptMsg->ulData[2]=aRxBuffer[2];
	  ptMsg->ulData[3]=aRxBuffer[3];
	  ptMsg->ulData[4]=aRxBuffer[4];
	  ptMsg->ulData[5]=aRxBuffer[5];
	  ptMsg->ulData[6]=aRxBuffer[6];
	  ptMsg->ulData[7]=aRxBuffer[7];
      ptMsg->usData[0]=aRxBuffer[3];;
		 /* ����Ϣ���з����� */
       xQueueSendFromISR(xQueue2,
                  (void *)&ptMsg,
                   &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        Key_AccessTimes(&key2,KEY_ACCESS_WRITE_CLEAR);
		  }
      
      if(aRxBuffer[0]==0xa1)//if(Key_AccessTimes(&key3,KEY_ACCESS_READ)!=0)//���������¹�
      {         
        printf("KEY3���£��������ζ�ʱ���жϣ�50ms���ڶ�ʱ���жϸ�����vTaskMsgPro������Ϣ\r\n");
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
         
         if(aRxBuffer[1]==0x01)
		 {
			   
			  g_uiCount=aRxBuffer[2];
			   printf("aRxBuffer[2]= %#x\n",aRxBuffer[2]);
			  /* ����Ϣ���з����� */
			  xQueueSendFromISR(xQueue1,
					  (void *)&g_uiCount,
					  &xHigherPriorityTaskWoken);

			 /* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
			 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			//HAL_TIM_OC_Start_IT(&htim2,TIM_CHANNEL_2);
			 Key_AccessTimes(&key3,KEY_ACCESS_WRITE_CLEAR);
		 }
		 if(aRxBuffer[2]==0x02)
		 {
		       
			   printf("aRxBuffer[3]= %#x\n",aRxBuffer[3]);
			   g_uiCount=aRxBuffer[3];
			   printf("g_uiCount= %#x\n",g_uiCount);
			 /* ����Ϣ���з����� */
			  xQueueSendFromISR(xQueue1,
					  (void *)&g_uiCount,
					  &xHigherPriorityTaskWoken);

			 /* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
			 portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			//HAL_TIM_OC_Start_IT(&htim2,TIM_CHANNEL_2);
			 Key_AccessTimes(&key3,KEY_ACCESS_WRITE_CLEAR);
		 
		 }
		  }
        taskYIELD();
     vTaskDelay(100);
  }
		
}

/********************************************************
  *
  * �������ƣ�
  * ��������: ������е���Ϣ,xQueue2 ,�ṹ���ַ&ptMsg.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  *
*********************************************************/
static void vTaskLED1(void *pvParameters)
{
	MSG_T *ptMsg;
	BaseType_t xResult;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* �������ȴ�ʱ��Ϊ200ms */
	

  while(1)
  {
    xResult = xQueueReceive(xQueue2,                   /* ��Ϣ���о�� */
                            (void *)&ptMsg,  		   /* �����ȡ���ǽṹ��ĵ�ַ */
                            (TickType_t)xMaxBlockTime);/* ��������ʱ�� */


    if(xResult == pdPASS)
    {
      /* �ɹ����գ���ͨ�����ڽ����ݴ�ӡ���� */
      printf("���յ���Ϣ��������ptMsg->ucMessageID = %#x\r\n", ptMsg->ucMessageID);
      printf("���յ���Ϣ��������ptMsg->ulData[0] = %#x\r\n", ptMsg->ulData[0]);
	  printf("���յ���Ϣ��������ptMsg->ulData[1] = %#x\r\n", ptMsg->ulData[1]);
	  printf("���յ���Ϣ��������ptMsg->ulData[2] = %#x\r\n", ptMsg->ulData[2]);
	  printf("���յ���Ϣ��������ptMsg->ulData[3] = %#x\r\n", ptMsg->ulData[3]);
	  printf("���յ���Ϣ��������ptMsg->ulData[4] = %#x\r\n", ptMsg->ulData[4]);
	  printf("���յ���Ϣ��������ptMsg->ulData[5] = %#x\r\n", ptMsg->ulData[5]);
	  printf("���յ���Ϣ��������ptMsg->ulData[6] = %#x\r\n", ptMsg->ulData[6]);
	  printf("���յ���Ϣ��������ptMsg->ulData[7] = %#x\r\n", ptMsg->ulData[7]);
      printf("���յ���Ϣ��������ptMsg->usData[3] = %#x\r\n", ptMsg->usData[3]);
    }
    else
    {
      LED1_TOGGLE;

    }
  }
}

/***************************************************************
  *
  * �������ƣ�
  * ��������: �������1�����ݣ�xQueue1,���ն���1�����ݡ�
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  *
***************************************************************/

static void vTaskLED2(void *pvParameters)
{
  BaseType_t xResult;
 // uint8_t  *ucQueueMsgValue;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(300); /* �������ȴ�ʱ��Ϊ300ms */
  uint8_t ucQueueMsgValue;

  while(1)
  {
   xResult = xQueueReceive(xQueue1,                   /* ��Ϣ���о�� */
                          (void *)&ucQueueMsgValue,  /* �洢���յ������ݵ�����ucQueueMsgValue�� */
                          (TickType_t)xMaxBlockTime);/* ��������ʱ�� */
  
    if(xResult == pdPASS)
    {
      /* �ɹ����գ���ͨ�����ڽ����ݴ�ӡ���� */
      printf("���յ���Ϣ��������ucQueueMsgValue = %#x\r\n", ucQueueMsgValue);
	  STEPMOTOR_AxisMoveRel(-1*6400*5, set_speed); //CCW��ʱ�뷽����ת
	  printf("Thi is stepmotor run \n");
	 
    }
    else
    {
      LED2_TOGGLE;
    }
  }
}

/**
  * ��������: LED3����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
static void vTaskLED3(void *pvParameters)
{
    while(1)
    {
      LED3_TOGGLE;
	  STEPMOTOR_AxisMoveRel(1*6400*5, set_speed);  //CW˳ʱ�뷽����ת
      vTaskDelay(1000);
    }
}

/**
  * ��������: ��������Ӧ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
static void AppTaskCreate (void)
{

    xTaskCreate( vTaskTaskUserIF,   	/* ������  */
                 "vTaskUserIF",     	/* ������    */
                 512,               	/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              	/* �������  */
                 1,                 	/* �������ȼ�*/
                 &xHandleTaskUserIF );  /* ������  */
	
    xTaskCreate( vTaskLED1,   	      /* ������  */
                 "vTaskLED1",     	  /* ������    */
                 512,               	/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              	/* �������  */
                 2,                 	/* �������ȼ�*/
                 &xHandleTaskLED1 );  /* ������  */
	
	
	xTaskCreate( vTaskLED2,    		      /* ������  */
                 "vTaskLED2",  		    /* ������    */
                 1024,         		    /* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,        		    /* �������  */
                 3,           		    /* �������ȼ�*/
                 &xHandleTaskLED2 );  /* ������  */
	
	xTaskCreate( vTaskLED3,     		    /* ������  */
                 "vTaskLED3",   		  /* ������    */
                 512,             		/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,           		  /* �������  */
                 4,               		/* �������ȼ�*/
                 &xHandleTaskLED3 );  /* ������  */
	
}

/***************************************************
 *
 *�������ƣ�AppObjCreate(void)
 *�������ܣ�������Ϣ���С�
 *
 *
****************************************************/
static void AppObjCreate (void)
{
	/* ����10��uint8_t����Ϣ���� */
	xQueue1 = xQueueCreate(10, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������� */
    }
	
	/* ����10���洢ָ���������Ϣ���У�����CM3/CM4�ں���32λ����һ��ָ�����ռ��4���ֽ� */
	xQueue2 = xQueueCreate(10, sizeof(struct Msg *));
    if( xQueue2 == 0 )
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������� */
    }
}
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
