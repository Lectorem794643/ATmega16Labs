#define F_CPU 16000000
#include <avr/interrupt.h>

#define PIN_INT1 PD3
#define LED_NEG PD0

unsigned char buttons;

/* ���������� ��� ���������� ������������ */
#define ST1RES PORTB&=~(1<<0)
#define ST1SET PORTB|=(1<<0)
#define ST2RES PORTB&=~(1<<1)
#define ST2SET PORTB|=(1<<1)
#define ST3RES PORTB&=~(1<<2)
#define ST3SET PORTB|=(1<<2)
#define SENRES PORTB&=~(1<<4)
#define SENSET PORTB|=(1<<4)

/* ���� �������� ��� 7-���������� ����������� */
unsigned char symbol_codes[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

void SPI_init()
{
	DDRB|=(7<<0)|(1<<4);  /* ���������� ������� ������ ��������� */
	DDRB|=(1<<5)|(1<<7);  /* ���������� ������� SPI */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

unsigned char SPI_WR(unsigned char data)
{
	unsigned char tmp;
	SPDR = data;
	while(!(SPSR & (1<<SPIF)))
	tmp=SPDR;
	return tmp;
}

void send_digit(unsigned char value,unsigned char pos)
{
	SPI_WR(symbol_codes[value]); // �� ��� ���������� ��������� ���� � ��� �� ���, �� ������� �� ����� ������ � �� ����������, �� ������� ����� ��������� ��� ������
	switch(pos)
	{
		case 1:	ST1SET; ST1RES; break;
		case 2:	ST2SET;	ST2RES;	break;
		case 3:	ST3SET;	ST3RES;	break;
	}
}

unsigned char ASK_btns(void)
{
	unsigned char btns;
	SENRES;
	SENSET;
	btns=SPI_WR(0);
	return btns;
}

void display_number(int number) {
	unsigned char hundreds = (number / 100) % 10; // �����
	unsigned char tens = (number / 10) % 10;      // �������
	unsigned char units = number % 10;       // �������

	send_digit(hundreds, 1); // ����� �� ������ ���������
	send_digit(tens, 2);     // ����� �� ������ ���������
	send_digit(units, 3);    // ����� �� ������ ���������
}

void init_INT1(void) {
	DDRD &= ~(1<<PIN_INT1);// �������� ����� �� ����
	PORTD |= (1<<PIN_INT1);// �������� ������������� ��������
	GICR |= (1<<INT1);// ��������� ������� ���������� �� INT1
	MCUCR |= (1<<ISC01)|(0<<ISC00);// �������� ������� ����������
	sei(); // �������� ���������� ����������
}

void configure_neg_indicator() {
	DDRD |= (1 << LED_NEG); // PD0 ��� ����� ��� ���������� �������������� �����
	PORTD |= (1 << LED_NEG); // ����� �������� �� ��������� (�������� ������� � ������)
}

// ���������� ���������� ��� INT1
ISR(INT1_vect) {
	buttons = ASK_btns();
	
	// ��������� ����� �� ��� 4-������ �����
	unsigned char num1 = (buttons >> 4) & 0x0F; // ������ ����� (������� 4 ����)
	unsigned char num2 = buttons & 0x0F;       // ������ ����� (������� 4 ����)
	int result = 0;

	// ���������� �������������� �������� � ����������� �� ������� ������
	if (!(PINA & (1 << PA0))) {
		result = num1 + num2; // ��������
		} else if (!(PINA & (1 << PA1))) {
		result = num1 - num2; // ���������
		} else if (!(PINA & (1 << PA2))) {
		result = num1 * num2; // ���������
		} else if (!(PINA & (1 << PA3))) {
		if (num2 != 0) {
			result = num1 / num2; // ������� (���� �������� �� 0)
			} else {
			result = 0; // ������������� ������� �� 0
		}
	}

	// ���� ��������� �������������, �������� �������� (������ �������), ����� �����
	if (result < 0) {
		PORTD &= ~(1 << LED_NEG); // �������� �������� ��� ������������� �����
		} else {
		PORTD |= (1 << LED_NEG); // ����� �������� ��� ������������� �����
	}
	
	// ������� ��������� �� �������������� ����������
	if (result < 0) {
		result = -result; // ����������� ������������� ����� � �������������
	}
	display_number(result);
}

// �������� ���������
int main(void) {
	SPI_init();
	init_INT1();
	configure_neg_indicator();
	
	DDRA &= ~((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)); // ������ ��� �����
	PORTA |= ((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)); // �������� ��� ������

	// ������������� �������� ����� �������
	buttons = 0;
	display_number(0); // ����������� ���� ��� ������
	PORTD |= (1 << LED_NEG); // ����� �������� ��� ������ (�������� ������� � ������)

	while (1) {	}
	return 0;
}