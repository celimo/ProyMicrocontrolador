/*
 * Interface Two PIC18F4550 microcontroller 
 * for communicating with each other using SPI Protocol
 * http://www.electronicwings.com
 */

#include <pic18f4550.h>
#define F_CPU 8000000

void USART_Init(long);
void USART_Tx(char);
char USART_Rx();
void MSdelay(unsigned int);

int juego = 0;
int ADC3 = 0;
int delgame1 = 0;

void main() 
{
    
    RCONbits.IPEN = 0;  // Se deshabilitan las de alta prioridad
    INTCONbits.GIEH = 1;  // Se activan las interrupciones no enmascaradas
    INTCON2bits.RBPU = 0;  // Enable internal Pull-up of PORTB
    
    TRISD = 0;  // Todos los puertos D como salida
    LATD = 0;  // Se le asigna el valor de 0 a RD
    ADCON1 = 0x0E;  // Se define el PIN RA0 como analógico
    
    TRISBbits.RB0 = 1;      // Puerto RB0 como entrada / Comenzar juego
    TRISBbits.RB4 = 1;      // Puerto RB4 como entrada
    TRISBbits.RB5 = 1;      // Puerto RB5 como entrada
    TRISBbits.RB6 = 1;      // Puerto RB6 como entrada
    TRISBbits.RB7 = 1;      // Puerto RB7 como entrada
    
    // ===========================
    // Configuración para la entrada analógica
    TRISAbits.RA0 = 1;      // El puerto RA1 como entrada

    // Inicialmente se elige el Canal AN0
    ADCON0bits.CHS3 = 0;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;

    // Selección de Tiempo de Adquisición y TAD
    ADCON2bits.ACQT = 0b001; // Tacq=2Tad minimo
    ADCON2bits.ADCS = 0b000; // Tad=2*Tosc (or Fosc/2)

    // Selección de Justificación del Dato
    // Inicialmente se usa Justificación Derecha
    ADCON2bits.ADFM = 1;
    
    // ===========================
    
    TRISAbits.RA4 = 1;      // Puerto RA4 como entrada de SELEC0
    TRISAbits.RA5 = 1;      // Puerto RA5 como entrada de SELEC1
    
    USART_Init(9600);
    
    INTCONbits.RBIE = 1;  // Se habilita la interrupcion de RB
    INTCONbits.RBIF = 0;  // Se vacia el flag de entrada RB4:RB7
    
    INTCONbits.INT0IF   = 0;   // Se limpia la bandera de INT0
    INTCON2bits.INTEDG0 = 1;   // INT0 en flanco positivo
    INTCONbits.INT0IE   = 1;   // Enable de INT0
    
    PIE1bits.RCIE = 1;   // Se habilida la interrupcion de llegada de datos
    PIR1bits.RCIF = 0;  // Se limpia el Flag de llegada de datos
    
    while(1){
        if (juego == 3){
            ADCON0bits.GO_DONE = 1;
            while( ADCON0bits.GO_DONE == 1);
            ADC3 = ADRESL;
            LATD = ADC3;
            USART_Tx(LATD);
            MSdelay(100);
        }
    }
}

void __interrupt() isr(void){
    int gano = 0;
    
    if (INTCONbits.INT0IF == 1){
        if (juego == 0){
            if (PORTAbits.RA5 == 1 && PORTAbits.RA4 == 1){
                USART_Tx(0x01);     // Se envia el código del juego
                juego = 1;
            }
            if (PORTAbits.RA5 == 1 && PORTAbits.RA4 == 0){
                LATD = 0;
                juego = 2;
                USART_Tx(0x02);       // Se enviar el código del juego
            }
            if (PORTAbits.RA5 == 0 && PORTAbits.RA4 == 1){
                // Enable de ADC
                ADCON0bits.ADON = 1;
                juego = 3;
                USART_Tx(0x03);
            }
        }
        
        INTCONbits.INT0IF = 0;
    }
    
    if (INTCONbits.RBIF == 1){ // Cuando cambia alguno desde el RB4:RB7
        if (juego == 1){
            if (PORTBbits.RB5 == 0){
                USART_Tx(4);
            }
            //////////////////////////////////////////////////////
            /////////////// SI SE TOCA EL BOTON RB6 //////////////
            //////////////////////////////////////////////////////
            if (PORTBbits.RB6 == 0){
                USART_Tx(2);
            }
            //////////////////////////////////////////////////////
            /////////////// SI SE TOCA EL BOTON RB7 //////////////
            //////////////////////////////////////////////////////
            if (PORTBbits.RB7 == 0){
                USART_Tx(1);
            }
        }else if (juego == 2){
            if (PORTBbits.RB4 == 0){
            LATDbits.LATD0 = ~LATDbits.LATD0;
            }
            if (PORTBbits.RB5 == 0){
                LATDbits.LATD1 = ~LATDbits.LATD1;
            }
            if (PORTBbits.RB6 == 0){
                LATDbits.LATD2 = ~LATDbits.LATD2;
            }
            if (PORTBbits.RB7 == 0){
                LATDbits.LATD3 = ~LATDbits.LATD3;
            }
            USART_Tx(LATD);
        }
        INTCONbits.RBIF = 0;
    }
    if (PIR1bits.RCIF == 1){  // Si recibe un dato
        gano = USART_Rx();
        
        if(gano == 0xFF){
            LATD = 0;
            juego = 0;
            // Enable de ADC
            ADCON0bits.ADON = 0;
        }
        PIR1bits.RCIF = 0;
    }
}

// =============================================== //
// =========== Configuracion USART =============== //
// =============================================== //

void USART_Init(long BAUD){
    //Configuración de los pines
    TRISCbits.RC6 = 0;      //RC6 = Tx -> Salida
    TRISCbits.RC7 = 1;      //RC7 = Rx -> Entrad
    
    //Baudios
    SPBRG = (unsigned char)(((F_CPU/BAUD)/64)-1);
    
    //Configuración 
    TXSTAbits.BRGH = 0;     //Low Speed
    TXSTAbits.SYNC = 0;     //Asincrono
    RCSTAbits.SPEN = 1;     //Habilitar Tx y Rx
    
    //Transmisión
    TXSTAbits.TX9 = 0;      //8 bits
    TXSTAbits.TXEN = 1;     //Activamos transmisión
    
    //Recepción
    RCSTAbits.RC9 = 0;      //8 bits
    RCSTAbits.CREN = 1;     //Activamos recepción
}

void USART_Tx(char data){
    TXREG = data;
}

char USART_Rx(){
    return RCREG; 
}

/*************************Delay Function****************************/
void MSdelay(unsigned int val)	/* Delay of 1 ms for 8MHz Freq. */
{
     unsigned int i,j;
        for(i = 0; i < val; i++)
            for(j = 0; j < 165; j++);
}