#include <reg51.h>
#include <stdio.h>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int

//������̶˿�
#define KEY_BOARD_PORT P1
//1602���ݶ˿�
#define DISPLAY_OUT P2
//1602��ʼ��ַ
#define BASIC_POSITION 0x80
#define SEC_LINE_POSITION BASIC_POSITION+0x40

void delay(uchar time);
void display_init();
void init();
uchar key_scan();
void reset();
uint check(uchar ch);
uchar get_char(uchar key);
int calculate(uchar* contents);
uchar* to_string(int result);
uchar is_operator(uchar ch);

void write_com(uchar com);
void write_data(uchar datas);
void write_data_pos(uint pos,uchar datas);
void write_string(uchar *str);
uchar read_data();
uchar read_data_pos(uint pos);
uint read_pointer();

sbit P10 = P1^0;
sbit P11 = P1^1;
sbit P12 = P1^2;
sbit P13 = P1^3;

//1602
sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;

//���̱���
uchar code key_pad_code[] = {0x7e,0x7d,0xbe,0xbd,0xbb,0xde,0xdd,0xdb,0xee,0xed,0xeb,0x77,0xb7,0xd7,0xe7,0x7b};
//���̱�������ʾ���ַ�
uchar code key_match_ch[] = {'c','0','1','2','3','4','5','6','7','8','9','+','-','*','/','='};

//һ�μ��������
uchar contents[16];
//��ǰλ��
uint pos = 0;

//��ǰ������
int result;

void main(){

	uchar key;
	init();

	while(1){
		uchar curr_ch;
		int result;		
		key = key_scan();
		P0 = key;

		switch(key){
			// on/c
			case 0x7e: 
				//���������������
				write_com(0x01);
				delay(10);
				write_data_pos(BASIC_POSITION,'0');
				reset();
				break;
			// 0
			case 0x7d:
			// 1
			case 0xbe:
			// 2
			case 0xbd:
			// 3
			case 0xbb:
			// 4
			case 0xde:
			// 5
			case 0xdd:
			// 6
			case 0xdb:
			// 7
			case 0xee:
			// 8
			case 0xed:
			// 9
			case 0xeb:
			// +
			case 0x77:
			// -
			case 0xb7:
			// *
			case 0xd7:
			// /
			case 0xe7:
				curr_ch = get_char(key);
				//���Ƴ����������'01'��������ֵ
				if(curr_ch != '0' && contents[pos-1] == '0' && is_operator(contents[pos-2])){
					write_com(read_pointer()-1);
				}
				if(check(curr_ch)){
					if(pos == 0){
						//������д
						write_com(0x01);
						delay(10);
						write_data_pos(BASIC_POSITION,curr_ch);
					}else{
						write_data(curr_ch);
					}
					contents[pos++] = curr_ch;	
				}
				break;
			// =
			case 0x7b:
				result = calculate(contents);
				//P0 = result;
				write_data_pos(SEC_LINE_POSITION,'=');
				write_string(to_string(result));
				//������ɣ�����
				reset();
				break; 
		}		
	}
	

}

/*����ǰ���ת��Ϊ�ַ���*/
uchar* to_string(int result){
	uchar* result_str;
	sprintf(result_str,"%d",result);
	return result_str;
}

/*���㵱ǰ���ݵ�������*/
int calculate(uchar* contents){
	// op_idx���������λ��
	int i,op_idx;
	uint head = 0,tail = 0,k;
	if(pos == 0){
		return result;
	}
	//�ҳ������λ��
	for(i=0;i<pos;i++){
		if(is_operator(contents[i])){
			op_idx = i;
			break;
		} 	
	}

	//����ͷ������
	k = 1;
   	for(i=op_idx-1;i>=0;i--){
		uint digit = contents[i] - 48;
		head += (k * digit);
	//	P0 = head;
		k *= 10;		
	}
	
	//����β������
	k = 1;
	for(i=pos-1;i>op_idx;i--){
		tail += k * (contents[i] - 48);
		k *= 10;
	}

	//����
	switch(contents[op_idx]){
		case '+':
			result = head + tail;
			break;
		case '-':
			result = head - tail;
			break;
		case '*':
			result = head * tail;
			break;
		case '/':
			result = head / tail;
			break;
	}

	return result;
}

/*���ݼ��̱���ֵ��ȡ��Ӧ�ַ�*/
uchar get_char(uchar key){
	uchar i;
	for(i=0;i<16;i++){
		if(key == key_pad_code[i]){
			return key_match_ch[i];
		}
	}
	return '\0';
}

/*�жϵ�ǰ�ַ��Ƿ�Ϊ�����*/
uchar is_operator(uchar ch){
	if(ch == '+' || ch == '-' || ch == '*' || ch == '/'){
		return 1;	
	}
	return 0;
}

/*����Ƿ���Խ��������ݴ洢�Լ��Ƿ���Դ�ӡ����ʾ����*/
uint check(uchar ch){
	uint op_count = 0,i;
	//�����㳤�Ȳ��ܳ���16
	if(pos > 16){
		return 0;
	}
	//��һ���ַ���0������һ���ַ����ܻ�Ϊ0
	if(contents[pos-1] == '0' && ch == '0'){
		return 0;
	}
	//��һ����������������������0
	if(pos == 0 && (is_operator(ch) || ch == '0') ){
		return 0;
	}	
	for(i=0;i<pos;i++){
		if(is_operator(contents[i])){
			op_count++;
		} 
	}
	//��ǰ��������������������ٰ������
	if(op_count >= 1 && is_operator(ch)){
		return 0;
	}
	return 1;
}

/*���ã���ʼ��һ������*/
void reset(){
	uchar i;
	write_com(BASIC_POSITION);
	pos = 0;
	for(i=0;i<pos;i++){
		contents[i] = '\0';
	}
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

/*ϵͳ��ʼ��*/
void init(){
	display_init();
	delay(100);
	write_string("000000");
}

/*����ɨ��*/
uchar key_scan(){  
	uchar key,i;
	KEY_BOARD_PORT = 0x0f;
	if((KEY_BOARD_PORT & 0x0f) == 0x0f){
		return -1;
	}
	delay(1);

	for(i=0;i<4;i++){
		KEY_BOARD_PORT = _cror_(0x7f,i);
		if(P10 == 0) break;
		if(P11 == 0) break;
		if(P12 == 0) break;
		if(P13 == 0) break;
	}
	key = KEY_BOARD_PORT;
	for(;KEY_BOARD_PORT!=0x0f;KEY_BOARD_PORT=0x0f) ;
	//while((KEY_BOARD_PORT & 0x0f) == 0x0f);
	return key;
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