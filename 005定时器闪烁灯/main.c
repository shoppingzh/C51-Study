#include <reg51.h>
#define uchar unsigned char 

uchar leds,count;

void main(){
	leds = 0x00;
	P2 = leds;

	TMOD = 0x01;
	TH0 = 0x3c;
	TL0 = 0xb0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;

	while(1);
}

void timer_int() interrupt 1{
	TH0 = 0x3c;
	TL0 = 0xb0;
	count++;
	if(count >= 10){
		leds = ~leds;
		P2 = leds;
		count = 0;
	}
}