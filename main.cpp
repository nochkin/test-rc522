#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

#include "bcm2835.h"
#include "rc522.h"
#include "rfid.h"
#include "db.h"
#include "reader.h"

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

	Reader reader;
	err = reader.init();
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

