  
#define F_CPU 8000000UL
#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd4bit.h"	// LCD library

//Ultrasonic Sensors

static volatile int distance0 = 0,distance1=0;
static volatile int i = 0;
int16_t count_a0 = 0,count_a1=0;
 

void USS1(void);//lid open
void USS0(void);//garbage level
void lcd0(void);
void lcd1(void);

unsigned char dis[2];

char show_a0[16];
char show_a1[16];


//Bluetooth
unsigned char Received;

//unsigned char Received;  

//bluetooth
void uart_init(void);
void send_uart(unsigned char val);
unsigned char receive_uart(void);
void bluetooth(void);

//Servo Motor
void Servo_motor(void);
void close_servo(void);
int main(void){
		
	DDRA = 0xff;		// Make trigger pins as output 
	DDRD = 0xf3;		// , //pin 5 for servo motor.
	DDRC = 0xFF;

	LCDInit(0);

	
	GICR |= 0xc0; //Enable External Interrupts INT0 and INT1
    MCUCR |=0x05;//ISC00 & ISC10 , any logical change on INT0 and INT1 generates an interrupt request.

	sei();			// Enable global interrupt 

	uart_init();	//Bluetooth

	while(1){
		
		USS0();//Level
				
		USS1();//Lid
		
		
		if(dis[1] >3 && dis[1] <=10){
			close_servo();
			Servo_motor();
			LCDWriteStringXY(2, 1, "LiD OPENING. ");
			close_servo();
		}
		if(dis[1] >10){
			LCDWriteStringXY(2, 1, "LiD CLOSED. ");
		}
		
		if(dis[0] >0 && dis[0] <=7 ) {
			
			LCDWriteStringXY(2, 0, "100% Filled. ");
			PORTC |= (1 << PC0)| (1 << PC1)|(1 << PC2)|(1 <<PC3);
		
		}
		else if(dis[0] > 7 && dis[0] <= 14 ) {
			
			LCDWriteStringXY(2, 0, "75% Filled. ");
			PORTC |= (1 << PC0)| (1 << PC1)|(1 << PC2);
			PORTC &= ~(1 << PC3);

		}
		else if(dis[0] >14 && dis[0] <= 21 ) {

			LCDWriteStringXY(2, 0, "50% Filled. ");
			PORTC |= (1 << PC0)|(1 << PC1);
			PORTC &= ~((1 << PC2)|(1 <<PC3));
			
		}
		else if(dis[0] > 21 ) {

			LCDWriteStringXY(2, 0, "25% Filled. ");
			PORTC |= (1 << PC0);
			PORTC &= ~((1 <<PC1)|(1 <<PC2)|(1 <<PC3));		
		}
		
		//PORTA 3-Right back, 4-Right front, 5-Left front, 6-Left Back
		
		//UP - 3, DOWN - 4 , Right -2 , Left - 1
		Received = receive_uart();

		 
		if(Received =='4'){ //front,DOWN Arrow Key
			
			PORTA |= (1 << PA4)|(1 << PA5);
			PORTA &= ~((1 << PA3)|(1 << PA6));
			_delay_ms(500);
			PORTA &= ~((1 << PA4)|(1 << PA5));
		
		}
		else if(Received =='3'){//back, UP Arrow Key
			
			PORTA |= (1 << PA3)|(1 << PA6);
			PORTA &= ~((1 << PA4)|(1 << 5));
			_delay_ms(500);
			PORTA &= ~((1 << PA3)|(1 << PA6));

		}
		else if(Received =='2'){//right, RIGHT Arrow Key
		
			PORTA |= (1 << PA3)|(1<< PA5);
			PORTA &= ~((1 << PA4)|(1 << 6));
			_delay_ms(200);
			PORTA &= ~((1<< PA3)|(1<< PA5));
		
		}
		else if(Received =='1'){//left, LEFT Arrow Key
		
			PORTA |= (1 << PA4)|(1<< PA6);
			PORTA &= ~((1 << PA3)|(1 << 5));
			_delay_ms(200);
			PORTA &= ~((1<< PA4)|(1<<PA6));
		
		}
			
			
		
	}
	return 0;
}
ISR(INT0_vect){
  
  if(i == 0){
    TCCR1B |= 1<<CS10;
    i = 1;
  }
  else{
    TCCR1B = 0;
    distance0 = TCNT1;
    TCNT1 = 0;
    i = 0;
  }
}
ISR(INT1_vect){
  
  if(i == 0){
    TCCR1B |= 1<<CS10;
    i = 1;
  }
  else{
    TCCR1B = 0;
    distance1 = TCNT1;
    TCNT1 = 0;
    i = 0;
  }
}
void USS1(void){ //USS for lid open & close.

	PORTA |= 1<<PINA1;
    _delay_ms(500);

    PORTA &= ~(1<<PINA1);
    count_a1 = (double)distance1/580;
	dis[1] = count_a1;
	
}

