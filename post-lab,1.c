/*
 * File:   post-lab,1.c
 * Author: sando
 *
 * Created on 7 de mayo de 2022, 12:29 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
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

//--------------- CONSTANTES --------------- //
#define _XTAL_FREQ 1000000          // FREQ. 1MHz

//--------------- VARIABLES --------------- //
uint8_t i = 0;                      // Posición del mensaje 
uint8_t i2 = 0;                     // Bandera para enviar potenciómetro
uint8_t valor_pot = 0;              // Valor del potenciómetro 
uint8_t valor_ASCII = 0;            // Valor en ASCII
uint8_t modo = 0;                   // Modo de operación 
char    dato[3] = {0,0,0};          // Mensaje del potenciometro
char    str;                        // Cadena de texto
char turno = 1;

// --------------- PROTOTIPO DE FUNCIONES --------------- //
void setup(void);                       // Configuraciones 
void calcular_valor(uint8_t valor);     // Obtener centenas, decenas y unidades en ASCII
void imprimir(char *str);               // Imprimir una cadena de texto 
void TX_usart(char data);               // Enviar valores de datos 

//--------------- INTERRUPCIONES --------------- //
void __interrupt() isr (void){
    if(PIR1bits.RCIF){                  // Se recibió un dato?
        valor_ASCII = RCREG;            // Valor que se recibió
        if(modo == 1){                  // Si está en modo enviar
            PORTB = valor_ASCII;        // Colocar ASCII en puerto B
            modo = 0;                   // Modo normal de regreso
            i = 0;                      // Se imprime el menu
        }
        turno = 0;
    }
    else if (PIR1bits.ADIF){            // Interrupcion de ADC
        valor_pot = ADRESH;             // Valor del potenciometro
        PIR1bits.ADIF = 0;              // Se limpia bandera de interrupcion
    }
    return;
}

// --------------- CICLO PRINCIPAL  --------------- //
void main(void) {
    setup();
    while(1){
        if(ADCON0bits.GO == 0){         // No hay proceso de conversion
            ADCON0bits.GO = 1;          // Se inicia el proceso de conversion
        }
        if (i == 0){                    // Imprimir la cadena de texto
            imprimir("\r\r Presione 1 para leer el potenciometro \r Presione 2 para enviar un caracter ASCII \r Presione 3 para leer el caracter ASCII  \r");
            i = 1;                      // Dejar de imprimir
        }
        if (valor_ASCII == '1'){        // Selección 1 del menu
            i = 0;                      // Imprimir el menu
            modo = 0;                   // Modo 1
            calcular_valor(valor_pot);   // Obtener centenas, unidades y decenas en ASCII
            i2 = 0;                     // Bandera para imprimir valor del potenciometro
            if (i2 == 0){     
                imprimir("\r El valor del potenciometro es: \n");
                TX_usart(dato[0]);      // Imprmir centenas
                TX_usart(dato[1]);      // Imprimir decenas
                TX_usart(dato[2]);      // Imprimir unidades
                i2 = 0;                 // Dejar de imprimir
            }
            valor_ASCII = 0;            // Limpiar el valor de ASCII
        }
        if (valor_ASCII == '2'){        // Seleccion 2 del menu
            modo = 1;                   // Cambio de modo 
            valor_ASCII = 0;            // Limpiar el valor de ASCII
        }
        if (valor_ASCII == '3'){
            if(PIR1bits.TXIF && turno == 0){
                imprimir("\r El valor del ASCII es: \n");
                TXREG = PORTB;
                turno = 1;
            }
            valor_ASCII = 0;            // Limpiar el valor de ASCII
        }
    }
    return;
}

// --------------- CONFIGURACION --------------- //  
void setup(void){
    ANSELH = 0;         // I/O digitales)
    ANSEL = 0x01; // AN0 como entrada analógica
    
    TRISA = 0x1; // AN0 como entrada
    PORTA = 0; 
    
    TRISB = 0x00; // Puerto B como salida
    PORTB = 0;
    
// --------------- CONFIGURACION DE OSCILADOR --------------- //   
    OSCCONbits.IRCF = 0b0100;   // 1MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
// --------------- CONFIGURACION DEL ADC --------------- //   
    ADCON0bits.ADCS = 0b00;     // 1MHz
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Seleccionamos el AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(20);             // Sample time

// --------------- CONFIGURACION DEL SERIAL --------------- //   
    TXSTAbits.SYNC = 0;         // Asincrónica
    TXSTAbits.BRGH = 1;         // Baud rate de alta velocidad 
    BAUDCTLbits.BRG16 = 1;      // 16 bits para baud rate 
    SPBRGH = 0;
    SPBRG = 25;                 // BAUD RATE APROX 9600
        
    RCSTAbits.SPEN = 1;         // HABILITMOS COMUNICACION SERIAL
    RCSTAbits.RX9 = 0;          // Solo 8 bits
    
    TXSTAbits.TXEN = 1;         // habilitamos transmisor
    RCSTAbits.CREN = 1;         // habilitamos receptor

// --------------- CONFIGURACION DE INTERRUPCIONES --------------- //  
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de perifericos
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    PIE1bits.RCIE = 1;          // Habilitamos interrupciones de recepción
}

// --------------- FUNCIONES --------------- //
void calcular_valor(uint8_t valor){
    dato[0] = valor/100;                            // Centenas
    dato[1] = (valor-dato[0]*100)/10;               // Decenas
    dato[2] = valor-dato[0]*100-dato[1]*10;         // Unidades
    
    dato[0] = dato[0]+0x30;
    dato[1] = dato[1]+0x30;
    dato[2] = dato[2]+0x30;
    
    dato[3] = PORTB;
    dato[4] = valor_ASCII;
}
void imprimir(char *str){
    while (*str != '\0'){        // Cuando no esté vacío
        TX_usart(*str);          // Enviar elemento de str
        str++;                   // Enviar siguiente elemento
    }
}

void TX_usart(char data){
    while(TXSTAbits.TRMT == 0); // Esperar a que se vacie
    TXREG = data;               // Enviar dato especificado
}