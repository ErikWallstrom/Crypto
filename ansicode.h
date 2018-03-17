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

#ifndef ANSICODE_H
#define ANSICODE_H

#include <stdarg.h>
#include <stdio.h>

struct ANSICodeFg 
{
	const char* name;
};

struct ANSICodeBg
{
	const char* name;
};

struct ANSICode
{
	const struct ANSICodeFg* fg;
	const struct ANSICodeBg* bg;
	int bold;
	int italic;
	int underline;
};

extern const struct ANSICodeFg* const ANSICODE_FG_BLACK;
extern const struct ANSICodeFg* const ANSICODE_FG_RED;
extern const struct ANSICodeFg* const ANSICODE_FG_GREEN;
extern const struct ANSICodeFg* const ANSICODE_FG_YELLOW;
extern const struct ANSICodeFg* const ANSICODE_FG_BLUE;
extern const struct ANSICodeFg* const ANSICODE_FG_MAGENTA;
extern const struct ANSICodeFg* const ANSICODE_FG_CYAN;
extern const struct ANSICodeFg* const ANSICODE_FG_WHITE;
extern const struct ANSICodeFg* const ANSICODE_FG_DEFAULT;

extern const struct ANSICodeBg* const ANSICODE_BG_BLACK;
extern const struct ANSICodeBg* const ANSICODE_BG_RED;
extern const struct ANSICodeBg* const ANSICODE_BG_GREEN;
extern const struct ANSICodeBg* const ANSICODE_BG_YELLOW;
extern const struct ANSICodeBg* const ANSICODE_BG_BLUE;
extern const struct ANSICodeBg* const ANSICODE_BG_MAGENTA;
extern const struct ANSICodeBg* const ANSICODE_BG_CYAN;
extern const struct ANSICodeBg* const ANSICODE_BG_WHITE;
extern const struct ANSICodeBg* const ANSICODE_BG_DEFAULT;

void ansicode_printf(struct ANSICode* self, const char* fmt, ...)
	__attribute__((format(printf, 2, 3)));
void ansicode_fprintf(struct ANSICode* self, FILE* file, const char* fmt, ...)
	__attribute__((format(printf, 3, 4)));
void ansicode_vprintf(struct ANSICode* self, const char* fmt, va_list args)
	__attribute__((format(printf, 2, 0)));

#endif
