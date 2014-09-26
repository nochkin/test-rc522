#include <sqlite3.h>
#include <stdio.h>

static int mydb_callback(void *NotUsed, int argc, char **argv, char **azColName){
	for(int i=0; i<argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int mydb_open(sqlite3 **db)
{
	char *sql;
	char *err_msg = 0;
	int rc;

	rc = sqlite3_open("rc522.db", db);
	if (rc) return rc;

	sql = "create table rfid_tags(id text primary key not null, timestamp int not null)";
	rc = sqlite3_exec(*db, sql, mydb_callback, 0, &err_msg);

	return rc;
}

int mydb_close(sqlite3 *db)
{
	return sqlite3_close(db);
}

