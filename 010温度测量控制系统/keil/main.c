#include <reg51.h>
#define uchar unsigned char
#define uint unsigned int

//1602���ݶ˿�
#define DISPLAY_OUT P2
//1602��ʼ��ַ
#define BASIC_POSITION 0x80
//�¶�ֵ���е�λ����ʾλ��
#define HIGH_POSITION BASIC_POSITION+9
#define MID_POSITION BASIC_POSITION+10
#define LOW_POSITION BASIC_POSITION+11
//ADC0808����������˿�
#define DEGITAL_OUT P1

void display_init();
void adc_clock_init();
void serial_init();

void delay(uint time);
void ad_transform();
void serial_write(uchar dat);
uint get_truth(uint x);

void write_com(uchar com);
void write_data(uchar datas);
void write_data_pos(uint pos,uchar datas);
void write_string(uchar *str);
uchar read_data();
uchar read_data_pos(uint pos);
uint read_pointer();

//1602
sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;

//ADC0808
sbit clk = P3^4;
sbit eoc = P3^2;
sbit st = P0^6;
sbit oe = P0^7;

//�¶�ֵ�ĸ��е���λ				 
int tem,low,mid,high;

void main(){

	display_init();
	adc_clock_init();
	serial_init();

	write_string("    Tem: ");

	while(1){
		ad_transform();
		oe = 1;
		tem = DEGITAL_OUT;
		tem = tem%100;
		low = tem % 10;
		mid = tem/10 % 10;
		high = tem / 100;

		//��������ʾ��1602Ƭ��
		write_data_pos(HIGH_POSITION,high+48);
		write_data_pos(MID_POSITION,mid+48);
		write_data_pos(LOW_POSITION,low+48);

		//�����ݷ��͵����ڣ���Labview��ʾ
		serial_write(get_truth(high));
		serial_write(get_truth(mid));
		serial_write(get_truth(low));
		delay(50);

		oe = 0;			
	}
	
	
}

/*��ʵ���͸����ڵ����ݣ��緢������1��ʵ��Ҫ�����ַ�'1'����ASCII��Ϊ49��*/
uint get_truth(uint x){
	if(x>= 0 && x <= 9){
		return x+48;
	}
	return x;
}

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

/*ADת����Ҫʱ�ӣ��ڸú����п���һ����ʱ�����ڲ���ʱ���ź�*/
void adc_clock_init(){
	TMOD = 	0x22;
	TH0 = 0xc0;
	TL0 = 0xc0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

/*adcʱ���źŶ�ʱ���ж�*/
void adc_clock_int() interrupt 1{
	clk = !clk;
}


/*���ڳ�ʼ��*/
void serial_init(){
	TMOD = 0x22;
	TH1 = 0xfd;
	TL1 = 0xfd;
	SCON = 0x40;
	PCON = 0x00;
	TR1 = 1;
}

/*ADת��*/
void ad_transform(){
	st = 0;
	st = 1;
	st = 0;
	while(!eoc);
}

/*���ڷ�������*/
void serial_write(uchar dat){
	SBUF = dat;
	while(!TI);		 
	TI = 0;
	delay(1);	
}


/*--------------------------------------------------���ò���--------------------------------------------------*/

/*��ʱ*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
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