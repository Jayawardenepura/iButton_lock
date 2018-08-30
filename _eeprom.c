#include <avr/io.h>
#include <util/atomic.h>
#include <string.h>
#include <stdbool.h>
#include "_eeprom.h"
#include "uart.h"

void eeprom_w_byte(uint16_t addr, uint8_t data)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
	  while(EECR & (1<<EEPE));
		EECR &= ~((1 << EEPM1) | (1 << EEPM0));
		EEAR = addr;
		EEDR = data;
		EECR |= (1<<EEMPE);
		EECR |= (1<<EEPE);
	}
}

uint8_t eeprom_r_byte(uint16_t addr)
{
  while(EECR & (1<<EEPE));
	EEAR = addr;
	EECR |= (1<<EERE);
  return EEDR;
}

uint8_t *read_from_cell(uint8_t pos,uint8_t *data)
{
	uint8_t start = 7*pos;
	uint8_t end   = 7+(7*pos); 
	/* (0..8),(8..16),.. - keys locations*/
	for(uint8_t localcnt=start,cnt = 0;localcnt<end;localcnt++,cnt++)
		data[cnt] = eeprom_r_byte(localcnt);
	return data;
}

bool put_in_cell(uint8_t *id,uint8_t crc,uint8_t pos)
{
	/* id_0(0..8),id_1(8..16) */
	uint8_t start = 7*pos;
	uint8_t end   = 6+(7*pos);
	/* (crc)(1..8),(crc)(9..17),.. - keys locations*/
	 eeprom_w_byte(start,crc);
	for(uint8_t eeprom_addr = start+1,cnt = 0;eeprom_addr < end+1;eeprom_addr++,cnt++)
		eeprom_w_byte(eeprom_addr , id[cnt]);
	return true;
}

bool clean_cell(uint8_t pos)
{
	uint8_t start = 7*pos;
	uint8_t end   = 6+(7*pos);
	/* reclean block */

	uint8_t *read_data,data[7];
	read_data = read_from_cell(pos,data);	
	if(read_data[0] == 0)
		return false;
	/* (crc)(1..8),(crc)(0x0000000),(crc)(17..25).. - zeroing out id*/
	eeprom_w_byte(end,0x00);
	for(uint8_t eeprom_addr=start,cnt=0; eeprom_addr<end;eeprom_addr++,cnt++)
		eeprom_w_byte(eeprom_addr , 0x00);
	return true;
}

uint8_t clean_all_cells(uint8_t number)
{
	for(uint8_t i=0;i<number;i++)
		clean_cell(i);
}

bool search_id(uint8_t *id,uint8_t crc,uint8_t pos)
{
	uint8_t casting=0;
	uint8_t start = 7*pos;
	uint8_t end   = 6+(7*pos);
	if(crc == eeprom_r_byte(start)) casting++; 
		else return false;
	for(uint8_t localcnt=start,cnt = 0;localcnt<end;++localcnt,++cnt){
		if(eeprom_r_byte(localcnt+1) == id[cnt])
			casting++;
		/* id was found ? */
		if(casting == 7)
			return true;
	}
	return false;
}
