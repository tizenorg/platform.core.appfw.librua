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
#include "rua_util.h"
#include "rua_internal.h"
#include "rua.h"
#include "db-schema.h"

int rua_add_history_for_uid(char *pkg_name, char *app_path, char *arg, uid_t uid)
{
	int r;
	char time_str[32] = {0,};
	bundle *b = NULL;

	if (pkg_name == NULL || app_path == NULL) {
		LOGE("invalid param");
		return -1;
	}

	b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}
	snprintf(time_str, sizeof(time_str), "%d", (int)time(NULL));
	bundle_add_str(b, AUL_K_RUA_PKGNAME, pkg_name);
	bundle_add_str(b, AUL_K_RUA_APPPATH, app_path);
	bundle_add_str(b, AUL_K_RUA_ARG, arg);
	bundle_add_str(b, AUL_K_RUA_TIME, time_str);

	r = aul_add_rua_history_for_uid(b, uid);
	LOGI("rua_add_history_for_uid result : %d ", r);
	bundle_free(b);
	return r;
}

int rua_delete_history_with_pkgname(char *pkg_name)
{
	return rua_delete_history_with_pkgname_for_uid(pkg_name, getuid());
}

int rua_delete_history_with_pkgname_for_uid(char *pkg_name, uid_t uid)
{
	int r;
	bundle *b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}
	bundle_add_str(b, AUL_K_RUA_PKGNAME, pkg_name);
	r = aul_delete_rua_history_for_uid(b, uid);
	LOGI("rua_delete_history_with_pkgname result : %d ", r);
	bundle_free(b);
	return r;
}

int rua_delete_history_with_apppath(char *app_path)
{
	return rua_delete_history_with_apppath_for_uid(app_path, getuid());
}

int rua_delete_history_with_apppath_for_uid(char *app_path, uid_t uid)
{
	int r;
	bundle *b = bundle_create();
	if (b == NULL) {
		LOGE("bundle_create fail out of memory.");
		return -1;
	}
	bundle_add_str(b, AUL_K_RUA_APPPATH, app_path);
	r = aul_delete_rua_history_for_uid(b, uid);
	LOGI("rua_delete_history_with_apppath result : %d ", r);
	bundle_free(b);
	return r;
}

int rua_clear_history(void)
{
	return rua_clear_history_for_uid(getuid());
}

int rua_clear_history_for_uid(uid_t uid)
{
	int r;
	r = aul_delete_rua_history_for_uid(NULL, uid);
	LOGI("rua_clear_history result : %d ", r);
	return r;
}

int rua_history_load_db(char ***table, int *nrows, int *ncols)
{
	return rua_history_load_db_for_uid(table, nrows, ncols, getuid());
}

int rua_history_load_db_for_uid(char ***table, int *nrows, int *ncols, uid_t uid)
{
	int r;
	char query[QUERY_MAXLEN];
	char *db_err = NULL;
	char **db_result = NULL;
	sqlite3 *db = NULL;

	if (table == NULL)
		return -1;
	if (nrows == NULL)
		return -1;
	if (ncols == NULL)
		return -1;

	r = _rua_util_open_db(&db, SQLITE_OPEN_READONLY, uid, RUA_DB_NAME);
	if (r != SQLITE_OK)
		return -1;

	snprintf(query, QUERY_MAXLEN,
		 "select pkg_name, app_path, arg, launch_time from %s order by launch_time desc;", RUA_HISTORY);
	LOGE("query @@@@@@@@@@ %s", query);

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
	LOGE("@@@@@@@@@@@@ get rec !!!!");
	char **db_result = NULL;
	char *tmp = NULL;

	if (rec == NULL)
		return -1;
	if (table == NULL)
		return -1;
	if (row >= nrows)
		return -1;

	db_result = table + ((row + 1) * ncols);

	tmp = db_result[RUA_COL_PKGNAME];
	if (tmp)
		rec->pkg_name = tmp;

	LOGE("@@@@@@@@@@@@ get rec !!!! %s", rec->pkg_name);

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
	return rua_is_latest_app_for_uid(pkg_name, getuid());
}

int rua_is_latest_app_for_uid(const char *pkg_name, uid_t uid)
{
	int r = -1;
	sqlite3_stmt *stmt;
	const unsigned char *ct;
	sqlite3 *db = NULL;
	char *query = "select pkg_name from rua_history order by launch_time desc limit 1;";

	if (!pkg_name)
		return -1;

	r = _rua_util_open_db(&db, SQLITE_OPEN_READONLY, uid, RUA_DB_NAME);
	if (r != SQLITE_OK)
		return -1;

	r = sqlite3_prepare(db, query, sizeof(query), &stmt, NULL);
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
