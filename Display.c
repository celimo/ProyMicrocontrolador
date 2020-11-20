/*
 * Interface Two PIC18F4550 microcontroller 
 * for communicating with each other using SPI Protocol
 * http://www.electronicwings.com
 */

#include <pic18f4550.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define _XTAL_FREQ 8000000
#define F_CPU 8000000

#pragma config WDT = OFF
#pragma config WDTPS = 1024

void USART_Init(long);
void USART_Tx(char);
char USART_Rx();

int x = 0; //variable para avanzar en la secuencia
int prender = 0; //para resolver casos donde el mismo led debe prenderse dos veces seguidas
int seq_done = 1; //indica si ya se mostr? toda la secuencia
int generar_secuencia = 1; //indica si se esta generando una nueva secuencia
int secuencia[6] = {0, 0, 0, 0, 0, 0}; //funcion que tome un valor random del arreglo data y lo coloque en secuencia
const int estado[3] = {1,2,4}; //vaores correspondientes a los botones/leds
int random = 0; //valor aleatorio entre 0 y 2
int ganador = 0; //indica la condici?n final de victoria del juego
int perdedor = 0; //indica la condici?n final de fallo del juego
int finfallo = 0; //controla cuando se encienden los leds de fin o fallo

int juego = 0;
// para el aleatorio
int aleatorio=0;
int duty_cicle = 100;

void main(){
    
    USART_Init(9600);
    ADCON1 = 0x0F;  // Se definen los pines digitales
    
    TRISAbits.RA0 = 0;  // Salida
    TRISAbits.RA1 = 0;  // Salida
    TRISD = 0;			/* PORT initialize as output */
    
    TRISB = 0;
    TRISBbits.RB3 = 0;  // Puerto se inicia como salida
    TRISBbits.RB4 = 0;  // Puerto se inicia como salida
    TRISBbits.RB5 = 0;  // Puerto se inicia como salida
    TRISBbits.RB6 = 0;  // Puerto se inicia como salida
    TRISBbits.RB7 = 0;  // Puerto se inicia como salida
    
    TRISCbits.RC1 = 0;  //Salida
    TRISCbits.RC2 = 0;  //Salida

    RCONbits.IPEN = 0;      // Se deshabilitan las de alta prioridad
    INTCONbits.GIE = 1;     // Se activan las interrupciones no enmascaradas
    INTCONbits.PEIE = 1;    // Se habilitan las interrupciones de perifericos
                            // no enmascadados
    INTCONbits.RBIE = 1;    // Interrupcion cuando RB cambia
    INTCONbits.INT0IE = 1;  // Se activa la interrupcion en INT0
    INTCON2bits.RBPU = 0;	// Enable internal Pull-up of PORTB
    PIE1bits.RCIE = 1;      // Habilita las interrupciones de EUSART
    
    LATBbits.LB3 = 1;
    LATBbits.LB4 = 0;
    LATBbits.LB5 = 0;
    LATBbits.LB6 = 0;
    LATBbits.LB7 = 0;
    
    LATD = 0x00;
    LATAbits.LA0 = 0;
    LATAbits.LA1 = 0;
    
    INTCONbits.RBIF = 0;
    
    //################# SETUP DEL CONTADOR ALEATORIO ####################
    //###################################################################
    PIE2bits.TMR3IE = 1;    // Se habilita el flag del timer3
    PIR2bits.TMR3IF = 0;    // Se baja el flag del timer3
    T3CONbits.RD16 = 1;     // Contador en 16 bits
    T3CONbits.TMR3CS = 0;   // Usa Clock Interno
    
    T3CONbits.T3CKPS0 = 0;  // 1:1 Preescala de 1
    T3CONbits.T3CKPS1 = 0;
    
    T3CONbits.TMR3ON = 1;   //Empezar contador
    
    while(1){
    }
}

