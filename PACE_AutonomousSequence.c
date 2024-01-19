/*header files for TM4C123 device*/
#include "TM4C123GH6PM.h"
#include <stdio.h>

void Delay_MicroSecond(int time); // generates delay in microseconds
void Servo_CW_untilswitch(void);  
void Servo_CCW_untilswitch(void); 
void Servo_tapping(void);
void Servo_start(void);

void cyl2_ext_ret(void);

void delayMs(int n);	// generates delay in milliseconds

void alarm(int mode); //RED LED
void Okay(int mode);	//Green LED
void Standby(int mode); //Yellow LED
void Rainbow(int mode); //Celrebration pattern LED
void Servo_GateOpen(void);	
void Servo_GateClosed(void);
void Servo_GateSuperClosed(void);

int main(void)
{
		int alarmState; //Integer to monitor stte of the alarm to prevent restart of process in case of alarm
		SYSCTL->RCGCGPIO |= 0x3F;    /* enable clock to PORT A B C D E & F*/
	
		/* configuring PORA - A7 START, A6 floor Limit Switch, A5 alarm LED*/   
	  GPIOA->DIR |= 0x20;         /* set A5 a digial output pin */
	  GPIOA->DEN |= 0xF8;         /* make A7  A6 A5 A3 digital pin */
	  GPIOA->PUR |= 0xD8;         /* Pull up for A6 A4 A3*/
	
	  /* configuring PORTB as a digital and output signal to control B0 Servo Gate, B1 green LED and B2 Yellow LED*/
	  GPIOB->DIR |= 0x07;         /* set B2 B1 B0 a digial output pin */
	  GPIOB->DEN |= 0x07; 

		/* configuring PORTD to control DCV - D6 front nozzle, D3 left nozzle, D2, right nozzle, D1 suction, D0 Cyl1*/
		GPIOD->DIR |= 0x4F;         /* set PD0 to D6 no D4 NOR D5 as a digial output pin */
	  GPIOD->DEN |= 0x4F;         /* make PD0 to D5 as digital pin */
	
	/*Configure PORTE - E5 side flap sensor, E4 front flap sensor, E3 back Limit Switch, E2 magazine Limit Switch, E1 front Limit Switch, E0 PWM motor*/  
	  GPIOE->DIR |= 0x01;         /* set PE0 as a digial output pin */
	  GPIOE->DEN |= 0x3F;         /* make E5 E4 E3 E2 E1 E0 digital pin */
	  GPIOE->PUR |= 0x3E; 	     /* enable pull up for pin E5 E4 E3 E2 E1 */
	
	/* configuring PORF - F1 cyl 2 */   
	  GPIOF->DIR |= 0x02;         /* set F1 a digial output pin */
	  GPIOF->DEN |= 0x02;
	
		while(1){
		Standby(1);
		if(( (GPIOA->DATA)&0x08)==0){//start button
				alarmState = 0;
				while(( (GPIOA->DATA)&0x40)==64 && alarmState==0){//magazine Limit switch 
						//initiation sequence
							alarm(0);
							Standby(0);
							Okay(1);
							GPIOF->DATA = ~0x00; // Cylinder off
							GPIOD->DATA = ~0x00; // Nozzles and cylinder off
							Servo_start();
							delayMs(500);
					//Box folding process
							GPIOD->DATA = ~0x01;	/*turn PD0 on extend cylinder 1*/
							delayMs(3900);
							GPIOD->DATA = ~0x3;	/* vacuum + cyl 1 extend*/
							delayMs(500);
							GPIOD->DATA = ~0x2;	/* vacuum + cyl 1 retract*/
							delayMs(5000);
							if(( (GPIOA->DATA)&0x10)==16){//check push button at the end position
								alarm(1);
								alarmState =1;
								Okay(0);
								continue;
							}
							Servo_GateSuperClosed();
							GPIOD->DATA = ~0x42;	/* vacuum + front nozz*/
							delayMs(400);
							
							GPIOD->DATA = ~0x0A;	/*left nozzle and suction cup*/
						  delayMs(1700);
							
							GPIOD->DATA = ~0x4A;	/* vacuum + front nozz*/
							delayMs(400);
							
							GPIOD->DATA = ~0x0E;	/* vacuum + right nozz+ + left nozz*/
							delayMs(1500);
							
							GPIOD->DATA = ~0x06;	/* vacuum + right nozz*/
							delayMs(1000);
							
							if(((GPIOE->DATA)&0x20)==0) //check side sensor, connect Tiva to NO on relay
							{
								alarm(1);
								alarmState =1;
								Okay(0);
								continue;
							}
							Servo_CW_untilswitch(); //rotate CW to close the door
							Servo_tapping();	//tap the lid to fully close
							Servo_CCW_untilswitch(); //rotate CCW to close the door
							delayMs(1500);
							if(((GPIOE->DATA)&0x10)==0) //check front sensor, connect Tiva to NO on relay
							{
								alarm(1);
								Okay(0);
								alarmState =1;
								continue;
							}
							GPIOD->DATA |= ~0x00;	/*vacuum and nozzles off*/
							
							delayMs(1000);
							Servo_GateOpen(); //Open gate 
							delayMs(500);
							cyl2_ext_ret();  //Push box out
							Servo_GateClosed();  //Close gate
							delayMs(500);
							Okay(0);
							Rainbow(1); 		//Celebration sequence
						} 
				} else{
							GPIOD->DATA |= ~0x00;	/*vacuum and nozzles off*/
							GPIOF->DATA |= ~0x00;	/*vacuum and nozzles off*/
							Standby(1);
				}
			}
}

