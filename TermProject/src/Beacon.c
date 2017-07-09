#include "Beacon.h"

void Buzzer_Init(void){
	char str[50];
	sprintf(str, "%s/run", BUZZERPATH);
	writePortInt(str, 1);
	Buzzer_off();
}

void Buzzer_scale(int Hz){
	char str[50];
	sprintf(str, "%s/period", BUZZERPATH);
	writePortInt(str, Hz);
	sprintf(str, "%s/duty", BUZZERPATH);
	writePortInt(str, Hz/95);
}

void Buzzer_off(void){
	char str[50];
	sprintf(str, "%s/duty", BUZZERPATH);
	writePortInt(str, 0);
}
void DoorOpen_scale(void){
	Buzzer_scale(Do_hz);
	usleep(230000);
	Buzzer_scale(Me_hz);
	usleep(230000);
	Buzzer_scale(Sol_hz);
	usleep(230000);
	Buzzer_scale(HDo_hz);
	usleep(230000);
	Buzzer_off();
}
void DoorClose_scale(void){
	Buzzer_scale(HDo_hz);
	usleep(230000);
	Buzzer_scale(Sol_hz);
	usleep(230000);
	Buzzer_scale(Me_hz);
	usleep(230000);
	Buzzer_scale(Do_hz);
	usleep(230000);
	Buzzer_off();
}
void Error_scale(void){
	char str[50];
	sprintf(str, "%s/period", BUZZERPATH);
	writePortInt(str, Do_hz);
	sprintf(str, "%s/duty", BUZZERPATH);
	writePortInt(str, Do_hz/2);
}
void PassInError_scale(void){
	Error_scale();
	usleep(230000);
	Buzzer_off();
	usleep(230000);
	Error_scale();
	usleep(230000);
	Buzzer_off();
	usleep(230000);
}

void Servo_init(){

	char str1[50], str2[50];
	sprintf(str1, "%spolarity", SERVOPATH);
	writePortInt(str1, 0);
	sprintf(str2, "%speriod", SERVOPATH);
	writePortInt(str2, PERIOD);
	sprintf(str2, "%srun", SERVOPATH);
	writePortInt(str2, 1);

}

void Servo_move(int value){
	char str[50];

	switch(value){
	case LEFT:
		sprintf(str, "%sduty", SERVOPATH);
		writePortInt(str, LEFT);
		break;
	case RIGHT:
		sprintf(str, "%sduty", SERVOPATH);
		writePortInt(str, RIGHT);
		break;
	case MID:
		sprintf(str, "%sduty", SERVOPATH);
		writePortInt(str, MID);
		break;
	default:
		puts("worng...");
		exit(-2);
	}

}

void buffer_clear(char *buffer, int length)
{
	int i;
	for(i=0; i<length; i++){
		buffer[i] = '\0';
	}
}

void InitialProcess(FILE **uart, struct termios *uart1, struct termios *old, char* buf, int *fd){
		Servo_init();
		*uart = fopen(BONEPATH, "w");
		if(*uart == NULL) printf("slots didn't open\n");
		fseek(*uart,0,SEEK_SET);


		fprintf(*uart, "ttyO1_armhf.com");
		strcat(buf, "O1");

		fflush(*uart);
		fclose(*uart);

		//open uart1 for tx/rx
		*fd = open(buf, O_RDWR | O_NOCTTY);
		if(*fd < 0) printf("port failed to open\n");

		//save current attributes
		tcgetattr(*fd,&*(old));
		bzero(&(*uart1),sizeof(*uart1));

		(*uart1).c_cflag = B9600 | CS8 | CLOCAL | CREAD;
		(*uart1).c_iflag = IGNPAR | ICRNL;
		(*uart1).c_oflag = 0;
		(*uart1).c_lflag = 0;

		(*uart1).c_cc[VTIME] = 0;
		(*uart1).c_cc[VMIN]  = 1;

		//clean the line and set the attributes
		tcflush(*fd,TCIFLUSH);
		tcsetattr(*fd,TCSANOW,&(*uart1));
		write(*fd,"AT+DISI?",8);

}

extract extraction (char * buffer){
	extract temp;
	char val[2] = {'\0', '\0'};
	int i,j;
	int cnt=0;
	buffer_clear(temp.MacAddr, ID+1);
	for(i=0; buffer[i]!='\0'; i++){
		cnt++;
	}
	if (cnt > 20){
		for(j = 0; j < ID; j++){
			temp.MacAddr[j] = buffer[58+j];
		}
		val[0] = buffer[RSSISTART];
		val[1] = buffer[RSSISTART+1];

		temp.RSSI = atoi(val);

	}
	else{
		usleep(100000);
		temp.RSSI=0;
		return temp;
	}
	return temp;
}

