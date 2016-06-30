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
 * @file        rua_util.h
 * @brief       RUA UTIL API declaration header file.
 * @author      Hyunho Kang (hhstark.kang@samsung.com)
 * @version     0.1
 * @history     0.1: RUA UTIL API Declarations, structure declaration
 */

#ifndef __RUA_UTIL_H__
#define __RUA_UTIL_H__

#include <unistd.h>
#include <stdlib.h>
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
#define BASE_UID 5000

char *_rua_util_get_db_path(uid_t uid, char *db_name);
int _rua_util_open_db(sqlite3 **db, int flags, uid_t uid, char *db_name);
int _rua_util_check_uid(uid_t target_uid);

#ifdef __cplusplus
}
#endif
#endif				/*__RUA_UTIL_H__*/
