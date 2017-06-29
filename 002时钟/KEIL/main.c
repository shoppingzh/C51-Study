#include <reg52.h>
#include <string.h>
#include <consts.h>
//�������ͱ���
#define uchar unsigned char
#define uint unsigned int	 

void delay(uint time);
uint to_int(uchar ch);
uchar to_char(uint i);
uint is_leap_year(uint year);

void write_com(uchar com);
void write_data(uchar datas);
void write_data_pos(uint pos,uchar datas);
void write_string(uchar *str);
uchar read_data();
uchar read_data_pos(uint pos);
uint read_pointer();

uint read_time(uint seq);
uint get_next_time(uint time,uint seq);
void refresh_time(uint seq);

uint check_init_data();

void display_init();
void timer_init();
void init();
uint key_scan();
void setting(uint key);
void print_date_time(uchar *date,uchar *time);

void start();
void stop();
void shutdown();
void storage_curr_time();
void adjust_time();
void confirm_time();
void shift_left();
void shift_right();
void digit_up();
void digit_down();

uint check_datetime();
void reset_time_from_store();

uint get_next_digit(uint curr_digit,uint curr_pointer);
uint get_prev_digit(uint curr_digit,uint curr_pointer);

struct DateTime get_date_time(uint *date_time);

//1602
sbit en = P3^5;
sbit rs = P3^6;
sbit rw = P3^7;
//��ʱ��
uchar count = 0;
//ÿ���µ���������
uint xdata month_day_arr[] = MONTH_DAY_ARR;
//����״̬��Ĭ�ϲ�������״̬
bit setting_status = 0;
//����ǰ��ʱ��״̬�����Ի�ԭʱ��
uint curr_date_time_arr[DATE_TIME_DIGIT_COUNT];
//����ʱ��ṹ��
typedef struct DateTime{
	uchar *date;
	uchar *time;
};

uint code position_arr[] = POSITION_ARRAY;
uint code position_max_value_arr[] = POSITION_MAX_VALUE_ARRAY;
uint code position_min_value_arr[] = POSITION_MIN_VALUE_ARRAY;

void main(){
	uint key;

	//��ʾ����ʼ��
	display_init();

	//���������ϼ���ʼ���ݣ����������ʾ������Ϣ
	if(check_init_data()){
		write_string(STRING_OUT_OF_BOUND);
		while(1);
	}	  
	//ʱ�ӳ�ʼ����д��ԭʼ����ʱ��
	init();
	timer_init();
	start();	

	//��ⰴ��
	while(1){
		key = key_scan();
		setting(key);
	}
}

/*��ʱ��50ms*/
void timer0() interrupt 1{
	if((count++) < 13){
		return;
	}
	count = 0;
	refresh_time(SECOND_SEQ);		
}

/*������ʱ*/
void start(){
	TR0 = 1;
}

/*ֹͣ��ʱ*/
void stop(){
	TR0 = 0;
}

/*�ػ�*/
void shutdown(){
	write_com(0x08);
}

/*ɨ�����*/
uint key_scan(){
	if((P1&0x3f) != 0x3f){
		delay(150);
		if((P1&0x3f) != 0x3f){
			return P1&0x3f;
		}
	}
	return 0;
}

/*����*/
void setting(uint key){
	switch(key){
		case 0x2f:
			if(setting_status){
				setting_status = 0;
				confirm_time();
			}else{
				setting_status = 1;
				//����ʱ��ǰ����ǰʱ��洢�������Ա���ԭʹ��
				storage_curr_time();
				adjust_time();	
			}
			break;
		//��������
		case 0x1f:
			//clock setting
			break;
		case 0x3d:
			if(setting_status){
				digit_up();
			}
			break;
		case 0x3b:
			shift_right();
			break;
		case 0x37:
			if(setting_status){
				digit_down();
			}
			break;
		case 0x3e:
			if(setting_status){
				shift_left();
			}
			break;	
		default:
			break;
	}	
}

