#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int

void delay(uint time);
void write_com(uchar com);
void write_data(uchar datas);

void init();

sbit en = P3^4;
sbit rs = P3^5;

uchar table[] = "2017-04-06 SET",*p = table;

void main(){

	uchar i = 0;
	
	init();	

	while(*p){
		i++;
		if(i>15){
			write_com(0x80+0x40);
			i=0;
		}
		write_data(*p);
		p++;
	}

	while(1);
}

/*初始化*/
void init(){

	en = 0;
	write_com(0x38);
	write_com(0x0c);
	write_com(0x06);
	write_com(0x01);
	write_com(0x80);

}

/*延时*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}
}

/*写指令*/
void write_com(uchar com){
	rs = 0;
	P0 = com;
	delay(5);
	en = 1;
	delay(5);
	en = 0;
}

/*写数据*/	  
void write_data(uchar datas){
	rs = 1;
	P0 = datas;
	delay(5);
	en = 1;
	delay(5);
	en = 0;
}
