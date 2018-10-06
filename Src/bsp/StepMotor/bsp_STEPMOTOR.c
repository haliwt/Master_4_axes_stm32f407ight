
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "StepMotor/bsp_STEPMOTOR.h" 
#include <math.h>
/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
TIM_HandleTypeDef htimx_STEPMOTOR;
speedRampData srd               = {STOP,CW,0};         // �Ӽ������߱���
__IO int32_t  step_position     = 0;           // ��ǰλ��
__IO uint8_t  MotionStatus      = 0;           //�Ƿ����˶���0��ֹͣ��1���˶�

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ���������GPIO��ʼ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
static void STEPMOTOR_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct; 
  
  /* ���Ŷ˿�ʱ��ʹ�� */
  STEPMOTOR_TIM_GPIO_CLK_ENABLE();
  STEPMOTOR_DIR_GPIO_CLK_ENABLE();
  STEPMOTOR_ENA_GPIO_CLK_ENABLE();
  
  /* �����������������IO��ʼ�� */
  GPIO_InitStruct.Pin = STEPMOTOR_TIM_PUL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AFx_TIMx;        // GPIO��������TIM���ù���
  HAL_GPIO_Init(STEPMOTOR_TIM_PUL_PORT, &GPIO_InitStruct);
  
  /* �����������������IO��ʼ�� */
  GPIO_InitStruct.Pin = STEPMOTOR_DIR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;       // GPIO��������ϵͳĬ�Ϲ���
  HAL_GPIO_Init(STEPMOTOR_DIR_PORT, &GPIO_InitStruct);
  
  /* �������ѻ�ʹ�ܿ�������IO��ʼ�� */
  GPIO_InitStruct.Pin = STEPMOTOR_ENA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;       // GPIO��������ϵͳĬ�Ϲ���
  HAL_GPIO_Init(STEPMOTOR_ENA_PORT, &GPIO_InitStruct);
  
  STEPMOTOR_DIR_FORWARD();
  STEPMOTOR_OUTPUT_DISABLE();
}

/**
  * ��������: ��������ʱ����ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void STEPMOTOR_TIMx_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;             // ��ʱ��ʱ��
  TIM_OC_InitTypeDef sConfigOC;                          // ��ʱ��ͨ���Ƚ����
  
  STEPMOTOR_TIM_RCC_CLK_ENABLE();
  
  /* STEPMOTOR���GPIO��ʼ������ */
  STEPMOTOR_GPIO_Init();
  
  /* ��ʱ�������������� */
  htimx_STEPMOTOR.Instance = STEPMOTOR_TIMx;                          // ��ʱ�����
  htimx_STEPMOTOR.Init.Prescaler = STEPMOTOR_TIM_PRESCALER;           // ��ʱ��Ԥ��Ƶ��
  htimx_STEPMOTOR.Init.CounterMode = TIM_COUNTERMODE_UP;              // �����������ϼ���
  htimx_STEPMOTOR.Init.Period = STEPMOTOR_TIM_PERIOD;                 // ��ʱ������
  htimx_STEPMOTOR.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;          // ʱ�ӷ�Ƶ
  HAL_TIM_Base_Init(&htimx_STEPMOTOR);

  /* ��ʱ��ʱ��Դ���� */
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;       		// ʹ���ڲ�ʱ��Դ
  HAL_TIM_ConfigClockSource(&htimx_STEPMOTOR, &sClockSourceConfig);

  /* ��ʱ���Ƚ�������� */
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;                // �Ƚ����ģʽ����ת���
  sConfigOC.Pulse = 0xFFFF;                            // ������
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;           // �������
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_LOW;         // ����ͨ���������
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;           // ����ģʽ
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;       // ���е�ƽ
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;     // ����ͨ�����е�ƽ
  HAL_TIM_OC_ConfigChannel(&htimx_STEPMOTOR, &sConfigOC, STEPMOTOR_TIM_CHANNEL_x);
  /* ʹ�ܱȽ����ͨ�� */
  TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_DISABLE);
  
  /* ���ö�ʱ���ж����ȼ���ʹ�� */
  HAL_NVIC_SetPriority(STEPMOTOR_TIMx_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(STEPMOTOR_TIMx_IRQn);
  
  __HAL_TIM_CLEAR_FLAG(&htimx_STEPMOTOR, STEPMOTOR_TIM_FLAG_CCx);
  /* ʹ�ܶ�ʱ���Ƚ���� */
  __HAL_TIM_ENABLE_IT(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx);
  /* Enable the main output */
  __HAL_TIM_MOE_ENABLE(&htimx_STEPMOTOR);  
  HAL_TIM_Base_Start(&htimx_STEPMOTOR);// ʹ�ܶ�ʱ��
}

/**
  * ��������: ������ʱ��Ӳ����ʼ������
  * �������: htim_base��������ʱ���������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==STEPMOTOR_TIMx)
  {
    /* ������ʱ������ʱ��ʹ�� */
    STEPMOTOR_TIM_RCC_CLK_ENABLE();
  }
}

