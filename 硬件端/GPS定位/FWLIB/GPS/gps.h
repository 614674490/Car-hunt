#ifndef __GPS_H
#define __GPS_H	 
#include "sys.h"  
 									   						    
//GPS NMEA-0183协议重要参数结构体定义 
//卫星信息
__packed typedef struct  
{										    
 	u8 num;		//卫星编号
	u8 eledeg;	//卫星仰角
	u16 azideg;	//卫星方位角
	u8 sn;		//信噪比		   
}nmea_slmsg;  
//UTC时间信息
__packed typedef struct  
{										    
 	u16 year;	//年份
	u8 month;	//月份
	u8 date;	//日期
	u8 hour; 	//小时
	u8 min; 	//分钟
	u8 sec; 	//秒钟
}nmea_utc_time;   	   
//NMEA 0183 协议解析后数据存放结构体
__packed typedef struct  
{										    
 	u8 svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
	float latitude;				//纬度 分扩大100000倍,实际要除以100000  √
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬				        √
	float longitude;			    //经度 分扩大100000倍,实际要除以100000  √
	u8 ewhemi;					//东经/西经,E:东经;W:西经                   √
	u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6 ,正在估算. √				  
 	u8 posslnum;				//用于定位的卫星数,0~12.
 	u8 possl[12];				//用于定位的卫星编号
	u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位             √
	u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
	u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
	u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0 

	float altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	          √ 
  float direct;         //航向 000.0-359.9度 以真北为参考基准的地面航向
	float speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时√	 
}nmea_msg; 
extern nmea_msg gpsx;
extern u8 GPSData[100];
extern u16 GPSDataNum;
extern u8 ATData[100];
int NMEA_Str2num(u8 *buf,u8*dx);
void GPS_Analysis(nmea_msg *gpsx,u8 *buf);
void Gps_Get_Send_Data(void);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,u8 *buf);
#endif  

 



