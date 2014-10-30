#include <algorithm>

#include "config.h"

config_t Config::config()
{
	return this->config_data;
}

int Config::load_config(std::string cfg_file)
{
	INIReader ini(cfg_file);
	if (ini.ParseError() < 0) {
		return 1;
	}
	this->config_data.uid = ini.GetInteger("", "uid", 0);
	this->config_data.db_path = ini.Get("", "db_path", "/var/www/db/config.db");
	this->config_data.rc522_interface = ini.Get("", "rc522_interface", "");
	this->config_data.rc522_spi_cs = ini.Get("", "rc522_spi_cs", "cs0");
	// this->config_data.rc522_i2c_address = ini.Get("", "rc522_i2c_address", "");

	tolower(&config_data.rc522_spi_cs);

	this->config_data.rc522_spi_cs_int = BCM2835_SPI_CS0;
	if (config_data.rc522_spi_cs == "cs0") {
		this->config_data.rc522_spi_cs_int = BCM2835_SPI_CS0;
	} else if (config_data.rc522_spi_cs == "cs1") {
		this->config_data.rc522_spi_cs_int = BCM2835_SPI_CS1;
	}

	return 0;
}

void Config::set_config(config_t config)
{
	this->config_data = config;
}

void Config::tolower(std::string *input)
{
	std::transform(input->begin(), input->end(), input->begin(), ::tolower);
}