void DoorClose(Pass_Tone *value){
	int light_value;
	int flag=0, pre_flag=0;
	while(1){
		light_value=GetADCValue(0);
		usleep(20000);
		if (light_value > 15){
			flag=1;
		}
		else if(light_value>0 && light_value <8) {
			flag=0;
		}
		if (pre_flag > flag){
			puts("close!!\n");
			usleep(1500000);
			Servo_move(LEFT);
			SetValue(L1,LED_OFF);
			SetValue(L2,LED_ON);
			value->tone_sel=2;
			usleep(10000);
			value->tone_sel=-1;
			return;
		}
		pre_flag=flag;
	}
}

void DoorOpen(Pass_Tone *value){
	Pass_Tone *p = value;
	Servo_move(MID);
	SetValue(L1,LED_ON);
	SetValue(L2,LED_OFF);
	puts("open!!\n");
	value->tone_sel=1;
	usleep(10000);
	value->tone_sel=-1;
	DoorClose(p);
	usleep(1000000);
}

void KeyInit(void){
	gpioExport( K3 );
	SetDirection( K3, "in" );
	gpioExport( K4 );
	SetDirection( K4, "in" );
	gpioExport( L1 );
	SetDirection( L1, "out" );
	gpioExport( L2 );
	SetDirection( L2, "out" );
	SetValue(L1,LED_OFF);
	SetValue(L2,LED_ON);
}

bool KeyEdge(int port){
	static int val;
	int temp;
	temp = val;
	val = GetValue(port);

	if (val < temp){
		return true;
	}
	return false;
}

void printConsol(extract* ptr, char * buffer){
	puts("");
	printf("receive data : %s\n", buffer);
	puts("");
	printf("ID : %s\n", ptr->MacAddr);
	printf("RSSI : %d\n", ptr->RSSI);
	puts("");
}
bool DetectSignal(int* fd, unsigned char *buf2){

	extract value;
	static char buffer[DATALEN];
	static int cnt=0;
	static int count = 0;
	if(read(*fd,&(*buf2),1) > 0){
		buffer[cnt] = *buf2;
	}
		cnt++;
		if(buffer[cnt-1] == 'E'){
				if(buffer[cnt-6] == '+'){
					write(*fd,"AT+DISI?",ATCOMMAND);
					value = extraction(buffer);
					cnt=0;
					if(value.RSSI != 0){
						if (strcmp(value.MacAddr, "ABCD1234") == 0){
							if (value.RSSI < 60 ){
								count++;
								if (count == 2){
									count=0;
									printConsol(&value, buffer);
									buffer_clear(buffer, DATALEN);
									return true;
								}else{
									buffer_clear(buffer, DATALEN);
									return false;
								}
							}
							else{
								printConsol(&value, buffer);
								buffer_clear(buffer, DATALEN);
							}

						}else{
							printConsol(&value, buffer);
							buffer_clear(buffer, DATALEN);
						}
					}
					else{
						puts("");
						printf("non detection\n");
						puts("");
						buffer_clear(buffer, DATALEN);
						return false;
					}
				}
		}

		return false;
}

void SetDirection( int port, char *direction ) {
	char str[50];

	sprintf(str, "%s/gpio%d/direction", GPIO, port);
	writePortStr( str, direction );
}

void SetValue( int port, int val ) {
	char str[50];

	sprintf(str, "%s/gpio%d/value", GPIO, port);
	writePortInt( str, val );
}

int GetValue( int port ) {
	char str[50];

	sprintf(str, "%s/gpio%d/value", GPIO, port);
	return readPortInt( str );
}

int GetADCValue( int port ) {
	char str[50];

	sprintf(str, "%s/in_voltage%d_raw", AIN, port );
	return readPortInt( str );
}


void gpioExport( int port )
{
	char str[50];
	DIR *dir;

	sprintf(str, "%s/gpio%d", GPIO, port);
	dir = opendir(str);

	if( dir == NULL )
		writePortInt(GPIO_EXPORT, port );
}


void writePortStr(char filename[], char value[]){
   FILE* fp;
   fp = fopen(filename, "w");
   fprintf(fp, "%s", value);
   fclose(fp);
}

void readPortStr(char filename[], char value[]){
   FILE* fp;
   fp = fopen(filename, "r");
   fscanf(fp, "%s", value);
   fclose(fp);
}

void writePortInt(char filename[], int value){
   FILE* fp;
   char str[10];

   sprintf( str, "%d", value );
   fp = fopen(filename, "w");
   fprintf(fp, "%s", str );
   fclose(fp);
}

int readPortInt(char filename[]){
   FILE* fp;
   char value[10];

   fp = fopen(filename, "r");
   fscanf(fp, "%s", value);
   fclose(fp);

   return atoi( value );
}




