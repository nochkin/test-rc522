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
	my_filename = "/tmp/rc522.db";
	this->mydb = NULL;
}

DB::DB(std::string db_filename)
{
	this->my_filename = db_filename;
	this->mydb = NULL;
}

int DB::open()
{
	int rc;

	rc = sqlite3_open_v2(my_filename.c_str(), &this->mydb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	printf("mydb: %li\n", this->mydb);
	if (rc == SQLITE_OK) {
		return create();
	}

	return rc;
}

int DB::close()
{
	return sqlite3_close(this->mydb);
}

std::string DB::get_error()
{
	std::string err_msg(sqlite3_errmsg(this->mydb));
	return err_msg;
}

int DB::create()
{
	char *err_msg = 0;
	int rc;

	rc = sqlite3_exec(this->mydb, DB_CREATE_TAGS, mydb_callback, 0, &err_msg);

	return rc;
}

int DB::add_new(std::string tag)
{
	printf("mydb: %li\n", this->mydb);
	int rc;
	static char const *args_s[] = {
		tag.c_str(),
		""
	};
	unsigned long *args_i[] = {
		NULL,
		(unsigned long*)time(NULL)
	};

	rc = run_sql(DB_INSERT_TAG, args_i, args_s, 2);

	/*
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
	*/

	return rc;
}

int DB::run_sql(std::string sql, unsigned long *args_i[], const char *args_s[], uint8_t args)
{
	int rc;
	sqlite3_stmt *stmt;

	printf("mydb: %li\n", this->mydb);
	rc = sqlite3_prepare_v2(this->mydb, sql.c_str(), strlen(sql.c_str()), &stmt, NULL);
	printf("prepare: %s\n", get_error().c_str());
	if (rc == SQLITE_OK) {
		for (uint8_t ii=0; ii<args; ii++) {
			if (args_i[ii] != NULL) {
				sqlite3_bind_text(stmt, ii+1, args_s[ii], strlen(args_s[ii]), 0);
			} else {
				sqlite3_bind_int(stmt, ii+1, *args_i[ii]);
			}
		}
		sqlite3_step(stmt);
		printf("step: %s\n", get_error().c_str());
		sqlite3_finalize(stmt);
		printf("finalize: %s\n", get_error().c_str());
	}

	return rc;
}

