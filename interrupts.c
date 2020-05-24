#include "utilities.h"
#include "interrupts.h"
#include <xc.h>

void AD_interrupt(void){
    PIR1bits.ADIF = 0;          //clear interrupt flag
    if (ADCON0bits.CHS0 == 0){  //AD channel 0
        selected_button = voltage_to_button(ADRESH);
        ADCON0bits.CHS0 = 1;
    }
    else {
        rotary_switch = voltage_to_switch_position(ADRESH);
        ADCON0bits.CHS0 = 0;
    }
    T3CONbits.TMR3ON = 1;
}

void EUSART_receive_interrupt(void){
    PIR1bits.RCIF = 0;
    if (LATAbits.LATA3 == 1){
    }
    else {
        LATA4 = 0;
        LATA3 = 1;
    }
}

void TMR0_interrupt(void){ 
    INTCONbits.TMR0IF = 0;          //Clear TMR0 interrupt flag

    nth_note++;                     
    if (nth_note > notes_per_beat) nth_note = 1;
    if (nth_note == 1){
        LATAbits.LATA2 = 1;         //Toggle LED
        //Transmit clock signal over MIDI
    }
    else LATAbits.LATA2 = 0;        //Toggle LED
    
    play_note
    
    TMR0H = 255-(tempo >> 8);       //set TMR0H
    TMR0L = 255-tempo;              //Set TMR0L
    T0CONbits.TMR0ON = 1;           //Restart Timer0
}

void TMR3_interrupt(void){
    PIR2bits.TMR3IF = 0;
    T3CONbits.TMR3ON = 0;
    TMR3H = 0x00;
    TMR3L = 0x00;
    ADCON0bits.GO_NOT_DONE = 1;
}

void Interrupt_on_change_interrupt(void){
    if(!octave) octave = true;
    else octave = false;
    /*
    //If a change is observed on PORTB check for input
    INTCONbits.RBIF = 0;
    if (!sequencer){                    //If machine is in arpegiator-mode
        if (PORTBbits.RB3 == 1){        //If shift is pressed
            if (PORTBbits.RB4) {        //Left arrow
                //?
            }
            else if (PORTBbits.RB5) {   //Right arrow
                //?
            }
            else if (PORTBbits.RB6) {   //Play/Pause/Save
                //?
            }
            else if (PORTBbits.RB7) {   //Rest/Legato/Oct
                if (octave) octave =0;  //Toggle octave mode
                else octave = 1;
            }
        }
        else {                          //If shift is not pressed
            if (PORTBbits.RB4) {        //Left arrow
                //?
            }
            else if (PORTBbits.RB5) {   //Right arrow
                //?
            }
            else if (PORTBbits.RB6) {   //Play/Pause/Save
                //?
            }
            else if (PORTBbits.RB7) {   //Rest/Legato/Oct
                if (octave) octave =0;  //Toggle octave mode
                else octave = 1;
            }
        }
    }
    else {                              //If machine is in sequencer-mode
        if (PORTBbits.RB3){        //If shift is pressed
            if (PORTBbits.RB4) {        //Left arrow
                if (PORTBbits.RB2){     //If Alt is pressed
                    //Decrease denominator
                }
                else; //Go to previous bank
            }
            else if (PORTBbits.RB5) {   //Right arrow
                if (PORTBbits.RB2){     //If Alt is pressed
                    //Increase denominator
                }
                else; //Go to next bank
            }
            else if (PORTBbits.RB6) {   //Play/Pause/Save
                if (PORTBbits.RB2){     //If Alt is pressed
                    //Save sequence
                }
                else;   //Retrieve sequence
            }
            else if (PORTBbits.RB7) {   //Rest/Legato/Oct
                //Insert Legato
                octave = 0;
            }
        }
        else {                          //If shift is not pressed
            if (PORTBbits.RB4) {        //Left arrow
                if (PORTBbits.RB2){     //If Alt is pressed
                    //Decrease numerator
                }
                //Go to previous set of 16 notes
            }
            else if (PORTBbits.RB5) {   //Right arrow
                if (PORTBbits.RB2){     //If Alt is pressed
                    //Increase numerator
                }
                //Go to next set of 16 notes
            }
            else if (PORTBbits.RB6) {   //Play/Pause/Save
                if (playing) playing =0;      //Toggle play/Pause
                else playing = 1;
            }
            else if (PORTBbits.RB7) {   //Rest/Legato/Oct
                //Insert rest
                octave = 0;
            }
        }
    }

     */
}
