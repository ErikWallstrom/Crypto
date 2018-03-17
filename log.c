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

#include "log.h"
#include "ansicode.h"
#include <assert.h>
#include <stdlib.h>

static LogErrorCallback errorcallback;
static void* errorudata;

void log_msg(FILE* file, enum LogMsgType type, const char* fmt, ...)
{
	log_assert(file, "is NULL");
	log_assert(fmt, "is NULL");
	log_assert(
		type == LOGMSGTYPE_INFO ||
			type == LOGMSGTYPE_ERROR ||
			type == LOGMSGTYPE_WARNING,
		"'%i' is not a valid LogMsgType",
		type
	);

	va_list vlist;
	va_start(vlist, fmt);

	if(file != stdout)
		//Disable color output for normal files
	{
		switch(type)
		{
		case LOGMSGTYPE_INFO:
			fputs("*INFO* ", file);
			break;
		case LOGMSGTYPE_ERROR:
			fputs("*ERROR* ", file);
			break;
		case LOGMSGTYPE_WARNING:
			fputs("*WARNING* ", file);
			break;
		}
	}
	else
	{
		switch(type)
		{
		case LOGMSGTYPE_INFO:
			ansicode_printf(
				&(struct ANSICode){
					.fg = ANSICODE_FG_GREEN,
					.bold = 1,
				},
				"*INFO* "
			);
			break;
		case LOGMSGTYPE_ERROR:
			ansicode_printf(
				&(struct ANSICode){
					.fg = ANSICODE_FG_RED,
					.bold = 1,
				},
				"*ERROR* "
			);
			break;
		case LOGMSGTYPE_WARNING:
			ansicode_printf(
				&(struct ANSICode){
					.fg = ANSICODE_FG_YELLOW,
					.bold = 1,
				},
				"*WARNING* "
			);
			break;
		}
	}

	vfprintf(file, fmt, vlist);
	fputs("\n", file);
	va_end(vlist);

	if(type == LOGMSGTYPE_ERROR)
	{
		if(errorcallback)
		{
			errorcallback(errorudata);
		}
	}
}

void log_seterrorhandler(LogErrorCallback callback, void* udata)
{
	errorcallback = callback;
	errorudata = udata;
}

void log_assert_(
	const char* expression,
	int result,
	int line,
	const char* func,
	const char* fmt,
	...
)
{
	assert(expression);
	assert(line > 0);
	assert(func);
	assert(fmt);

	va_list vlist;
	va_start(vlist, fmt);

	if(!result)
	{
		//TODO: Improve and colorize more of the assertion output
		ansicode_printf(
			&(struct ANSICode){
				.fg = ANSICODE_FG_YELLOW
			},
			"*DEBUG* "
		);
		printf("Assertion failed ('%s': ", expression);
		vprintf(fmt, vlist);
		printf(") in <%s> at [%i]\n", func, line);
		abort();
	}

	va_end(vlist);
}

