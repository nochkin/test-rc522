#include <stdio.h>

#include "reader.h"

Reader::Reader()
{
	memset(tag_full_sn, 0, 7);
}

int Reader::init_spi(int8_t RST)
{
	if (bcm2835_init()) {
		bcm2835_spi_begin();
		bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		// 16 MHz SPI bus, but Worked at 62 MHz also
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
		// Setup reset pin direction as output
		// bcm2835_gpio_fsel(RST, BCM2835_GPIO_FSEL_OUTP);
		_if_type = IF_SPI;
		return init();
	} else {
		return 1;
	}
}

int Reader::init_i2c()
{
	_if_type = IF_I2C;
	return 1;
}

int Reader::read_tag()
{
	uint8_t tag_type[MAX_RLEN];
	uint8_t sn[10];

	memset(tag_full_sn, 0, 16);

	uint8_t status = request(PICC_REQIDL, tag_type);
	memcpy(tag_full_sn, tag_type, 2);
	write(BitFramingReg, 0x07);
	if (status == TAG_OK) {
		status = anticoll(sn);
		memcpy(tag_full_sn+2, sn, 5);
		status = select_sn(sn);
		// memcpy(tag_full_sn+2+5, sn, 1);
	}
	// printf("full sn: ");
	// for (int ii=0; ii<7; ii++) {
 	// 	printf("%02X:", full_sn[ii]);
	// }
	// printf("\n");
	return status;
}

uint8_t *Reader::get_tag()
{
	return tag_full_sn;
}

std::string Reader::get_tag_str(std::string delim)
{
	std::string tag_full_str = "";
	char tmp_buf[3];

	for (int ii=0; ii<7; ii++) {
		sprintf(tmp_buf, "%02X", tag_full_sn[ii]);
		tag_full_str.append(tmp_buf);
		tag_full_str.append(delim);
	}
	tag_full_str.resize(tag_full_str.size() - delim.size());
	return tag_full_str;
}

int Reader::init()
{
	write(CommandReg, PCD_RESETPHASE);	// Soft reset
	usleep(10000);
	write(TModeReg, 0x8d);			// Timer
	write(TPrescalerReg, 0x3e);		// Timer
	write(TReloadRegL, 30);
	write(TReloadRegH, 0);
	write(TxASKReg, 0x40);			// Transmit modulation
	write(ModeReg, 0x3d);			// Tx/Rx settings, CRC=0x6363

	antenna(true);
	return 0;
}

interface_t Reader::get_interface() {
	return _if_type;
}

uint8_t Reader::request(uint8_t req_mode, uint8_t *tag_type)
{
	uint8_t status;
	uint8_t buf[MAX_RLEN];
	uint8_t buf_len = 0;

	write(BitFramingReg, 0x07);
	buf[0] = req_mode;
	status = send_to_card(PCD_TRANSCEIVE, buf, 1, buf, &buf_len);
	if (buf_len > 1) {
		tag_type[0] = buf[0];
		tag_type[1] = buf[1];
	}

	return status;
}

uint8_t Reader::anticoll(uint8_t *sn)
{
	uint8_t status;
	uint8_t sn_check = 0;
	uint8_t sn_len = 0;
	uint8_t ii;

	write(BitFramingReg, 0x00);
	sn[0] = PICC_ANTICOLL1;
	sn[1] = 0x20;
	status = send_to_card(PCD_TRANSCEIVE, sn, 2, sn, &sn_len);

	if (status == TAG_OK) {
		for (ii=0; ii<4; ii++) {
			sn_check ^= sn[ii];
		}
		if (sn_check != sn[ii]) {
			status = TAG_ERROR;
		}
	}

	return status;
}

uint8_t Reader::select_sn(uint8_t *sn)
{
	uint8_t status;
	uint8_t buf[MAX_RLEN];
	uint8_t buf_len = 0;

	buf[0] = PICC_ANTICOLL1;
	buf[1] = 0x70;
	for (int ii=0; ii<5; ii++) {
		buf[ii+2] = *(sn+ii);
	}

	calculate_crc(buf, 7, &buf[7]);
	// clear_bitmask(Status2Reg, 0x08);

	status = send_to_card(PCD_TRANSCEIVE, buf, 9, buf, &buf_len);
	// printf("buf(%i): ", buf_len);
	for (int ii=0; ii<buf_len; ii++) {
		// printf("%02X:", buf[ii]);
	}
	// printf("\n");
	if (buf_len != 0x18) {
		status = TAG_ERROR;
	}

	return status;
}

