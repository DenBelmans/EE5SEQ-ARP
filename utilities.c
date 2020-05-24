#include <xc.h>

#include "main.h"
#include "sequence.h"
#include "I2C.h"
#include "MIDI.h"
#include "utilities.h"

#define true 1
#define false 0
/*
 * I had the 'pleasure' of coding 32 different digits and letters for 7-segment displays
 */
const unsigned char SEGMENTS[32] = { 
    0b11000000,                     //0
    0b11111001,                     //1
    0b10100100,                     //2
    0b10110000,                     //3
    0b10011001,                     //4
    0b10010010,                     //5
    0b10000010,                     //6
    0b11111000,                     //7
    0b10000000,                     //8
    0b10010000,                     //9
    0b10001000,                     //A
    0b10000011,                     //b
    0b11000110,                     //C
    0b10100001,                     //d
    0b10000110,                     //E
    0b10001110,                     //F
    0b11000010,                     //G
    0b10001001,                     //H
    0b11001111,                     //I
    0b11100001,                     //J
    0b11000111,                     //L
    0b10101011,                     //n
    0b10100011,                     //o
    0b10001100,                     //P
    0b10011000,                     //Q
    0b10101111,                     //r
    0b10000111,                     //t
    0b11000001,                     //U
    0b10010001,                     //y
    0b10100111,                     //c
    0b10001011,                     //h
    0b11111011                      //i
};

//This method writes the given data to the given address on the PIC's internal EEPROM
void write_EEPROM(unsigned char address, unsigned char data){
    EECON1bits.WREN = 1;                //Enable writes
    EEADR = address;                     
    EEDATA = data;
    INTCONbits.GIE = 0;                 //Disable interrupts
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;                  //Initiate write
    INTCONbits.GIE = 1;                 //Enable interrupts while waiting
    while(EECON1bits.WR){}              //Wait for end of write cycle
    EECON1bits.WREN = 0;                //Disable writes
}

//This method reads the data stored on the given address of the PIC's internal EEPROM and returns it
unsigned char read_EEPROM(unsigned char address){
    EEADR = address;                     //Read length at address bank*2
    EECON1bits.RD = 1;                  //Initiate read
    unsigned char data = EEDATA;                    //Store retrieved data    
    return data;
}

//This method takes an ADC conversion result and determines if and which of the 16 buttons is pressed
unsigned char voltage_to_button(unsigned char ADRESULT){
    
    unsigned char button = 0;
    
    if (ADRESULT < 7) button = 0;              //No button pressed
    else if (ADRESULT < 24) button = 16;         //Button 0 pressed
    else if (ADRESULT < 38) button = 15;        //Button 1 pressed
    else if (ADRESULT < 53) button = 14;        //Button 2 pressed
    else if (ADRESULT < 67) button = 13;        //Button 3 pressed
    else if (ADRESULT < 83) button = 12;        //Button 4 pressed
    else if (ADRESULT < 97) button = 11;        //Button 5 pressed
    else if (ADRESULT < 112) button = 10;        //Button 6 pressed
    else if (ADRESULT < 127) button = 9;        //Button 7 pressed
    else if (ADRESULT < 142) button = 8;        //Button 8 pressed
    else if (ADRESULT < 157) button = 7;        //Button 9 pressed
    else if (ADRESULT < 172) button = 6;       //Button 10 pressed
    else if (ADRESULT < 188) button = 5;       //Button 11 pressed
    else if (ADRESULT < 203) button = 4;       //Button 12 pressed
    else if (ADRESULT < 218) button = 3;       //Button 13 pressed
    else if (ADRESULT < 232) button = 2;       //Button 14 pressed
    else if (ADRESULT > 232) button = 1;       //Button 15 pressed
    
    return button;
}

//This method takes an ADC conversion result and determines the position of the rotary switch
unsigned char voltage_to_switch_position(unsigned char ADRESULT){
    
    unsigned char switch_position = 0;
    
    if (ADRESULT < 16) switch_position = 0;
    else if (ADRESULT < 54) switch_position = 6;
    else if (ADRESULT < 90) switch_position = 5;
    else if (ADRESULT < 128) switch_position = 4;
    else if (ADRESULT < 163) switch_position = 3;
    else if (ADRESULT < 200) switch_position = 2;
    else if (ADRESULT > 200) switch_position = 1;
    
    return switch_position;
}

