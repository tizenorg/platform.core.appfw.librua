/*
 *  RUA
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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


#include <stdio.h>
#include <sys/time.h>

unsigned int _perf_measure_start(const char *tag, char *func_name, int line)
{
	struct timeval t;
	unsigned int ts_start;

	if (!tag)
		return 0;

	gettimeofday(&t, NULL);
	ts_start = t.tv_sec * 1000000UL + t.tv_usec;

	printf("### START [%s:%d] timestamp(%u)\n", func_name, line, ts_start);

	return ts_start;
}

unsigned int _perf_measure_end(const char *tag, unsigned int ts_start,
				char *func_name, int line)
{
	struct timeval t;
	unsigned int ts_end;
	unsigned int elapsed_time = 0;

	gettimeofday(&t, NULL);
	ts_end = t.tv_sec * 1000000UL + t.tv_usec;

	if (ts_start)
		elapsed_time = ts_end - ts_start;

	printf("### END [%s:%d] timestamp(%u)\n", func_name, line, ts_end);

	if (elapsed_time)
		printf("### ELAPSED [%s:%d] timestamp(%u)\n", func_name, line,
			elapsed_time);

	return ts_end;
}

