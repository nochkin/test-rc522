#include <algorithm>
#include <cstdlib>

#include "bcm2835.h"

#include "config.h"

using namespace mpc_rfid;

const config_t Config::config()const
{
	return config_data;
}

int Config::load_config(const std::string &cfg_file)
{
	INIReader ini(cfg_file);
	if (ini.ParseError() < 0) {
		return 1;
	}
	config_data.uid = ini.GetInteger("", "uid", 0);
	config_data.db_path = ini.Get("", "db_path", "/var/www/db/config.db");
	config_data.rc522_interface = ini.Get("", "rc522_interface", "");
	config_data.rc522_spi_cs = ini.Get("", "rc522_spi_cs", "cs0");
	config_data.rc522_i2c_address = strtoul(ini.Get("", "rc522_i2c_address", "").c_str(), NULL, 0);
	config_data.rc522_rx_gain = ini.GetInteger("", "rc522_rx_gain", 3);

	this->tolower(config_data.rc522_spi_cs);

	config_data.rc522_spi_cs_int = BCM2835_SPI_CS0;
	if (config_data.rc522_spi_cs == "cs0") {
		config_data.rc522_spi_cs_int = BCM2835_SPI_CS0;
	} else if (config_data.rc522_spi_cs == "cs1") {
		config_data.rc522_spi_cs_int = BCM2835_SPI_CS1;
	} else {
		return 1;
	}

	return 0;
}

void Config::tolower(std::string &input)
{
	std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}

