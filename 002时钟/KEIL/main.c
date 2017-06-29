#include <reg52.h>
#include <string.h>
#include <consts.h>
//定义类型别名
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
//定时器
uchar count = 0;
//每个月的天数数组
uint xdata month_day_arr[] = MONTH_DAY_ARR;
//设置状态，默认不在设置状态
bit setting_status = 0;
//设置前的时间状态，用以还原时间
uint curr_date_time_arr[DATE_TIME_DIGIT_COUNT];
//日期时间结构体
typedef struct DateTime{
	uchar *date;
	uchar *time;
};

uint code position_arr[] = POSITION_ARRAY;
uint code position_max_value_arr[] = POSITION_MAX_VALUE_ARRAY;
uint code position_min_value_arr[] = POSITION_MIN_VALUE_ARRAY;

void main(){
	uint key;

	//显示器初始化
	display_init();

	//启动后马上检查初始数据，如果错误，显示错误信息
	if(check_init_data()){
		write_string(STRING_OUT_OF_BOUND);
		while(1);
	}	  
	//时钟初始化：写入原始设置时间
	init();
	timer_init();
	start();	

	//检测按键
	while(1){
		key = key_scan();
		setting(key);
	}
}

/*定时器50ms*/
void timer0() interrupt 1{
	if((count++) < 13){
		return;
	}
	count = 0;
	refresh_time(SECOND_SEQ);		
}

/*启动计时*/
void start(){
	TR0 = 1;
}

/*停止计时*/
void stop(){
	TR0 = 0;
}

/*关机*/
void shutdown(){
	write_com(0x08);
}

/*扫描键盘*/
uint key_scan(){
	if((P1&0x3f) != 0x3f){
		delay(150);
		if((P1&0x3f) != 0x3f){
			return P1&0x3f;
		}
	}
	return 0;
}

