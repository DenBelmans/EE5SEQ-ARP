/* 
 * File:   main.c
 * Author: Sander
 *
 * Created on 24 maart 2020, 15:58
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include "main.h"
#include "initialization.h"
#include "sequence.h"
#include "utilities.h"
#include "I2C.h"
#include "MIDI.h"

#define true 1
#define false 0

/*
 * Attributes
 */
unsigned char ADchannel = 0;        //variable that switches between channel 0 and 1
sequence_t sequence;                //global sequence variable
message_t MIDI_message;             //this MIDI_message contains the midi data when it's not yet complete. Once complete it is saved in the MIDI_buffer.
MIDI_buffer_t MIDI_buffer;          //Buffer to contain fast consecutive MIDI messages before they can be handled.

//Start-up configuration that are not changed by first input reading
unsigned int tempo = 23438;         //Default is 120bpm
unsigned int TMR2_count = 0;        //Additional software timer for blinking the active note LED.
unsigned char bank = 0;             //Start up in bank 1. Bank refers to sequence number in the manual of the device.
unsigned char set = 0;              //Start up in the first set. set number 0 is displayed as set 1 on the device.
unsigned char nth_note = 0;         //Variable which keeps track of current note per beat

//Start-up configuration that will change after first input reading
bool sequencer = false;             //Start up in Arpeggiator mode
bool shift = false;                 //Shift is not pressed
bool alt = false;                   //Alt is not pressed
bool playing = false;               //Start up in pause mode
unsigned char rotary_switch = 0;    //Start up with rotary switch position 0
unsigned char selected_button = 0;  //Non of the 16 buttons pressed
unsigned char prev_selected_button = 0; //Variable to check whether or not the selected button has changed since previous loop
unsigned char root = 0;             //Variable stores the pressed note when using the root mode of the sequencer
bool previous_rotA = true;          //Variable to chech whether or not the rotary switch has been turned since last loop
bool new_note = false;              //Variable that is set to true if a new note has to be played
bool new_arpeggio = false;          //Variable that is set to true when the arpeggio has to be restarted
bool velocity_mode = false;         //Start up with velocity mode deactivated
bool active_led = false;            //Variable that makes the active LED blink.
bool button_prev[4];                //Boolean array that stores button values of previous loop
bool hold = false;                  //Hold variable

int main() {
    initialize();                   //configure PIC-peripherals see initialization.c
    all_notes_off();                //Stop any note that might still be playing due to a reset
    setup_IOexpanders();            //Set default register values of IO expanders
    
    //Initialize random number generator
    unsigned char random = read_EEPROM(255); //The value in EEPROM address 255 is incremented everytime this device boots, this is used as seed for the random number generator
    srand(random);
    write_EEPROM(255, random++);
    
    //Initialize sequence parameters
    sequence.length = 0;
    sequence.notes_per_beat = 0;
    sequence.active_note = 0;
    
    //Initialize MIDI_buffer
    MIDI_message.status = 0;
    MIDI_message.data1 = 0;
    MIDI_message.data2 = 0;
    MIDI_buffer.length = 0;
    
    //Start timers and ADC
    ADCON0bits.GO_NOT_DONE = 1;     //Start AD conversion
    T0CONbits.TMR0ON = 1;           //Start Timer0
    T2CONbits.TMR2ON = 1;           //Start Timer2

    //Main while loop
    while(1){
        check_input();      //Read all inputs
        /*
         * In arpeggiator mode, if the arpeggio has to be restarted, the midi buffer is checked and read if not empty
         * after which the sequence is sorted based on the position of the rotary switch
         * Then if a new note has to be played we advance the note in the sequence which outputs the MIDI data.
         */
        if (!sequencer){ //Main loop in arpeggiator mode
            if (new_arpeggio) {
                if (MIDI_buffer.length != 0) read_buffer(&MIDI_buffer, &sequence);
                sort_sequence(rotary_switch, &sequence);
            }
            if(new_note && (sequence.length !=0)){
                advance_note(&sequence);
                new_note = false;
            }
        }
        /*
         * In sequencer mode, if playing check if a new note has to be played and in that case, advance the note
         * else check for new input if the rotary switch is in position 4.
         */
        else { //Main loop in sequencer mode
            if (playing) {
                if(new_note && (sequence.length !=0)){
                    advance_note(&sequence);
                    new_note = false;
                }
            }
            else if(MIDI_buffer.length != 0 && rotary_switch == 4) read_buffer(&MIDI_buffer, &sequence);
        }
        set_output();       //Set correct output based on inputs
    }
    return 0;
}
/*
 * This function checks the state of the mode switch and handles changes to it
 * The state of the alt and shift buttons is read and saved in the corresponding variables
 * The rotary encoder is checked for input and the tempo is changed appropriately
 * The same thing goes for the 16 buttons.
 * Then the state of all the other buttons is checked and the correct actions are triggered when necessary
 */
