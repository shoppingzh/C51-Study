#define DISPLAY_OUT P2
#define uchar unsigned char
#define uint unsigned int

sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;

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