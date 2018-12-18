#include <LPC17xx.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <stdint.h> 
#include <stdio.h>
#include "uart.h"
#include "GLCD.h"
#include <RTL.h>
#include <math.h>
#include "goalieBeforeGrey.c"
#include "goalieCenterSaveGrey.c"

#define Whiteu 0xffffu
#define Redu   0xf800u
#define Blacku 0x0000u
#define Yellowu 0xffe0u
#define Greenu 0x03e0u
#define DarkGreenu 0x1a00u
#define Blueu  0x001fu
#define Greyu 0x7befu
#define RectSize 400

unsigned char colorRect[RectSize], colorRect2[RectSize], colorRect3[RectSize]; 
unsigned char *bitmap  = colorRect;
unsigned char *bitmap2  = colorRect2;
unsigned char *bitmap3  = colorRect3;
unsigned int pixelSize;

uint32_t count = 0; 
uint32_t joystick_input = 0x00;
uint8_t direction_input, temp = 0x00, bitShift = 0x07, score = 0x00; 
int countrySelection, country1, country2, countryCount, shotDir, saveDir, joystickDir, numTurns =0;
bool joystickLeft, joystickRight, joystickForward, selectCountry, joystick, ready, ledReady, goal = 0; 
OS_SEM sem1, sem2, sem3, sem4, sem5, sem6, sem7; 
OS_TID tsk1, tsk2, tsk3, tsk4, tsk5, tsk6, tsk7; 

//Updates colour of bitmap array
void updateBitMapColor (unsigned int color, unsigned char *bitmapArray){
	int i = 0; 
		for (i = 0; i < RectSize; i+=2){
			*(bitmapArray+ (i+1)) = ( (color & 0xff00) >> 8) ;
			*(bitmapArray+ i) = (color & 0x00ff);
		}
}

//Clears flags and country name instead of using GLCD_Clear increase speed
void clearFlags(){
	int i,j = 0; 
	updateBitMapColor(Greyu, bitmap);
	for(i = 40; i<270;i+= pixelSize){
		for (j = 25; j<220;j+= pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
		}
	}
}

