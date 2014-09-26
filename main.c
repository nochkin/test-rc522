#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

#include "bcm2835.h"
#include "rfid.h"

uint8_t HW_init(uint32_t spi_speed, uint8_t gpio) {
	uint16_t sp;

	sp=(uint16_t)(250000L/spi_speed);
	if (!bcm2835_init()) {
		syslog(LOG_DAEMON|LOG_ERR,"Can't init bcm2835!\n");
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
	return 0;
}

int main(int argc, char *argv[])
{
	uint8_t SN[10];
	uint16_t CType=0;
	uint8_t SN_len=0;
	char status;
	int tmp;
	int uid = 33; // www-data

	char *p;
	char sn_str[23];

	uint32_t spi_speed=10000000L;
	uint8_t gpio=255;

	if (HW_init(spi_speed,gpio)) return 1;
	setuid(uid);
	InitRc522();

	while(1)
	{
		status = find_tag(&CType);
		if (status == TAG_NOTAG)
		{
			usleep(200000);
			continue;
		} else if ((status != TAG_OK) && (status != TAG_COLLISION)) continue;
		if (select_tag_sn(SN, &SN_len) != TAG_OK) continue;

		p=sn_str;
		*(p++)='[';
		for (tmp=0;tmp<SN_len;tmp++) {
			sprintf(p,"%02x",SN[tmp]);
			p+=2;
		}
	}

	return 0;
}

