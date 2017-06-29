#include <reg51.h>
#include <stdio.h>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int

//矩阵键盘端口
#define KEY_BOARD_PORT P1
//1602数据端口
#define DISPLAY_OUT P2
//1602初始地址
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

//键盘编码
uchar code key_pad_code[] = {0x7e,0x7d,0xbe,0xbd,0xbb,0xde,0xdd,0xdb,0xee,0xed,0xeb,0x77,0xb7,0xd7,0xe7,0x7b};
//键盘编码所表示的字符
uchar code key_match_ch[] = {'c','0','1','2','3','4','5','6','7','8','9','+','-','*','/','='};

//一次计算的内容
uchar contents[16];
//当前位置
uint pos = 0;

//当前计算结果
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
				//按下清零键，清屏
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
				//控制程序不允许出现'01'这样的数值
				if(curr_ch != '0' && contents[pos-1] == '0' && is_operator(contents[pos-2])){
					write_com(read_pointer()-1);
				}
				if(check(curr_ch)){
					if(pos == 0){
						//可以重写
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
				//计算完成，重置
				reset();
				break; 
		}		
	}
	

}

/*将当前结果转换为字符串*/
uchar* to_string(int result){
	uchar* result_str;
	sprintf(result_str,"%d",result);
	return result_str;
}

/*计算当前内容的运算结果*/
int calculate(uchar* contents){
	// op_idx：运算符的位置
	int i,op_idx;
	uint head = 0,tail = 0,k;
	if(pos == 0){
		return result;
	}
	//找出运算符位置
	for(i=0;i<pos;i++){
		if(is_operator(contents[i])){
			op_idx = i;
			break;
		} 	
	}

	//计算头操作数
	k = 1;
   	for(i=op_idx-1;i>=0;i--){
		uint digit = contents[i] - 48;
		head += (k * digit);
	//	P0 = head;
		k *= 10;		
	}
	
	//计算尾操作数
	k = 1;
	for(i=pos-1;i>op_idx;i--){
		tail += k * (contents[i] - 48);
		k *= 10;
	}

	//计算
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

/*根据键盘编码值获取对应字符*/
uchar get_char(uchar key){
	uchar i;
	for(i=0;i<16;i++){
		if(key == key_pad_code[i]){
			return key_match_ch[i];
		}
	}
	return '\0';
}

/*判断当前字符是否为运算符*/
uchar is_operator(uchar ch){
	if(ch == '+' || ch == '-' || ch == '*' || ch == '/'){
		return 1;	
	}
	return 0;
}

/*检查是否可以将按键内容存储以及是否可以打印在显示屏上*/
uint check(uchar ch){
	uint op_count = 0,i;
	//总运算长度不能超过16
	if(pos > 16){
		return 0;
	}
	//上一个字符是0，则下一个字符不能还为0
	if(contents[pos-1] == '0' && ch == '0'){
		return 0;
	}
	//第一个输入的数不能是运算符和0
	if(pos == 0 && (is_operator(ch) || ch == '0') ){
		return 0;
	}	
	for(i=0;i<pos;i++){
		if(is_operator(contents[i])){
			op_count++;
		} 
	}
	//当前运算已有运算符，则不能再按运算符
	if(op_count >= 1 && is_operator(ch)){
		return 0;
	}
	return 1;
}

/*重置，开始下一个运算*/
void reset(){
	uchar i;
	write_com(BASIC_POSITION);
	pos = 0;
	for(i=0;i<pos;i++){
		contents[i] = '\0';
	}
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

/*系统初始化*/
void init(){
	display_init();
	delay(100);
	write_string("000000");
}

/*键盘扫描*/
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

/*延时*/
void delay(uchar time){
	uchar x,y;
	for(x=0;x<time;x++){
		for(y=0;y<50;y++);
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