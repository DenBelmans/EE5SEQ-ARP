/* 
 * File:   MIDI.h
 * Author: Sander
 *
 * Created on 13 april 2020, 12:37
 */

#ifndef MIDI_H
#define	MIDI_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct {
        unsigned char status;
        unsigned char data1;
        unsigned char data2;
    } message_t;
    
    typedef struct {
        message_t received_messages[32];
        unsigned char length;
    } MIDI_buffer_t;

void play_note(unsigned char new_note, unsigned char velocity);
static void note_on(unsigned char note, unsigned char velocity);
static void note_off(unsigned char note);
void MIDI_parser(message_t * message, sequence_t * sequence);
void all_notes_off(void);
void read_buffer(MIDI_buffer_t * buffer, sequence_t * sequence);

#ifdef	__cplusplus
}
#endif

#endif	/* MIDI_H */

