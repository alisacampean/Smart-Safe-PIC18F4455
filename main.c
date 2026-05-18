#include <p18f4455.h>
#include "delays.h"

// VARIABILE GLOBALE

char PIN_corect[4] = {'4', '2', '0', '6'};
char PIN_introdus[4];
int index_pin = 0;
int stare_seif = 0; 
char last_tasta = 0; 

// La 4MHz: 100 = 1.0ms (0 grade, Inchis), 150 = 1.5ms (90 grade, Deschis)
volatile unsigned char pozitie_servo = 100; // PORNIM INCHIS


// FUNCTII

void rutina_intrerupere(void);
void delay_ms(unsigned int ms);
void beep_scurt(void);
void beep_lung(void);
void beep_eroare(void);
char scaneaza_tastatura(void);

void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(unsigned char data);
void OLED_Cmd(unsigned char cmd);
void OLED_Data(unsigned char data);
void OLED_Clear(void);
void OLED_DrawChar(char c);
void OLED_Print_Stare(int is_open);


// FUNCTII ECRAN OLED 

void I2C_Start(void) { 
	SSPCON2bits.SEN = 1; 
	while(SSPCON2bits.SEN); 
}

void I2C_Stop(void) { 
	SSPCON2bits.PEN = 1; 
	while(SSPCON2bits.PEN); 
}

void I2C_Write(unsigned char data) { 
	PIR1bits.SSPIF = 0; 
	SSPBUF = data; // buffer
	while(!PIR1bits.SSPIF); 
}

void OLED_Cmd(unsigned char cmd) {
    I2C_Start(); 
	I2C_Write(0x78); // adresa ecran
	I2C_Write(0x00); // byte control
	I2C_Write(cmd); 
	I2C_Stop();
}

void OLED_Data(unsigned char data) {
    I2C_Start(); 
	I2C_Write(0x78); // adresa ecran
	I2C_Write(0x40); // byte control
	I2C_Write(data); 
	I2C_Stop();
}

void OLED_Clear(void) {
    unsigned char i, j;
    for(i = 0; i < 8; i++) {
        OLED_Cmd(0xB0 + i); // randul urmator
		OLED_Cmd(0x00);
		OLED_Cmd(0x10); // pune cursorul la inceputul randului
        for(j = 0; j < 128; j++) { //coloane
			OLED_Data(0x00); 
			_asm clrwdt _endasm 
		}
    }
}

void OLED_DrawChar(char c) {
    unsigned char i, font[5];
    if(c=='O')      { font[0]=0x3E; font[1]=0x41; font[2]=0x41; font[3]=0x41; font[4]=0x3E; }
    else if(c=='P') { font[0]=0x7F; font[1]=0x09; font[2]=0x09; font[3]=0x09; font[4]=0x06; }
    else if(c=='E') { font[0]=0x7F; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x41; }
    else if(c=='N') { font[0]=0x7F; font[1]=0x04; font[2]=0x08; font[3]=0x10; font[4]=0x7F; }
    else if(c=='C') { font[0]=0x3E; font[1]=0x41; font[2]=0x41; font[3]=0x41; font[4]=0x22; }
    else if(c=='L') { font[0]=0x7F; font[1]=0x40; font[2]=0x40; font[3]=0x40; font[4]=0x40; }
    else if(c=='S') { font[0]=0x26; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x32; }
    else if(c=='D') { font[0]=0x7F; font[1]=0x41; font[2]=0x41; font[3]=0x22; font[4]=0x1C; }
    else            { font[0]=0x00; font[1]=0x00; font[2]=0x00; font[3]=0x00; font[4]=0x00; }
    
    for(i=0; i<5; i++) { 
		OLED_Data(font[i]); 
	}
    OLED_Data(0x00); 
	_asm clrwdt _endasm 
}

void OLED_Print_Stare(int is_open) {
    OLED_Cmd(0xB0 + 4);   // pagina 4 - mijloc pe verticala
	OLED_Cmd(0x00 + 46); // mijloc pe orizontala (128-36)/2=46
	OLED_Cmd(0x10); // jumatatea coloanei
    if(is_open) {
        OLED_DrawChar('O'); OLED_DrawChar('P'); OLED_DrawChar('E'); OLED_DrawChar('N');
        OLED_DrawChar(' '); OLED_DrawChar(' '); 
    } else {
        OLED_DrawChar('C'); OLED_DrawChar('L'); OLED_DrawChar('O'); OLED_DrawChar('S'); 
        OLED_DrawChar('E'); OLED_DrawChar('D');
    }
}


// INTRERUPERI 

#pragma code my_section = 0x808   
void intrerupere (void) { _asm goto rutina_intrerupere _endasm }
#pragma code

#pragma interrupt rutina_intrerupere

void rutina_intrerupere(void) {
    if(INTCONbits.TMR0IF == 1) {
        INTCONbits.TMR0IF = 0; 
        WriteTimer0(45536); 
        
        LATCbits.LATC0 = 1; // Servo
        Delay10TCYx(pozitie_servo); // curent fix 1ms sau 1.5ms
        LATCbits.LATC0 = 0;
    }
}


// PROGRAMUL PRINCIPAL

