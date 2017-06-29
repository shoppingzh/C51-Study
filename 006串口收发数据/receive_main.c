#include <reg51.h>
#define uchar unsigned char

void main(){
	uchar dat;

	TMOD = 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	SCON = 0x50;
	PCON = 0x00;
	TR1 = 1;

   	
	while(1){
		while(!RI);
		RI = 0;
		dat = SBUF;
		//将接收到的数据用于显示
		P0 = dat;	
	}
}