/*设置*/
void setting(uint key){
	switch(key){
		case 0x2f:
			if(setting_status){
				setting_status = 0;
				confirm_time();
			}else{
				setting_status = 1;
				//调整时间前将当前时间存储下来，以备还原使用
				storage_curr_time();
				adjust_time();	
			}
			break;
		//闹钟设置
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

/*储存当前的时间信息*/
void storage_curr_time(){
	curr_date_time_arr[0] = read_time(YEAR_SEQ);
	curr_date_time_arr[1] = read_time(MONTH_SEQ);
	curr_date_time_arr[2] = read_time(DAY_SEQ);
	curr_date_time_arr[3] = read_time(HOUR_SEQ);
	curr_date_time_arr[4] = read_time(MINUTE_SEQ);
	curr_date_time_arr[5] = read_time(SECOND_SEQ);	
}

/*调整时间*/
void adjust_time(){
	//停止计时，显示光标，并将光标放置在秒的低位位置
	stop();
	write_com(0x0f);
   	write_com(SECOND_LOW_POSITION);
}

/*确认时间，调整时间完成*/
void confirm_time(){
	//开始计时，隐藏光标
	write_com(0x0c);
	//确认时间前先检查调整的时间是否正确
	if(!check_datetime()){
		//时间修改错误，从存储区恢复
		print_date_time(" Setting error!",NULL);
		delay(2000);
		reset_time_from_store();
	}
	start();	
}

/*检查日期时间是否符合规范*/
uint check_datetime(){
	uint year,month,day,hour,minute,second;
	year = read_time(YEAR_SEQ);	
	month = read_time(MONTH_SEQ);
	day = read_time(DAY_SEQ);
	hour = read_time(HOUR_SEQ);
	minute = read_time(MINUTE_SEQ);
	second = read_time(SECOND_SEQ);
	//检查范围是否超出
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
	//如果调整的月份为2月，判断日期是否超出范围
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

/*从存储区恢复时间*/
void reset_time_from_store(){
	struct DateTime datetime;
	datetime = get_date_time(curr_date_time_arr);
	print_date_time(datetime.date,datetime.time);				
}

/*在显示器上打印日期和时间*/
void print_date_time(uchar *date,uchar *time){
	//清屏，打印日期，换行，打印时间
	write_com(0x01);	
	write_string(date);
	write_com(SECOND_LINE_FIRST_POSITION);
	write_string(time);
}

/*通过日期时间数组获得以个标准的日期时间格式字符串结构体
,如通过[2017,12,21,02,12,56]数组获得date[]="2017-12-21",time[]="02:12:56"*/
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

/*左移*/ 
void shift_left(){
	//移动到年份的第一位再左移定位到秒针低位
	//移动到时针高位再左移定位到天的低位
	//不移动到分隔符上
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

/*右移*/
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

/*上移数字*/
void digit_up(){
	uint curr_p;
	uchar curr_ch;
	curr_p = read_pointer();
	curr_ch = read_data();
	//write_com(0x10);
	write_data(to_char(get_next_digit(to_int(curr_ch),curr_p)));
	write_com(0x10);
}

/*下移数字*/
void digit_down(){
	uint curr_p;
	uchar curr_ch;
	curr_p = read_pointer();
	curr_ch = read_data();
	write_data(to_char(get_prev_digit(to_int(curr_ch),curr_p)));
	write_com(0x10);
}

/*获取下一个数字*/
uint get_next_digit(uint curr_digit,uint curr_pointer){
	uint next_digit,i;
	for(i=0;i<POSITION_ARRAY_LENGTH;i++){
		if(curr_pointer == position_arr[i]){
			//当前数字等于最大数字，则下一个数字为最小数字，否则加1即可
			if(curr_digit == position_max_value_arr[i]){
				next_digit = position_min_value_arr[i];				
			}else{
				next_digit = curr_digit + 1;
			}
		}
	}
	return next_digit;
}

/*获取上一个数字*/
uint get_prev_digit(uint curr_digit,uint curr_pointer){
	uint prev_digit,i;
	for(i=0;i<POSITION_ARRAY_LENGTH;i++){
		if(curr_pointer == position_arr[i]){
			//当前数字等于最小数字，则上一个数字为最大数字，否则减1即可
			if(curr_digit == position_min_value_arr[i]){
				prev_digit = position_max_value_arr[i];				
			}else{
				prev_digit = curr_digit - 1;
			}
		}
	}
	return prev_digit;
}

/*初始化*/
void init(){
	//写入初始日期和时间
	print_date_time(DEFAULT_DATE,DEFAULT_TIME);
}

/*检查初始化数据*/
uint check_init_data(){
	uchar *p_date = DEFAULT_DATE,*p_time = DEFAULT_TIME;
	if(strlen(p_date) > 16 || strlen(p_time) > 16){
		return 1;
	}
	return 0;
}							   

/*定时器初始化*/
void timer_init(){
	TMOD = 0x01;
	TH0 = (65536-50000)/256;
	TL0 = (65536-50000)%256;
	EA = 1;
	ET0 = 1;
}

/*刷新时间（按照序列号）*/
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
			//暂时不作处理
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


/*根据序列号（已给出）读取任一时间（年月日时分秒）*/
uint read_time(uint seq){
	uint time_high,time_low;
	//年份特殊处理
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
			//暂时不作处理
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
	//读取数据
	return to_int(read_data_pos(time_high))*10 + to_int(read_data_pos(time_low));
}

/*获取下一个时刻*/
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
			//暂时不作处理
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

/*延时*/
void delay(uint time){
	uint x,y;
	for(x=0;x<time;x++){
		for(y=0;y<100;y++);
	}
}

/*字符转数字*/
uint to_int(uchar ch){
	return ch - 48;
}

/*数字转字符*/
uchar to_char(uint i){
	return i + 48;
}

/*判断当年是否为闰年*/
uint is_leap_year(uint year){
	if((year%4==0 && year%100!=0) || year%400==0){
		return 1;	
	}
	return 0;
}


/*********************************1602**********************************************/

/*1602片初始化*/
void display_init(){
	en = 0;
	//默认为写操作
	rw = 0;
	//基础设置
	write_com(0x38);
	//显示打开，不显示光标
	write_com(0x0c);
	//读写一个字符指针和光标同时加1
	write_com(0x06);
	//清屏
	write_com(0x01);
	//从第一个位开始写
	write_com(0x80);
}

/*写指令*/
void write_com(uchar com){
	rw = 0;
	rs = 0;
	P0 = com;
	delay(1);
	en = 1;
	delay(1);
	en = 0;
}

/*写数据*/	  
void write_data(uchar datas){
	rw = 0;
	rs = 1;
	P0 = datas;
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
	P0 = 0xff;  //为什么?
	rw = 1;
	rs = 1;
	delay(1);
	en = 1;
	//delay(1);
	c = P0;
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