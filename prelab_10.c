/* 
 * File:   prelab_10.c
 * Author: sando
 *
 * Created on 2 de mayo de 2022, 03:23 PM
 */

#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 1000000 

// ---------- variables ---------- //
char mensaje[5] = {'D', 'a', 'b', 'o', ',', ' '};
uint8_t indice = 0;
char turno = 1;

// ---------- Prototipos ---------- //
void setup(void);
void isr (void);

// ---------- Interrupciones ---------- //
void __interrupt() isr (void){
    if(PIR1bits.RCIF){              // hay un dato en el seria?
        PORTB = RCREG-48; 
        turno = 0;
    }
    return;
}
    

void main(void) {
    setup();    
    while(1){
        __delay_ms(50);
        //printf("\r introduzca un numero: \r");
        if(PIR1bits.TXIF && turno == 0){
            TXREG = PORTB+48;
            PIR1bits.TXIF = 0;
            turno = 1;
        }      
    }
    return;
}

void setup(void){
  ANSEL = 0;
  ANSELH = 0;       // I/O digitales
 
  
  TRISB = 0X00;
  PORTB = 0X00;
  
// --------------- CONFIGURACION DE OSCILADOR --------------- //    
  OSCCONbits.IRCF = 0b100;     // iMHz
  OSCCONbits.SCS = 1;           // reloj interno
  
// --------------- CONFIGURACION DEL SERIAL --------------- // 
  TXSTAbits.SYNC = 0;
  TXSTAbits.BRGH = 1;
  BAUDCTLbits.BRG16 = 1;
  SPBRGH = 0;
  SPBRG = 25;                   // BAUD RATE APROX 9600
  
  RCSTAbits.SPEN = 1;           // HABILITMOS COMUNICACION SERIAL
  RCSTAbits.RX9 = 0;            // solo 8 bits
  
  TXSTAbits.TXEN = 1;           // habilitamos transmisor
  RCSTAbits.CREN = 1;           // habilitamos receptor
  
  INTCONbits.GIE = 1;           // Habilitamos interrupciones globales
  INTCONbits.PEIE = 1;          // Habilitamos interrupciones perifericas
  PIE1bits.RCIE = 1;            // Habilitamos interrupcion de recepcion 
    
}  
 
 