void main(void) {
    char tasta; 
    
    // Fortam 4 MHz 
    OSCCONbits.IRCF2 = 1; 
	OSCCONbits.IRCF1 = 1; 
	OSCCONbits.IRCF0 = 0; 
	OSCCONbits.SCS1 = 1;

    ADCON1 = 0x0F; CMCON = 0x07;
    TRISCbits.TRISC0 = 0; // Servo - iesire
    TRISCbits.TRISC1 = 0; // Buzzer 
    
	// Tastatura 
	// linii- iesiri
	// coloane- intrari
    TRISEbits.TRISE2 = 0; TRISEbits.TRISE1 = 0; TRISEbits.TRISE0 = 0; TRISAbits.TRISA5 = 0; 
    TRISAbits.TRISA4 = 1; TRISAbits.TRISA3 = 1; TRISAbits.TRISA2 = 1; TRISAbits.TRISA1 = 1; 

    // Init I2C si OLED
    SSPSTAT = 0x80; // viteza standard
	SSPCON1 = 0x28; // activare pini seriali 
	SSPADD = 9; // viteza 100kHz - oscilator 4MHz
    delay_ms(100);
	// setari din fabrica - aprindere ecran, contrast ...
    OLED_Cmd(0xAE); OLED_Cmd(0x20); OLED_Cmd(0x02); OLED_Cmd(0x81); OLED_Cmd(0x7F);
    OLED_Cmd(0xA1); OLED_Cmd(0xC8); OLED_Cmd(0x8D); OLED_Cmd(0x14); OLED_Cmd(0xAF);
    OLED_Clear();
    OLED_Print_Stare(0); // Scrie CLOSED

    // Timer 0
    T0CONbits.T08BIT = 0; 
    T0CONbits.T0CS = 0;   // Servo
    T0CONbits.PSA = 1;   
    WriteTimer0(45536);
    INTCONbits.TMR0IF = 0; 
    INTCONbits.TMR0IE = 1; 
    T0CONbits.TMR0ON = 1; 
    INTCONbits.GIE = 1; 
    INTCONbits.PEIE = 1;

    beep_lung(); 

    while(1) {
        _asm clrwdt _endasm 
        
        tasta = scaneaza_tastatura();
        
        if(tasta != 0 && tasta != last_tasta) {
            beep_scurt(); 
            delay_ms(50); 
            
            if(stare_seif == 0 && tasta >= '0' && tasta <= '9') {
                PIN_introdus[index_pin] = tasta;
                index_pin++;
                
                if(index_pin == 4) {
                    if(PIN_introdus[0]==PIN_corect[0] && PIN_introdus[1]==PIN_corect[1] && 
                       PIN_introdus[2]==PIN_corect[2] && PIN_introdus[3]==PIN_corect[3]) {
                        
                        stare_seif = 1;      
                        pozitie_servo = 150; // muta pe OPEN
                        OLED_Print_Stare(1); // deseneaza OPEN
                        beep_lung();         
                        
                    } else {
                        index_pin = 0;       
                        beep_eroare();       
                    }
                }
            }
            
            // INCHIDERE SEIF CU #
            if(tasta == '#') {
                stare_seif = 0;
                index_pin = 0;
                pozitie_servo = 100; // muta inapoi pe CLOSED
                OLED_Print_Stare(0); // deseneaza CLOSED
                beep_lung();        
            }
        }
        last_tasta = tasta; 
    }
}


// FUNCTII

// Delay10KTCYx() dar in bucati - watchdog 
void delay_ms(unsigned int ms) {
    unsigned int i;
    for(i = 0; i < ms; i++) { Delay1KTCYx(1); _asm clrwdt _endasm }
}

void beep_scurt(void) { 
	LATCbits.LATC1 = 1; 
	delay_ms(50); 
	LATCbits.LATC1 = 0; 
}

void beep_lung(void) { 
	LATCbits.LATC1 = 1; 
	delay_ms(200); 
	LATCbits.LATC1 = 0; 
}

void beep_eroare(void) { 
	LATCbits.LATC1 = 1; 
	delay_ms(100); 
	LATCbits.LATC1 = 0; 
	delay_ms(100); 
	LATCbits.LATC1 = 1; 
	delay_ms(100); 
	LATCbits.LATC1 = 0; 
}

char scaneaza_tastatura(void) {
	// pun randurile pe 1
    LATEbits.LATE2 = 1; LATEbits.LATE1 = 1; LATEbits.LATE0 = 1; LATAbits.LATA5 = 1;
    
	// daca si coloana si randul sunt 0 
    LATEbits.LATE2 = 0; delay_ms(1); 
    if(!PORTAbits.RA4) return '1'; if(!PORTAbits.RA3) return '2'; if(!PORTAbits.RA2) return '3'; if(!PORTAbits.RA1) return 'A'; 
    LATEbits.LATE2 = 1; 
    
    LATEbits.LATE1 = 0; delay_ms(1); 
    if(!PORTAbits.RA4) return '4'; if(!PORTAbits.RA3) return '5'; if(!PORTAbits.RA2) return '6'; if(!PORTAbits.RA1) return 'B'; 
    LATEbits.LATE1 = 1;
    
    LATEbits.LATE0 = 0; delay_ms(1); 
    if(!PORTAbits.RA4) return '7'; if(!PORTAbits.RA3) return '8'; if(!PORTAbits.RA2) return '9'; if(!PORTAbits.RA1) return 'C'; 
    LATEbits.LATE0 = 1;
    
    LATAbits.LATA5 = 0; delay_ms(1); 
    if(!PORTAbits.RA4) return '*'; if(!PORTAbits.RA3) return '0'; if(!PORTAbits.RA2) return '#'; if(!PORTAbits.RA1) return 'D'; 
    LATAbits.LATA5 = 1;
    
    return 0; 
}