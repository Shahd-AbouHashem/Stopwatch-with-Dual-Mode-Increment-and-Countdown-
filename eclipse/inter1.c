/*****************************************************************************************************************************
                                    Created on: Sep 12, 2024
                                     Author: SHAHD diploma90
                                            MINI PROJECT 2
 ****************************************************************************************************************************/

#include<avr/io.h>
#include <util/delay.h>
#include<avr/interrupt.h>

unsigned char Flag = 0;
unsigned char seconds, minutes, hours;
unsigned char secondDecrementFlag, secondIncrementFlag;
unsigned char minuteDecrementFlag, minuteIncrementFlag;
unsigned char hourDecrementFlag, hourIncrementFlag;


void TIMER1_CTC_INIT() {
	// Timer setup for 1 second intervals using a 1024 prescaler
	TCNT1 = 0;  // Reset timer
	TCCR1A = (1 << FOC1A);  // Force output compare
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);  // CTC mode, prescaler 1024
	OCR1A = 15625;  // Compare value for 1 second
	TIMSK |= (1 << OCIE1A);  // Enable output compare interrupt
	SREG |= (1 << 7);  // Enable global interrupt
}


ISR(TIMER1_COMPA_vect) {
	/*After Compare interrupt seconds++*/
	if (!Flag) {
		// Increment timer: up counting mode
		PORTD &= ~(1 << PD0);  // Buzzer off
		PORTD |= (1 << PD4);  // Red LED on
		PORTD &= ~(1 << PD5);  // Yellow LED off
		seconds++;
		if (seconds == 60) {
			minutes++;
			seconds = 0;
		}
		if (minutes == 60) {
			hours++;
			minutes = 0;
		}
		if (hours == 100) {
			hours = minutes = seconds = 0;
		}
	} else {
		// Decrement timer: down counting mode
		PORTD &= ~(1 << PD4);  // Red LED off
		PORTD |= (1 << PD5);  // Yellow LED on
		if (seconds == 0) {
			if (minutes == 0) {
				if (hours == 0) {
					PORTD |= (1 << PD0);  // Buzzer on if time is zero
				} else {
					hours--;
					minutes = seconds = 59;
				}
			} else {
				minutes--;
				seconds = 59;
			}
		} else {
			seconds--;
		}
	}
}


void INT0_Init(){
	MCUCR |= (1 << ISC01);  // Set to trigger on falling edge
	GICR |= (1 << INT0);  // Enable INT0
	DDRD &= ~(1 << PD2);  // Set PD2 as input
	PORTD |= (1 << PD2);  // Enable pull-up resistor on PD2
}


ISR(INT0_vect){
	// Reset timer and counters
	TCNT1 = seconds = minutes = hours = 0;
}


void INT1_Init() {
	MCUCR |= (1 << ISC11) | (1 << ISC10);  // Trigger on rising edge
	GICR |= (1 << INT1);  // Enable INT1
	DDRD &= ~(1 << PD3);  // Set PD3 as input
}


ISR(INT1_vect) {
	// Pause timer
	TCCR1B &= ~(1 << CS12) & ~(1 << CS11) & ~(1 << CS10);
}


void INT2_Init() {
	MCUCSR &= ~(1 << ISC2);  // Trigger on falling edge
	GICR |= (1 << INT2);  // Enable INT2
	DDRB &= ~(1 << PB2);  // Set PB2 as input
	PORTB |= (1 << PB2);  // Enable pull-up resistor on PB2
}


ISR(INT2_vect){
	// Resume timer with prescaler 1024
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS12);
}


void setup_button_pins() {
	// Set buttons as inputs with pull-up resistors
	DDRB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB3) & ~(1 << PB2) & ~(1 << PB5) & ~(1 << PB6);
	PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB3) | (1 << PB4) | (1 << PB5) | (1 << PB6);
}


int main() {
	unsigned char currentButtonState, previousButtonState = 1;
	PORTD |= (1 << PD4);  // Start in up counter mode (Red LED on)
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD0);  // Set LEDs as outputs
	DDRB &= ~(1 << PB7);  // Set PB7 as input
	PORTB |= (1 << PB7);  // Enable pull-up on PB7
	DDRC |= 0x0F;  // Set PORTC first 4 pins as outputs for 7-segment display
	PORTC &= 0xF0;  // Clear PORTC output
	DDRA |= 0x3F;  // Set PORTA first 6 pins as outputs for enabling 7-segment displays
	PORTA &= 0xC0;  // Clear PORTA output

	TIMER1_CTC_INIT();
	INT0_Init();
	INT1_Init();
	INT2_Init();
	setup_button_pins();

	while (1) {
		currentButtonState = PINB & (1 << PB7);
		if (currentButtonState == 0 && previousButtonState == (1 << PB7)) {
			Flag ^= 1;  // Toggle mode between up and down counting
			PORTD ^= (1 << PD4);  // Toggle Red LED
			PORTD ^= (1 << PD5);  // Toggle Yellow LED
			_delay_ms(5);  // Debounce delay
		}
		previousButtonState = currentButtonState;

		// Handle button presses for adjusting seconds, minutes, and hours
		// Second decrement button
		if (!(PINB & (1 << PB5))) {
			if (!secondDecrementFlag && seconds > 0) {
				seconds--;
				secondDecrementFlag = 1;
			}
		} else {
			secondDecrementFlag = 0;
		}
		// Second increment button
		if (!(PINB & (1 << PB6))) {
			if (!secondIncrementFlag) {
				seconds++;
				secondIncrementFlag = 1;
			}
		} else {
			secondIncrementFlag = 0;
		}
		// Minute decrement button
		if (!(PINB & (1 << PB3))) {
			if (!minuteDecrementFlag && minutes > 0) {
				minutes--;
				minuteDecrementFlag = 1;
			}
		} else {
			minuteDecrementFlag = 0;
		}
		// Minute increment button
		if (!(PINB & (1 << PB4))) {
			if (!minuteIncrementFlag) {
				minutes++;
				minuteIncrementFlag = 1;
			}
		} else {
			minuteIncrementFlag = 0;
		}
		// Hour decrement button
		if (!(PINB & (1 << PB0))) {
			if (!hourDecrementFlag && hours > 0) {
				hours--;
				hourDecrementFlag = 1;
			}
		} else {
			hourDecrementFlag = 0;
		}
		// Hour increment button
		if (!(PINB & (1 << PB1))) {
			if (!hourIncrementFlag) {
				hours++;
				hourIncrementFlag = 1;
			}
		} else {
			hourIncrementFlag = 0;
		}

		// Update 7-segment displays
		// First 7-segment: display unit digit of seconds
		PORTA = 0x20;
		PORTC = (PORTC & 0xF0) | (seconds % 10);
		_delay_ms(2);

		// Second 7-segment: display tens digit of seconds
		PORTA = 0x10;
		PORTC = (PORTC & 0xF0) | (seconds / 10);
		_delay_ms(2);

		// Third 7-segment: display unit digit of minutes
		PORTA = 0x08;
		PORTC = (PORTC & 0xF0) | (minutes % 10);
		_delay_ms(2);

		// Fourth 7-segment: display tens digit of minutes
		PORTA = 0x04;
		PORTC = (PORTC & 0xF0) | (minutes / 10);
		_delay_ms(2);

		// Fifth 7-segment: display unit digit of hours
		PORTA = 0x02;
		PORTC = (PORTC & 0xF0) | (hours % 10);
		_delay_ms(2);

		// Sixth 7-segment: display tens digit of hours
		PORTA = 0x01;
		PORTC = (PORTC & 0xF0) | (hours / 10);
		_delay_ms(2);
	}
}