static void check_input(void){
    
    //Set correct mode
    bool prev_sequencer = sequencer;
    sequencer = PORTAbits.RA5;
    if(prev_sequencer != sequencer){ //Clear sequence when switching modes
        all_notes_off();
        if(!sequencer){
            sequence.length = 0;
            sequence.notes_per_beat = 0;
            sequence.active_note = 0;
            T0CONbits.TMR0ON = 1;
        }
        else {
            sequence.length = 0;
            sequence.notes_per_beat = 4;
            sequence.active_note = 0;
            T0CONbits.TMR0ON = 0;
            hold = false;
        }
    }
    //Check state of shift and alt buttons
    shift = PORTBbits.RB5;
    alt = PORTBbits.RB4;
    
    //Check for rotary encoder activity. This method is found in utilities.c
    read_rotary_encoder(&previous_rotA, &tempo);
    
    //Check for input on the 16 LEDs
    if(selected_button != 0 && selected_button != prev_selected_button) {
        if(!playing && (set*16+selected_button-1) <= sequence.length) sequence.active_note = set*16+selected_button-1;
        else if(set*16+selected_button-1 < sequence.length && sequence.notes[set*16+selected_button-1] != 128) sequence.notes[set*16+selected_button-1] += 128;
    }
    prev_selected_button = selected_button;
    
    if (sequencer){ //In sequencer mode. read_button returns true if the button was 0 on previous loop and now 1.
        if(read_button(PORTBbits.RB2, &button_prev[0])) { //Right button action
            if(!alt && !shift){ //Increase set
                set++;
                if(set > get_set_number()) set = 0;
            }
            else if(!alt && shift){ //Increase sequence number
                bank++;
                if(bank > 31) bank = 0;
            }
            else if(alt && !shift){ //Increase notes per beat
                if (sequence.notes_per_beat < 32) sequence.notes_per_beat++;
            }
            else if (alt && shift){ //Nop
            }
        }
        if(read_button(PORTBbits.RB3, &button_prev[1])) { //Left button action
            if(!alt && !shift){ //Decrease set
                if (set > 0){
                set--;
                }
            }
            else if(!alt && shift){ //Decrease sequence number
                bank--;
                if(bank > 31) bank = 31;
            }
            else if(alt && !shift){ //Decrease notes per beat
                if (sequence.notes_per_beat > 1) sequence.notes_per_beat--;
            }
            else if (alt && shift){ //Decrease length
                sequence.length--;
                if(sequence.active_note > sequence.length) sequence.active_note = sequence.length;
            }
        }
        if(read_button(PORTBbits.RB6, &button_prev[2])) { //Play button action
            if(!alt && !shift){ //Play/pause
                if(playing == 0) {
                    playing = 1;
                    T0CONbits.TMR0ON = 1;
                }
                else {
                    playing = 0;
                    T0CONbits.TMR0ON = 0;
                    all_notes_off();
                }
            }
            else if(!alt && shift){ //Save sequence
                save_sequence(bank, &sequence);
            }
            else if(alt && !shift){ //Load sequence
                load_sequence(bank, &sequence);
            }
            else if (alt && shift){ //New sequence
                sequence.length = 0;
                sequence.notes_per_beat = 4;
                sequence.active_note = 0;
                set = 0;
            }
        }
        if(read_button(PORTBbits.RB7, &button_prev[3])) { //Rest button action
            if(!alt && !shift){ //rest
                if(playing || rotary_switch != 4) return;
                MIDI_buffer.received_messages[MIDI_buffer.length].status = 0x90;    //Note On
                MIDI_buffer.received_messages[MIDI_buffer.length].data1 = 0x80;     //Note 128 
                MIDI_buffer.received_messages[MIDI_buffer.length].data2 = 0x01;     //Velocity 1
                MIDI_buffer.length++;
                MIDI_buffer.received_messages[MIDI_buffer.length].status = 0x90;
                MIDI_buffer.received_messages[MIDI_buffer.length].data1 = 0;
                MIDI_buffer.received_messages[MIDI_buffer.length].data2 = 0;
                MIDI_buffer.length++;
            }
            else if(!alt && shift){ //Legato
                if(playing || rotary_switch != 4) return;
                unsigned char previous;
                for(unsigned char i = 1; sequence.active_note-i >= 0; i++){
                    if(sequence.velocities[sequence.active_note-i] < 127){
                        previous = sequence.active_note-i;
                        break;
                    }
                }
                MIDI_buffer.received_messages[MIDI_buffer.length].status = 0x90;    //Note On
                MIDI_buffer.received_messages[MIDI_buffer.length].data1 = sequence.notes[previous];     //Previous Note
                MIDI_buffer.received_messages[MIDI_buffer.length].data2 = sequence.velocities[previous]+128;     //Previous Velocity
                MIDI_buffer.length++;
                MIDI_buffer.received_messages[MIDI_buffer.length].status = 0x90;
                MIDI_buffer.received_messages[MIDI_buffer.length].data1 = 0;
                MIDI_buffer.received_messages[MIDI_buffer.length].data2 = 0;
                MIDI_buffer.length++;
            }
            else if(alt && !shift){ //velocity toggle
                if(velocity_mode){
                    velocity_mode = false;
                }
                else velocity_mode = true;
            }
        }
    }
    else { //In Arpeggiator mode
        if(read_button(PORTBbits.RB7, &button_prev[3])) { //Velocity toggle
            if(velocity_mode){
                velocity_mode = false;
            }
            else velocity_mode = true;
        }
        if(read_button(PORTBbits.RB6, &button_prev[2])) { //Hold toggle
            if(!hold) hold = true;
            else {
                hold = false;
                sequence.length = 0;
                sequence.notes_per_beat = 0;
                all_notes_off();
            }
        }
    }
}
/*
 * This function performs all visible output operations. write_LEDs and write_displays can be found in utilities.c
 */
