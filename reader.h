#ifndef _READER_H
#define _READER_H

#include <string>
#include <stdint.h>

#include "bcm2835.h"
#include "rc522.h"
#include "rfid.h"

class Reader
{
	public:
		Reader();
		int init();
		int init(uint32_t spi_speed, uint8_t gpio);
		std::string read_tag();
	private:
		int init_rc522();
};

#endif
