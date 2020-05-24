#include <xc.h>
#include <stdlib.h>

#include "main.h"
#include "sequence.h"
#include "utilities.h"
#include "I2C.h"
#include "MIDI.h"

#define true 1
#define false 0

void add_note_arp(unsigned char note, unsigned char velocity, sequence_t * sequence){
    sequence->notes[sequence->length] = note;
    sequence->velocities[sequence->length] = velocity;
    sequence->length++;
}

void add_note_seq(unsigned char note, unsigned char velocity, sequence_t * sequence){
    if(sequence->active_note == 128) sequence->active_note = 0;
    sequence->notes[sequence->active_note] = note;
    sequence->velocities[sequence->active_note] = velocity;
    if(sequence->active_note == sequence->length) sequence->length++;
}

void remove_note(unsigned char note, sequence_t * sequence){
    for (unsigned char i = 0; i <= (sequence->length-1); i++){
        if (sequence->notes[i] == note){
            for (unsigned char j = i; j < (sequence->length); j++){              //Shift all other notes and velocities one forward
                sequence->notes[j] = sequence->notes[j+1];
                sequence->velocities[j] = sequence->velocities[j+1];
            }
            sequence->length--;
            i--;
        }
    }
}

void remove_note_index(sequence_t * sequence, unsigned char index){
    for(unsigned char i= index; i < (sequence->length-1); i++){
        sequence->notes[i] = sequence->notes[i+1];
        sequence->velocities[i] = sequence->velocities[i+1];
    }
    sequence->length--;
}

void sort_sequence(unsigned char rotary_switch, sequence_t * sequence){
    switch (rotary_switch) {
        case 0:
            break;
        case 1:         //Arpeggios in UP order
            bubblesort_sequence(sequence);
            remove_doubles(sequence);
            break;
            
        case 2:         //Arpeggios in DOWN order
            bubblesort_sequence(sequence);
            remove_doubles(sequence);
            reverse_sequence(sequence);
            break;
        case 3:         //Arpeggios in UP-DOWN order, outer notes single
            bubblesort_sequence(sequence);
            if (sequence->length < 3) break;
            remove_doubles(sequence);
            for (int i = (sequence->length-2); i > 0; i--){
                add_note_arp(sequence->notes[i], sequence->velocities[i], sequence);
            }
            sequence->notes_per_beat = sequence->length;
            break;
        case 4:         //Arpeggios in UP-DOWN order, outer notes double
            bubblesort_sequence(sequence);
            remove_doubles(sequence);
            for (int i = (sequence->length-1); i >= 0; i--){
                add_note_arp(sequence->notes[i], sequence->velocities[i], sequence);
            }
            sequence->notes_per_beat = sequence->length;
            break;
        case 5:         //Arpeggios in played order
            break;
        case 6:         //Arpeggios in random order
            shuffle_sequence(sequence);
            break;
    }
}

void reverse_sequence(sequence_t * sequence){
    unsigned char temp_note;
    unsigned char temp_velocity;
    unsigned char i;
    for (i = 0; i < (sequence->length)/2; i++){
        temp_note = sequence->notes[i];
        sequence->notes[i] = sequence->notes[sequence->length-1-i];
        sequence->notes[sequence->length-1-i] = temp_note;
        temp_velocity = sequence->velocities[i];
        sequence->velocities[i] = sequence->velocities[sequence->length-1-i];
        sequence->velocities[sequence->length-1-i] = temp_velocity;
    }
}

void bubblesort_sequence(sequence_t * sequence){
    unsigned char i, j;
    unsigned char temp;
    for (i = 0; i < sequence->length-1; i++){
        for (j = 0; j < (sequence->length-i-1); j++){
            if (sequence->notes[j] > sequence->notes[j+1]) {
                temp = sequence->notes[j+1];
                sequence->notes[j+1] = sequence->notes[j];
                sequence->notes[j] = temp;
                temp = sequence->velocities[j+1];
                sequence->velocities[j+1] = sequence->velocities[j];
                sequence->velocities[j] = temp;
            }
        }
    }
}

void remove_doubles(sequence_t * sequence){
    for (unsigned char i = 0; i < (sequence->length-1); i++){
        if (sequence->notes[i] == sequence->notes[i+1]) {
            remove_note_index(sequence, i+1);
        }
    }
}

void shuffle_sequence(sequence_t * sequence){
    unsigned int array[64];
    unsigned char i, j;
    unsigned char new_notes[64];
    unsigned char new_velocities[64];
    for (i = 0; i < 64; i++){
        array[i] = rand();
        if(array[i] == 65535) array[i]--;
    }
    for (i = 0; i < 64 && i < sequence->length; i++){
        unsigned char smallest = 0;
        for (j = 0; j < 64 && j < sequence->length; j++){
            if(array[j] < array[smallest]) smallest = j;
        }
        new_notes[i] = sequence->notes[smallest];
        new_velocities[i] = sequence->velocities[smallest];
        array[smallest] = 65535;
    }
    for(i = 0; i < sequence->length; i++){
        sequence->notes[i] = new_notes[i];
        sequence->velocities[i] = new_velocities[i];
    }
}

void advance_note(sequence_t * sequence){
    char note_shift = 0;
    if(get_rotary_switch() == 2 && get_sequencer()) {
        note_shift = get_root()-sequence->notes[0];
        if(get_root() == 0) note_shift = 0;
    }
    if(sequence->notes[sequence->active_note] > 127) note_shift = 0;
    play_note((sequence->notes[sequence->active_note]+note_shift), sequence->velocities[sequence->active_note]);
    sequence->active_note++;
    if (sequence->active_note >= sequence->length) sequence->active_note = 0;
    unsigned char set_number = get_set();
    if (sequence->active_note - (16*(set_number-1)) >= 16) set_set(set_number++);
    set_active_led(false);
    set_set(sequence->active_note/16);
}