/*���浱ǰ��ʱ����Ϣ*/
void storage_curr_time(){
	curr_date_time_arr[0] = read_time(YEAR_SEQ);
	curr_date_time_arr[1] = read_time(MONTH_SEQ);
	curr_date_time_arr[2] = read_time(DAY_SEQ);
	curr_date_time_arr[3] = read_time(HOUR_SEQ);
	curr_date_time_arr[4] = read_time(MINUTE_SEQ);
	curr_date_time_arr[5] = read_time(SECOND_SEQ);	
}

/*����ʱ��*/
void adjust_time(){
	//ֹͣ��ʱ����ʾ��꣬��������������ĵ�λλ��
	stop();
	write_com(0x0f);
   	write_com(SECOND_LOW_POSITION);
}

/*ȷ��ʱ�䣬����ʱ�����*/
void confirm_time(){
	//��ʼ��ʱ�����ع��
	write_com(0x0c);
	//ȷ��ʱ��ǰ�ȼ�������ʱ���Ƿ���ȷ
	if(!check_datetime()){
		//ʱ���޸Ĵ��󣬴Ӵ洢���ָ�
		print_date_time(" Setting error!",NULL);
		delay(2000);
		reset_time_from_store();
	}
	start();	
}

/*�������ʱ���Ƿ���Ϲ淶*/
uint check_datetime(){
	uint year,month,day,hour,minute,second;
	year = read_time(YEAR_SEQ);	
	month = read_time(MONTH_SEQ);
	day = read_time(DAY_SEQ);
	hour = read_time(HOUR_SEQ);
	minute = read_time(MINUTE_SEQ);
	second = read_time(SECOND_SEQ);
	//��鷶Χ�Ƿ񳬳�
	if(year > MAX_YEAR || year < MIN_YEAR){
		return 0;
	}
	if(month > MAX_MONTH || month < MIN_MONTH){
		return 0;
	}
	if(day > MAX_DAY || day < MIN_DAY){
		return 0;
	}
	if(hour > MAX_HOUR || hour < MIN_HOUR){
		return 0;
	}
	if(minute > MAX_MINUTE || minute < MIN_MINUTE){
		return 0;
	}
	if(second > MAX_SECOND || second < MIN_SECOND){
		return 0;
	}
	//����������·�Ϊ2�£��ж������Ƿ񳬳���Χ
	if(month == 2){
		if(is_leap_year(year)){
			if(day > FEBRUARY_LONG){
				return 0;
			}
		}else{
			if(day > FEBRUARY_SHORT){
				return 0;
			}
		}
	}

	return 1;
}

/*�Ӵ洢���ָ�ʱ��*/
void reset_time_from_store(){
	struct DateTime datetime;
	datetime = get_date_time(curr_date_time_arr);
	print_date_time(datetime.date,datetime.time);				
}

/*����ʾ���ϴ�ӡ���ں�ʱ��*/
void print_date_time(uchar *date,uchar *time){
	//��������ӡ���ڣ����У���ӡʱ��
	write_com(0x01);	
	write_string(date);
	write_com(SECOND_LINE_FIRST_POSITION);
	write_string(time);
}

/*ͨ������ʱ���������Ը���׼������ʱ���ʽ�ַ����ṹ��
,��ͨ��[2017,12,21,02,12,56]������date[]="2017-12-21",time[]="02:12:56"*/
struct DateTime get_date_time(uint *date_time){
	uint year,month,day,hour,minute,second;
	uchar date[13],time[12];
	struct DateTime datetime;

	year = date_time[0];	
	month = date_time[1];
	day = date_time[2];
	hour = date_time[3];
	minute = date_time[4];
	second = date_time[5];
	
