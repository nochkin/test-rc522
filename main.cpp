#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

#include "db.h"
#include "reader2.h"

int main(int argc, char *argv[])
{
	char status;
	int uid = 33; // www-data
	int err;

	DB mydb;
	status = mydb.open();
	if (status)
	{
		fprintf(stderr, "DB: error opening: %s\n", mydb.get_error().c_str());
		return 2;
	}

	Reader2 reader;
	err = reader.init_spi();
	if (err == 1) {
		syslog(LOG_DAEMON|LOG_ERR,"Failed init bcm2835\n");
		return 1;
	}
	setuid(uid);

	while(1)
	{
		usleep(200000);
	}

	status = mydb.close();

	return 0;
}

