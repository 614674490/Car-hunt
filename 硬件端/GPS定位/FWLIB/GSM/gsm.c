#include "gsm.h"
#include "usart2.h" 
#include "delay.h"
#include "usart.h"
#include "gps.h"
#include "delay.h"
#include "string.h"
#include "led.h"
//----------------------------------------------------------------------------------------------------------------------------------------------
//���ַ����в���ָ���ַ�
//������sdat--Դ�ַ�����tdat--Ҫ���ҵ�ָ���ַ���num--Դ�ַ���Ҫ���ҵ����ݸ�����num2--ָ���ַ��������ݸ���
//����ֵ��1-1000:���ҳɹ���������ʼ�ֽ�����λ�ã�0:δ�ҵ�
u16 FindStr(u8 *sdat,u8 *tdat,u16 num,u16 num2)
{
	u16 i,j,k=0;
	for(i=0;i<num;i++)
	{
		k=0;
		if(*sdat==*tdat)
		{
			k=1;
			for(j=1;j<num2;j++)
			{
					if(*(sdat+j)==*(tdat+j))
							k++;
					else break;
			}
		}
		if(k==num2)
			break;
		sdat++;
	}
	if(k==num2)
		return (i+1);
	else return 0;
}

//�ȴ������ַ�������ʱʱ��10��
//����ֵ��0--��ȷ������--����
u8 WaitForStr(u8 *p,u16 num,u8 timeout)
{
	u8 temp;
	u8 i=0,j=0,tim=0,err=0;
	while(j<timeout)
	{
		if(USART2_RX_STA&0x8000)
		{
			temp=USART2_RX_STA&0x7fff;  //�õ����ݳ���
			USART2_RX_STA=0;
			i=FindStr(USART2_RX_BUF,p,temp,num);
			if(i>0)    
				break;            
		}
		delay_ms(100);
		tim++;
		if(tim>10)     //��ʱ
		{
			j++;
			tim=0;            
		}
	}
	if(j>=timeout)
		err=1;
	return err;
} 

//�ȴ�A9G��ʼ�����
//0����� 1��ʧ��
u8 A9G_Init(void)
{
	u8 temp;
	SendAT("AT+CREG?",8);   //SIM���Ƿ�ע������
	temp=WaitForStr("+CREG: 1,1",10,5);           //��ע�᱾������
	//temp=WaitForStr("READY",5,30);//READY
	return temp;
}
//GPS��ʼ��
//����ֵ 0���ɹ� 1��ʧ��
u8 GPS_Init(void)
{
	u8 temp;
	temp=GPS_ON();          //����GPS
	if(temp>0) return temp;
	SendAT("AT+GPSMD=1",10);   //����ΪGPSģʽ
	temp=WaitForStr("OK",2,2); 
	if(temp>0) return temp;
	return temp;
}

