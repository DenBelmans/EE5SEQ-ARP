#include <xc.h>

#include "main.h"
#include "sequence.h"
#include "utilities.h"
#include "I2C.h"
#include "MIDI.h"

#define true 1
#define false 0

void play_note(unsigned char new_note, unsigned char velocity);
void note_on(unsigned char note, unsigned char velocity);
void note_off(unsigned char note);

unsigned char last_status_byte;
unsigned char previous_note;
unsigned char hold_buffer = 0;

void all_notes_off(void){
    while(TXSTAbits.TRMT == 0);
    TXREG = 0xB0;                   //All notes off
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = 0x7B;                   
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = 0x00;
    last_status_byte = 0xB0;
}

void play_note(unsigned char new_note, unsigned char velocity){
    if(velocity < 0x80) {   //If >127 legato, so nothing changes.
        note_off(previous_note);
        if(new_note < 0x80 && new_note != 0){    //If >127 muted, 
            note_on(new_note, velocity);
        }
    }
    previous_note = new_note;
}

void note_on(unsigned char note, unsigned char velocity){
    if(get_velocity_mode() == false) velocity = 80;
    if (last_status_byte != 0x90) {
        while(TXSTAbits.TRMT == 0);
        TXREG = 0x90;                   //Note on code
        last_status_byte = 0x90;
    }
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = note;                   //Send note data
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = velocity;
}

void note_off(unsigned char note){      //Practically all MIDI implementations use note_on zero-velocity instead of note_off messages
    if (note > 127) return;
    if ((last_status_byte != 0x90) || (last_status_byte != 0x80)) {
        while(TXSTAbits.TRMT == 0);     //Wait
        TXREG = 0x90;                   //Note on code
        last_status_byte = 0x90;
    }   //Note on with velocity 0 = note off
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = note;                   //Send note data
    while(TXSTAbits.TRMT == 0);     //Wait
    TXREG = 0x00;                   //Send velocity data
}

void ARP_parser(message_t * message, sequence_t * sequence){
    if ((message->status == 0x90) && (message->data2 != 0)) {       //Note on with non-zero velocity
        if(get_hold()){
            unsigned char i = 0;
            if(get_rotary_switch() == 3) i = ((sequence->length+2)/2)-2;
            else if(get_rotary_switch() == 4) i = sequence->length/2;
            if(sequence->length <= hold_buffer+i) {
                sequence->length = 0;
                hold_buffer = 0;
                all_notes_off();
            }
        }
        add_note_arp(message->data1, message->data2, sequence);
    }
    else if(message->status == 0x80 || ((message->status == 0x90) && (message->data2 == 0))) {   //Note off or Note on with zero velocity
        if(!get_hold()) remove_note(message->data1, sequence);
        else hold_buffer++;
    }
    if (sequence->length == 0) all_notes_off();
    sequence->notes_per_beat = sequence->length;
}

void SEQ_parser(message_t * message, sequence_t * sequence){
    if ((message->status == 0x90) && (message->data2 != 0)) {       //Note on with non-zero velocity
        add_note_seq(message->data1, message->data2, sequence);
        sequence->active_note++;
        if(sequence->active_note > 127) sequence->active_note = 0;
        set_set(sequence->active_note/16);
    }
}

void read_buffer(MIDI_buffer_t * buffer, sequence_t * sequence){
    unsigned char i = 0;
    if(get_sequencer()){
        for (i = 0; i < buffer->length; i++) {
            SEQ_parser(&(buffer->received_messages[i]), sequence);
        }
    }
    else {
        for (i = 0; i < buffer->length; i++) {
            ARP_parser(&(buffer->received_messages[i]), sequence);
        }
    }
    buffer->length = 0;
}