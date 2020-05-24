/* 
 * File:   sequence.h
 * Author: Sander
 *
 * Created on 22 april 2020, 0:03
 */

#ifndef SEQUENCE_H
#define	SEQUENCE_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char notes[128];
    unsigned char velocities[128];
    unsigned char length;
    unsigned char notes_per_beat;
    unsigned char active_note;
} sequence_t;

void add_note_arp(unsigned char note, unsigned char velocity, sequence_t * sequence);
void add_note_seq(unsigned char note, unsigned char velocity, sequence_t * sequence);
void remove_note(unsigned char note, sequence_t * sequence);
void remove_note_index(sequence_t * sequence, unsigned char index);
void sort_sequence(unsigned char rotary_switch, sequence_t * sequence);
void reverse_sequence(sequence_t * sequence);
void bubblesort_sequence(sequence_t * sequence);
void advance_note(sequence_t * sequence);
void remove_doubles(sequence_t * sequence);
void shuffle_sequence(sequence_t * sequence);
void swap(sequence_t * sequence, unsigned char index1, unsigned char index2);


#ifdef	__cplusplus
}
#endif

#endif	/* SEQUENCE_H */

