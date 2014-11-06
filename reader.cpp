#include <stdio.h>

#include "reader.h"

Reader::Reader() : _if_type(IF_NOT_SET)
{
	memset(this->tag_full_sn, 0, 7);
}

int Reader::init_spi(uint8_t cs)
{
	if (bcm2835_init()) {
		bcm2835_spi_begin();
		bcm2835_spi_chipSelect(cs);
		bcm2835_spi_setChipSelectPolarity(cs, LOW);
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		// 16 MHz SPI bus, but Worked at 62 MHz also
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);

		_if_type = IF_SPI;
		return init();
	}
	return 1;
}

int Reader::init_i2c(uint8_t address)
{
	if (bcm2835_init()) {
		bcm2835_i2c_begin();
		i2c_address = address;
		bcm2835_i2c_setSlaveAddress(i2c_address);
		// Set clock to 400 KHz
		bcm2835_i2c_set_baudrate(400000);
		// Setup reset pin direction as output
		// bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);

		_if_type = IF_I2C;
		return init();
	}
	return 1;
}

uint8_t Reader::read_tag()
{
	uint8_t tag_type[MAX_READ_LEN];

	memset(tag_full_sn, 0, TAG_LEN);

	uint8_t status = request(PICC_REQIDL, tag_type);
	memcpy(tag_full_sn, tag_type, TAG_TYPE_LEN);
	write(BitFramingReg, 0x07);
	if (status == TAG_OK) {
		uint8_t sn[10];
		status = anticoll(sn);
		memcpy(tag_full_sn+TAG_TYPE_LEN, sn, TAG_LEN_SHORT-2);
		status = select_sn(sn);
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

std::string Reader::get_tag_str(const std::string &delim)const
{
	std::string tag_full_str;
	char tmp_buf[3];

	for (int ii=0; ii<TAG_LEN_SHORT; ++ii) {
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

interface_t Reader::get_interface()const {
	return _if_type;
}

uint8_t Reader::request(uint8_t req_mode, uint8_t *tag_type)
{
	uint8_t buf[MAX_READ_LEN];
	uint8_t buf_len = 0;

	write(BitFramingReg, 0x07);
	buf[0] = req_mode;
	uint8_t status = send_to_card(PCD_TRANSCEIVE, buf, 1, buf, &buf_len);
	if (buf_len > 1) {
		tag_type[0] = buf[0];
		tag_type[1] = buf[1];
	}

	return status;
}

uint8_t Reader::anticoll(uint8_t *sn)
{
	uint8_t sn_len = 0;

	write(BitFramingReg, 0x00);
	sn[0] = PICC_ANTICOLL1;
	sn[1] = 0x20;
	uint8_t status = send_to_card(PCD_TRANSCEIVE, sn, 2, sn, &sn_len);

	if (status == TAG_OK) {
		uint8_t ii, sn_check = 0;
		for (ii=0; ii<4; ++ii) {
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
	uint8_t buf[MAX_READ_LEN];
	uint8_t buf_len = 0;

	buf[0] = PICC_ANTICOLL1;
	buf[1] = 0x70;
	for (int ii=0; ii<TAG_LEN_SHORT-TAG_CRC_LEN; ++ii) {
		buf[ii+2] = *(sn++);
	}

	calculate_crc(buf, TAG_LEN_SHORT, &buf[TAG_LEN_SHORT]);
	// clear_bitmask(Status2Reg, 0x08);

	uint8_t status = send_to_card(PCD_TRANSCEIVE, buf, TAG_LEN_SHORT+2, buf, &buf_len);
	if (buf_len != 0x18) {
		status = TAG_ERROR;
	}

	return status;
}

uint8_t Reader::halt()
{
	uint8_t buf[4];
	uint8_t buf_len = 0;

	buf[0] = PICC_HALT;
	buf[1] = 0;
	calculate_crc(buf, 2, &buf[2]);

	return send_to_card(PCD_TRANSCEIVE, buf, 2+TAG_CRC_LEN, buf, &buf_len);
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
				if (n > MAX_READ_LEN) {
					n = MAX_READ_LEN;
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

void Reader::write(uint8_t addr, uint8_t data)const
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

uint8_t Reader::read(uint8_t addr)const
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

void Reader::set_bitmask(uint8_t addr, uint8_t mask)const
{
	uint8_t data = read(addr);
	write(addr, data | mask);
}

void Reader::clear_bitmask(uint8_t addr, uint8_t mask)const
{
	uint8_t data = read(addr);
	write(addr, data & ~mask);
}