/**
  * ��������: ������ʱ��Ӳ������ʼ������
  * �������: htim_base��������ʱ���������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==STEPMOTOR_TIMx)
  {
    /* ������ʱ������ʱ�ӽ��� */
    STEPMOTOR_TIM_RCC_CLK_DISABLE();
    HAL_GPIO_DeInit(STEPMOTOR_TIM_PUL_PORT,STEPMOTOR_TIM_PUL_PIN);
    HAL_GPIO_DeInit(STEPMOTOR_DIR_PORT,STEPMOTOR_DIR_PIN);
    HAL_GPIO_DeInit(STEPMOTOR_ENA_PORT,STEPMOTOR_ENA_PIN);
    
    HAL_NVIC_DisableIRQ(STEPMOTOR_TIMx_IRQn);
  }
} 
/**
  * ��������: ���λ���˶����˶������Ĳ���
  * �������: step���ƶ��Ĳ��� (����Ϊ˳ʱ�룬����Ϊ��ʱ��).
              speed  ����ٶ�,ʵ��ֵΪspeed*0.05*rad/sec
  * �� �� ֵ: ��
  * ˵    ��: �Ը����Ĳ����ƶ�����������ȼ��ٵ�����ٶȣ�Ȼ���ں���λ�ÿ�ʼ
  *           ������ֹͣ��ʹ�������˶�����Ϊָ���Ĳ���������Ӽ��ٽ׶κ̲ܶ���
  *           �ٶȺ������ǻ�û�ﵽ����ٶȾ�Ҫ��ʼ����
  */
void STEPMOTOR_AxisMoveRel( int32_t step, uint32_t speed)
{  
   uint16_t tim_count;
   if(MotionStatus != 0)
     return ;
	//�����趨
  if(step < 0) // ����Ϊ����
  {
    srd.dir = CCW; // ��ʱ�뷽����ת
    STEPMOTOR_DIR_REVERSAL();
    step =-step;   // ��ȡ��������ֵ
  }
  else
  {
    srd.dir = CW; // ˳ʱ�뷽����ת
    STEPMOTOR_DIR_FORWARD();
  }
  srd.step  = step;
  
  if(step != 0)  // ���Ŀ���˶�������Ϊ0
  {
    // ���ǵ��������û��ֲ�����ϸ�ļ��㼰�Ƶ�����

    // ��������ٶȼ���, ����õ�min_delay���ڶ�ʱ���ļ�������ֵ��
    // min_delay = (alpha / tt)/ w
    srd.min_delay = (int32_t)(A_T_x10/speed);
		srd.run_state = RUN;
  }
	MotionStatus = 1;
  tim_count=__HAL_TIM_GET_COUNTER(&htimx_STEPMOTOR);
  __HAL_TIM_SET_COMPARE(&htimx_STEPMOTOR,STEPMOTOR_TIM_CHANNEL_x,tim_count+srd.min_delay); // ���ö�ʱ���Ƚ�ֵ
  TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_ENABLE);// ʹ�ܶ�ʱ��ͨ�� 
  STEPMOTOR_OUTPUT_ENABLE();
}


/**
  * ��������: ��ʱ���жϷ�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ʵ�ּӼ��ٹ���
  */
void STEPMOTOR_TIMx_IRQHandler(void)//��ʱ���жϴ���
{ 
  uint32_t tim_count = 0;
  uint32_t tmp = 0;
  // ���ƶ�����������
   static uint32_t step_count = 0;
  //��ʱ��ʹ�÷�תģʽ����Ҫ���������жϲ����һ����������
   static uint8_t i=0;
  
  if(__HAL_TIM_GET_IT_SOURCE(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx) !=RESET)
  {
    // �����ʱ���ж�
    __HAL_TIM_CLEAR_IT(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx);
    
    // ���ñȽ�ֵ
    tim_count=__HAL_TIM_GET_COUNTER(&htimx_STEPMOTOR);
    tmp = tim_count+srd.min_delay;
    __HAL_TIM_SET_COMPARE(&htimx_STEPMOTOR,STEPMOTOR_TIM_CHANNEL_x,tmp);

    i++;     // ��ʱ���жϴ�������ֵ
    if(i==2) // 2�Σ�˵���Ѿ����һ����������
    {
      i=0;   // ���㶨ʱ���жϴ�������ֵ
      switch(srd.run_state) // �Ӽ������߽׶�
      {
        case STOP:
          step_count = 0;  // ���㲽��������
          MotionStatus = 0;// ֹͣ״̬
          // �ر�ͨ��
          TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_DISABLE);        
          __HAL_TIM_CLEAR_FLAG(&htimx_STEPMOTOR, STEPMOTOR_TIM_FLAG_CCx);
          STEPMOTOR_OUTPUT_DISABLE(); 
          break;
        case RUN:
          step_count++;  // ������1
          if(srd.dir==CW)
          {	  	
            step_position++; // ����λ�ü�1
          }
          else
          {
            step_position--; // ����λ�ü�1
          }
					if(step_count == srd.step)
					{
						srd.run_state = STOP;
					}
					break;
				
				default :break;
      }
    }
  }
}

