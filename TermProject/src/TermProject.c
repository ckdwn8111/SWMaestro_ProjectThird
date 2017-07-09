#include "Beacon.h"
#include <pthread.h>

static int running=RUN;
static int fd;
static unsigned char buf2 = 0xA7;
static pthread_t Beacon, KeyInput, ChangePass, ToneScale;

FILE *uart;
char buf[30] = TTYPATH;
struct termios uart1,old;

void *KeyInThread( void *ptr );
void *BeaconThread( void *ptr );
void *ChangePassword(void *ptr);

void *ChangePassword(void *ptr){
	Pass_Tone *temp = (Pass_Tone *)ptr;
	while(1){
		if(KeyEdge(K4)){
			running=STOP;
			pthread_cancel(KeyInput);
			printf("input new password : ");
			scanf("%s", temp->password);
			printf("complete!!\n");
			running=RUN;
			if( pthread_create( &KeyInput, NULL, KeyInThread, (void *)temp ) != 0 ) exit(-1);
			if( pthread_create( &Beacon, NULL, BeaconThread, (void *)temp ) != 0 ) exit(-1);
		}

	}
	return NULL;
}
void *KeyInThread( void *ptr ) {
	Pass_Tone *temp = (Pass_Tone *)ptr;
	char pass_in[5];
	while(running){
		if (KeyEdge(K3)){
			running = STOP;
			printf("input password : ");
			scanf("%s", pass_in);

			if(strcmp(pass_in,temp->password) == 0){
				DoorOpen(temp);
				running=RUN;
				if( pthread_create( &Beacon, NULL, BeaconThread, (void *)temp ) != 0 ) exit(-1);
			}
			else{
				puts("wrong number");
				temp->tone_sel=3;
				usleep(10000);
				temp->tone_sel=-1;
				running=RUN;
				if( pthread_create( &Beacon, NULL, BeaconThread, (void *)temp ) != 0 ) exit(-1);
			}
			while(getchar()!='\n');
		}
	}
	return NULL;
}

void *BeaconThread( void *ptr ) {
	Pass_Tone *temp = (Pass_Tone *)ptr;

	while(running){

		if(DetectSignal(&fd, &buf2)){
			DoorOpen(temp);
		}

	}


	return NULL;
}

void *Tone_Scale( void *ptr){
	Pass_Tone *select = (Pass_Tone *)ptr;
	while(1){
		switch(select->tone_sel){
		case 1:
			DoorOpen_scale();
			break;
		case 2:
			DoorClose_scale();
			break;
		case 3:
			PassInError_scale();
			break;
		default :
			Buzzer_off();
		}
	}
}

int main(int argc, char *argv[])
{
	Pass_Tone value;
	char init_password[5] ={'0','0','0','0','\0'} ;
	value.password = init_password;
	value.tone_sel = -1;
	puts("/////////Smart door lock using Beacon/////////");

	InitialProcess(&uart, &uart1, &old, buf, &fd);

	Servo_move(LEFT);
	usleep(1000000);

	KeyInit();

	if( pthread_create( &KeyInput, NULL, KeyInThread, (void *)&value ) != 0 ) exit(-1);
	if( pthread_create( &Beacon, NULL, BeaconThread, (void *)&value ) != 0 ) exit(-1);
	if( pthread_create( &ChangePass, NULL, ChangePassword, (void *)&value ) != 0 ) exit(-1);
	if( pthread_create( &ToneScale, NULL, Tone_Scale, (void *)&value ) != 0 ) exit(-1);

	while(1)
	{
		if (KeyEdge(K2)){
			break;
		}
		usleep(100000);
	}

	running=STOP;

	pthread_cancel(ChangePass);
	pthread_cancel(ToneScale);
	pthread_join( KeyInput, NULL );
	pthread_join( Beacon, NULL );

	puts("/////////Program is terminated////////");

	close(fd);

	return 0;
}

