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

#ifndef STR_H
#define STR_H

#include <stddef.h>

struct Str
{ 
	size_t len;
	char* data;
};

struct Str* str_ctor(struct Str* self, const char* str);
struct Str* str_ctorfmt(struct Str* self, const char* fmt, ...) 
	__attribute__((format (printf, 2, 3)));
void str_insert(struct Str* self, size_t index, const char* str);
void str_insertfmt(struct Str* self, size_t index, const char* fmt, ...)
	__attribute__((format (printf, 3, 4)));
void str_append(struct Str* self, const char* str);
void str_appendfmt(struct Str* self, const char* fmt, ...)
	__attribute__((format (printf, 2, 3)));
void str_prepend(struct Str* self, const char* str);
void str_prependfmt(struct Str* self, const char* fmt, ...)
	__attribute__((format (printf, 2, 3)));
void str_dtor(struct Str* self);

#endif