void __interrupt() isr(void){
    
    if (PIR2bits.TMR3IF == 1){
        TMR3 = 0;
        PIR2bits.TMR3IF = 0;
    }
    
    int valores = 0;
    if (PIR1bits.RCIF == 1){  // Si recibe un dato
        if (LATAbits.LA1 == 0){
            juego = USART_Rx();
            if (juego == 1){
                LATAbits.LA1 = 1;
                LATAbits.LA0 = 0;
                //#############################################################
                //#############################################################

                // Setup Referente a Temporizador
                INTCONbits.TMR0IE = 1; // Enable de interrupcion por timer
                INTCONbits.TMR0IF = 0; //Flag de interrupcion en 0

                T0CONbits.T08BIT = 0; // Timer0 a 16 bit timer
                T0CONbits.T0CS = 0; // Usa Clock Interno
                T0CONbits.T0SE = 0; // Flanco positivo para la patilla T0CKl
                T0CONbits.PSA = 0; // Si usa pre escala

                T0CONbits.T0PS2 = 1;
                T0CONbits.T0PS1 = 1; // 1:256 Preescala de 256
                T0CONbits.T0PS0 = 1;

                T0CONbits.TMR0ON = 1; // Empieza el timer
                TMR0 = 0xFFF8; // Se inicia el Registro del Timer para dar un 
                               //tiempo de 1ms entre generación de numeros
            }
            if (juego == 2){
                srand(TMR3);
                LATD = rand()%15 + 1;
                LATAbits.LA1 = 1;
                LATAbits.LA0 = 0;
                LATB = 0;
                LATBbits.LATB3 = 0;
            }
            if (juego == 3){
                LATAbits.LA1 = 1;
                LATAbits.LA0 = 0;
                LATBbits.LATB3 = 0; // Quita el LED que indica selección
                aleatorio = 1;
                
                // Configuracion PWM
                //PWM CPP1
                PR2 = 0xFF;
                CCPR1L = 0xF0;
                T2CON = 0x03;   // Prescaler 16 Timer 2 OFF
                CCP1CON = 0x0C;
                TMR2 = 0;
                T2CONbits.TMR2ON = 1; //Timer 2 ON
                
                //PWM CPP2
                CCPR2L = 0x0F;
                T2CON = 0x03;   // Prescaler 16 Timer 2 OFF
                CCP2CON = 0x0C;
                TMR2 = 0;
                T2CONbits.TMR2ON = 1; //Timer 2 ON
                // FIN de la configuracion PWM
                srand(TMR3);
                CCPR1L = rand()%211 + 30;
                LATD = USART_Rx();
                CCPR2L = LATD;
                LATB = CCPR1L;
            }
        }else if (LATAbits.LA1 == 1){
            if (juego == 1){ // Condicion para el juego 1
                valores = USART_Rx();
                LATB = 0;
                if (valores == secuencia[x]) {
                    x = x + 1; //ir al siguiente valor de secuencia
                    if (valores == 4){
                        LATBbits.LATB5 = 1;
                    }else if (valores == 2){
                        LATBbits.LATB6 = 1;
                    }else if (valores == 1){
                        LATBbits.LATB7 = 1;
                    }
                } else {        //reinicio
                    x = 0;      //reinicia contador de secuencia
                    perdedor = 1;
                    LATD = 7; //prende led perdió en RA5
                    USART_Tx(0); //Enviar el dato 0 indica apagar botones
                    INTCONbits.TMR0IE = 1; // Enable de interrupci?n por timer
                    INTCONbits.TMR0IF = 0; //Flag de interrupcion en 0
                    T0CONbits.TMR0ON = 1; // Encender timer
                    TMR0 = 0xC2F7; // Se inicia el Registro del Timer para dar un tiempo de 2s             
                }
                if (x == 6) {// condicion de que si llega, gano
                    x = 0;
                    ganador = 1;
                    LATAbits.LA0 = 1; //prende led ganó en RA0
                    USART_Tx(0); //Enviar el dato 0 indica apagar botones
                    INTCONbits.TMR0IE = 1; // Enable de interrupci?n por timer
                    INTCONbits.TMR0IF = 0; //Flag de interrupcion en 0
                    T0CONbits.TMR0ON = 1; // Encender timer
                    TMR0 = 0xC2F7; // Se inicia el Registro del Timer para dar un tiempo de 2s  
                }
            }
            if (juego == 2){ // Juego de apagar luces
                valores = USART_Rx();
                LATBbits.LB4 = ((valores & 0b00000001) == 0b00000001);
                LATBbits.LB5 = ((valores & 0b00000010) == 0b00000010);
                LATBbits.LB6 = ((valores & 0b00000100) == 0b00000100);
                LATBbits.LB7 = ((valores & 0b00001000) == 0b00001000);

                if ((LATBbits.LB7 == LATDbits.LD0) && (LATBbits.LB6 == LATDbits.LD1)
                  && (LATBbits.LB5 == LATDbits.LD2) && (LATBbits.LB4 == LATDbits.LD3)){
                    LATAbits.LA0 = 1;
                    LATAbits.LA1 = 0;
                    juego = 0;
                    LATD = 0;
                    LATBbits.LB4 = 0;
                    LATBbits.LB5 = 0;
                    LATBbits.LB6 = 0;
                    LATBbits.LB7 = 0;
                    USART_Tx(0xFF);
                    LATBbits.LATB3 = 1;
                }
                else{
                    LATAbits.LA0 = 0;
                    LATAbits.LA1 = 1;
                }
            }
            if (juego == 3){    //PWM
                LATD = USART_Rx();
                CCPR2L = LATD;
                if((0.975*CCPR1L) <= CCPR2L){
                    if(CCPR2L <= (1.025*CCPR1L)){
                        aleatorio = 0;// Para el aleatorio
                        CCPR1L = 0;
                        LATAbits.LA0 = 1;
                        LATAbits.LA1 = 0;
                        juego = 0;
                        LATD = 0;
                        LATB = 0;
                        USART_Tx(0xFF);
                        LATBbits.LATB3 = 1;
                        T2CONbits.TMR2ON = 0; //Timer 2 OFF
                    }
                }
            }
        }
        
        PIR1bits.RCIF = 0;
    }
    
    //##########################################################################
    //################### SECCION DONDE SE GENERA LA SECUENCIA DE LUCES ########
    //##########################################################################
    
    if (generar_secuencia == 1) {   //se encuentra generando la nueva secuencia
        srand(TMR3); //agarra el valor del contador como la semilla para hacer el aleatorio
        // Handler particular para al interrupci?n TMR0: Toggle de la nueva secuencia
        if (INTCONbits.TMR0IF == 1) { //cada 1ms
            INTCONbits.TMR0IF = 0; // Se reinicia la Interrupcion
            TMR0 = 0xFFF8; // Se inicia el Registro del Timer para dar un tiempo de 1m entre generación de numeros
            if (x < 6) {
                random = rand() % 3; // se asigna un valor aleatorio entre 0 y 2
                secuencia[x] = estado[random];
                x = x + 1;
            } else {
                generar_secuencia = 0; //apaga el generar secuencia
                seq_done = 0; //empieza la secuencia
                x = 0; //reinicia contador de secuencia
                TMR0 = 0xE17C; // Se inicia el Registro del Timer para dar un tiempo de 1s
            }
        }
    }
    
    //##########################################################################
    //################### SECCION DONDE SE MUESTRA LA SECUENCIA DE LUCES #######
    //##########################################################################

    if (seq_done == 0) { //mientras no se haya terminado de mostrar la secuencia...
        // Handler particular para al interrupci?n TMR0: Toggle de la secuencia de LEDS
        if (INTCONbits.TMR0IF == 1) { //cada vez que pase 1s
            INTCONbits.TMR0IF = 0; // Se reinicia la Interrupcion
            TMR0 = 0xE17C; // Se inicia el Registro del Timer para dar un tiempo de 1s
            if (secuencia[x] == secuencia[x - 1]) { //permite secuencias con el mismo led varias veces seguidas
                if (prender == 0) { //pausa entre leds si hay repeticion
                    LATD = 0; //apaga todos los leds, dando tiempo de ver que son dos encendidos seguidos
                    prender = 1; //ya se dio el tiempo intermedio, continuar la secuencia
                } else { //ya se dio el tiempo intermedio, continuar la secuencia
                    LATD = secuencia[x]; // Toggle a la Salida
                    prender = 0; //generar pausa en caso de otra repeticion
                    x = x + 1; //pasar al Siguiente LED en la secuencia
                    if (x == 6) { //si ya se complet? la secuencia
                        x = 0; //volver al primer elemento de la secuencia
                        seq_done = 1; //indicar el fin de la secuencia
                        finfallo = 0;
                    }
                }
            } else {
                LATD = secuencia[x]; // Toggle a la Salida
                x = x + 1; //pasar al Siguiente LED en la secuencia
                if (x == 6) { //si ya se complet? la secuencia
                    x = 0; //volver al primer elemento de la secuencia
                    seq_done = 1; //indicar el fin de la secuencia
                    finfallo = 0;
                }
            }
        }
    } else if ((seq_done == 1) && (finfallo == 0)) { //al terminar la secuencia 
                                                     //y sin usar leds de 
                                                     //fin/fallo...
        if (INTCONbits.TMR0IF == 1) { //dar 1s de apagado al ultimo led
            INTCONbits.TMR0IE = 0; // Disable de interrupcion por timer
            INTCONbits.TMR0IF = 0; // Se reinicia la Interrupcion
            T0CONbits.TMR0ON = 0; // Apagar timer
            LATD = 0; //apaga todos los leds
            finfallo = 1; //seguro para evitar volver a entrar a esta condición
            USART_Tx(0xF0); //Enviar el dato 0xF0 indica activar botones
        }
    }
    
    //##########################################################################
    //############ SECCION DONDE SE DECLARA VICTORIA O FALLO DEL JUEGO #########
    //##########################################################################
    
    if((ganador == 1) && (INTCONbits.TMR0IF == 1)){
        ganador = 0;
        seq_done = 0; //volver a mostar nueva secuencia
        finfallo = 0;
        LATAbits.LA0 = 0; //apagar led ganó
        INTCONbits.TMR0IE = 0; // Disable de interrupci?n por timer
        INTCONbits.TMR0IF = 0; //Flag de interrupcion en 0
        T0CONbits.TMR0ON = 0; // Apagar timer
        TMR0 = 0xE17C; // Se inicia el Registro del Timer para dar un tiempo de 1s
        generar_secuencia = 1; //volver a llamar la función generadora de secuencias
        LATAbits.LA1 = 0;
        juego = 0;
        LATB = 0;
        TMR3 = 0;
        USART_Tx(0xFF);
        LATBbits.LATB3 = 1;
    }
    
    if((perdedor == 1) && (INTCONbits.TMR0IF == 1)){
        perdedor = 0;
        seq_done = 0; //volver a mostar nueva secuencia
        finfallo = 0;
        LATD = 0; //apagar led perdio
        INTCONbits.TMR0IF = 0; //Flag de interrupcion en 0
        TMR0 = 0xE17C; // Se inicia el Registro del Timer para dar un tiempo de 1s
        generar_secuencia = 1; //volver a llamar la función generadora de secuencias
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