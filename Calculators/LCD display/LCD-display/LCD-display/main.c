#define F_CPU 16000000
#include <avr/interrupt.h>
#include <stdio.h>

#define PIN_INT1 PD3
#define LED_NEG PD0

unsigned char buttons;

/* ���������� ��� ���������� ������������ */
#define SENRES PORTB&=~(1<<4)
#define SENSET PORTB|=(1<<4)

#include <util/delay.h>
#include <avr/io.h>
#define RS 7
#define RW 6
#define EN 5

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

void lcd_com(unsigned char p){
	PORTD &= ~(1<<RS);
	PORTD |= (1<<EN); // EN=1 ������ ������ �������
	PORTC = p; // ����� ������� �� ���� ������ ������
	_delay_us(500);
	PORTD &=~(1<<EN); // EN=1 ����� ������ �������
	_delay_us(500);
}

void lcd_dat(unsigned char p){
	PORTD|=(1<<RS)|(1<<EN); // ������ ������ ������
	PORTC=p; // ��������� ��������
	_delay_us(500);
	PORTD&=~(1<<EN); // ����� ������ ������
	_delay_us(500);
}

void lcd_init(void){
	DDRD |= (1<<RS)|(1<<RW)|(1<<EN); // ���������� �� �����
	
	// �������� ���� ���������� � ��� ������
	PORTD=0x00;
	DDRC=0xFF;
	PORTC=0x00;
	
	_delay_us(500);
	lcd_com(0x08); // ������ ��������� �������
	_delay_us(500);
	lcd_com(0x3C); // 8 ��� ������ 2 ������
	_delay_us(500);
	lcd_com(0x01); // ������� ������
	_delay_us(500);
	lcd_com(0x06); // ����� ������� ������
	_delay_us(900);
	lcd_com(0x0F); // ������ ������� � ������
}

void lcd_string(char *str){
	char data=0;
	while(*str){
		data=*str++;
		lcd_dat(data);
	}
}

void init_INT1(void) {
	DDRD &= ~(1<<PIN_INT1);  // �������� ����� �� ����
	PORTD |= (1<<PIN_INT1);   // �������� ������������� ��������
	GICR |= (1<<INT1);        // ��������� ������� ���������� �� INT1
	MCUCR |= (1<<ISC01) | (0<<ISC00);  // �������� ������� ����������
	sei();  // �������� ���������� ����������
}

void display_number(int num1, int num2, char operator, int result) {
	char str[20];  // ������ ��� �������� ������
	// ����������� ������ ���� "num1 operator num2 = result"
	sprintf(str, "%d %c %d = %d", num1, operator, num2, result);
	
	lcd_com(0x01); // ������� ������
	lcd_com(0x80); // ������ �� ������ ������ ������
	lcd_string(str);  // �������� ������ � lcd_string
	lcd_com(0xE9); // ������� ������
}

ISR(INT1_vect) {
	buttons = ASK_btns();
	
	// ��������� ����� �� ��� 4-������ �����
	unsigned char num1 = (buttons >> 4) & 0x0F;  // ������ ����� (������� 4 ����)
	unsigned char num2 = buttons & 0x0F;         // ������ ����� (������� 4 ����)
	int result = 0;
	char operator;  // ��������

	// ���������� �������������� �������� � ����������� �� ������� ������
	if (!(PINA & (1 << PA0))) {
		result = num1 + num2;  // ��������
		operator = '+';  // ������������� ��������
		} else if (!(PINA & (1 << PA1))) {
		result = num1 - num2;  // ���������
		operator = '-';
		} else if (!(PINA & (1 << PA2))) {
		result = num1 * num2;  // ���������
		operator = '*';
		} else if (!(PINA & (1 << PA3))) {
		if (num2 != 0) {
			result = num1 / num2;  // ������� (���� �������� �� 0)
			} else {
			result = 0;  // ������������� ������� �� 0
		}
		operator = '/';
	}

	// ������� ��������� �� ��-�������
	display_number(num1, num2, operator, result);
}


int main(void) {
	SPI_init();
	lcd_init();
	
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
