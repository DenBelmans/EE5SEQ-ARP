#include "initialization.h"
#include <xc.h>

void initialize_pins(void);
void initialize_adc(void);
void initialize_TMR0(void);
void initialize_TMR1(void);
void initialize_TMR2(void);
void initialize_TMR3(void);
void initialize_EUSART(void);
void initialize_MSSP(void);
void initialize_EEPROM(void);
void initialize_interrupts(void);

//Initialize all hardware peripherals
void initialize(void){
    initialize_pins();
    initialize_adc();
    initialize_EUSART();
    initialize_MSSP();
    initialize_TMR0();
    initialize_TMR1();
    initialize_TMR2();
    initialize_TMR3();
    initialize_EEPROM();
    initialize_interrupts();
}

void initialize_pins(void){
    LATA = 0x00;                //Set outputs off
    LATB = 0x00;                //Set outputs off
    LATC = 0x00;                //Set outputs off
    
    TRISAbits.TRISA0 = 1;       // Analog 16 button input
    TRISAbits.TRISA1 = 1;       // Analog rotary switch input
    TRISAbits.TRISA2 = 0;       // Tempo LED
    TRISAbits.TRISA3 = 0;       // Velocity_mode LED
    TRISAbits.TRISA4 = 0;       // OCT LED
    TRISAbits.TRISA5 = 1;       // ARP/SEQ switch
    
    TRISBbits.TRISB0 = 1;       // I2C SDA set as 1
    TRISBbits.TRISB1 = 1;       // I2C SCL set as 1
    TRISBbits.TRISB2 = 1;       // Right arrow button
    TRISBbits.TRISB3 = 1;       // Left arrow button
    TRISBbits.TRISB4 = 1;       // Alt
    TRISBbits.TRISB5 = 1;       // Shift
    TRISBbits.TRISB6 = 1;       // Play/Pause/Retrieve/Save button
    TRISBbits.TRISB7 = 1;       // Rest/Legato/2nd Oct button
    
    TRISCbits.TRISC0 = 1;       // µcontroller button
    TRISCbits.TRISC1 = 1;       // Rotary encoder A
    TRISCbits.TRISC2 = 1;       // Rotary encoder B
    TRISCbits.TRISC6 = 1;       // MIDI OUT  set as 1
    TRISCbits.TRISC7 = 1;       // MIDI IN set as 1
}

void initialize_adc(void){
    ADCON0bits.CHS3 = 0;
    ADCON0bits.CHS2 = 0;        //Channel 0 selected
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;
    ADCON0bits.GO_NOT_DONE = 0; //ADC Idle
    ADCON0bits.ADON = 1;        //ADC enabled
    ADCON1 = 0x0D;              //Use two ADC channels, source voltage is reference
    ADCON2bits.ADFM = 0;        //Left justified
    ADCON2bits.ACQT2 = 1;
    ADCON2bits.ACQT1 = 1;       //Acquisition time = 20 TAD. Not long enough due to high impedance
    ADCON2bits.ACQT0 = 1;
    ADCON2bits.ADCS2 = 1;
    ADCON2bits.ADCS1 = 1;       //Fosc/64 as ADC clock
    ADCON2bits.ADCS0 = 0;
    PIR1bits.ADIF = 0;          //Clear AD interrupt flag
}

void initialize_TMR0(void){
    T0CONbits.T08BIT = 0;       //16-bit timer
    T0CONbits.T0CS = 0;         //Internal clock source
    T0CONbits.PSA = 0;          //Use prescaler
    T0CONbits.T0PS2 = 1;        //
    T0CONbits.T0PS1 = 1;        //Set prescaler to 256
    T0CONbits.T0PS0 = 1;        //
    TMR0H = 0xA4;               //
    TMR0L = 0x72;               //Set initial tempo to 120 bpm
}

void initialize_TMR1(void){
    T1CONbits.RD16 = 0;         //Register read/write in two 8-bit operations
    T1CONbits.T1RUN = 0;        //Device clock derived from another source
    T1CONbits.T1CKPS1 = 0;      //Prescaler 1:1
    T1CONbits.T1CKPS1 = 0;      
    T1CONbits.T1OSCEN = 0;      //Disable T1 oscillator
    T1CONbits.TMR1CS = 0;       //Use internal clock (Fosc/4)
}

