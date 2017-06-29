#include <reg51.h>
#include <absacc.h>
#define DAC0832 XBYTE[0x7fff]

void delay(){
	int i;
	for(i=0;i<400;i++);
}

void main(){
	while(1){
		DAC0832 = 1;
		delay();
		DAC0832 = 0;
		delay();
	}
}