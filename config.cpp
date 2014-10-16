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
	this->config_data.rc522_i2c_address = ini.Get("", "rc522_i2c_address", "");

	return 0;
}

void Config::set_config(config_t config)
{
	this->config_data = config;
}

