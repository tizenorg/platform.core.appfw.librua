/*
 *  RUA
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jayoun Lee <airjany@samsung.com>
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
 *
 */

#ifndef __RUA_H__
#define __RUA_H__

#include <sqlite3.h>
#include <time.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct rua_rec {
	int id;
	char *pkg_name;
	char *app_path;
	char *arg;
	time_t launch_time;
};

/* API */
/**
 * Clear history 
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_clear_history(void);

/* API */
/**
 * Delete history with pkg_name 
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_delete_history_with_pkgname(char *pkg_name);

/* API */
/**
 * Delete history with app_path
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_delete_history_with_apppath(char *app_path);

/* API */
/**
 * Delete history with app_path (Will be deprecated!)
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_delete_history(char *app_path);

/* API */
/**
 * Add application to recently used application list
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_add_history(struct rua_rec *rec);

/**
 * Load recently used application history db.
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_history_load_db(char ***table, int *nrows, int *ncols);

/**
 * Unload recently used application history db.
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_history_unload_db(char ***table);

/**
 * Load recently used application record.
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_history_get_rec(struct rua_rec *rec, char **table,
				int nrows, int ncols, int row);

/**
 * Check some package is latest or not with package name
 * return This method returns 0 (true) or -1 (false)
 */
API int rua_is_latest_app(const char *pkg_name);

/**
 * initialize rua 
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_init(void);

/**
 * finalize rua
 * return This method returns 0 (SUCCESS) or -1 (FAIL)
 */
API int rua_fini(void);

#ifdef __cplusplus
}
#endif
#endif				/*__RUA_H__*/
