#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <stddef.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>

#define LED_ON			0
#define LED_OFF			1

#define K1 	0
#define K2	115
#define K3	60
#define K4	61

#define L1	66
#define L2	69
#define L3	68
#define L4  67

#define BUZZER		51
#define BUZZER_ON	0
#define BUZZER_OFF 	1

#define RUN			1
#define STOP		0

#define NUM_LED	4
#define NUM_KEY	4

#define TRUE 	1
#define FALSE	0

#define AIN "/sys/bus/iio/devices/iio:device0"

#define GPIO "/sys/class/gpio"
#define GPIO_EXPORT "/sys/class/gpio/export"

#define TTYPATH		"/dev/tty"
#define BONEPATH	"/sys/devices/bone_capemgr.9/slots"
#define DATALEN		94
#define ID		8
#define RSSISTART	84
#define ATCOMMAND	8

#define SERVOPATH "/sys/devices/ocp.3/pwm_test_P9_22.17/"
#define LEFT 	570000
#define RIGHT 	2350000
#define MID 	1460000
#define PERIOD	20000000

#define BUZZERPATH "/sys/devices/ocp.3/pwm_test_P9_16.18"
#define Do_hz	3821861
#define Me_hz	3033726
#define Sol_hz	2551050
#define HDo_hz	1911000



void Buzzer_Init(void);
void Buzzer_scale(int Hz);
void Buzzer_off(void);
void Error_scale(void);
void DoorOpen_scale(void);
void DoorClose_scale(void);
void PassInError_scale(void);

void Servo_init();
void Servo_move(int value);

void buffer_clear(char *buffer, int length);

typedef struct Pass_Tone{
	char *password;
	int tone_sel;
}Pass_Tone;

typedef struct extract{
	char MacAddr[ID+1];
	int RSSI;
} extract;

typedef enum
{
   false = 0, true = 1
} bool;

void InitialProcess (FILE **uart, struct termios* uart1, struct termios *old, char* buf, int *fd);
extract extraction (char * buffer);
void DoorClose(Pass_Tone *value);
void DoorOpen(Pass_Tone *value);
void KeyInit();
bool KeyEdge(int port);
bool DetectSignal(int* fd, unsigned char *buf2);
void printConsol(extract* ptr, char * buffer);
void readPortStr(char filename[], char value[]);  //function prototypes
void writePortStr(char filename[], char value[]);
void writePortInt(char filename[], int value);
int readPortInt(char filename[]);
void SetDirection( int port, char *direction);
void SetValue( int port, int val );
void gpioExport( int port );
int GetValue( int port );
void SetEdge( int port, char *edge );
int GetADCValue( int port );

#endif
