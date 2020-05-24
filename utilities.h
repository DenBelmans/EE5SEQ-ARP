/* 
 * File:   utilities.h
 * Author: Sander
 *
 * Created on 27 maart 2020, 14:24
 */

#ifndef UTILITIES_H
#define	UTILITIES_H

#ifdef	__cplusplus
extern "C" {
#endif

extern const unsigned char SEGMENTS[32];

void write_EEPROM(unsigned char address, unsigned char data);
unsigned char read_EEPROM(unsigned char address);
unsigned char voltage_to_button(unsigned char ADRESULT);
unsigned char voltage_to_switch_position(unsigned char ADRESULT);
void read_rotary_encoder(bool * previous_rotA, unsigned int * tempo);
bool read_button(bool current_button_state, bool * previous_button_state);
void write_LEDs(sequence_t * sequence);
void write_displays(sequence_t * sequence);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILITIES_H */

