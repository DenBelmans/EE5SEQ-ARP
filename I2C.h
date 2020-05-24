/* 
 * File:   I2C.h
 * Author: Sander
 *
 * Created on 13 april 2020, 12:36
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

static void I2C_idle(void);
static void I2C_wait(void);
static void I2C_start(void);
static void I2C_restart(void);
static void I2C_stop(void);
static void I2C_write(unsigned char data);
static unsigned char I2C_read(void);
static void I2C_ack(void);
static void I2C_nack(void);
    
void setup_IOexpanders(void);
void write_IOexpander(unsigned char Opcode, unsigned char address, unsigned char data);
void byte_write_external_EEPROM(unsigned char addressH, unsigned char addressL, unsigned char data);
unsigned char random_read_external_EEPROM(unsigned char addressH, unsigned char addressL);
void save_sequence(unsigned char sequence_number, sequence_t *sequence);
void load_sequence(unsigned char sequence_number, sequence_t *sequence);

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

