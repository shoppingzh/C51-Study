#include <reg51.h>
#include <intrins.h>
#define uchar unsigned char 
#define uint unsigned int

#define HOUR_PORT P1
#define MINUTE_PORT P2
#define SECOND_PORT P3

void delay(uchar time);
void delay500ms();
uint get_time_code(uint time);
uchar key_scan();

void init_time(uint h,uint m,uint s);
void display_time(uint h,uint m,uint s);
void timer_init();
void refresh_time();
void warning();
void toggle_status();

//时分秒
uint hour,minute,second;

uint count = 0;

//设置状态
bit setting = 0;

//时分秒的每一个二进制位
sbit h1 = P1^0;
sbit h2 = P1^1;
sbit h3 = P1^2;
sbit h4 = P1^3;
sbit h5 = P1^4;
sbit m1 = P2^0;
sbit m2 = P2^1;
sbit m3 = P2^2;
sbit m4 = P2^3;
sbit m5 = P2^4;
sbit m6 = P2^5;
sbit s1 = P3^0;
sbit s2 = P3^1;
sbit s3 = P3^2;
sbit s4 = P3^3;
sbit s5 = P3^4;
sbit s6 = P3^5;

//当前正在设置状态的位
uchar curr_set = 0;

sbit set_led = P0^0;

void main(){

	uchar key;
	init_time(14,01,07);
	timer_init();
	TR0 = 1;
	
	while(1){
		key = key_scan();
		switch(key){
			case 0xe0:
			   	if(!setting){
					setting = 1;
					set_led = 0x00;
					TR0 = 0;
					warning();
				}else{
					setting = 0;
					set_led = 0xff;
					curr_set = 0;
					//时间保存
					hour = 0xff - HOUR_PORT;
					minute = 0xff - MINUTE_PORT;
					second = 0xff - SECOND_PORT;

					TR0 = 1;		
				}
				break;
			case 0xb0:
				if(setting){
				 	if(curr_set >= 16){
						curr_set = 0;
					}else{
						curr_set++;
					}
					warning();	
				}
				break;
			case 0x70:
				if(setting){
					if(curr_set <= 0){
						curr_set = 16;
					}else{
						curr_set--;
					}
					warning();
				}
				break;
			case 0xd0:
				if(setting){
					toggle_status();
				}
				break;
		}				
	}

}

/*定时器中断*/
void timer0_int() interrupt 1{
	TH0 = 0x3c;
	TL0 = 0xb0;
	if(count++ != 18){
		return;
	}

	//刷新时间
	refresh_time();
	display_time(hour,minute,second);
	
	count = 0;
}

/*初始化时间*/
void init_time(uint h,uint m,uint s){
	display_time(h,m,s);
}

/*显示时间*/
void display_time(uint h,uint m,uint s){
	hour = h;
	minute = m;
	second = s;
	HOUR_PORT  = get_time_code(h);
	MINUTE_PORT  = get_time_code(m);
	SECOND_PORT  = get_time_code(s);
}

/*定时器初始化*/
void timer_init(){
	TMOD = 0x01;
	TH0 = 0x3c;
	TL0 = 0xb0;
	EA = 1;
	ET0 = 1;
}

/*刷新时间*/
void refresh_time(){
	if(second != 59){
		second++;
	   	return;
	}

	second = 0;
	if(minute != 59){
		minute++;
		return;
	}

	minute = 0;
	if(hour != 23){
		hour++;
		return;
	}

	hour = 0;

}

/*警示当前位置*/
void warning(){
	uint i;
	switch(curr_set){
		case 0:
			for(i=0;i<4;i++){
				s1 = !s1;
				delay500ms();
			}
			break;
		case 1:
			for(i=0;i<4;i++){
				s2 = !s2;
				delay500ms();
			}
			break;
		case 2:
			for(i=0;i<4;i++){
				s3 = !s3;
				delay500ms();
			}
			break;
		case 3:
			for(i=0;i<4;i++){
				s4 = !s4;
				delay500ms();
			}
			break;
		case 4:
			for(i=0;i<4;i++){
				s5 = !s5;
				delay500ms();
			}
			break;
		case 5:
			for(i=0;i<4;i++){
				s6 = !s6;
				delay500ms();
			}
			break;

		case 6:
			for(i=0;i<4;i++){
				m1 = !m1;
				delay500ms();
			}
			break;
		case 7:
			for(i=0;i<4;i++){
				m2 = !m2;
				delay500ms();
			}
			break;
		case 8:
			for(i=0;i<4;i++){
				m3 = !m3;
				delay500ms();
			}
			break;
		case 9:
			for(i=0;i<4;i++){
				m4 = !m4;
				delay500ms();
			}
			break;
		case 10:
			for(i=0;i<4;i++){
				m5 = !m5;
				delay500ms();
			}
			break;
		case 11:
			for(i=0;i<4;i++){
				m6 = !m6;
				delay500ms();
			}
			break;

		case 12:
			for(i=0;i<4;i++){
				h1 = !h1;
				delay500ms();
			}
			break;
		case 13:
			for(i=0;i<4;i++){
				h2 = !h2;
				delay500ms();
			}
			break;
		case 14:
			for(i=0;i<4;i++){
				h3 = !h3;
				delay500ms();
			}
			break;
		case 15:
			for(i=0;i<4;i++){
				h4 = !h4;
				delay500ms();
			}
			break;
		case 16:
			for(i=0;i<4;i++){
				h5 = !h5;
				delay500ms();
			}
			break;
	}		
}

/*改变当前设置位的状态*/
void toggle_status(){
	switch(curr_set){
		case 0:
			s1 = !s1;
			break;
		case 1:
			s2 = !s2;
			break;
		case 2:
			s3 = !s3;
			break;
		case 3:
			s4 = !s4;
			break;
		case 4:
			s5 = !s5;
			break;
		case 5:
			s6 = !s6;
			break;

		case 6:
			m1 = !m1;
			break;
		case 7:
			m2 = !m2;
			break;
		case 8:
			m3 = !m3;
			break;
		case 9:
			m4 = !m4;
			break;
		case 10:
			m5 = !m5;
			break;
		case 11:
			m6 = !m6;
			break;

		case 12:
			h1 = !h1;
			break;
		case 13:
			h2 = !h2;
			break;
		case 14:
			h3 = !h3;
			break;
		case 15:
			h4 = !h4;
			break;
		case 16:
			h5 = !h5;
			break;	
	}
}

/***************************************功能函数***************************************/

/*延时*/
void delay(uchar time){
	uchar x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}	
}

/*延时500ms*/
void delay500ms(){
	uchar i, j, k;

	_nop_();
	_nop_();
	i = 2;
	j = 5;
	k = 5;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/*获取指定时间对应的端口状态编码*/
uint get_time_code(uint time){
	return 0xff - time;	
}

/*独立键盘扫描*/
uchar key_scan(){
	uchar key;
	if((P0 & 0xf0) != 0xf0){
		 delay(1);
		 if((P0 & 0xf0) != 0xf0){
		 	key = P0 & 0xf0;
		 	while((P0 | 0x0f) != 0xff);
		 	return key;
		 }
	}

	return 0;
}

