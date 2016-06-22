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

/**
 * @file        rua.h
 * @brief       RUA INTERNAL API declaration header file.
 * @author      hyunho kang (hhstark.kang@samsung.com)
 * @version     0.1
 * @history     0.1: RUA INTERNAL API Declarations, structure declaration
 */

#ifndef __RUA_INTERNAL_H__
#define __RUA_INTERNAL_H__

#include <sqlite3.h>
#include <bundle.h>
#include <time.h>
#include <dlog.h>

#include "rua.h"

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RUA"
#define RUA_DB_NAME	".rua.db"
#define RUA_HISTORY	"rua_history"
#define QUERY_MAXLEN	4096
#define MAX_UID_STR_BUFSZ 20

/**
 * @brief	Delete history from DB
 * @return 	0 on success, otherwise a nagative error value
 * @retval	0 on successful
 * @retval	-1 on failed
 */
API int rua_db_delete_history(bundle *b);

/**
 * @brief	Add application to recently used application list
 * @param[in]	rec  record to add history
 * @return	0 on success, otherwise a nagative error value
 * @retval	0 on successful
 * @retval	-1 on failed
 */
API int rua_db_add_history(struct rua_rec *rec);

#ifdef __cplusplus
}
#endif
#endif		/*__RUA_INTERNAL_H__*/
