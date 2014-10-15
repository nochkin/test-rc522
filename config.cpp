#include "config.h"

config_t Config::config()
{
	return this->config_data;
}

bool Config::load_config(std::string cfg_file)
{
	bool status = true;

	INIReader ini(cfg_file);

	return status;
}

void Config::set_config(config_t config)
{
	this->config_data = config;
}

