#include <reg51.h>
#include <intrins.h>
#define uchar unsigned char

uchar leds,count;
						   
void main(){
	leds = 0xfe;

	TMOD = 0x21;
	TH1 = 0xfd;
	TL1 = 0xfd;
	SCON = 0x40;
	PCON = 0x00;
	TR1 = 1;

	//开启计时定时器，指定时间发一次数据
	TH0 = 0x3c;
	TL0 = 0xb0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;	
	
	while(1);
}

//定时器0中断服务子程序
void timer0_int() interrupt 1{
	TH0 = 0x3c;
	TL0 = 0xb0;
	count++;

	if(count >= 10){
		count = 0;
		P2 = leds;
		SBUF = leds;
		//等待发送完毕
		while(!TI);
		TI = 0;
		leds = _crol_(leds,1);
	}
}