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
 * @file 	rua_stat_internal.h
 * @brief       RUA STATUS INTERNAL API declaration header file.
 * @author      Hyunho Kang (hhstark.kang@samsung.com)
 * @version     0.1
 * @history     0.1: RUA STAT INTERNAL API Declarations, structure declaration
 */

#ifndef __RUA_STAT_INTERNAL_H__
#define __RUA_STAT_INTERNAL_H__

#include <sqlite3.h>
#include <dlog.h>

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

#define RUA_STAT_DB_NAME	".rua_stat.db"
#define QUERY_MAXLEN	4096
#define WIN_SCORE 100
#define LOSE_SCORE_RATE 0.7f

int _rua_stat_init(sqlite3 **db, int flags);
int _rua_stat_fini(sqlite3 *db);

/**
 * @brief	Add application launch status.
 * @param[in]	caller, rua_stat_tag
 * @return	0 on success, otherwise a nagative error value
 * @retval	0 on successful
 * @retval	-1 on failed
 */
API int rua_stat_db_update(char *caller, char *rua_stat_tag);

#ifdef __cplusplus
}
#endif
#endif /*__RUA_STAT_INTERNAL_H__*/
