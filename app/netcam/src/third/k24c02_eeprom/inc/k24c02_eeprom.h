#ifndef __K24C02_EEPROM__
#define __K24C02_EEPROM__

#define K24C02_EEPROM_DEVICE_ADDR	 (0xA0)
#define K24C02_EEPROM_MAX_LEN		 256

int k24c02_eeprom_i2c_read(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int len);
int k24c02_eeprom_i2c_write(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int len);
int k24c02_eeprom_data_check(void);

#endif