uint8_t Reader::halt()
{
	uint8_t status;
	uint8_t buf[4];
	uint8_t buf_len = 0;

	buf[0] = PICC_HALT;
	buf[1] = 0;
	calculate_crc(buf, 2, &buf[2]);

	status = send_to_card(PCD_TRANSCEIVE, buf, 4, buf, &buf_len);
	return status;
}

void Reader::calculate_crc(uint8_t *data, uint8_t data_len, uint8_t *buf)
{
	uint8_t ii, n;

	clear_bitmask(DivIrqReg, 0x04);
	set_bitmask(FIFOLevelReg, 0x80);

	for (ii=0; ii<data_len; ii++) {
		write(FIFODataReg, data[ii]);
	}
	write(CommandReg, PCD_CALCCRC);

	ii = 255;
	do {
		n = read(DivIrqReg);
		ii--;
	} while ((ii!=0) && !(n&0x04));

	buf[0] = read(CRCResultRegL);
	buf[1] = read(CRCResultRegM);
}

uint8_t Reader::send_to_card(uint8_t command, uint8_t *data, uint8_t data_len, uint8_t *buf, uint8_t *buf_len)
{
	uint8_t irq_en = 0x00;
	uint8_t wait_irq = 0x00;
	uint8_t status = TAG_ERROR;
	uint8_t last_bits, n, ii = 0;

	switch (command) {
		case PCD_AUTHENT:
			irq_en = 0x12;
			wait_irq = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irq_en = 0x77;
			wait_irq = 0x30;
			break;
	}

	write(ComIEnReg, irq_en | 0x80);
	clear_bitmask(ComIrqReg, 0x80);		// Clear IRQ
	set_bitmask(FIFOLevelReg, 0x80);	// FIFO init

	write(CommandReg, PCD_IDLE);

	// Write to FIFO
	// printf("%i bytes to fifo\n", data_len);
	for (ii = 0; ii < data_len; ii++) {
		write(FIFODataReg, data[ii]);
	}

	// Execute command
	write(CommandReg, command);
	if (command == PCD_TRANSCEIVE) {
		set_bitmask(BitFramingReg, 0x80);	// start of data transmission
	}

	// Waiting for received data to complete
	ii = 150;
	do {
		usleep(200);
		n = read(ComIrqReg);
		ii--;
	} while ((ii != 0) && (!(n & 0x01)) && (!(n & wait_irq)));

	clear_bitmask(BitFramingReg, 0x80);

	if (ii > 0) {
		if (!(read(ErrorReg) & 0x1b)) {
			status = TAG_OK;
			if (n & irq_en & 0x01) {
				status = TAG_NOTAG;
			}
			if (command == PCD_TRANSCEIVE) {
				n = read(FIFOLevelReg);
				last_bits = read(ControlReg) & 0x07;
				if (last_bits) {
					*buf_len = (n-1)*8 + last_bits;
				} else {
					*buf_len = n*8;
				}
				if (n == 0) {
					n = 1;
				}
				if (n > MAX_RLEN) {
					n = MAX_RLEN;
				}
				// Read FIFO data received
				// printf("sn: ");
				for (ii=0; ii<n; ii++) {
					buf[ii] = read(FIFODataReg);
				// 	printf("%02X:", buf[ii]);
				}
				// printf("\n");
			}
		}
	}

	return status;
}

void Reader::antenna(bool on)
{
	if (on) {
		set_bitmask(TxControlReg, 0x03);
	} else {
		clear_bitmask(TxControlReg, 0x03);
	}
}

void Reader::write(uint8_t addr, uint8_t data)
{
	char buf[2];
	buf[0] = (addr << 1) & 0x7f;
	buf[1] = data;

	switch (get_interface()) {
		case IF_SPI:
			bcm2835_spi_transfern(buf, 2);
			break;
		case IF_I2C:
			break;
		default:
			break;
	}
}

uint8_t Reader::read(uint8_t addr)
{
	char buf[2];
	buf[0] = ((addr << 1) & 0x7e) | 0x80;
	buf[1] = 0;

	switch (get_interface()) {
		case IF_SPI:
			bcm2835_spi_transfern(buf, 2);
			break;
		case IF_I2C:
			break;
		default:
			break;
	}
	return (uint8_t)buf[1];
}

void Reader::set_bitmask(uint8_t addr, uint8_t mask)
{
	uint8_t data = read(addr);
	write(addr, data | mask);
}

void Reader::clear_bitmask(uint8_t addr, uint8_t mask)
{
	uint8_t data = read(addr);
	write(addr, data & ~mask);
}

