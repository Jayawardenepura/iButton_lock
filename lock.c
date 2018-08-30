#ifndef F_CPU
#define 	F_CPU   16000000UL
#endif

/* able to save 1kb/8b keys */ 
#define NUMBER_OF_KEYS 20

/* bytes in read and write buffers */
#define BUFFER_LEN (65)

#define choose_key(buff,key_1,key_2) if(atomic_str_eq(buff, key_1) || atomic_str_eq(buff, key_2))

#define OK    (1 << 1) /* ACCESS LED */
#define ERROR (1 << 2) /* DENY LED   */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
/* Power management. For more see: https://www.nongnu.org/avr-libc/user-manual/group__avr__power.html */
#include <avr/power.h>
/* Atomic operations */
#include <util/atomic.h>

#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "onewire.h"
#include "uart.h"
#include "_eeprom.h"

#include "sha256.h"
#include "hmac-sha256.h"

void soft_delay_us(uint16_t time)
{
	while (time--) {
		_delay_us(1.0);
	}
}

void line_low(void)
{
	PORTB &= ~(1 << 0);
	DDRB |= (1 << 0);
}

void line_release(void)
{
	DDRB &= ~(1 << 0);
	PORTB |= (1 << 0);
}

bool line_read(void)
{
	uint8_t val = PINB & 0x01;
	return val;
}

struct handler{
 char *key_f;
 char *key_alter;
};

struct handler hndl_add_arr[] = {{"q", "0"}, {"w", "1"}, {"e", "2"}, {"r", "3"}, {"t", "4"} , {"y", "5"},
			         {"u", "6"}, {"i", "7"}, {"o", "8"}, {"p", "9"}, {"[", "10"},{"]", "11"},
		                 {"a", "12"},{"s", "13"},{"d", "14"},{"f", "15"},{"g", "16"},{"h", "17"},
 	 	 	         {"j", "18"},{"k", "19"},{"l", "20"},{";", "21"},{"'", "22"},{"z", "23"},
 	 	 	         {"x", "24"},{"c", "25"},{"v", "26"},{"b", "27"},{"n", "28"},{"m", "29"},
 	 	 	         {"<", "30"},{">", "31"},{"Q", "32"},{"W", "33"},{"E", "34"},{"R", "35"},
 	 	 	         {"T", "36"},{"Y", "37"},{"U", "38"},{"I", "39"},{"O", "40"},{"P", "41"},
 	 	 	         {"A", "42"},{"S", "43"},{"D", "44"},{"F", "45"},{"G", "46"},{"H", "47"},
 	 	 	         {"J", "48"},{"K", "49"},{"L", "50"},{"Z", "51"},{"X", "52"},{"C", "53"},
 	 	 	         {"V", "54"},{"B", "55"},{"N", "56"},{"M", "57"},{"~", "58"},{"!", "59"}
 	 	 	       };
struct handler hndl_del_arr[] = {{"-q", "-0"}, {"-w", "-1"}, {"-e", "-2"}, {"-r", "-3"}, {"-t", "-4"} , {"-y", "-5"},
			         {"-u", "-6"}, {"-i", "-7"}, {"-o", "-8"}, {"-p", "-9"}, {"-[", "-10"},{"-]", "-11"},
		                 {"-a", "-12"},{"-s", "-13"},{"-d", "-14"},{"-f", "-15"},{"-g", "-16"},{"-h", "-17"},
 	 	 	         {"-j", "-18"},{"-k", "-19"},{"-l", "-20"},{"-;", "-21"},{"-'", "-22"},{"-z", "-23"},
 	 	 	         {"-x", "-24"},{"-c", "-25"},{"-v", "-26"},{"-b", "-27"},{"-n", "-28"},{"-m", "-29"},
 	 	 	         {"-<", "-30"},{"->", "-31"},{"-Q", "-32"},{"-W", "-33"},{"-E", "-34"},{"-R", "-35"},
 	 	 	         {"-T", "-36"},{"-Y", "-37"},{"-U", "-38"},{"-I", "-39"},{"-O", "-40"},{"-P", "-41"},
 	 	 	         {"-A", "-42"},{"-S", "-43"},{"-D", "-44"},{"-F", "-45"},{"-G", "-46"},{"-H", "-47"},
 	 	 	         {"-J", "-48"},{"-K", "-49"},{"-L", "-50"},{"-Z", "-51"},{"-X", "-52"},{"-C", "-53"},
 	 	 	         {"-V", "-54"},{"-B", "-55"},{"-N", "-56"},{"-M", "-57"},{"-~", "-58"},{"-!", "-59"}
 	 	 	       };

