
/* write bytes directly in eeprom */
void eeprom_w_byte(uint16_t addr, uint8_t data);

/* write bytes directly from eeprom */
uint8_t eeprom_r_byte(uint16_t addr);

/* reading id by position */
uint8_t *read_from_cell(uint8_t pos,uint8_t *data);

/* searching cuncrurrences */
bool search_id(uint8_t *id,uint8_t crc,uint8_t pos);

/* free id by position from eeprom */
bool clean_cell(uint8_t pos);

/* free all id's from eeprom */
uint8_t clean_all_cells(uint8_t number);

/* add id by position */
bool put_in_cell(uint8_t *id,uint8_t crc,uint8_t pos);