//GPRS����
////����ֵ 0���ɹ� 1��ʧ��
u8 i=0;
u8 SIM_Connect(void)
{
	u8 temp;
	i++;
	if(i>=6) 
	{
		i=0;
	}
	SendAT("AT+CCID",7);   //��ѯSIM���� ���SIM�Ƿ����
	temp=WaitForStr("OK",2,5); 
	if(temp>0) {printf("\r\nCCID Failed");/*return temp;*/}
	
	SendAT("AT+CREG?",8);   //SIM���Ƿ�ע������
	temp=WaitForStr("+CREG: 1,1",10,5);           //��ע�᱾������
	if(temp>0) temp=WaitForStr("+CREG: 1,5",10,5);   //ע������ ��������״̬
	if(temp>0) {printf("\r\nCREG Failed");return temp;}
	
	SendAT("AT+CGATT=1",10);   //�������� �����Ҫ���� ��ָ���ʡȥ
	temp=WaitForStr("OK",2,5); 
	if(temp>0) {printf("\r\nCGATT Failed");}
	
	SendAT("AT+CGDCONT=1,\"IP\",\"CMNET\"",25);   //����PDP����
	temp=WaitForStr("OK",2,5); 
	if(temp>0) {printf("\r\nCGDCONT Failed");}
	
	SendAT("AT+CGACT=1,1",12);   //����PDP
	temp=WaitForStr("OK",2,5); 
	if(temp>0) {printf("\r\nCGACT Failed");}

	SendAT("AT+CGACT?",9);           
	temp=WaitForStr("1, 1",4,5);
	if(temp>0) {printf("\r\nConnect Failed\r\n");return temp;}
	return temp;
}
//��ȡIMEI
//AT+GSN 867959033020180 OK
u8 IMEI[15];
void AssertIMEI(void)
{
	u8 i,j,k,tim=0,err=0,*p;
	u16 num;
	SendAT("AT+CGSN",7);
	while(1)
	{
		i=WaitForStr("GSN",3,3);
		if(i==0) break;
		delay_ms(20);
		tim++;
		if(tim>100)
		{
			err=1;                            //��ʱ���
			printf("\r\nOUT TIME!\r\n");
			break;
		}
	}
	if(err==0)  //�ǳ�ʱ
	{        
			p=USART2_RX_BUF;            
			k=0;
			for(j=0;j<30;j++)
			{
				p++;
				if(*p>0x2F && *p<0x3A)     //����
				{
					IMEI[k]=*p;
					k++;  		
				}
				if(k>=15) break;                 
			}
			sprintf((char*)ATData,"AT+HTTPGET=\"http://119.23.14.83?IMEI=%s\"",IMEI);
			num=strlen((char *)ATData);
			printf("\r\n%s\r\n",ATData);                      //������ʾ��Ҫ���͵�����
			SendAT(ATData,num);
			if(WaitForStr("OK",2,5)) 
			{
				printf("\r\nSend Error\r\n");
				SendAT("AT+CIPSTATUS?",13);                 //��ѯIP��·״̬
				if(WaitForStr("GPRSACT",7,5))
				{
					printf("\r\nSIM Error  Reconnect......\r\n");
					SIM_Connect();    //SIM��������
				}
				else  
				{
					i=0;
					printf("\r\nInternet Error\r\n");          //����������
				}
			}
			else  
			{
				printf("\r\nSend OK\r\n");
				SendAT("EXIT",4);
				if(WaitForStr("+CME ERROR",10,5)) printf("\r\nExit Failed\r\n");
				else printf("\r\nExit OK\r\n");
			}
	 }  
}
//HTTPGET ����GPS��Ϣ
void HttpGet(u8 *dat)
{
	static u8 check=0;
	u8 temp=0;
	u16 num;
	u16 t=360;
	//AT+HTTPGET="http://39.97.235.93:80/?execute=10&longitude=116.853543&latitude=38.354140&direction=59.3"
	sprintf((char*)ATData,"AT+HTTPGET=\"http://39.97.235.93:80/?execute=10&%s\"",dat);
	num=strlen((char *)ATData);
	printf("\r\n%s\r\n",ATData);                      //������ʾ��Ҫ���͵�����
	SendAT(ATData,num);
	delay_s(5);
	printf("\r\nSend Fininsh\r\n");
	check++;
	if(check>2)
	{
		SendAT("AT+CGACT?",9);           
		temp=WaitForStr("1, 1",4,5);
		if(temp) 
		{
			printf("\r\nConnect Failed\r\n");
			while(SIM_Connect());
		}
		check=0;
	}
}
//TCP����GPS����
void TcpGet(u8 *dat)
{
	u8 k=3;
	while(k--)           //��෢������
	{
		SendAT("AT+CIPSTART=\"TCP\",\"122.114.122.174\",34466",41);   //�������ſɵ�TCP����
		if(WaitForStr("CONNECT OK",10,5))
		{
			printf("\r\nTCP Connected Failed");
			SendAT("AT+CIPCLOSE",11);
	    if(WaitForStr("CLOSE",5,10))  printf("\r\nCLOSE FAILED");
	    else printf("\r\nLast Close Failed RECLOSE OK");
			SendAT("AT+CGACT?",9);
			if(WaitForStr("+CGACT: 1,1",11,5))    //����������
			{
				printf("\r\nSIM Connected Error");
				SIM_Connect();
			}
			else 
			{
				printf("\r\nInteret Error");
				i=0;
			}
		}
		else 
		{
			printf("\r\nTCP OK");
			k=3;
			break;                          //�˳�����ѭ��
		}
	}
	if(k==0) 
	{
		printf("Unsure Error!!!");
		//Init();                   //�޷�����Ĵ��� ȫ����ʼ��
	}
	SendAT("AT+CIPSEND",10);        
  WaitForStr(">",1,2);              //AT+CIPSTART="TCP","115.28.48.200",80 OK CONNECT OK   
  SendATData(dat);                 //������Ϣ
	if(WaitForStr("OK",2,5)) printf("\r\nSend Failed");
	else printf("\r\nSend OK");
	printf("\r\n%s",dat);                      //������ʾ��Ҫ���͵�����
	SendAT("AT+CIPCLOSE",11);
	if(WaitForStr("CLOSE",5,5))  printf("\r\nCLOSE FAILED\r\n");
	else printf("\r\nCLOSE OK\r\n");
	delay_ms(200);    //��ʱ ��ֹ�������ݵ�Ƶ�ʹ��쵼��A9G��Ӧ������
}
//����GPS
u8 GPS_ON(void)
{
	u8 temp;
	SendAT("AT+GPS=1",8);
	temp=WaitForStr("OK",2,2);	
	return temp;
}
u8 GPS_OFF(void)
{
	u8 temp;
	SendAT("AT+GPS=0",80);
	temp=WaitForStr("OK",2,2); 
	return temp;
}
u8 j=0;
void Init(void)
{
	led=0;
	delay_s(30);//�ȴ�A9G�ϵ�����ʱ����
	printf("A9G Start Init...\n");
	led=1;
	while(SIM_Connect())
	{
		     ;
	}
	printf("\r\nSIM Init OK\r\n");
	led=0;
	while(GPS_Init())
	{
		 printf("\r\nGPS Init...\r\n");
	}
	printf("\r\nGPS Init OK\r\n");
	led=1;
}





