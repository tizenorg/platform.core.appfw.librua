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

/**
 * @file 	rua_stat.h
 * @brief       RUA STATUS API declaration header file.
 * @author      Hyunho Kang (hhstark.kang@samsung.com)
 * @version     0.1
 * @history     0.1: RUA STAT API Declarations, structure declaration
 */

#ifndef __RUA_STAT_H__
#define __RUA_STAT_H__

#include <sqlite3.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief	Get rua status tag list
 * @param[in]	caller, callback, user data
 * @return	0 on success, otherwise a nagative error value
 * @retval	0 on successful
 * @retval	-1 on failed
 */
API int rua_stat_get_stat_tags(char *caller,
		int (*rua_stat_tag_iter_fn)(const char *rua_stat_tag, void *data), void *data);
API int rua_stat_get_stat_tags_for_uid(char *caller,
		int (*rua_stat_tag_iter_fn)(const char *rua_stat_tag, void *data),
		void *data, uid_t uid);
#ifdef __cplusplus
}
#endif
#endif /*__RUA_STAT_H__*/