void initialize_TMR2(void){
    T2CONbits.TOUTPS3 = 1;      //Prescaler 16
    T2CONbits.TOUTPS2 = 1;
    T2CONbits.TOUTPS1 = 1;
    T2CONbits.TOUTPS0 = 1;      //Postscaler 16
    T2CONbits.T2CKPS1 = 1;
}

void initialize_TMR3(void){
   T3CONbits.RD16 = 0;
   T3CONbits.T3CCP2 = 0;
   T3CONbits.T3CCP1 = 0;
   T3CONbits.T3CKPS1 = 1;
   T3CONbits.T3CKPS0 = 1;
   T3CONbits.nT3SYNC = 0;
   T3CONbits.TMR3CS = 0;
}

void initialize_EUSART(void){
    TXSTAbits.TX9 = 0;          //8-bit transmission
    TXSTAbits.SYNC = 0;         //Asynchronous mode
    TXSTAbits.TXEN = 1;         //Enable transmission
    TXSTAbits.SYNC = 0;         //Sync break transmission completed
    TXSTAbits.BRGH = 0;         //Low speed baud rate
    
    RCSTAbits.SPEN = 1;         //Enable serial port
    RCSTAbits.RX9 = 0;          //8-bit reception
    RCSTAbits.CREN = 1;         //Enable reception
    
    BAUDCONbits.RXDTP = 0;      //RX data is not inverted
    BAUDCONbits.TXCKP = 0;      //TX data is not inverted
    BAUDCONbits.BRG16 = 0;      //8-bit Baud Rate Generator
    BAUDCONbits.ABDEN = 0;      //Disable baud rate measurement
    
    SPBRG = 0x17;               //Set Baud rate to 31250 bps
}

void initialize_MSSP(void){
    SSPSTATbits.SMP = 0x00;     //Slew rate control enabled (400kHz mode)
    SSPCON1bits.SSPEN = 1;      //Enable serial port
    SSPCON1bits.SSPM3 = 1;
    SSPCON1bits.SSPM2 = 0;
    SSPCON1bits.SSPM1 = 0;      //I²C master mode, clk = Fosc/(4x(SSPADD+1))
    SSPCON1bits.SSPM0 = 0;
    SSPSTAT = 0x00;             //Reset SSPSTAT register
    SSPCON2 = 0x00;             //Reset Control Register
    SSPADD = 0x1D;              //Set I²C clock speed: 400kHz
}

void initialize_EEPROM(void){
    EECON1bits.EEPGD = 0;       //Access the data EEPROM memory
    EECON1bits.CFGS = 0;        //Access data EEPROM memory
    EECON1bits.FREE = 0;        //Write-only (not relevant for data EEPROM)
    EECON1bits.WRERR = 0;       //Write operation completed
    EECON1bits.WREN = 0;        //Disable writes to EEPROM
    EECON1bits.WR = 0;          //Write cycle complete
    EECON1bits.RD = 0;          //Do not initiate an EEPROM read
}

void initialize_interrupts(void){
    INTCONbits.PEIE = 1;        //Enable peripheral interrupts
    IPR1bits.ADIP = 1;          //AD interrupt is high priority
    RCONbits.IPEN = 1;          //Enable priority levels on interrupts
    PIE1bits.ADIE = 1;          //Enable AD interrupts
    PIE1bits.RCIE = 1;          //Enable EUSART receive interrupts
    INTCONbits.TMR0IE = 1;      //Enable TMR0 interrupts
    PIE1bits.TMR1IE = 0;        //Disable TMR1IF, poll during waiting
    PIE1bits.TMR2IE = 1;        //Enable TMR2 interrupts
    PIE2bits.TMR3IE = 1;        //Enable TMR3 interrupts
    PIR1bits.SSPIF = 0;         //Disable MSSP Interrupts
    
    INTCONbits.GIE  = 1;        //Global interrupt enable
}