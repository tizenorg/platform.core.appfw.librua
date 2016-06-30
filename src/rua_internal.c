/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tzplatform_config.h>
#include <db-util.h>
#include <aul.h>

#include "rua_internal.h"
#include "db-schema.h"
#include "rua_util.h"

static int __exec(sqlite3 *db, char *query)
{
	int r;
	char *errmsg = NULL;

	if (db == NULL)
		return -1;

	r = sqlite3_exec(db, query, NULL, NULL, &errmsg);

	if (r != SQLITE_OK) {
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}

static int __create_table(sqlite3 *db)
{
	int r;

	r = __exec(db, CREATE_RUA_HISTORY_TABLE);
	if (r == -1)
		return -1;

	return 0;
}

static sqlite3 *__db_init()
{
	int r;
	sqlite3 *db = NULL;

	r = _rua_util_open_db(&db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, getuid(), RUA_DB_NAME);
	if (r != SQLITE_OK)
		return NULL;

	r = __create_table(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	return db;
}

int rua_db_delete_history(bundle *b)
{
	int r;
	sqlite3 *db = NULL;
	char query[QUERY_MAXLEN];
	char *pkg_name = NULL;
	char *app_path = NULL;
	char *errmsg = NULL;
	int result = 0;

	db = __db_init();
	if (db == NULL) {
		LOGE("Error db null");
		return -1;
	}

	if (b != NULL) {
		bundle_get_str(b, AUL_K_RUA_PKGNAME, &pkg_name);
		bundle_get_str(b, AUL_K_RUA_APPPATH, &app_path);
	}

	if (pkg_name != NULL)
		snprintf(query, QUERY_MAXLEN, "delete from rua_history where pkg_name = '%s';", pkg_name);
	else if (app_path != NULL)
		snprintf(query, QUERY_MAXLEN, "delete from rua_history where app_path = '%s';", app_path);
	else
		snprintf(query, QUERY_MAXLEN, "delete from rua_history;");

	LOGI("rua_delete_history_from_db : %s", query);
	r = sqlite3_exec(db, query, NULL, NULL, &errmsg);

	if (r != SQLITE_OK) {
		LOGE("fail to exec delete query %s : %s", query, errmsg);
		sqlite3_free(errmsg);
		result = -1;
	}

	if (db != NULL)
		db_util_close(db);

	return result;
}

int rua_db_add_history(struct rua_rec *rec)
{
	int r;
	char query[QUERY_MAXLEN];
	sqlite3 *db = NULL;

	db = __db_init();
	if (db == NULL) {
		LOGE("Error db null");
		return -1;
	}

	if (rec == NULL) {
		LOGE("Error rec null");
		db_util_close(db);
		return -1;
	}

	snprintf(query, QUERY_MAXLEN,
		"insert or replace into %s ( pkg_name, app_path, arg, launch_time) "
		" values ( \"%s\", \"%s\", \"%s\", %d) ",
		RUA_HISTORY,
		rec->pkg_name ? rec->pkg_name : "",
		rec->app_path ? rec->app_path : "",
		rec->arg ? rec->arg : "", (int)rec->launch_time);

	r = __exec(db, query);
	if (r == -1) {
		LOGE("[RUA ADD HISTORY ERROR] %s\n", query);
		db_util_close(db);
		return -1;
	}

	db_util_close(db);
	return r;
}
