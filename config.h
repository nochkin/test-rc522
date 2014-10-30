#include "bcm2835.h"

#include "INIReader.h"

struct config_t
{
	std::string db_path;
	std::string rc522_interface;
	std::string rc522_spi_cs;
	uint8_t rc522_spi_cs_int;
	uint8_t rc522_i2c_address;
	int uid;
};

class Config
{
	public:
		static Config& get()
		{
			static Config instance;
			return instance;
		}
		config_t config();
		int load_config(std::string);
		void set_config(config_t);
	private:
		Config() {};
		Config(Config const&);
		void operator=(Config const&);

		void tolower(std::string *);

		config_t config_data;
};