static void set_output(void){
    LATAbits.LATA3 = hold;              //Adjust octave LED
    LATAbits.LATA4 = velocity_mode;     //Adjust velocity_mode LED
    write_LEDs(&sequence);              //Set the correct on/off states for the 16 LEDs
    write_displays(&sequence);          //Set the correct numbers for the displays
}
/*
 * Interrupt service routine. This method locates the source of the interrupt and calls that method. These are found below
 */
void __interrupt() ISR(void){
    
	if (PIR1bits.ADIF) AD_interrupt();
    else if (PIR1bits.RCIF) EUSART_receive_interrupt();
    else if (INTCONbits.TMR0IF) TMR0_interrupt();
    else if (PIR1bits.TMR2IF) TMR2_interrupt();
    else if (PIR2bits.TMR3IF) TMR3_interrupt();
	
}

static void AD_interrupt(void){
    PIR1bits.ADIF = 0;          //clear interrupt flag
    if (ADCON0bits.CHS0 == 0){  //AD channel 0
        selected_button = voltage_to_button(ADRESH);    //ADC value is transformed to a button number
        ADCON0bits.CHS0 = 1;    //Set AD channel to 1
    }
    else {
        rotary_switch = voltage_to_switch_position(ADRESH); //ADC value is transformed to a switch position
        ADCON0bits.CHS0 = 0;    //Set AD channel to 0
    }
    T3CONbits.TMR3ON = 1;       //Implements delay between channel measurements, needed because of high circuit impedance
}