void USS0(void){ //USS for garbage level

	PORTA |= 1<<PINA0;
    _delay_ms(500);

    PORTA &= ~(1<<PINA0);
    count_a0 = (double)distance0/580;
	dis[0] = count_a0;

}
void lcd0(void){
		
		//dtostrf(count_a0, 2, 2, show_a0);// distance to string. number of decimal places and integers.
		//strcat(show_a0, " cm     ");	// Concat unit i.e.cm 
		//LCDWriteStringXY(0, 0, "Level= ");
		//LCDWriteStringXY(7, 0, show_a0);	// Print distance 
		//_delay_ms(100);
}

void lcd1(void){

		//dtostrf(count_a1, 2, 2, show_a1);// distance to string. number of decimal places and integers.
		//strcat(show_a1, " cm     ");	// Concat unit i.e.cm 
		//LCDWriteStringXY(0, 1, "Lid  = ");
		//LCDWriteStringXY(7, 1, show_a1);	// Print distance 
		//_delay_ms(100);
}

void uart_init(void) {
	
	UCSRB |= (1 << RXEN) | (1 << TXEN);  // Transmission & Reception Enable (TXEN=1, RXEN=1)

	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); 
	UBRRH = (BAUD_PRESCALE >> 8); 
	UBRRL = BAUD_PRESCALE;
}

void send_uart(unsigned char val){
	while ((UCSRA & (1 << UDRE)) == 0);	//wait until transmition is finished
	UDR = val;								//Transmit the charater
}
unsigned char receive_uart(void){
	if (!((UCSRA & (1 << RXC)) == 0)){		//wait until receiveing is finished
		return(UDR);							//retrun the charater
	}else{
		return('n');
	}
	
}

void Servo_motor(void){

	TCNT1 = 0;		//Set timer1 count zero 
	ICR1 = 2499;		// Set TOP count for timer1 in ICR1 register 

	// Set Fast PWM, TOP in ICR1, Clear OC1A on compare match, clk/64 
	TCCR1A = TCCR1A | (1<<WGM11)|(1<<COM1A1);
	TCCR1B = TCCR1B | (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);
	
	
	OCR1A = 75;	// Set servo shaft at -90� position 75
	_delay_ms(3000);
	OCR1A = 230 ;	// Set servo at +90� position 230
	_delay_ms(3000);
	
}

void close_servo(void) {

	TCNT1 = 0;		// Set timer1 count zero 
	ICR1 = 0;		// Set TOP count for timer1 in ICR1 register to zero 
	//close registers
	TCCR1A = TCCR1A & ~(1<<WGM11);
	TCCR1A = TCCR1A & ~(1<<COM1A1);
	
	TCCR1B = TCCR1B & ~(1<<WGM12);
	TCCR1B = TCCR1B & ~(1<<WGM13);
	TCCR1B = TCCR1B & ~(1<<CS11);
	TCCR1B = TCCR1B & ~(1<<CS10);
}