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

/**������д��һ������*/
void ser_write_data(int dat){
	SBUF = dat;
	while(!TI);
	TI = 0;
	delay();
}

/**���ڳ�ʼ��*/
void serial_init(){
	TMOD = 0x20;
	SCON = 0x40;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;
}

/**��ʱ*/
void delay(){
	int x,y;
	for(x=0;x<400;x++){
		for(y=0;y<10;y++);
	}
}