void delayMs(int n){ //Create a delay of n seconds
	int i, j;
	for(i = 0 ; i < n; i++)
		for(j = 0; j < 3180; j++){} /* do nothing for 1 ms */
}

void cyl2_ext_ret(void){ //Extend and then retract cylinder 2
		GPIOF->DATA = ~0x02;	/*turn PF1 to extend cylinder 2*/
		delayMs(1000);
		GPIOF->DATA = ~0x00;	/*turn PF1 off to extend cylinder 2*/
		delayMs(500);
}

void Servo_start(void) 
{//Position servo motor at starting location
	int i=0;
	for(i=0; i<50; i++) 
	{	
			/* Given 10us trigger pulse */
			GPIOE->DATA |= 0x01; /* make control  pin high */
			Delay_MicroSecond(650); /*1.4ms seconds delay */
			GPIOE->DATA &= 0x00; /* make control  pin low */
			Delay_MicroSecond(19350); /*1.86ms seconds delay */
	}
}

void Servo_CW_untilswitch(void) 
{// Provide variable PWM to lid servo motor checking if it has hit the Limit Switch
	 int d=0;
   int i=0; 
	 for( d=0; d<35; d++) 
	{
		 GPIOD->DATA = ~0x46;	/* vacuum + right nozz*/
		 for(i=0; i<10; i++) 
			{	
				GPIOE->DATA |= 0x01; /* make control  pin high */
				Delay_MicroSecond(650+50*d); 
				GPIOE->DATA &= 0x00; /* make control  pin low */
				Delay_MicroSecond(19350-50*d); 
			}
			if(( (GPIOE->DATA)&0x2)==2){ //stop when Limit switch is pressed
				GPIOD->DATA = ~0x02;
				break;
			}
	 }
	 delayMs(1500); //Stop to have box lid againt the plastic lid by pressured air stream
	 
	 for( d=0; d<30; d++) 
	{
		 GPIOD->DATA = ~0x46;	/* vacuum + right nozz*/
		 for(i=0; i<10; i++) 
			{	
				GPIOE->DATA |= 0x01; /* make control  pin high */
				Delay_MicroSecond(2300+200*d); 
				GPIOE->DATA &= 0x00; /* make control  pin low */
				Delay_MicroSecond(17700-200*d); 
			}
			if(( (GPIOE->DATA)&0x2)==2){  //stop when Limit switch is pressed
				GPIOD->DATA = ~0x02;
				break;
			}
	 }
	
}
void Servo_tapping(void)
{ //bring arm slightly up the down to push lid down
	int d=0;
   int i=0; 
	 for( d=0; d<15; d++) 
	{	 
		 for(i=0; i<10; i++) 
			{	
			GPIOE->DATA |= 0x01; 
			Delay_MicroSecond(2900+100*d); /*1.4ms seconds delay */
			GPIOE->DATA &= 0x00; 
			Delay_MicroSecond(17100-100*d); /*1.86ms seconds delay */
			}
			if(( (GPIOE->DATA)&0x2)==2){ 
			GPIOD->DATA = ~0x02;
			break;
			}
	 }
 }
