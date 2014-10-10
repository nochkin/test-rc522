#include "reader.h"

Reader::Reader() {
}

int Reader::init() {
	uint32_t spi_speed=10000000L;
	uint8_t gpio=255;

	return init(spi_speed, gpio);
}

int Reader::init(uint32_t spi_speed, uint8_t gpio) {
	uint16_t sp;

	sp=(uint16_t)(250000L/spi_speed);
	if (!bcm2835_init()) {
		return 1;
	}
	if (gpio<28) {
		bcm2835_gpio_fsel(gpio, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_write(gpio, LOW);
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	bcm2835_spi_setClockDivider(sp); // The default
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

	init_rc522();

	return 0;
}

int Reader::init_rc522() {
	InitRc522();
	return 0;
}

std::string read_tag() {
	uint8_t SN[10];
	uint16_t CType=0;
	uint8_t SN_len=0;
	char status;
	int tmp;
	char *p;
	char sn_str[23];

		status = find_tag(&CType);
		if ((status == TAG_NOTAG)
				|| ((status != TAG_OK) && (status != TAG_COLLISION))) {
			return "";
		}
		if (select_tag_sn(SN, &SN_len) != TAG_OK) {
			return "";
		}

		p=sn_str;
		*(p++)='[';
		for (tmp=0;tmp<SN_len;tmp++) {
			sprintf(p,"%02x",SN[tmp]);
			p+=2;
		}
	return "";
}