//This method checks whether or not the rotary encoder has changed position since last loop and, if so, changes the tempo 
void read_rotary_encoder(bool * previous_rotA, unsigned int * tempo) {
    if (*previous_rotA != PORTCbits.RC1){                   //If change on A
        if (PORTCbits.RC2 != PORTCbits.RC1) {               //If B != A -> CW rotation
            if (*tempo < 2548) *tempo = 2048;
            else *tempo -= 500;
        }
        else {                                              //Else B == A -> CCW rotation
                if (*tempo > (unsigned int) 65036) *tempo = (unsigned int)65535;
                else *tempo += 500;
        }
    }
    *previous_rotA = PORTCbits.RC1;
}
/*
 * This method returns true on the first time the button input is high.
 * This prevents that a certain action is repeated more than once per button press.
 */
bool read_button(bool current_button_state, bool * previous_button_state){
    if(current_button_state == 1 && *previous_button_state == 0){
        *previous_button_state = 1;
        return 1;
    }
    else if (current_button_state == 0) *previous_button_state = 0;
    return 0;
}

//This method outputs the correct data to the IO-expander that controls the 7-segment display
void write_displays(sequence_t * sequence){
    
    unsigned char left_display = 0xFF;
    unsigned char right_display = 0xFF;
    if (!get_sequencer()){
        right_display = SEGMENTS[sequence->notes_per_beat];
    }
    else{
        if (!get_shift() && !get_alt()){
            //Display Bank and Set, dots: 01
            left_display = SEGMENTS[get_bank()];
            right_display = SEGMENTS[get_set()+1];
            right_display -= 128;
        }
        else if (get_shift() && !(get_alt())){
            //Display Bank and Set, dots: 10
            left_display = SEGMENTS[get_bank()];
            right_display = SEGMENTS[get_set()+1];
            left_display -= 128;
        }
        else if (!(get_shift()) && get_alt()){
            //Display notes per beat, dots: 00
            left_display = SEGMENTS[get_bank()];
            right_display = SEGMENTS[sequence->notes_per_beat];
        }
        else if (get_shift() && get_alt()){
            //Display velocity of active note, dots: 11
            left_display = SEGMENTS[(sequence->length >> 4)];
            right_display = SEGMENTS[(sequence->length & 0b00001111)];
            left_display -= 128;
            right_display -= 128;
        }
    }
    //See I2C.c for this function
    write_IOexpander(0x42, 0x15, left_display);                        //Set left side of LED's
    write_IOexpander(0x42, 0x14, right_display);                       //Set right set of LED's
}

//This method outputs the correct data to the IO-expander that controls the 16 LEDs
void write_LEDs(sequence_t * sequence){
    unsigned char left_LEDs = 0;
    unsigned char right_LEDs = 0;
    unsigned char i;
    unsigned char set = get_set();
    unsigned char recording = 0;
    //If recording, the active note can be set to the next to be recorded note position.
    if(sequence->active_note == sequence->length && get_sequencer()) recording = 1;
    for(i = 0; i < 8; i++){ //if note is not muted && note is a member of sequence && (note is not active note || active_led is true)
        if (sequence->notes[(set*16)+i] < 128 && (set*16)+i < (sequence->length+recording) && (sequence->active_note != (set*16)+i || !get_active_led())){
            left_LEDs = left_LEDs | 0b00000001 << i;
        }
    }
    for(i = 0; i < 8; i++){ //Same as above, but for the next 8 notes.
        if (sequence->notes[(set*16)+8+i] < 128 && (set*16)+8+i < (sequence->length+recording) && (sequence->active_note != (set*16)+8+i || !get_active_led())){
            right_LEDs = right_LEDs | 0b00000001 << i;
        }
    }    
    //See I2C.c for this function
    write_IOexpander(0x40, 0x15, left_LEDs);                        //Set left side of LED's
    write_IOexpander(0x40, 0x14, right_LEDs);                       //Set right set of LED's
}