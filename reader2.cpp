#include <stdio.h>

#include "reader2.h"

Reader2::Reader2()
{
}

int Reader2::init_spi(int8_t RST)
{
	if (bcm2835_init()) {
		bcm2835_spi_begin();
		// bcm2835_spi_chipSelect(cs);
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		// 16 MHz SPI bus, but Worked at 62 MHz also
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);
		// Set the pin that will control DC as output
		// bcm2835_gpio_fsel(dc, BCM2835_GPIO_FSEL_OUTP);
		// Setup reset pin direction as output
		bcm2835_gpio_fsel(RST, BCM2835_GPIO_FSEL_OUTP);
		_if_type = IF_SPI;
		return init();
	} else {
		return -1;
	}
}

int Reader2::init_i2c()
{
	_if_type = IF_I2C;
	return -1;
}

int Reader2::read_card()
{
	uint8_t tag_type[MAX_RLEN];
	uint8_t status = request(PICC_REQIDL, tag_type);
	write(BitFramingReg, 0x07);
	if (status == TAG_OK) {
		//status = anticoll();
	}
	return 0;
}

int Reader2::init()
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

interface_t Reader2::get_interface() {
	return _if_type;
}

uint8_t Reader2::request(uint8_t req_mode, uint8_t *tag_type)
{
	uint8_t status;
	uint8_t buf[MAX_RLEN];
	uint8_t buf_len = 0;

	write(BitFramingReg, 0x07);
	status = send_to_card(PCD_TRANSCEIVE, tag_type, 1, buf, &buf_len);
	return status;
}

uint8_t Reader2::send_to_card(uint8_t command, uint8_t *data, uint8_t data_len, uint8_t *buf, uint8_t *buf_len)
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
				printf("sn: ");
				for (ii=0; ii<n; ii++) {
					buf[ii] = read(FIFODataReg);
					printf("%02X:", buf[ii]);
				}
				printf("\n");
			}
		}
	}

	return status;
}

void Reader2::antenna(bool on)
{
	if (on) {
		set_bitmask(TxControlReg, 0x03);
	} else {
		clear_bitmask(TxControlReg, 0x03);
	}
}

void Reader2::write(uint8_t addr, uint8_t data)
{
	char buf[2];
	buf[0] = addr & 0x7f;
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

uint8_t Reader2::read(uint8_t addr)
{
	char buf[2];
	buf[0] = addr | (1<<7);
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

void Reader2::set_bitmask(uint8_t addr, uint8_t mask)
{
	uint8_t data = read(addr);
	write(addr, data | mask);
}

void Reader2::clear_bitmask(uint8_t addr, uint8_t mask)
{
	uint8_t data = read(addr);
	write(addr, data & ~mask);
}