uint8_t key[] = {
  0x3d, 0xc6, 0xca, 0xa4, 0x82, 0x4a, 0x6d, 0x28,
  0x87, 0x67, 0xb2, 0x33, 0x1e, 0x20, 0xb4, 0x31,
  0x66, 0xcb, 0x85, 0xd9 
};


int main()
{
	uart_init();
	sei();

	/* We use internal pullup resitor for 1-wire line */
	DDRB = (1 << 1) | (1 << 3) | (1 << 2);
	DDRB |= (1 << 1) | (1 << 3);
	PORTB |= (1 << 0);

	PORTB |= OK;

	ow_Pin pin;
	ow_Pin_init(&pin, &line_low, &line_release, &line_read, &soft_delay_us, 5, 60, 60, 5);
	ow_err err;

	uint8_t ibutton_id[8];
	uint8_t crc;
	
	char temp[] = "";
	char hash_str[41];
	uint8_t access_flag;
	uint8_t id_msg[7];
	uint8_t hash[20];
	while (1) {
	
	 	err = ow_cmd_readrom(&pin, ibutton_id, &crc, true, false);
		
		switch(err){
		case(OW_EOK):
		    for(uint8_t key_count_r = 0;key_count_r < NUMBER_OF_KEYS;key_count_r++){

		 	 access_flag = (true == search_id(ibutton_id,crc,key_count_r)) ? 0xFF : 0xF;

			    switch(access_flag){	
				case(0xFF):
				 PORTB &= ~OK;
				 PORTB |= ERROR;

				 id_msg[0] = crc;
				 for(int i = 0,j = 1; i < 6; ++i,j++)
					id_msg[j] = ibutton_id[i];

				  /* only 7 bytes of the button id */
				 hmac_sha256(hash,key,160,id_msg,56);

				 int byte;
				 for(byte = 0;byte < 32; ++byte)
					sprintf(&hash_str[byte*2], "%02x", hash[byte]);

					uart_put(hash_str);
					uart_put("\n\n");
				 default:
				  break;
			   }
		      }
		  break;
		
		  default:
			 PORTB |= OK;
			 PORTB &= ~ERROR;
			 break;
		}

		/*--------------------------------------------------------------------------------------------*/

		if (atomic_str_eq(rdbuff, "la")){

			uint8_t id_bowl[8],*id_which_read;

			uart_put("\n\n");
			//uart_put("iButton Database\n");
			for(uint8_t key_count_r = 0;key_count_r < NUMBER_OF_KEYS;key_count_r++){
			 
				uart_put("|[");
				itoa(key_count_r,temp,10);
				uart_put(temp);
				uart_put("] ");

				id_which_read = read_from_cell(key_count_r,id_bowl);
			
				for(uint8_t i = 0;i < 8;i++){

					sprintf(&hash_str[i*2], "%02x", hash[i]);
					itoa(id_which_read[i],temp,16);
					uart_put(temp);
					
				}	
				uart_put("\n");
				uart_put("|------------------------------ |\n");
			}
			  _delay_ms(50);
		}

		/* if you are lazy you should push "cla" (clean all) for database cleaning:) */

		if (atomic_str_eq(rdbuff, "cla")){
		  clean_all_cells(NUMBER_OF_KEYS);
		  uart_put("\n\nAll keys have been deleted\n\n");
		  _delay_ms(50);
		}

		/*--------------------------------------------------------------------------------------------*/

		/* interface for deleting and adding keys to database -> Push: "-q"->[0] "-w"->[1] "-e"->[2] ..... "-p"->[9] */

	    	if(err == OW_EOK)
		for(int selection = 0;selection < NUMBER_OF_KEYS; selection++){
			
			/* is key already exist -> deny to write*/
			if(access_flag == 0xF)
		 	 choose_key(rdbuff, hndl_add_arr[selection].key_f, hndl_add_arr[selection].key_alter){ /* if N-key has been pressed -> put in */
				put_in_cell(ibutton_id,crc,selection);
				_delay_ms(10);
				uart_put("\n");
				uart_put("\nKEY has been added\n");
				uart_put("\n");
				_delay_ms(50);
			 }

			 choose_key(rdbuff, hndl_del_arr[selection].key_f, hndl_del_arr[selection].key_alter){
				clean_cell(selection);
				_delay_ms(10);
				uart_put("\n");
				uart_put("\nKEY has been deleted\n");
				uart_put("\n");
				_delay_ms(50);
	 	         }
		}
	}	
}