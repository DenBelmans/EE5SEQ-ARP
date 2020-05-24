/* 
 * File:   main.h
 * Author: Sander
 *
 * Created on 13 april 2020, 14:34
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef unsigned char bool;
    
    static void check_input(void);
    static void set_output(void);
    
    void __interrupt() ISR(void);
    static void AD_interrupt(void);
    static void EUSART_receive_interrupt(void);
    static void TMR0_interrupt(void);
    static void TMR2_interrupt(void);
    static void TMR3_interrupt(void);
    
    static void check_input(void);
    static void set_output(void);
    
    bool get_shift(void);
    bool get_alt(void);
    bool get_sequencer(void);
    unsigned char get_bank(void);
    unsigned char get_set(void);
    void set_set(unsigned char value);
    unsigned char get_set_number(void);
    bool get_velocity_mode(void);
    void set_velocity_mode(bool boolean);
    bool get_active_led(void);
    void set_active_led(bool boolean);
    bool get_playing(void);
    unsigned char get_rotary_switch(void);
    unsigned char get_root(void);
    bool get_hold(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

