/*
 * Copyright (c) 2000 - 2016 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <db-util.h>
#include <aul.h>

/* For multi-user support */
#include <tzplatform_config.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RUA"

#include "rua.h"
#include "db-schema.h"
#include "perf-measure.h"

#define RUA_DB_NAME	".rua.db"
#define RUA_HISTORY	"rua_history"
#define QUERY_MAXLEN	4096
#define Q_LATEST \
	"select pkg_name from rua_history " \
	"order by launch_time desc limit 1 "

static int __exec(sqlite3 *db, char *query);
static int __create_table(sqlite3 *db);
static sqlite3 *__db_init();

int rua_delete_history_from_db(bundle *b)
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

int rua_clear_history(void)
{
	int r;
	r = aul_delete_rua_history(NULL);
	LOGI("rua_clear_history result : %d ", r);
	return r;
}

int rua_delete_history_with_pkgname(char *pkg_name)
{
	int r;
	bundle *b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}

	bundle_add_str(b, AUL_K_RUA_PKGNAME, pkg_name);
	r = aul_delete_rua_history(b);
	LOGI("rua_delete_history_with_pkgname result : %d ", r);
	bundle_free(b);
	return r;
}

int rua_delete_history_with_apppath(char *app_path)
{
	int r;
	bundle *b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}

	bundle_add_str(b, AUL_K_RUA_APPPATH, app_path);
	r = aul_delete_rua_history(b);
	LOGI("rua_delete_history_with_apppath result : %d ", r);
	bundle_free(b);

	return r;
}

int rua_add_history(struct rua_rec *rec)
{
	int r;
	int cnt = 0;
	char query[QUERY_MAXLEN];
	sqlite3_stmt *stmt;
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
		"select count(*) from %s where pkg_name = '%s';", RUA_HISTORY,
		rec->pkg_name);

	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
	if (r != SQLITE_OK) {
		LOGE("Error sqlite3_prepare fail");
		db_util_close(db);
		return -1;
	}

	r = sqlite3_step(stmt);
	if (r == SQLITE_ROW)
		cnt = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	if (cnt == 0)
		/* insert */
		snprintf(query, QUERY_MAXLEN,
			"insert into %s ( pkg_name, app_path, arg, launch_time ) "
			" values ( \"%s\", \"%s\", \"%s\", %d ) ",
			RUA_HISTORY,
			rec->pkg_name ? rec->pkg_name : "",
			rec->app_path ? rec->app_path : "",
			rec->arg ? rec->arg : "", (int)time(NULL));
	else
		/* update */
		snprintf(query, QUERY_MAXLEN,
			"update %s set arg='%s', launch_time='%d' where pkg_name = '%s';",
			RUA_HISTORY,
			rec->arg ? rec->arg : "", (int)time(NULL), rec->pkg_name);

	r = __exec(db, query);
	if (r == -1) {
		LOGE("[RUA ADD HISTORY ERROR] %s\n", query);
		db_util_close(db);
		return -1;
	}

	db_util_close(db);
	return r;
}

int rua_history_load_db(char ***table, int *nrows, int *ncols)
{
	int r;
	char query[QUERY_MAXLEN];
	char *db_err = NULL;
	char **db_result = NULL;
	sqlite3 *db = NULL;

	char defname[FILENAME_MAX];
	const char *rua_db_path = tzplatform_getenv(TZ_USER_DB);
	if (rua_db_path == NULL) {
		LOGE("fail to get rua_db_path");
		return -1;
	}
	snprintf(defname, sizeof(defname), "%s/%s", rua_db_path, RUA_DB_NAME);

	if (table == NULL)
		return -1;
	if (nrows == NULL)
		return -1;
	if (ncols == NULL)
		return -1;

	r = db_util_open_with_options(defname, &db, SQLITE_OPEN_READONLY, NULL);
	if (r) {
		db_util_close(db);
		return -1;
	}

	snprintf(query, QUERY_MAXLEN,
		 "select * from %s order by launch_time desc;", RUA_HISTORY);

	r = sqlite3_get_table(db, query, &db_result, nrows, ncols, &db_err);

	if (r == SQLITE_OK)
		*table = db_result;
	else
		sqlite3_free_table(db_result);

	db_util_close(db);

	return r;
}

int rua_history_unload_db(char ***table)
{
	if (*table) {
		sqlite3_free_table(*table);
		*table = NULL;
		return 0;
	}
	return -1;
}

int rua_history_get_rec(struct rua_rec *rec, char **table, int nrows, int ncols,
			int row)
{
	char **db_result = NULL;
	char *tmp = NULL;

	if (rec == NULL)
		return -1;
	if (table == NULL)
		return -1;
	if (row >= nrows)
		return -1;

	db_result = table + ((row + 1) * ncols);

	tmp = db_result[RUA_COL_ID];
	if (tmp)
		rec->id = atoi(tmp);

	tmp = db_result[RUA_COL_PKGNAME];
	if (tmp)
		rec->pkg_name = tmp;

	tmp = db_result[RUA_COL_APPPATH];
	if (tmp)
		rec->app_path = tmp;

	tmp = db_result[RUA_COL_ARG];
	if (tmp)
		rec->arg = tmp;

	tmp = db_result[RUA_COL_LAUNCHTIME];
	if (tmp)
		rec->launch_time = atoi(tmp);

	return 0;
}

int rua_is_latest_app(const char *pkg_name)
{
	int r = -1;
	sqlite3_stmt *stmt;
	const unsigned char *ct;
	sqlite3 *db;

	char defname[FILENAME_MAX];
	const char *rua_db_path = tzplatform_getenv(TZ_USER_DB);
	if (rua_db_path == NULL) {
		LOGE("fail to get rua_db_path");
		return -1;
	}
	snprintf(defname, sizeof(defname), "%s/%s", rua_db_path, RUA_DB_NAME);

	if (!pkg_name)
		return -1;

	r = db_util_open_with_options(defname, &db, SQLITE_OPEN_READONLY, NULL);
	if (r) {
		db_util_close(db);
		return -1;
	}

	r = sqlite3_prepare(db, Q_LATEST, sizeof(Q_LATEST), &stmt, NULL);
	if (r != SQLITE_OK) {
		db_util_close(db);
		return -1;
	}

	r = sqlite3_step(stmt);
	if (r == SQLITE_ROW) {
		ct = sqlite3_column_text(stmt, 0);
		if (ct == NULL || ct[0] == '\0') {
			r = -1;
			goto out;
		}

		if (strncmp(pkg_name, (const char *)ct, strlen(pkg_name)) == 0) {
			r = 0;
			goto out;
		}
	}

out:
	if (stmt)
		sqlite3_finalize(stmt);
	if (db)
		db_util_close(db);

	return r;
}

int rua_init(void)
{
	return 0;
}

int rua_fini(void)
{
	return 0;
}

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

	char defname[FILENAME_MAX];
	const char *rua_db_path = tzplatform_getenv(TZ_USER_DB);
	if (rua_db_path == NULL) {
		LOGE("fail to get rua_db_path");
		return NULL;
	}
	snprintf(defname, sizeof(defname), "%s/%s", rua_db_path, RUA_DB_NAME);

	r = db_util_open_with_options(defname, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	r = __create_table(db);
	if (r) {
		db_util_close(db);
		return NULL;
	}

	return db;
}
