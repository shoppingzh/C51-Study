#include <reg51.H>
#define uchar unsigned char
#define uint unsigned int

#define ANALOG_CHANNEL P0
#define DEGITAL_OUT P1
#define DISPLAY_OUT P2

//1602Ƭ�Ļ���λ��
#define BASIC_POSITION 0x80
#define SECOND_LINE_FIRST_POSITION BASIC_POSITION+0x40
#define HIGH_POSITION BASIC_POSITION+8
#define MID_POSITION BASIC_POSITION+9
#define LOW_POSITION BASIC_POSITION+10

void timer_init();
void delay(uint time);

//1602
void write_com(uchar com);
void write_data(uchar datas);
void write_data_pos(uint pos,uchar datas);
void write_string(uchar *str);
uchar read_data();
uchar read_data_pos(uint pos);
uint read_pointer();
void display_init();

//1602
sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;

//ADC
sbit CLOCK = P3^4;
sbit ST = P3^0;
sbit OE = P3^1;
sbit EOC = P3^2;

int datas;

void main(){
	
	int low,mid,high;	

	timer_init();
	display_init();
	
	//ѡ��ģ��ͨ��0
	ANALOG_CHANNEL = 0x00;
	
	write_string("    Tem: ");
   	
	while(1){
		//����ADת��
		ST = 0;
		ST = 1;
		ST = 0;

		while(!EOC);
	
		OE = 1;
		datas = DEGITAL_OUT;
		low = datas % 10;
		mid = datas/10 % 10;
		high = datas / 100;
		write_data_pos(HIGH_POSITION,high+48);
		write_data_pos(MID_POSITION,mid+48);
		write_data_pos(LOW_POSITION,low+48);
		
		OE = 0;
	}
	

}

void timer0_int() interrupt 1{
	CLOCK = !CLOCK;
}

/**��ʱ����ʼ��*/
void timer_init(){
	TMOD = 0x02;
	TH0 = 0xc0;
	TL0 = 0xc0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

/*��ʱ*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}
}

/*********************************1602**********************************************/

/*1602Ƭ��ʼ��*/
void display_init(){
	en = 0;
	//Ĭ��Ϊд����
	rw = 0;
	//��������
	write_com(0x38);
	//��ʾ�򿪣�����ʾ���
	write_com(0x0c);
	//write_com(0x0f);
	//��дһ���ַ�ָ��͹��ͬʱ��1
	write_com(0x06);
	//����
	write_com(0x01);
	//�ӵ�һ��λ��ʼд
	write_com(0x80);
}

/*дָ��*/
void write_com(uchar com){
	rw = 0;
	rs = 0;
	DISPLAY_OUT = com;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*д����*/	  
void write_data(uchar datas){
	rw = 0;
	rs = 1;
	DISPLAY_OUT = datas;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*��ָ��λ��д����*/
void write_data_pos(uint pos,uchar datas){
	write_com(pos);
	write_data(datas);
}

/*д���ַ�������*/
void write_string(char *str){
	while(*str){
		write_data(*str);
		str++;
	}
}

/*��ȡ���ݣ�Ĭ�϶�ȡ��ǰ��괦�����ݣ�*/
uchar read_data(){
	uchar c = 0;
	DISPLAY_OUT = 0xff;  //Ϊʲô?
	rw = 1;
	rs = 1;
	delay(1);
	en = 1;
	//delay(1);
	c = DISPLAY_OUT;
	en = 0;
	//��ȡ���ݺ󣬽��������
	write_com(0x10);
	return c;
}

/*��ȡָ��λ�õ�����*/
uchar read_data_pos(uint pos){
	uchar c;
	uint pointer = read_pointer();
	write_com(pos);
	c = read_data();
	write_com(pointer);
	return c;
}

/*��ȡ��ǰָ���λ��*/
uint read_pointer(){
	uint read_pointer = 0;
	DISPLAY_OUT = 0xff; 
	rw = 1;
	rs = 0;
	delay(5);
	en = 1;
	delay(5);
	read_pointer = DISPLAY_OUT;
	en = 0;
	return read_pointer+BASIC_POSITION;	
}