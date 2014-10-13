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
	std::string sql = DB_CREATE_TAGS;
	char *err_msg = 0;
	int rc;

	rc = sqlite3_exec(db, sql.c_str(), mydb_callback, 0, &err_msg);

	return rc;
}

int DB::add_new(std::string tag)
{
	int rc;
	sqlite3_stmt *stmt;
	const char *pz;
	std::string sql = DB_INSERT_TAG;

	rc = sqlite3_prepare(db, sql.c_str(), sql.size(), &stmt, &pz);
	if (rc == SQLITE_OK) {
		unsigned long timestamp = time(NULL);
		const char *tag_c = tag.c_str();
		sqlite3_bind_text(stmt, 1, tag_c, strlen(tag_c), 0);
		sqlite3_bind_int(stmt, 2, timestamp);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}

	return rc;
}

