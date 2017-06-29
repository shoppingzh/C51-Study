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

	//������ʱ��ʱ����ָ��ʱ�䷢һ������
	TH0 = 0x3c;
	TL0 = 0xb0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;	
	
	while(1);
}

//��ʱ��0�жϷ����ӳ���
void timer0_int() interrupt 1{
	TH0 = 0x3c;
	TL0 = 0xb0;
	count++;

	if(count >= 10){
		count = 0;
		P2 = leds;
		SBUF = leds;
		//�ȴ��������
		while(!TI);
		TI = 0;
		leds = _crol_(leds,1);
	}
}