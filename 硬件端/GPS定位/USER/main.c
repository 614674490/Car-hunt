#include "stm32f10x.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart2.h"
#include "usart.h"
#include "gps.h"
#include "gsm.h"
#include "usart3.h"
nmea_msg gpsx; 											//GPS��Ϣ
int main()
{
	u16 i,rxlen;
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���Բ���
	USART2_Init(115200);  //GPRSͨ��
	USART3_Init(9600);    //GPSͨ��
	LED_Init();
	led=1;
	Init();
	while(1)
	{
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			rxlen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���	   
			USART3_TX_BUF[rxlen]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART3_RX_BUF);//�����ַ���
			Gps_Get_Send_Data();		               //��ȡ��������Ϣ
			for(i=0;i<1000;i++)
    		USART3_RX_BUF[i]=0;
			USART3_RX_STA=0;		   	//������һ�ν���
			led=!led;
 		}
	}
}




