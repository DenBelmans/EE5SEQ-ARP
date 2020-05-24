/* 
 * File:   interrupts.h
 * Author: Sander
 *
 * Created on 13 april 2020, 12:02
 */

#ifndef INTERRUPTS_H
#define	INTERRUPTS_H

#ifdef	__cplusplus
extern "C" {
#endif

void AD_interrupt(void);
void EUSART_receive_interrupt(void);
void TMR0_interrupt(void);
void TMR3_interrupt(void);
void Interrupt_on_change_interrupt(void);


#ifdef	__cplusplus
}
#endif

#endif	/* INTERRUPTS_H */

