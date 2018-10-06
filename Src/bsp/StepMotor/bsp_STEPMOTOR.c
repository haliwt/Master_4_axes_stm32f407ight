
/* 包含头文件 ----------------------------------------------------------------*/
#include "StepMotor/bsp_STEPMOTOR.h" 
#include <math.h>
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
TIM_HandleTypeDef htimx_STEPMOTOR;
speedRampData srd               = {STOP,CW,0};         // 加减速曲线变量
__IO int32_t  step_position     = 0;           // 当前位置
__IO uint8_t  MotionStatus      = 0;           //是否在运动？0：停止，1：运动

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 驱动器相关GPIO初始化配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
static void STEPMOTOR_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct; 
  
  /* 引脚端口时钟使能 */
  STEPMOTOR_TIM_GPIO_CLK_ENABLE();
  STEPMOTOR_DIR_GPIO_CLK_ENABLE();
  STEPMOTOR_ENA_GPIO_CLK_ENABLE();
  
  /* 驱动器脉冲控制引脚IO初始化 */
  GPIO_InitStruct.Pin = STEPMOTOR_TIM_PUL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AFx_TIMx;        // GPIO引脚用做TIM复用功能
  HAL_GPIO_Init(STEPMOTOR_TIM_PUL_PORT, &GPIO_InitStruct);
  
  /* 驱动器方向控制引脚IO初始化 */
  GPIO_InitStruct.Pin = STEPMOTOR_DIR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;       // GPIO引脚用做系统默认功能
  HAL_GPIO_Init(STEPMOTOR_DIR_PORT, &GPIO_InitStruct);
  
  /* 驱动器脱机使能控制引脚IO初始化 */
  GPIO_InitStruct.Pin = STEPMOTOR_ENA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_TRACE;       // GPIO引脚用做系统默认功能
  HAL_GPIO_Init(STEPMOTOR_ENA_PORT, &GPIO_InitStruct);
  
  STEPMOTOR_DIR_FORWARD();
  STEPMOTOR_OUTPUT_DISABLE();
}

/**
  * 函数功能: 驱动器定时器初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void STEPMOTOR_TIMx_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;             // 定时器时钟
  TIM_OC_InitTypeDef sConfigOC;                          // 定时器通道比较输出
  
  STEPMOTOR_TIM_RCC_CLK_ENABLE();
  
  /* STEPMOTOR相关GPIO初始化配置 */
  STEPMOTOR_GPIO_Init();
  
  /* 定时器基本环境配置 */
  htimx_STEPMOTOR.Instance = STEPMOTOR_TIMx;                          // 定时器编号
  htimx_STEPMOTOR.Init.Prescaler = STEPMOTOR_TIM_PRESCALER;           // 定时器预分频器
  htimx_STEPMOTOR.Init.CounterMode = TIM_COUNTERMODE_UP;              // 计数方向：向上计数
  htimx_STEPMOTOR.Init.Period = STEPMOTOR_TIM_PERIOD;                 // 定时器周期
  htimx_STEPMOTOR.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;          // 时钟分频
  HAL_TIM_Base_Init(&htimx_STEPMOTOR);

  /* 定时器时钟源配置 */
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;       		// 使用内部时钟源
  HAL_TIM_ConfigClockSource(&htimx_STEPMOTOR, &sClockSourceConfig);

  /* 定时器比较输出配置 */
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;                // 比较输出模式：反转输出
  sConfigOC.Pulse = 0xFFFF;                            // 脉冲数
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;           // 输出极性
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_LOW;         // 互补通道输出极性
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;           // 快速模式
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;       // 空闲电平
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;     // 互补通道空闲电平
  HAL_TIM_OC_ConfigChannel(&htimx_STEPMOTOR, &sConfigOC, STEPMOTOR_TIM_CHANNEL_x);
  /* 使能比较输出通道 */
  TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_DISABLE);
  
  /* 配置定时器中断优先级并使能 */
  HAL_NVIC_SetPriority(STEPMOTOR_TIMx_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(STEPMOTOR_TIMx_IRQn);
  
  __HAL_TIM_CLEAR_FLAG(&htimx_STEPMOTOR, STEPMOTOR_TIM_FLAG_CCx);
  /* 使能定时器比较输出 */
  __HAL_TIM_ENABLE_IT(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx);
  /* Enable the main output */
  __HAL_TIM_MOE_ENABLE(&htimx_STEPMOTOR);  
  HAL_TIM_Base_Start(&htimx_STEPMOTOR);// 使能定时器
}

/**
  * 函数功能: 基本定时器硬件初始化配置
  * 输入参数: htim_base：基本定时器句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==STEPMOTOR_TIMx)
  {
    /* 基本定时器外设时钟使能 */
    STEPMOTOR_TIM_RCC_CLK_ENABLE();
  }
}

