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
	int rc;
	static char const *args_s[] = {
		"",
		tag.c_str()
	};
	unsigned long args_i[] = {
		(unsigned long)time(NULL),
		0
	};
	uint8_t args[] = {
		SQLITE_INTEGER,
		SQLITE_TEXT
	};

	rc = run_sql(DB_INSERT_TAG, args_i, args_s, args, 2);
	if (rc == SQLITE_OK) {
		rc = run_sql(DB_UPDATE_TIME, args_i, args_s, args, 2);
		if (rc == SQLITE_OK) {
			printf("time updated\n");
		} else {
			printf("update_time: %s\n", get_error().c_str());
		}
	} else {
		printf("insert_tag: %s\n", get_error().c_str());
	}

	return rc;
}

std::string DB::get_playfile(std::string tag)
{
	int rc;

	static char const *args_s[] = {
		tag.c_str()
	};
	unsigned long args_i[] = {
		0
	};
	uint8_t args[] = {
		SQLITE_TEXT
	};

	rc = run_sql(DB_SELECT_FILE, args_i, args_s, args, 1);
	if (rc == SQLITE_OK) {
	}

	return "";
}

int DB::run_sql(std::string sql, unsigned long args_i[], const char *args_s[], uint8_t args[], uint8_t argn)
{
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_prepare_v2(this->mydb, sql.c_str(), strlen(sql.c_str()), &stmt, NULL);
	if (rc == SQLITE_OK) {
		for (uint8_t ii=0; ii<argn; ii++) {
			switch (args[ii]) {
				case SQLITE_TEXT:
					sqlite3_bind_text(stmt, ii+1, args_s[ii], strlen(args_s[ii]), 0);
					break;
				case SQLITE_INTEGER:
					sqlite3_bind_int(stmt, ii+1, args_i[ii]);
					break;
			}
		}
		while (1) {
			rc = sqlite3_step(stmt);
			// printf("step(%i): %s\n", rc, get_error().c_str());
			if (rc == SQLITE_DONE) break;
			if (rc == SQLITE_ROW) {
				std::string column_text(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
				printf("column: %s\n", column_text.c_str());
			}
		}
		rc = sqlite3_finalize(stmt);
		// printf("finalize(%i): %s\n", rc, get_error().c_str());
	}

	return rc;
}

