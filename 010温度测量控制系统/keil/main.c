#include <reg51.h>
#define uchar unsigned char
#define uint unsigned int

//1602数据端口
#define DISPLAY_OUT P2
//1602初始地址
#define BASIC_POSITION 0x80
//温度值高中低位的显示位置
#define HIGH_POSITION BASIC_POSITION+9
#define MID_POSITION BASIC_POSITION+10
#define LOW_POSITION BASIC_POSITION+11
//ADC0808数字量输出端口
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

//温度值的高中低三位				 
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

		//将数据显示在1602片中
		write_data_pos(HIGH_POSITION,high+48);
		write_data_pos(MID_POSITION,mid+48);
		write_data_pos(LOW_POSITION,low+48);

		//将数据发送到串口，供Labview显示
		serial_write(get_truth(high));
		serial_write(get_truth(mid));
		serial_write(get_truth(low));
		delay(50);

		oe = 0;			
	}
	
	
}

/*真实发送给串口的数据（如发送数字1其实需要发送字符'1'，其ASCII码为49）*/
uint get_truth(uint x){
	if(x>= 0 && x <= 9){
		return x+48;
	}
	return x;
}

/*1602片初始化*/
void display_init(){
	en = 0;
	//默认为写操作
	rw = 0;
	//基础设置
	write_com(0x38);
	//显示打开，不显示光标
	write_com(0x0c);
	//write_com(0x0f);
	//读写一个字符指针和光标同时加1
	write_com(0x06);
	//清屏
	write_com(0x01);
	//从第一个位开始写
	write_com(0x80);
}

/*AD转换需要时钟，在该函数中开启一个定时器用于产生时钟信号*/
void adc_clock_init(){
	TMOD = 	0x22;
	TH0 = 0xc0;
	TL0 = 0xc0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

/*adc时钟信号定时器中断*/
void adc_clock_int() interrupt 1{
	clk = !clk;
}


/*串口初始化*/
void serial_init(){
	TMOD = 0x22;
	TH1 = 0xfd;
	TL1 = 0xfd;
	SCON = 0x40;
	PCON = 0x00;
	TR1 = 1;
}

/*AD转换*/
void ad_transform(){
	st = 0;
	st = 1;
	st = 0;
	while(!eoc);
}

/*串口发送数据*/
void serial_write(uchar dat){
	SBUF = dat;
	while(!TI);		 
	TI = 0;
	delay(1);	
}


/*--------------------------------------------------常用操作--------------------------------------------------*/

/*延时*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}
}


/*--------------------------------------------------1602--------------------------------------------------*/

/*写指令*/
void write_com(uchar com){
	rw = 0;
	rs = 0;
	DISPLAY_OUT = com;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*写数据*/	  
void write_data(uchar datas){
	rw = 0;
	rs = 1;
	DISPLAY_OUT = datas;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*在指定位置写数据*/
void write_data_pos(uint pos,uchar datas){
	write_com(pos);
	write_data(datas);
}

/*写入字符串数据*/
void write_string(char *str){
	while(*str){
		write_data(*str);
		str++;
	}
}

/*读取数据（默认读取当前光标处的数据）*/
uchar read_data(){
	uchar c = 0;
	DISPLAY_OUT = 0xff;  //为什么?
	rw = 1;
	rs = 1;
	delay(1);
	en = 1;
	//delay(1);
	c = DISPLAY_OUT;
	en = 0;
	//读取数据后，将光标拉回
	write_com(0x10);
	return c;
}

/*读取指定位置的数据*/
uchar read_data_pos(uint pos){
	uchar c;
	uint pointer = read_pointer();
	write_com(pos);
	c = read_data();
	write_com(pointer);
	return c;
}

/*读取当前指针的位置*/
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