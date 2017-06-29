#include <reg51.h>
#define uchar unsigned char

//0-9数码管段码表
uchar code digit_table[ ]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};

//pos:当前显示数字位于段码表中的索引
uchar count,pos;

void main(){
	pos = 0;

	TMOD = 0x21;
	TH1 = 0xfd;
	TL1 = 0xfd;
	SCON = 0x40;
	PCON = 0x00;
	TR1 = 1;

	EA = 1;
	ET0 = 1;
	TH0 = 0x3c;
	TL0 = 0xb0;
	TR0 = 1;

	while(1);
	
}

//获取下一个数字
int next_digit(){
	if(pos >= 9){
		pos = -1;
	}
	return digit_table[++pos];
}

void timer0_int() interrupt 1{
	TH0 = 0x3c;
	TL0 = 0xb0;
	count++;
	if(count >= 10){
		count = 0;
		SBUF = next_digit();
		while(!TI);
		TI = 0;
	}
}