	date[0] = ' ';
	date[1] = ' ';
	date[2] = ' ';
	date[3] = to_char(year/1000);
	date[4] = to_char(year%1000/100);
	date[5] = to_char(year%100/10);
	date[6] = to_char(year%10);
	date[7] = '-';	
	date[8] = to_char(month/10);
	date[9] = to_char(month%10);
	date[10] = '-';
	date[11] = to_char(day/10);
	date[12] = to_char(day%10);

	time[0] = ' ';
	time[1] = ' ';
	time[2] = ' ';
	time[3] = ' ';
	time[4] = to_char(hour/10);
	time[5] = to_char(hour%10);
	time[6] = ':';
	time[7] = to_char(minute/10);
	time[8] = to_char(minute%10);
	time[9] = ':';
	time[10] = to_char(second/10);
	time[11] = to_char(second%10);

   	datetime.date = date;
	datetime.time = time;
	return datetime;
}

/*����*/ 
void shift_left(){
	//�ƶ�����ݵĵ�һλ�����ƶ�λ�������λ
	//�ƶ���ʱ���λ�����ƶ�λ����ĵ�λ
	//���ƶ����ָ�����
	uint curr_point;
	uchar curr_ch;
	curr_point = read_pointer();
	if(curr_point == YEAR_ONE_POSITION){
		write_com(SECOND_LOW_POSITION);
		return;
	}
	if(curr_point == HOUR_HIGH_POSITION){
		write_com(DAY_LOW_POSITION);
		return;
	}
	curr_ch = read_data_pos(curr_point-1);
	if(curr_ch == ':' || curr_ch == '-'){
		write_com(0x10);	
	}
	write_com(0x10);		
}

/*����*/
void shift_right(){
	uint curr_point;
	uchar next_ch;
	curr_point = read_pointer();
	if(curr_point == DAY_LOW_POSITION){
		write_com(HOUR_HIGH_POSITION);
		return;
	}
	if(curr_point == SECOND_LOW_POSITION){
		write_com(YEAR_ONE_POSITION);
		return;
	}
	next_ch = read_data_pos(curr_point+1);
	//write_com(0x10);
	if(next_ch == ':' || next_ch == '-'){
		write_com(0x14);	
	}	

	write_com(0x14);	
}

/*��������*/
void digit_up(){
	uint curr_p;
	uchar curr_ch;
	curr_p = read_pointer();
	curr_ch = read_data();
	//write_com(0x10);
	write_data(to_char(get_next_digit(to_int(curr_ch),curr_p)));
	write_com(0x10);
}

/*��������*/
void digit_down(){
	uint curr_p;
	uchar curr_ch;
	curr_p = read_pointer();
	curr_ch = read_data();
	write_data(to_char(get_prev_digit(to_int(curr_ch),curr_p)));
	write_com(0x10);
}

/*��ȡ��һ������*/
uint get_next_digit(uint curr_digit,uint curr_pointer){
	uint next_digit,i;
	for(i=0;i<POSITION_ARRAY_LENGTH;i++){
		if(curr_pointer == position_arr[i]){
			//��ǰ���ֵ���������֣�����һ������Ϊ��С���֣������1����
			if(curr_digit == position_max_value_arr[i]){
				next_digit = position_min_value_arr[i];				
			}else{
				next_digit = curr_digit + 1;
			}
		}
	}
	return next_digit;
}

/*��ȡ��һ������*/
uint get_prev_digit(uint curr_digit,uint curr_pointer){
	uint prev_digit,i;
	for(i=0;i<POSITION_ARRAY_LENGTH;i++){
		if(curr_pointer == position_arr[i]){
			//��ǰ���ֵ�����С���֣�����һ������Ϊ������֣������1����
			if(curr_digit == position_min_value_arr[i]){
				prev_digit = position_max_value_arr[i];				
			}else{
				prev_digit = curr_digit - 1;
			}
		}
	}
	return prev_digit;
}

/*��ʼ��*/
void init(){
	//д���ʼ���ں�ʱ��
	print_date_time(DEFAULT_DATE,DEFAULT_TIME);
}

