#include <reg51.h>

void delay();
void serial_init();

void ser_write_data(int dat);

void main(){
	
	int i = 0;
	serial_init();
	while(1){
		ser_write_data(i++);
	}

}

/**往串口写入一个数据*/
void ser_write_data(int dat){
	SBUF = dat;
	while(!TI);
	TI = 0;
	delay();
}

/**串口初始化*/
void serial_init(){
	TMOD = 0x20;
	SCON = 0x40;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;
}

/**延时*/
void delay(){
	int x,y;
	for(x=0;x<400;x++){
		for(y=0;y<10;y++);
	}
}