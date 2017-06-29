#include <reg51.h>

char str[] = "Hello,world!";

void delay(){
	int x,y;
	for(x=0;x<400;x++){
		for(y=0;y<10;y++);
	}
}

void main(){

	int i;

	TMOD = 0x20;
	SCON = 0x40;
	PCON = 0x00;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;

	for(i=0;i<12;i++){
		SBUF = str[i];
		while(!TI);
		TI = 0;
		delay();
	}

	while(1);
	
}