/**
  * 函数功能: 基本定时器硬件反初始化配置
  * 输入参数: htim_base：基本定时器句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==STEPMOTOR_TIMx)
  {
    /* 基本定时器外设时钟禁用 */
    STEPMOTOR_TIM_RCC_CLK_DISABLE();
    HAL_GPIO_DeInit(STEPMOTOR_TIM_PUL_PORT,STEPMOTOR_TIM_PUL_PIN);
    HAL_GPIO_DeInit(STEPMOTOR_DIR_PORT,STEPMOTOR_DIR_PIN);
    HAL_GPIO_DeInit(STEPMOTOR_ENA_PORT,STEPMOTOR_ENA_PIN);
    
    HAL_NVIC_DisableIRQ(STEPMOTOR_TIMx_IRQn);
  }
} 
/**
  * 函数功能: 相对位置运动：运动给定的步数
  * 输入参数: step：移动的步数 (正数为顺时针，负数为逆时针).
              speed  最大速度,实际值为speed*0.05*rad/sec
  * 返 回 值: 无
  * 说    明: 以给定的步数移动步进电机，先加速到最大速度，然后在合适位置开始
  *           减速至停止，使得整个运动距离为指定的步数。如果加减速阶段很短并且
  *           速度很慢，那还没达到最大速度就要开始减速
  */
void STEPMOTOR_AxisMoveRel( int32_t step, uint32_t speed)
{  
   uint16_t tim_count;
   if(MotionStatus != 0)
     return ;
	//方向设定
  if(step < 0) // 步数为负数
  {
    srd.dir = CCW; // 逆时针方向旋转
    STEPMOTOR_DIR_REVERSAL();
    step =-step;   // 获取步数绝对值
  }
  else
  {
    srd.dir = CW; // 顺时针方向旋转
    STEPMOTOR_DIR_FORWARD();
  }
  srd.step  = step;
  
  if(step != 0)  // 如果目标运动步数不为0
  {
    // 我们的驱动器用户手册有详细的计算及推导过程

    // 设置最大速度极限, 计算得到min_delay用于定时器的计数器的值。
    // min_delay = (alpha / tt)/ w
    srd.min_delay = (int32_t)(A_T_x10/speed);
		srd.run_state = RUN;
  }
	MotionStatus = 1;
  tim_count=__HAL_TIM_GET_COUNTER(&htimx_STEPMOTOR);
  __HAL_TIM_SET_COMPARE(&htimx_STEPMOTOR,STEPMOTOR_TIM_CHANNEL_x,tim_count+srd.min_delay); // 设置定时器比较值
  TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_ENABLE);// 使能定时器通道 
  STEPMOTOR_OUTPUT_ENABLE();
}


/**
  * 函数功能: 定时器中断服务函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 实现加减速过程
  */
void STEPMOTOR_TIMx_IRQHandler(void)//定时器中断处理
{ 
  uint32_t tim_count = 0;
  uint32_t tmp = 0;
  // 总移动步数计数器
   static uint32_t step_count = 0;
  //定时器使用翻转模式，需要进入两次中断才输出一个完整脉冲
   static uint8_t i=0;
  
  if(__HAL_TIM_GET_IT_SOURCE(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx) !=RESET)
  {
    // 清楚定时器中断
    __HAL_TIM_CLEAR_IT(&htimx_STEPMOTOR, STEPMOTOR_TIM_IT_CCx);
    
    // 设置比较值
    tim_count=__HAL_TIM_GET_COUNTER(&htimx_STEPMOTOR);
    tmp = tim_count+srd.min_delay;
    __HAL_TIM_SET_COMPARE(&htimx_STEPMOTOR,STEPMOTOR_TIM_CHANNEL_x,tmp);

    i++;     // 定时器中断次数计数值
    if(i==2) // 2次，说明已经输出一个完整脉冲
    {
      i=0;   // 清零定时器中断次数计数值
      switch(srd.run_state) // 加减速曲线阶段
      {
        case STOP:
          step_count = 0;  // 清零步数计数器
          MotionStatus = 0;// 停止状态
          // 关闭通道
          TIM_CCxChannelCmd(STEPMOTOR_TIMx, STEPMOTOR_TIM_CHANNEL_x, TIM_CCx_DISABLE);        
          __HAL_TIM_CLEAR_FLAG(&htimx_STEPMOTOR, STEPMOTOR_TIM_FLAG_CCx);
          STEPMOTOR_OUTPUT_DISABLE(); 
          break;
        case RUN:
          step_count++;  // 步数加1
          if(srd.dir==CW)
          {	  	
            step_position++; // 绝对位置加1
          }
          else
          {
            step_position--; // 绝对位置减1
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

