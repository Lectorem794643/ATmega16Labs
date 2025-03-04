#define F_CPU 16000000
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define PIN_INT1 PD3
#define LED_NEG PD0

/* ���������� ��� ���������� ������������ */
#define SENRES PORTB&=~(1<<4)
#define SENSET PORTB|=(1<<4)

unsigned char buttons;

void SPI_init()
{
	DDRB|=(7<<0)|(1<<4);  /* ���������� ������� ������ ��������� */
	DDRB|=(1<<5)|(1<<7);  /* ���������� ������� SPI */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

unsigned char SPI_WR(unsigned char data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF))) {} // ������� ��������� ��������
	return SPDR;// ������ ������ ����� ���������� ��������
}

unsigned char ASK_btns(void)
{
	SENRES;
	SENSET;
	return SPI_WR(0);
}

void init_INT1(void) {
	DDRD &= ~(1<<PIN_INT1);  // �������� ����� �� ����
	PORTD |= (1<<PIN_INT1);   // �������� ������������� ��������
	GICR |= (1<<INT1);        // ��������� ������� ���������� �� INT1
	MCUCR |= (1<<ISC01) | (0<<ISC00);  // �������� ������� ����������
	sei();  // �������� ���������� ����������
}

ISR(INT1_vect) {
	buttons = ASK_btns();
	
	// ��������� ����� �� ��� 4-������ �����
	unsigned char num1 = (buttons >> 4) & 0x0F;  // ������ ����� (������� 4 ����)
	unsigned char num2 = buttons & 0x0F;         // ������ ����� (������� 4 ����)
	int result = 0;
	
	// ���������� �������������� �������� � ����������� �� ������� ������
	if (!(PINA & (1 << PA0))) {
		result = num1 + num2;  // ��������
		} else if (!(PINA & (1 << PA1))) {
		result = num1 - num2;  // ���������
		} else if (!(PINA & (1 << PA2))) {
		result = num1 * num2;  // ���������
		} else if (!(PINA & (1 << PA3))) {
		if (num2 != 0) {
			result = num1 / num2;  // ������� (���� �������� �� 0)
			} else {
			result = 0;  // ������������� ������� �� 0
		}
	}

	// ������� ��������� �� ���� C (� �������� ����)
	PORTC = ~result;  // ����������� ������ ����� ������� �� ���� C
}


int main(void) {
	SPI_init();
	
	DDRC = 0xFF;  // �������� ���� C ��� �����
	init_INT1();  // ������������� INT1
	
	DDRA &= ~((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)); // ������ ��� �����
	PORTA |= ((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)); // �������� ��� ������

	// ������������� �������� ����� �������
	buttons = 0;
	PORTD |= (1 << LED_NEG);  // ����� �������� ��� ������ (�������� ������� � ������)

	while (1) {  }
	return 0;
}