//Draws Polish flag
void drawPoland (bool small, int side){
	int i,j = 0; 
	
	//For small flags in the gameplay screen
	if (small){
		//Draws top white half
		updateBitMapColor(Whiteu,bitmap);
		for (i = side; i < side+40; i+=pixelSize){
			for (j = 10; j<20; j+=pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
		
		//Draws bottom red half
		updateBitMapColor(Redu,bitmap);
		for (i = side; i < side+40; i+=pixelSize){
			for (j = 20; j<30; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
	}
	
	//Large flags on startup screen 
	else{
		clearFlags();
		
		//Draws top white half
		updateBitMapColor(Whiteu,bitmap);
		updateBitMapColor(Redu,bitmap2);
		
		for (i = 75; i < 250; i+=pixelSize){
			for (j = 0; j<= 64; j+=pixelSize){
				GLCD_Bitmap(i,j+80,pixelSize,pixelSize,bitmap);
				GLCD_Bitmap(i,j+149,pixelSize,pixelSize,bitmap2);
			}	
		}
	}
}

//Draws flag of Japan
void drawJapan (bool small, int16_t side){
	int i,j = 0; 

	//Small flag for gameplay screen
	if (small){
		//Draws white background
		updateBitMapColor(Whiteu,bitmap);			
		for (i = side; i <= side+40; i+=pixelSize){
			for (j = 10; j<=30; j+=pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
		
		updateBitMapColor(Redu,bitmap);
		for (i = side+20; i <= side+25; i+=pixelSize){
			for (j = 20; j<25; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
		
		updateBitMapColor(Redu,bitmap);
		for (i = 0; i < 2; i++){
			for (j = 0; j <= i ; j++){
					GLCD_Bitmap(side+20 + i, 20 - j,5,5,bitmap);
			  	GLCD_Bitmap(side+25 - i, 20 - j,5,5,bitmap);
				  GLCD_Bitmap(side+20 + i, 25 + j,5,5,bitmap);
			  	GLCD_Bitmap(side+25 - i, 25 + j,5,5,bitmap);
			}	
		}
		
	}
	//Large flag for startup screen
	else{
		clearFlags();

		//Draws white background
		updateBitMapColor(Whiteu,bitmap);			
		for (i = 75; i < 240; i+=pixelSize){
			for (j = 75; j<193; j+=pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
		
		updateBitMapColor(Redu,bitmap);
		for (i = 127; i <= 187; i+=pixelSize){
			for (j = 113; j<153; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}	
		}
		
		updateBitMapColor(Redu,bitmap);
		for (i = 0; i < 20; i+=5){
			for (j = 0; j <= i ; j+=5){
					GLCD_Bitmap(127 + i, 113 - j,5,5,bitmap);
			  	GLCD_Bitmap(192 - i, 113 - j,5,5,bitmap);
				  GLCD_Bitmap(127 + i, 153 + j,5,5,bitmap);
			  	GLCD_Bitmap(192 - i, 153 + j,5,5,bitmap);
			}	
		}
		
		for (i = 20; i < 35; i+=5){
			for (j = 0; j <= 20 ; j+=5){
					GLCD_Bitmap(127 + i, 113 - j,5,5,bitmap);
			  	GLCD_Bitmap(192 - i, 113 - j,5,5,bitmap);
				  GLCD_Bitmap(127 + i, 153 + j,5,5,bitmap);
			  	GLCD_Bitmap(192 - i, 153 + j,5,5,bitmap);
			}	
		}
	}
}

//Draws flags of three colour countries 
void drawThreeColour (bool dir, bool small, int side, int colour1, int colour2, int colour3){
	int i, j = 0; 
	//Small flags for gameplay screen
	if (small){
		updateBitMapColor(colour1,bitmap);
		updateBitMapColor(colour2,bitmap2);
		updateBitMapColor(colour3,bitmap3);
		//Vertical flag (i.e. France)
		if (!dir){
			for (i = 10; i < 40; i+=pixelSize){
				for (j = 0; j<=15 ; j+=pixelSize){
					GLCD_Bitmap(j+side,i,pixelSize,pixelSize,bitmap);
					GLCD_Bitmap(j+side+15,i,pixelSize,pixelSize,bitmap2);
					GLCD_Bitmap(j+side+30,i,pixelSize,pixelSize,bitmap3);
				}	
			}
		}
		//Horizontal flag (i.e. Russia)
		else{
			for (i = side; i < side+50; i+=4){
				for (j = 10; j <= 20; j+=4){
					GLCD_Bitmap(i,j,4,4,bitmap);
					GLCD_Bitmap(i,j+10,4,4,bitmap2);
					GLCD_Bitmap(i,j+20,4,4,bitmap3);
				}
			}
		}
	}
	
	//Large flags for startup screen
	else{
		clearFlags();
		updateBitMapColor(colour1,bitmap);
		updateBitMapColor(colour2,bitmap2);
		updateBitMapColor(colour3,bitmap3);
		
		//Vertical flag (i.e. France)
		if (!dir){			
			for (i = 75; i < 203; i+=pixelSize){
				for (j = 0; j<=58; j+=pixelSize){
					GLCD_Bitmap(j+75, i,pixelSize,pixelSize,bitmap);
					GLCD_Bitmap(j+133,i,pixelSize,pixelSize,bitmap2);
					GLCD_Bitmap(j+192,i,pixelSize,pixelSize,bitmap3);
				}	
			}		
		}
		//Horizontal flag (i.e. Russia)
		else{
			for (i = 75; i < 250; i+=pixelSize){
				for (j = 0; j<=42; j+=pixelSize){
					GLCD_Bitmap(i,j+75,pixelSize,pixelSize,bitmap);
					GLCD_Bitmap(i,j+118,pixelSize,pixelSize,bitmap2);
					GLCD_Bitmap(i,j+161,pixelSize,pixelSize,bitmap3);
				}	
			}
		}
	}
}

void drawBall(){
	int i,j = 0; 
	//Draw starting position of ball
		updateBitMapColor(Whiteu,bitmap);
		for (i = 155; i <= 165; i+=pixelSize){
			for (j = 177; j<=182; j+=pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}
		}
		
		//trapizoidal part to a certain max 
		for (i = 0; i <= 3; i++){
			for (j = 0; j <= i ; j++){
					GLCD_Bitmap(155 + i, 177 - j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 177 - j,1,1,bitmap);
				  GLCD_Bitmap(155 + i, 187 + j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 187 + j,1,1,bitmap);
			}	
		}
		//fills in the gap to complete the trapizoidal 
		for (i = 3; i <= 10; i++){
			for (j = 0; j <= 3 ; j++){
					GLCD_Bitmap(155 + i, 177 - j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 177 - j,1,1,bitmap);
				  GLCD_Bitmap(155 + i, 187 + j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 187 + j,1,1,bitmap);
			}	
		}
		
		updateBitMapColor(Blacku,bitmap);
    GLCD_Bitmap(155 + 5, 177 + 10,2,2,bitmap);
		GLCD_Bitmap(174 - 5, 177 + 10,2,2,bitmap);
		GLCD_Bitmap(155 + 5, 187 - 10,2,2,bitmap);
		GLCD_Bitmap(174 - 5, 187 - 10,2,2,bitmap);
		GLCD_Bitmap(174 - 9, 187 - 5,2,2,bitmap);
		
}

//Clear soccer ball and graphics inside the net instead of using GLCD_Clear to increase speed
void clearNet(){
	int i, j = 0; 
	
	//Clears inside the net
	updateBitMapColor(Greyu,bitmap);
	for(i = 100; i<=220;i+= pixelSize){
		for (j = 60; j<120;j+= pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
		}
	}
	
	//Clears starting position of soccer ball
	updateBitMapColor(DarkGreenu,bitmap);
	for (i = 155; i <= 165; i+=pixelSize){
			for (j = 177; j<=182; j+=pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
			}
		}
		
		//trapizoidal part to a certain max 
		for (i = 0; i <= 3; i++){
			for (j = 0; j <= i ; j++){
					GLCD_Bitmap(155 + i, 177 - j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 177 - j,1,1,bitmap);
				  GLCD_Bitmap(155 + i, 187 + j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 187 + j,1,1,bitmap);
			}	
		}
		//fills in the gap to complete the trapizoidal 
		for (i = 3; i <= 10; i++){
			for (j = 0; j <= 3 ; j++){
					GLCD_Bitmap(155 + i, 177 - j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 177 - j,1,1,bitmap);
				  GLCD_Bitmap(155 + i, 187 + j,1,1,bitmap);
			  	GLCD_Bitmap(174 - i, 187 + j,1,1,bitmap);
			}	
		}
}
void resetField(){
	//Delay and then reset screen with inital setup 
		os_itv_set(200);
		os_itv_wait();
		clearNet();
	
		//GLCD_Bitmap_GIMP(142,60,33,42,(unsigned char *) &GOALIEBEFORE_pixel_data);
			GLCD_Bitmap_GIMP(150,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);

		//Draw starting position of ball
		drawBall();
}

void drawField(){
	int i, j = 0; 
	
	//Draw field
	updateBitMapColor(DarkGreenu,bitmap);
	for(i = 0; i<320;i+= pixelSize){
		for (j = 120; j<240;j+= pixelSize){
				GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
		}
	}
	
	//Draw crossbar
	updateBitMapColor(Blacku,bitmap);
	for (i = 90; i <230; i++){
		for (j = 24; j < 26; j++){
			GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
		}
	}
	
	//Draw left goalpost 
	for (i = 90; i < 100; i++){
		for (j = 26; j < 120; j++){
			GLCD_Bitmap(i,j,1,1,bitmap);
			GLCD_Bitmap(i+140,j,1,1,bitmap);
		}
	}
	
	//Draw Country 1 flag
	switch(country1){
		//Poland
		case 1:
			drawPoland(1,20);	
			break;
		//Germany
		case 2:
			drawThreeColour(1,1,20,Blacku,Redu,Yellowu);
			break;
		//Russia
		case 3:
			drawThreeColour(1,1,20,Whiteu,Blueu,Redu);
			break;
		//Belgium
		case 4:
			drawThreeColour(0,1,20,Blacku,Redu,Yellowu);		
			break;
		//France
		case 5:
			drawThreeColour(0,1,20,Blueu,Whiteu,Redu);		
			break;
		//Italy
		case 6:
			drawThreeColour(0,1,20,Greenu,Whiteu,Redu);		
			break;
		//Nigeria
		case 7:
			drawThreeColour(0,1,20,Greenu,Whiteu,Greenu);			
			break;
		//Japan
		case 8:
			drawJapan(1,20);
			break; 
	}
	
	//Draw Country 2 flag
	switch(country2){
	//Poland
		case 1:
			drawPoland(1,260);		
			break;
		//Germany
		case 2:
			drawThreeColour(1,1,260,Blacku,Redu,Yellowu);
			break;
		//Russia
		case 3:
			drawThreeColour(1,1,260,Whiteu,Blueu,Redu);
			break;
		//Belgium
		case 4:
			drawThreeColour(0,1,260,Blacku,Redu,Yellowu);		
			break;
		//France
		case 5:
			drawThreeColour(0,1,260,Blueu,Whiteu,Redu);		
			break;
		//Italy
		case 6:
			drawThreeColour(0,1,260,Greenu,Whiteu,Redu);		
			break;
		//Nigeria
		case 7:
			drawThreeColour(0,1,260,Greenu,Whiteu,Greenu);			
			break;
		//Japan
		case 8:
			drawJapan(1,260);
			break; 
	}
	
	//Draw goalie before shot
	if (numTurns == 0){
		//Delay and then reset screen with inital setup 
		GLCD_Bitmap_GIMP(150,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
		drawBall();
	}
	//Draw shot saved
	else if (shotDir == saveDir){
		clearNet();
		switch(shotDir){
			//Left save
			case 0:
				GLCD_Bitmap_GIMP(105,75,22,40,(unsigned char *) &GOALIECENTERSAVEGREY_pixel_data);
				//GLCD_Bitmap_GIMP(105,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
				break;
			//Center save
			case 1:
				GLCD_Bitmap_GIMP(150,75,22,40,(unsigned char *) &GOALIECENTERSAVEGREY_pixel_data);
				//GLCD_Bitmap_GIMP(142,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
				break;
			//Right save
			case 2: 
				GLCD_Bitmap_GIMP(195,75,22,40,(unsigned char *) &GOALIECENTERSAVEGREY_pixel_data);
				//GLCD_Bitmap_GIMP(195,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
				break;
		}
		resetField();
		bitShift--;
	}
	
	//Draw goal scored
	else{
		clearNet();
		updateBitMapColor(Whiteu,bitmap);
		updateBitMapColor(Blacku,bitmap2);
		//Left goal
		if (shotDir == 0){
			//Draw ball
			for (i = 105; i < 111; i+=pixelSize){
				for (j = 80; j<86; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
				}
			}
			GLCD_Bitmap(109,84,2,2,bitmap2);
			//Draw goalie based on save location choice
			switch (saveDir){
				case 1:
						GLCD_Bitmap_GIMP(150,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
				case 2:
						GLCD_Bitmap_GIMP(195,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
			}
		}
		//Center goal
		else if (shotDir == 1){
			//Draw ball
			for (i = 150; i < 156; i+=pixelSize){
				for (j = 80; j<86; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
				}
			}
			GLCD_Bitmap(154,84,2,2,bitmap2);
			//Draw goalie based on save location choice
			switch (saveDir){
				case 0:
						GLCD_Bitmap_GIMP(105,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
				case 2:
						GLCD_Bitmap_GIMP(195,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
			}
		}
		//Right goal
		else{
			//Draw ball
			for (i = 205; i < 211; i+=pixelSize){
				for (j = 80; j<86; j+=pixelSize){
					GLCD_Bitmap(i,j,pixelSize,pixelSize,bitmap);
				}
			}
			GLCD_Bitmap(208,84,2,2,bitmap2);
			//Draw goalie based on save location choice
			switch (saveDir){
				case 0:
					//GLCD_Bitmap_GIMP(105,60,33,42,(unsigned char *) &GOALIEBEFORE_pixel_data);
						GLCD_Bitmap_GIMP(105,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
				case 1:
					//GLCD_Bitmap_GIMP(142,60,33,42,(unsigned char *) &GOALIEBEFORE_pixel_data);
						GLCD_Bitmap_GIMP(150,75,33,42,(unsigned char *) &GOALIEBEFOREGREY_pixel_data);
					break;
			}
		}
		resetField();
		score += (1<<bitShift);
		bitShift--;
	}
}

//Turns LEDs on to represent a decimal number in binary 
void LED_on (uint8_t decimal){
	LPC_GPIO1-> FIOCLR |= 0xB0000000; 
	LPC_GPIO2-> FIOCLR |= 0x0000007C; 
  LPC_GPIO1-> FIODIR |= 0xB0000000;
	LPC_GPIO2-> FIODIR |= 0x0000007C;
	
	temp = 0x00;
	
	temp |= (decimal & (0x07 << 5)) >> 5; 
	LPC_GPIO1->FIOSET |=  ( ( ((temp>>2) & (1<<0)) | (1<<2) | ((temp & (1<<1))) | (((temp<<2)&(1<<2))<<1))<<28);
	temp = 0x00;
	temp |= (decimal & (0x1F));
	LPC_GPIO2->FIOSET |= ( ( ((temp>>4) & (1<<0)) | ((temp>>2) & (1<<1)) | (temp & (1<<2)) | (( temp<<2) & (1<<3)) | ((temp<<4) & (1<<4))) << 2);
}

//Declaring tasks
__task void startupScreenTask(void);
__task void startupJoystickTask(void);
__task void startupPushbuttonTask(void);
__task void gameplayScreenTask(void);
__task void gameplayJoystickTask(void);
__task void gameplayPushbuttonTask(void);
__task void gameplayLEDTask(void);


//Start startup screen tasks
__task void start_startupScreenTasks(){
	tsk1 = os_tsk_create(startupScreenTask,1);
	tsk2 = os_tsk_create(startupJoystickTask,1);
	tsk3 = os_tsk_create(startupPushbuttonTask,1);
	
	//Delete startup task 
	os_tsk_delete_self();
}

//Start gameplay screen tasks
__task void start_gameplayTasks(){
	tsk4 = os_tsk_create(gameplayScreenTask,1);
	tsk5 = os_tsk_create(gameplayJoystickTask,1);
	tsk6 = os_tsk_create(gameplayPushbuttonTask,1);
	tsk7 = os_tsk_create(gameplayLEDTask,1);
	
	//Delete startup task
	os_tsk_delete_self();
}

//Refreshes flags and country name on startup screen 
__task void startupScreenTask(){
	os_sem_init(&sem1,0);
	GLCD_SetBackColor(0x7bef);
	GLCD_SetTextColor(0xffff);
	GLCD_DisplayString(2,7,1,"Kicker");
	GLCD_DisplayString(4,3,1,"Select Country");
	countryCount++;
	os_sem_send(&sem2);
	os_sem_wait(&sem1,0xffff);
	while(countrySelection<2){
		os_sem_send(&sem2);
		os_sem_wait(&sem1,0xffff);
		if (joystickLeft || joystickRight){
			switch (countryCount){
				//Poland
				case 1:
					if (joystickLeft){
						drawJapan(0,0);
						GLCD_DisplayString(1,7,1,"Japan");
						countryCount = 8;
					}
					else{
						drawThreeColour(1,0,0,Blacku,Redu,Yellowu);
						GLCD_DisplayString(1,7,1,"Germany");
						countryCount++;
					}
					break;
				//Germany
				case 2:
					if (joystickLeft){
						drawPoland(0,0);
						printf("got here\r\n");
						GLCD_DisplayString(1,7,1,"Poland");
						countryCount--;
					}
					else{
						drawThreeColour(1,0,0,Whiteu,Blueu,Redu);
						GLCD_DisplayString(1,7,1,"Russia");
						countryCount++;
					}
					break;
				//Russia
				case 3:
					if (joystickLeft){
						drawThreeColour(1,0,0,Blacku,Redu,Yellowu);
						GLCD_DisplayString(1,7,1,"Germany");
						countryCount--;
					}
					else{
						drawThreeColour(0,0,0,Blacku,Yellowu,Redu);
						GLCD_DisplayString(1,7,1,"Belgium");
						countryCount++;
					}
					break;
				//Belgium
				case 4:
					if (joystickLeft){
						drawThreeColour(1,0,0,Whiteu,Blueu,Redu);
						GLCD_DisplayString(1,7,1,"Russia");
						countryCount--;
					}
					else{
						drawThreeColour(0,0,0,Blueu,Whiteu,Redu);
						GLCD_DisplayString(1,7,1,"France");
						countryCount++;
					}
					break;
				//France
				case 5:
					if (joystickLeft){
						drawThreeColour(0,0,0,Blacku,Yellowu,Redu);
						GLCD_DisplayString(1,7,1,"Belgium");
						countryCount--;
					}
					else{
						drawThreeColour(0,0,0,Greenu,Whiteu,Redu);
						GLCD_DisplayString(1,7,1,"Italy");
						countryCount++;
					}
					break;
				//Italy
				case 6:
					if (joystickLeft){
						drawThreeColour(0,0,0,Blueu,Whiteu,Redu);
						GLCD_DisplayString(1,7,1,"France");
						countryCount--;
					}
					else{
						drawThreeColour(0,0,0,Greenu,Whiteu,Greenu);
						GLCD_DisplayString(1,7,1,"Nigeria");
						countryCount++;
					}
					break;
				//Nigeria
				case 7:
					if (joystickLeft){
						drawThreeColour(0,0,0,Greenu,Whiteu,Redu);
						GLCD_DisplayString(1,7,1,"Italy");
						countryCount--;
					}
					else{
						drawJapan(0,0);
						GLCD_DisplayString(1,7,1,"Japan");
						countryCount++;
					}
					break;
				//Japan
				case 8:
					if (joystickLeft){
						drawThreeColour(0,0,0,Greenu,Whiteu,Greenu);
						GLCD_DisplayString(1,7,1,"Nigeria");
						countryCount--;
					}
					else{
						drawPoland(0,0);
						GLCD_DisplayString(1,7,1,"Poland");
						countryCount = 1;
					}
					break;				
			}
			joystickLeft = 0;
			joystickRight = 0;
			if(countrySelection == 2){
				os_tsk_delete(tsk1);
			}
		}			
	}
}

//Receives input from joystick to switch countries
__task void startupJoystickTask(){
	os_sem_init(&sem2,0);
	os_sem_send(&sem3);
	os_sem_wait(&sem2,0xffff);
	while(countrySelection<2){
		os_sem_send(&sem3);
		os_sem_wait(&sem2,0xffff);
		joystick_input = 0x00;
		direction_input = 0x00;
		joystick_input = LPC_GPIO1->FIOPIN; 
		direction_input |= ((joystick_input & (0x0f<<23))>>23);
		
		//Left direction
		if (direction_input == 0x0e){
			joystickLeft = 1;
			selectCountry = 1; 
		}	
		//Right direction
		if (direction_input == 0x0b){
			joystickRight = 1; 
			selectCountry = 1; 
		}
		if(countrySelection == 2){
			os_tsk_delete(tsk2);
		}
	}
}

//Receives input from interrupt button to confirm whether player has selected country 
__task void startupPushbuttonTask(){
	uint32_t data = 0; 
	os_sem_init(&sem3,0);
	os_sem_send(&sem1);
	os_sem_wait(&sem3,0xffff);
	while(countrySelection<2){
		os_sem_send(&sem1);
		os_sem_wait(&sem3,0xffff);
		data = (LPC_GPIO2->FIOPIN & (1<<10));
		if (data == 0 && selectCountry){
			switch(countrySelection){
				case 0: 
				country1 = countryCount;
				countrySelection++;
				countryCount = 1; 
				clearFlags();
				GLCD_DisplayString(2,4,1,"Goalkeeper");
				GLCD_DisplayString(4,3,1,"Select Country");
				break;
		
				case 1:
				country2 = countryCount;
				countrySelection++;
				countryCount = 1; 
				clearFlags();			
				break;
			}
			selectCountry = 0; 
		}
		if(countrySelection == 2){
			os_sys_init(start_gameplayTasks);
			os_tsk_delete(tsk3);
		}
	}
}

//Controls the graphics of the gameplay screen, primarily the movement of the goalie and the ball
__task void gameplayScreenTask(){
	os_sem_init(&sem4,0);
	os_sem_send(&sem5);
	os_sem_wait(&sem4,0xffff);
	GLCD_Clear(0x7bef);	
	drawField();
	while(numTurns<17){
		os_sem_send(&sem5);
		os_sem_wait(&sem4,0xffff);
		if(numTurns%2 == 0 && ready){
			drawField();
			ready = 0;
		}
	}
}

//Receives input from joystick to determine shot and save direction
__task void gameplayJoystickTask(){
	os_sem_init(&sem5,0);
	os_sem_send(&sem6);
	os_sem_wait(&sem5,0xffff);
	while(numTurns<17){
		os_sem_send(&sem6);
		os_sem_wait(&sem5,0xffff);
		joystick_input = 0x00;
		direction_input = 0x00;
		joystick_input = LPC_GPIO1->FIOPIN; 
		direction_input |= ((joystick_input & (0x0f<<23))>>23);
		
		//Left direction
		if (direction_input == 0x0e){
			joystickDir = 0;
			joystick = 1; 
		}
		//Center direction
		if (direction_input == 0x0d){
			joystickDir = 1;
			joystick = 1; 
		}
		//Right direction
		if (direction_input == 0x0b){
			joystickDir = 2; 
			joystick = 1; 
		}
	}
}

//Receives input from the pushbutton to confirm when player has selected shot/save direction
__task void gameplayPushbuttonTask(){
	uint32_t data = 0; 
	os_sem_init(&sem6,0);
	os_sem_send(&sem7);
	os_sem_wait(&sem6,0xffff);
	while(numTurns<17){
		os_sem_send(&sem7);
		os_sem_wait(&sem6,0xffff);
		data = (LPC_GPIO2->FIOPIN & (1<<10));
		if (data == 0 && joystick){
			switch(joystickDir){
				case 0:
					//Kicker selects left direction
					if (numTurns%2 == 0)
						shotDir = 0; 
					//Goalkeeper selects left direction
					else 
						saveDir = 0; 
						ready = 1;
					break;
				case 1:
					//Kicker selects center direction
					if (numTurns%2 == 0)
						shotDir = 1;
					//Goalkeeper selects center direction
					else
						saveDir = 1;
						ready = 1;
					break;
				case 2:
					//Kicker selects right direction
					if (numTurns%2 == 0)
						shotDir = 2;
					//Goalkeeper selects right direction
					else
						saveDir = 2;
						ready = 1;
					break;
			}
			numTurns++;
			joystick = 0; 
		}
	}
}

//Updates row of LEDs to reflect whether a penalty kick opportunity has been converted
__task void gameplayLEDTask(){
	os_sem_init(&sem7,0);
	os_sem_send(&sem4);
	os_sem_wait(&sem7,0xffff);
	while(numTurns<17){
		os_sem_send(&sem4);
		os_sem_wait(&sem7,0xffff);
		if (shotDir != saveDir && numTurns%2 == 0){
			LED_on(score);
		}
	}
}

int main (void){
	//Increment of pixels for drawing of bitmap
	pixelSize = sqrt(RectSize)/2;
	SystemInit();
	
	//Initalizing LCD screen
	GLCD_Init();
	GLCD_Clear(0x7bef);
	
	//Initializing LEDs
	LPC_GPIO1-> FIOCLR &= 0x00000000; 
	LPC_GPIO2-> FIOCLR &= 0x00000000;
	
	//Turns LEDs off initially
	LED_on(0);
	
	printf("Starting tasks\r\n");
	//Starts startup task
	os_sys_init(start_startupScreenTasks);
}
