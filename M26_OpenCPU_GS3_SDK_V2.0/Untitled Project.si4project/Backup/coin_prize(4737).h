

#ifndef __COIN_PRIZE_H__
#define __COIN_PRIZE_H__

/**********************************************************************/
#define  NONC_CHECK_TIME   1000   //���������Զ�����ʱ�䣬��λms��
#define  COIN_MAX_VALUE    300    //������������

/**********************************************************************/
#define COIN_UP_PIN    PINNAME_NETLIGHT//PINNAME_RTS
#define COIN_IN_PIN    PINNAME_RI

#define COIN_PRIZE_PIN    PINNAME_DCD

//#define NETLIGHT_PIN   PINNAME_NETLIGHT

#define COIN_INPUT_CHECK  Ql_GPIO_GetLevel( COIN_IN_PIN )   
#define COIN_UP_GPIO_OUT_SET(x)  Ql_GPIO_SetLevel( COIN_UP_PIN, x )

#define COIN_PRIZE_CHECK  Ql_GPIO_GetLevel( COIN_PRIZE_PIN )    




/***��������***********************************************************/
typedef enum { OPEN = 0, CLOSE = 1 } PLUSE_MODE;

/**********************************************************************/
typedef struct
{
  //ע��POW:pulse out width(����������)�� POI:pulse out inerval(����������)�� 
  //    PIWmin:pulse int width minimum(������������Сֵ)��PIWmax:pulse in width maximum(�����������ֵ)��
  //    PIImin:pulse in minimum( ����������С��� )
  u8 CoinUp_POW;     //�ϱ�����������.
  u8 PrizeOut_POW;   //�˽����������� 
    
  u8 CoinUp_POI;      //�ϱ�������������
  u8 PrizeOut_POI;    //�˽�������������
  
  u8 CoinIn_PIWmin;    //Ͷ����������������Сֵ.
  u8 CoinIn_PIWmax;    //Ͷ�����������������ֵ.
  
  u8 PrizeOut_PIWmin;  //�˽�������������Сֵ.
  u16 PrizeOut_PIWmax;  //�˽��������������ֵ.
  
  u8 CoinIn_PIImin;     //Ͷ��������������С���.
  u8 PrizeOut_PIImin;   //�˽�����������С���.
  
  u8 PrizeOut_NONC;  //�˽��źų�������.
  u8 LocalSaveFlag;  //���ر����־.
  u8 CoinUp_NONC;    //Ͷ���źų�������.  
  
}IOdata_sut;  //ע������˽ṹ��ı�����ʱ���ʼ����

/**********************************************************************/
void gpioInit(void);
void Callback_CoinInPulseCheck( u32 timerId, void* param );
void Callback_CoinUpPulseOut( u32 timerId, void* param );
void bill_Write(u32 wirteAddr,Bill_sut *pNewBill, Bill_sut *pOldBill);
void bill_Read(void);


#endif



