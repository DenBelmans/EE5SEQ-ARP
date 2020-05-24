#include <xc.h>

#include "main.h"
#include "sequence.h"
#include "utilities.h"
#include "I2C.h"

//Implementation of the I2C-protocol actions.
static void I2C_idle(void){
    //Wait for all condition bits to be idle and no transmit is in progress
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

static void I2C_wait(void){
    while(!PIR1bits.SSPIF);
    SSPIF = 0;
}

static void I2C_start(void){
    I2C_idle();                 //Wait for I2C idle state
    SSPCON2bits.SEN = 1;        //Send I2C start signal
    I2C_wait();
}

static void I2C_restart(void){
    SSPCON2bits.RSEN = 1;       //Initiate restart condition
    I2C_wait();
}

static void I2C_stop(void){
    SSPCON2bits.PEN = 1;        //Send I2C stop signal
    I2C_wait();                 //Wait untill event is completed
}

static void I2C_write(unsigned char data){
    Label1 : SSPBUF = data;     //Start sending data by writing it to SSPBUF
    I2C_wait();                 //Wait for data to be sent
    while(ACKSTAT){             //Check for acknowledgement
        I2C_restart();          //If no ACK, try again
        goto  Label1;
    }
}

static unsigned char I2C_read(void){
    SSPCON2bits.RCEN = 1;       //Enable receive in master
    while (!SSPSTATbits.BF);    //Wait untill SSPBUF is full
    SSPCON2bits.RCEN = 0;       //Disable receive
    return SSPBUF;              //Return received data
}

static void I2C_ack(void){
    ACKDT = 0;                  //Set as acknowledgement
    ACKEN = 1;                  //Initiate acknowledgement signal
    I2C_wait();
}

static void I2C_nack(void){
    ACKDT = 1;                  //Set as not acknowledgement
    ACKEN = 1;                  //Initiate NACK signal
    I2C_wait();
}

//This function is called once at startup, to set the correct outputs on the IO-expanders
void setup_IOexpanders(void){
    write_IOexpander(0x40, 0x00, 0x00);    //Set GPIO port A as output
    write_IOexpander(0x40, 0x01, 0x00);    //Set GPIO port B as output
    write_IOexpander(0x42, 0x00, 0x00);    //Set GPIO port A as output
    write_IOexpander(0x42, 0x01, 0x00);    //Set GPIO port B as output
}

/*
 * This function is used for communication with the IO-expanders.
 * Only write operations are done since they only control output.
 * Opcode IOexpanders:  0b0100 A2 A1 A0 0 (always write)
 * The address contains the register address of the IO-expander.
 * The data is then stored on in that address.
 */
void write_IOexpander(unsigned char Opcode, unsigned char address, unsigned char data){
    I2C_start();                        //Send start signal
    I2C_write(Opcode);                  //Write Opcode
    I2C_write(address);                 //Write register address
    I2C_write(data);                    //Write data
    I2C_stop();                         //Send stop signal
}

/*
 * sequence_number is a number between [0 31]
 * Sequence length and notes_per_beat get stored in local EEPROM
 * notes and velocities are stored on external EEPROM
 * Since the sequence length is stored on local EEPROM, loading sequences can be performed more efficiently.
 */
void save_sequence(unsigned char sequence_number, sequence_t *sequence){
    if(sequence->length == 0) return;
    write_EEPROM(sequence_number*2, sequence->length);              //Save sequence length in PIC EEPROM
    write_EEPROM((sequence_number*2)+1, sequence->notes_per_beat);  //Save number of notes per beat in PIC EEPROM
    unsigned char i, j;                                             //iteration variables
    //Saving note data
    for(i = 0; i < ((sequence->length-1)/64)+1; i++){               //If sequence is longer than 64, break in page write
        I2C_start();                                                //Start I2C communication
        I2C_write(0b10100100);                                      //Send Opcode with address 011 and write bit
        I2C_write(sequence_number);                                 //Write sequence_number on high byte of memory address (256 bytes per sequence, so every sequence increases high byte by 1)
        I2C_write(0+(i*64));                                        //Low byte address
        for(j = 0; j < 64; j++){
            I2C_write(sequence->notes[j+(64*i)]);                   //Store ith sequence entry on external EEPROM
        }
        I2C_stop();                                                 //Stop I2C communication if all notes are written or 64 bytes have been written consecutively
        T1CONbits.TMR1ON = 1;                                       //Generate 5ms delay for external EEPROM to clear cache and write data to permanent storage
        while(!PIR1bits.TMR1IF);                                    //T1 will overflow in 5.4ms
        T1CONbits.TMR1ON = 0;                               
        TMR1H = 0x00;
        TMR1L = 0x00;
    }
    //Saving velocity data
    for(i = 0; i < ((sequence->length-1)/64)+1; i++){               //If sequence is longer than 64, break in page write
        I2C_start();                                                //Start I2C communication
        I2C_write(0b10100100);                                      //Send Opcode with address 011 and write bit
        I2C_write(sequence_number);                                 //Write sequence_number on high byte of memory address (256 bytes per sequence, so every sequence increases high byte by 1)
        I2C_write(128+(i*64));                                      //Low byte address
        for(j = 0; j < 64; j++){
            I2C_write(sequence->velocities[j+(64*i)]);              //Store ith sequence entry on external EEPROM
        }
        I2C_stop();                                                 //Stop I2C communication
        T1CONbits.TMR1ON = 1;                                       //Generate 5ms delay for external EEPROM to clear cache and write data to permanent storage
        while(!PIR1bits.TMR1IF);                                    //T1 will overflow in 5.4ms
        T1CONbits.TMR1ON = 0;                               
        TMR1H = 0x00;
        TMR1L = 0x00;
    }
}

/*
 * This method loads saved sequences. If the sequence is empty or the length is 255 (after reprogramming of PIC EEPROM is filled with ones) no sequence is loaded
 * By loading the sequence length from local EEPROM we can only ask the EEPROM to send the necessary data from memory.
 */ 
void load_sequence(unsigned char sequence_number, sequence_t *sequence){
    unsigned char length = read_EEPROM((sequence_number*2));
    if(length == 0 || length > 128) return; //Invalid sequence lengths, no sequence is loaded
    sequence->length = length;
    sequence->notes_per_beat = read_EEPROM((sequence_number*2)+1);
    unsigned char i;
    //External EEPROM write procedure
    I2C_start();
    I2C_write(0b10100100);                          //Send write opcode to EEPROM
    I2C_write(sequence_number);                     //Write high address byte
    I2C_write(0x00);                                //Write low addresss byte
    I2C_start();                                    //Send start again
    I2C_write(0b10100101);                          //Send read opcode to EEPROM
    for(i = 0; i < sequence->length; i++){          //Consecutive reads can be done through the whole memory space, no pause necessary.
        sequence->notes[i] = I2C_read();
        if(i < (sequence->length-1)) I2C_ack();     //After reading the last byte, do not send ACK but stop the loop and send NACK+STOP
    }
    I2C_nack();
    I2C_stop();
    I2C_start();                                    //Repeat the previous read procedure for the velocities
    I2C_write(0b10100100);
    I2C_write(sequence_number);
    I2C_write(0x80);
    I2C_start();
    I2C_write(0b10100101);
    for(i = 0; i < sequence->length; i++){
        sequence->velocities[i] = I2C_read();
        if(i < (sequence->length-1)) I2C_ack();
    }
    I2C_nack();
    I2C_stop();
    sequence->active_note = 0;
}