static void EUSART_receive_interrupt(void){
    PIR1bits.RCIF = 0;
    if (RCREG > 127) MIDI_message.status = RCREG;   //IF MSB is 1 the received byte is a status byte
    else if (MIDI_message.data1 == 0) MIDI_message.data1 = RCREG;   //If data1 is zero, received byte is first data byte
    else if (MIDI_message.data2 == 0) { //else received byte is second data byte after which the full message is put in the buffer and reset to zero.
        MIDI_message.data2 = RCREG;
        MIDI_buffer.received_messages[MIDI_buffer.length].status = MIDI_message.status;
        MIDI_buffer.received_messages[MIDI_buffer.length].data1 = MIDI_message.data1;
        MIDI_buffer.received_messages[MIDI_buffer.length].data2 = MIDI_message.data2;
        if (sequencer && rotary_switch == 2) root = MIDI_message.data1;
        if (!sequencer || (sequencer && rotary_switch == 4 && !playing)) MIDI_buffer.length++;
        MIDI_message.data1 = 0;
        MIDI_message.data2 = 0;
    }
}
/*
 * This interrupt function handles the timing of the music.
 * nth_note is a variable that keeps track of where in the beat we are.
 * To make the tempo LED blink, we need to enter this function at least two times per beat which is why nth_note per beat goes from zero to (notes_per_beat*2)-1
 * If notes_per_beat is larger than 1 the tempo has to be divided to play multiple notes per beat.
 * Then the timer value is set and the timer is restarted.
 */
static void TMR0_interrupt(void){ 
    INTCONbits.TMR0IF = 0;          //Clear TMR0 interrupt flag
    nth_note++;
    if (nth_note > (sequence.notes_per_beat)*2) {
        nth_note = 1;
        new_arpeggio = 1;
        LATAbits.LATA2 = 1;
    }
    else {
        LATAbits.LATA2 = 0;        //Toggle LED
        new_arpeggio = 0;
    }
    if (nth_note % 2 != 0){         //Check if new note should be played
        new_note = true;
    }
    
    unsigned int divided_tempo = tempo;
    if (sequence.notes_per_beat == 0) {     //Timer off when nothing to play
        TMR0H = 255;
        TMR0L = 254;    //Ensures fast startup
        
    }
    else {
        divided_tempo = tempo/((sequence.notes_per_beat)*2);  //Reset timer
        TMR0H = 255-(divided_tempo >> 8);       //set TMR0H
        TMR0L = 255-divided_tempo;              //Set TMR0L
    }
    T0CONbits.TMR0ON = 1;                   //Restart Timer0
}

//This function makes the active_note LED blink on and off.
static void TMR2_interrupt(void){
    PIR1bits.TMR2IF = 0;
    TMR2_count++;
    if(TMR2_count > 25){
        if(active_led) {
            active_led = false;
        }
        else active_led = true;
        TMR2_count = 0;
    }
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
}

//Produces delay between AD channel measurements
static void TMR3_interrupt(void){          
    PIR2bits.TMR3IF = 0;
    T3CONbits.TMR3ON = 0;
    TMR3H = 0x00;
    TMR3L = 0x00;
    ADCON0bits.GO_NOT_DONE = 1;
}

/*
 * What follows are a bunch of getters and setters to access and change variables from other places of the program
 * This is probably not the proper way of doing things, but it did does the job :)
 */

bool get_shift(void){
    return shift;
}
    
bool get_alt(void){
    return alt;
}

bool get_sequencer(void){
    return sequencer;
}
  
unsigned char get_bank(void){
    return bank;
}
    
unsigned char get_set(void){
    return set;
}

void set_set(unsigned char value){
    if(value > 7) value = 0;
    set = value;
}

unsigned char get_set_number(void){
    if(sequence.length == 0) return 0;
    return ((sequence.length-1)/16);
}

bool get_velocity_mode(void){
    return velocity_mode;
}

bool get_active_led(void){
    return active_led;
}

void set_active_led(bool boolean){
    active_led = boolean;
}

unsigned char get_rotary_switch(void){
    return rotary_switch;
}

unsigned char get_root(void){
    return root;
}

bool get_hold(void){
    return hold;
}