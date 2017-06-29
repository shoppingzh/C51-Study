#include <reg51.h>
#include <stdio.h>
#define uchar unsigned char
#define uint unsigned int

#define BASIC_POSITION 0x80
//1602���ݶ˿�
#define DISPLAY_OUT P2

void display_init();
void write_com(uchar com);
void write_data(uchar datas);
void write_data_pos(uint pos,uchar datas);
void write_string(uchar *str);
uchar read_data();
uchar read_data_pos(uint pos);
uint read_pointer();


void delay(uchar time);

//1602
sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;


void main(){

	display_init();
	delay(100);
	
	write_com(BASIC_POSITION);
	write_data('n');
		
	while(1){}
	
}



/*��ʱ*/
void delay(uchar time){
	uchar x,y;
	for(x=0;x<time;x++){
		for(y=0;y<50;y++);
	}
}

/*--------------------------------------------------1602--------------------------------------------------*/

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