void Servo_CCW_untilswitch(void)
{// Provide variable PWM to lid servo motor removing from the top of the box, checking if it has hit the Limit Switch
	 int d=0;
   int i=0; 
	 for( d=0; d<50; d++) 
	{
		 for(i=0; i<10; i++) 
			{	
		/* Given 10us trigger pulse */
			GPIOE->DATA |= 0x01; /* make control  pin high */
			Delay_MicroSecond(2900-100*d); /*1.4ms seconds delay */
			GPIOE->DATA &= 0x00; /* make control  pin low */
			Delay_MicroSecond(17100+100*d); /*1.86ms seconds delay */
			}
		  
			if(((GPIOE->DATA)&0x8)==8){ 
			break;	
			}
	 }
}

void Delay_MicroSecond(int time)
{//created delay in microseconds used for PWM
    int i;
    SYSCTL->RCGCTIMER |= 2;     /* enable clock to Timer Block 1 */
    TIMER1->CTL = 0;            /* disable Timer before initialization */
    TIMER1->CFG = 0x04;         /* 16-bit option */ 
    TIMER1->TAMR = 0x02;        /* periodic mode and down-counter */
    TIMER1->TAILR = 16 - 1;  /* TimerA interval load value reg */
    TIMER1->ICR = 0x1;          /* clear the TimerA timeout flag */
    TIMER1->CTL |= 0x01;        /* enable Timer A after initialization */

    for(i = 0; i < time; i++)
    {
        while ((TIMER1->RIS & 0x1) == 0) ;      /* wait for TimerA timeout flag */
        TIMER1->ICR = 0x1;      /* clear the TimerA timeout flag */
    }
}


void Servo_GateOpen(void)
{//Open servo gate with PWM signal
	int i=0;
	for(i=0; i<50; i++) 
			{	
		/* Given 10us trigger pulse */
			GPIOB->DATA |= 0x01; /* make control  pin high */
			Delay_MicroSecond(2400); 
			GPIOB->DATA &= ~0x01; /* make control  pin low */
			Delay_MicroSecond(17600);
			}
}

void Servo_GateClosed(void)
{ //Close servo gate with PWM signal
	int i=0;
	for(i=0; i<50; i++) 
			{	
			GPIOB->DATA |= 0x01; /* make control  pin high */
			Delay_MicroSecond(650); 
			GPIOB->DATA &= ~0x01; /* make control  pin low */
			Delay_MicroSecond(19350); 
			}
}
void Servo_GateSuperClosed(void){
//Function in case the gate open slightly to push it completely closed
	int i=0;
	for(i=0; i<50; i++) 
			{	
			GPIOB->DATA |= 0x01; /* make control  pin high */
			Delay_MicroSecond(600); 
			GPIOB->DATA &= ~0x01; /* make control  pin low */
			Delay_MicroSecond(19400); 
			}
}
void Standby(int mode)
{//control Yellow LED on control panel indicating process is on standy and ready to start
    if (mode == 1) {
        GPIOB->DATA |= 0x04;	/*turn on Yellow LED*/
    } else if (mode == 0) {
        GPIOB->DATA &= ~0x04;	/*turn off Yellow LED*/
    }
}

void Okay(int mode)
{//control Green LED on control panel indicating pricess is ongoing with no issues
    if (mode == 1) {
        GPIOB->DATA |= 0x02;	/*turn on Green  LED*/
    } else if (mode == 0) {
        GPIOB->DATA &= ~0x02;	/*turn off Green LED*/
    }
}
void alarm(int mode)
{//control Red LED on control panel and stop process
    if (mode == 1) {
        GPIOA->DATA |= 0x20;	/*turn on Red LED*/
				GPIOF->DATA |= ~0x00;
				GPIOD->DATA |= ~0x00;
				Okay(0);
				GPIOB->DATA &= ~0x02;	/*turn off Green  LED*/
    } else if (mode == 0) {
        GPIOA->DATA &= ~0x20;	/*turn off Red LED*/
    }
}

void Rainbow(int mode){ //Celebration pattern with LEDs
	if (mode == 1) {
		for(int i=1; i<4; i++){
        Standby(1);
				delayMs(50);
				alarm(1);
				delayMs(50);
				Okay(1);
				delayMs(50);
		
				Standby(0);
				delayMs(50);
				alarm(0);
				delayMs(50);
				Okay(0);
				delayMs(50);
		}
  } else if (mode == 0) {
        GPIOB->DATA &= ~0x02;	/*turn off Green LED*/
    }
}