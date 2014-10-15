#include "config.h"

config_t Config::config()
{
	return this->config_data;
}

bool Config::load_config(std::string cfg_file)
{
	bool status = true;

	INIReader ini(cfg_file);
	if (ini.ParseError() < 0) {
		return false;
	}
	this->config_data.uid = ini.GetInteger("", "uid", 0);
	this->config_data.db_path = ini.Get("", "db_path", "/var/www/db/config.db");
	this->config_data.rc522_interface = ini.Get("", "rc522_intercace", "");
	this->config_data.rc522_i2c_address = ini.Get("", "rc522_i2c_address", "");
	this->config_data.ssd3306_interface = ini.Get("", "ssd3306_interface", "");
	this->config_data.ssd3306_i2c_address = ini.Get("", "ssd3306_i2c_address", "");

	return status;
}

void Config::set_config(config_t config)
{
	this->config_data = config;
}

