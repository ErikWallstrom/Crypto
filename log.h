/*
	Copyright (C) 2017 Erik Wallström

	This file is part of Erwall.

	Erwall is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Erwall is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Erwall.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

//NOTE: Uses GNU extension for optional va_args
#define log_info(fmt, ...) \
	log_msg(stdout, LOGMSGTYPE_INFO, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) \
	log_msg(stdout, LOGMSGTYPE_ERROR, fmt, ##__VA_ARGS__)
#define log_warning(fmt, ...) \
	log_msg(stdout, LOGMSGTYPE_WARNING, fmt, ##__VA_ARGS__)

#ifndef NDEBUG
//NOTE: expr ? 1 : 0 converts for example pointer to int
#define log_assert(expr, fmt, ...) \
	log_assert_( \
		#expr, \
		expr ? 1 : 0, \
		__LINE__, \
		__func__, \
		fmt, \
		##__VA_ARGS__ \
	)
#else
//XXX: Works, don't know if it's the best way to disable assert though
#define log_assert(expr, fmt, ...) \
	(void)sizeof(expr);
#endif

typedef void(*LogErrorCallback)(void*);

enum LogMsgType
{
	LOGMSGTYPE_INFO,
	LOGMSGTYPE_ERROR,
	LOGMSGTYPE_WARNING,
};

//Should this be FILE* or struct File?
void log_msg(FILE* file, enum LogMsgType type, const char* fmt, ...)
	__attribute__((format (printf, 3, 4)));

//TODO: Set different error fatal for different files, avoid global 
void log_seterrorhandler(LogErrorCallback callback, void* udata);

void log_assert_(
	const char* expression, 
	int result, 
	int line, 
	const char* func, 
	const char* fmt,
	...
) __attribute__((format (printf, 5, 6)));

#endif
