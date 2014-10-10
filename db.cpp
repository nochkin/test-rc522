#include "db.h"

static int mydb_callback(void *NotUsed, int argc, char **argv, char **azColName){
	for(int i=0; i<argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

DB::DB()
{
	db_filename = "rc522.db";
}

DB::DB(std::string db_filename)
{
	db_filename = db_filename;
}

int DB::open()
{
	int rc;

	rc = sqlite3_open(db_filename.c_str(), &db);
	if (rc == 0) return rc;

	return create();
}

int DB::close()
{
	return sqlite3_close(db);
}

std::string DB::get_error()
{
	std::string err_msg(sqlite3_errmsg(db));
	return err_msg;
}

int DB::create()
{
	std::string sql;
	char *err_msg = 0;
	int rc;

	sql = "create table rfid_tags(id text primary key not null, timestamp int not null)";
	rc = sqlite3_exec(db, sql.c_str(), mydb_callback, 0, &err_msg);

	return rc;
}