/*����ʼ������*/
uint check_init_data(){
	uchar *p_date = DEFAULT_DATE,*p_time = DEFAULT_TIME;
	if(strlen(p_date) > 16 || strlen(p_time) > 16){
		return 1;
	}
	return 0;
}							   

/*��ʱ����ʼ��*/
void timer_init(){
	TMOD = 0x01;
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	EA = 1;
	ET0 = 1;
}

/*ˢ��ʱ�䣨�������кţ�*/
void refresh_time(uint seq){
	uint curr_time,next_time;

	switch(seq){
		case YEAR_SEQ:
			curr_time = read_time(YEAR_SEQ);
			next_time = get_next_time(curr_time,YEAR_SEQ);
			write_data_pos(YEAR_ONE_POSITION,to_char(next_time/1000));
			write_data_pos(YEAR_TWO_POSITION,to_char(next_time%1000/100));
			write_data_pos(YEAR_THREE_POSITION,to_char(next_time%100/10));
			write_data_pos(YEAR_FOUR_POSITION,to_char(next_time%10));
			break;
		case MONTH_SEQ:
			curr_time = read_time(MONTH_SEQ);
			next_time = get_next_time(curr_time,MONTH_SEQ);
			write_data_pos(MONTH_HIGH_POSITION,to_char(next_time/10));
			write_data_pos(MONTH_LOW_POSITION,to_char(next_time%10));
			if(next_time == MIN_MONTH){
				refresh_time(YEAR_SEQ);
			}
			break;
		case DAY_SEQ:
			curr_time = read_time(DAY_SEQ);
			next_time = get_next_time(curr_time,DAY_SEQ);
			write_data_pos(DAY_HIGH_POSITION,to_char(next_time/10));
			write_data_pos(DAY_LOW_POSITION,to_char(next_time%10));
			if(next_time == MIN_DAY){
				refresh_time(MONTH_SEQ);
			}
			break;
		case WEEK_SEQ:
			//��ʱ��������
			break;
		case HOUR_SEQ:
			curr_time = read_time(HOUR_SEQ);
			next_time = get_next_time(curr_time,HOUR_SEQ);
			write_data_pos(HOUR_HIGH_POSITION,to_char(next_time/10));
			write_data_pos(HOUR_LOW_POSITION,to_char(next_time%10));
			if(next_time == MIN_HOUR){
				refresh_time(DAY_SEQ);
			}
			break;
		case MINUTE_SEQ:
			curr_time = read_time(MINUTE_SEQ);
			next_time = get_next_time(curr_time,MINUTE_SEQ);
			write_data_pos(MINUTE_HIGH_POSITION,to_char(next_time/10));
			write_data_pos(MINUTE_LOW_POSITION,to_char(next_time%10));
			if(next_time == MIN_MINUTE){
				refresh_time(HOUR_SEQ);
			}
			break;
		case SECOND_SEQ:
			curr_time = read_time(SECOND_SEQ);
			next_time = get_next_time(curr_time,SECOND_SEQ);
			write_data_pos(SECOND_HIGH_POSITION,to_char(next_time/10));
			write_data_pos(SECOND_LOW_POSITION,to_char(next_time%10));
			if(next_time == MIN_SECOND){
				refresh_time(MINUTE_SEQ);
			}
			break;
		default:
			break;
	}
}


/*�������кţ��Ѹ�������ȡ��һʱ�䣨������ʱ���룩*/
uint read_time(uint seq){
	uint time_high,time_low;
	//������⴦��
	uint year_one,year_two,year_three,year_four;
	switch(seq){
		case YEAR_SEQ:
			year_one = YEAR_ONE_POSITION;
			year_two = YEAR_TWO_POSITION;
			year_three = YEAR_THREE_POSITION;
			year_four = YEAR_FOUR_POSITION;
			return to_int(read_data_pos(year_one))*1000 + to_int(read_data_pos(year_two))*100 
					+ to_int(read_data_pos(year_three))*10 + to_int(read_data_pos(year_four));
			break;
		case MONTH_SEQ:
			time_high = MONTH_HIGH_POSITION;
			time_low = MONTH_LOW_POSITION;
			break;
		case DAY_SEQ:
			time_high =DAY_HIGH_POSITION;
			time_low = DAY_LOW_POSITION;
			break;
		case WEEK_SEQ:
			//��ʱ��������
			//time_high = WEEK_HIGH_POSITION;
			//time_low = WEEK_LOW_POSITION;
			break;
		case HOUR_SEQ:
			time_high = HOUR_HIGH_POSITION;
			time_low = HOUR_LOW_POSITION;
			break;
		case MINUTE_SEQ:
			time_high = MINUTE_HIGH_POSITION;
			time_low = MINUTE_LOW_POSITION;
			break;
		case SECOND_SEQ:
			time_high = SECOND_HIGH_POSITION;
			time_low = SECOND_LOW_POSITION;
			break;
		default:
			break;
	}
	//��ȡ����
	return to_int(read_data_pos(time_high))*10 + to_int(read_data_pos(time_low));
}

/*��ȡ��һ��ʱ��*/
uint get_next_time(uint time,uint seq){
	uint next_time,curr_mon;
	switch(seq){
		case YEAR_SEQ:
			if(time == MAX_YEAR){
				next_time = MIN_YEAR;
			}else{
				next_time = time + 1;
			}
			break;
		case MONTH_SEQ:
			if(time == MAX_MONTH){
				next_time = MIN_MONTH;
			}else{
				next_time = time + 1;
			}
			break;
		case DAY_SEQ:
			if(is_leap_year(read_time(YEAR_SEQ))){
				month_day_arr[1] = FEBRUARY_LONG;
			}else{
				month_day_arr[1] = FEBRUARY_SHORT;
			}
			curr_mon = read_time(MONTH_SEQ);
			if(time == month_day_arr[curr_mon-1]){
				next_time = MIN_DAY;
			}else{
				next_time = time + 1;
			}
			break;
		case WEEK_SEQ:
			//��ʱ��������
			//time_high = WEEK_HIGH_POSITION;
			//time_low = WEEK_LOW_POSITION;
			break;
		case HOUR_SEQ:
			if(time == MAX_HOUR){
				next_time = MIN_HOUR;
			}else{
				next_time = time + 1;
			}
			break;
		case MINUTE_SEQ:
			if(time == MAX_MINUTE){
				next_time = MIN_MINUTE;
			}else{
				next_time = time + 1;
			}
			break;
		case SECOND_SEQ:
			if(time == MAX_SECOND){
				next_time = MIN_SECOND;
			}else{
				next_time = time + 1;
			}
			break;
		default:
			break;
	}
	
	return next_time;		
}

/**********************************************COMMON*****************************************************/

/*��ʱ*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}
}

/*�ַ�ת����*/
uint to_int(uchar ch){
	return ch - 48;
}

/*����ת�ַ�*/
uchar to_char(uint i){
	return i + 48;
}

/*�жϵ����Ƿ�Ϊ����*/
uint is_leap_year(uint year){
	if((year%4==0 && year%100!=0) || year%400==0){
		return 1;	
	}
	return 0;
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
	P0 = com;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*д����*/	  
void write_data(uchar datas){
	rw = 0;
	rs = 1;
	P0 = datas;
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
	P0 = 0xff;  //Ϊʲô?
	rw = 1;
	rs = 1;
	delay(1);
	en = 1;
	//delay(1);
	c = P0;
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
	P0 = 0xff; 
	rw = 1;
	rs = 0;
	delay(5);
	en = 1;
	delay(5);
	read_pointer = P0;
	en = 0;
	return read_pointer+BASIC_POSITION;	
}