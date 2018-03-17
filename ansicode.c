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

#include "ansicode.h"
#include "log.h"
#include <stdio.h>

#define ANSICODE_BEGIN		"\x1b["
#define ANSICODE_END 		"m"
#define ANSICODE_RESET 		"0"
#define ANSICODE_SEPARATOR 	";"

//NOTE: Not useful; makes text bold. Dark is now used as default
/*
#define ANSICODE_DARK 		"m"
#define ANSICODE_BRIGHT 	";1m"
*/

//Attributes
#define ANSICODE_BOLD 		"1"
#define ANSICODE_ITALIC		"3"
#define ANSICODE_UNDERLINE	"4"

//Colors
const struct ANSICodeFg* const ANSICODE_FG_BLACK	= &(struct ANSICodeFg){"30"};
const struct ANSICodeFg* const ANSICODE_FG_RED 		= &(struct ANSICodeFg){"31"};
const struct ANSICodeFg* const ANSICODE_FG_GREEN 	= &(struct ANSICodeFg){"32"};
const struct ANSICodeFg* const ANSICODE_FG_YELLOW 	= &(struct ANSICodeFg){"33"};
const struct ANSICodeFg* const ANSICODE_FG_BLUE 	= &(struct ANSICodeFg){"34"};
const struct ANSICodeFg* const ANSICODE_FG_MAGENTA 	= &(struct ANSICodeFg){"35"};
const struct ANSICodeFg* const ANSICODE_FG_CYAN 	= &(struct ANSICodeFg){"36"};
const struct ANSICodeFg* const ANSICODE_FG_WHITE 	= &(struct ANSICodeFg){"37"};
const struct ANSICodeFg* const ANSICODE_FG_DEFAULT 	= &(struct ANSICodeFg){"39"};

const struct ANSICodeBg* const ANSICODE_BG_BLACK 	= &(struct ANSICodeBg){"40"};
const struct ANSICodeBg* const ANSICODE_BG_RED 		= &(struct ANSICodeBg){"41"};
const struct ANSICodeBg* const ANSICODE_BG_GREEN 	= &(struct ANSICodeBg){"42"};
const struct ANSICodeBg* const ANSICODE_BG_YELLOW 	= &(struct ANSICodeBg){"43"};
const struct ANSICodeBg* const ANSICODE_BG_BLUE 	= &(struct ANSICodeBg){"44"};
const struct ANSICodeBg* const ANSICODE_BG_MAGENTA 	= &(struct ANSICodeBg){"45"};
const struct ANSICodeBg* const ANSICODE_BG_CYAN 	= &(struct ANSICodeBg){"46"};
const struct ANSICodeBg* const ANSICODE_BG_WHITE 	= &(struct ANSICodeBg){"47"};
const struct ANSICodeBg* const ANSICODE_BG_DEFAULT 	= &(struct ANSICodeBg){"49"};

static int ansicode_checkfg(struct ANSICode* self)
{
	if(self->fg == ANSICODE_FG_BLACK ||
		self->fg == ANSICODE_FG_RED ||
		self->fg == ANSICODE_FG_GREEN ||
		self->fg == ANSICODE_FG_YELLOW ||
		self->fg == ANSICODE_FG_BLUE ||
		self->fg == ANSICODE_FG_MAGENTA ||
		self->fg == ANSICODE_FG_CYAN ||
		self->fg == ANSICODE_FG_WHITE ||
		self->fg == ANSICODE_FG_DEFAULT
	)
	{
		return 1;
	}

	return 0;
}

static int ansicode_checkbg(struct ANSICode* self)
{
	if(self->bg == ANSICODE_BG_BLACK ||
		self->bg == ANSICODE_BG_RED ||
		self->bg == ANSICODE_BG_GREEN ||
		self->bg == ANSICODE_BG_YELLOW ||
		self->bg == ANSICODE_BG_BLUE ||
		self->bg == ANSICODE_BG_MAGENTA ||
		self->bg == ANSICODE_BG_CYAN ||
		self->bg == ANSICODE_BG_WHITE ||
		self->bg == ANSICODE_BG_DEFAULT
	)
	{
		return 1;
	}

	return 0;
}

void ansicode_printf(struct ANSICode* self, const char* fmt, ...)
{
	log_assert(self, "is NULL");
	log_assert(fmt, "is NULL");

	//If NULL or unknown use default
	if(!self->fg)
	{
		self->fg = ANSICODE_FG_DEFAULT;
	}
	if(!self->bg)
	{
		self->bg = ANSICODE_BG_DEFAULT;
	}

	log_assert(
		ansicode_checkfg(self), 
		"'%s' is not a valid foreground", 
		self->fg->name
	);
	log_assert(
		ansicode_checkbg(self), 
		"'%s' is not a valid background", 
		self->bg->name
	);

	va_list vlist;
	va_start(vlist, fmt);

	printf(
		"%s%s%s%s", 
		ANSICODE_BEGIN, 
		self->fg->name, 
		ANSICODE_SEPARATOR, 
		self->bg->name
	);

	if(self->bold)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_BOLD);
	}

	if(self->italic)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_ITALIC);
	}

	if(self->underline)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_UNDERLINE);
	}

	printf("%s", ANSICODE_END);
	vprintf(fmt, vlist);
	printf("%s%s%s", ANSICODE_BEGIN, ANSICODE_RESET, ANSICODE_END);

	va_end(vlist);
}

void ansicode_fprintf(struct ANSICode* self, FILE* file, const char* fmt, ...)
{
	log_assert(self, "is NULL");
	log_assert(file, "is NULL");
	log_assert(fmt, "is NULL");

	//If NULL or unknown use default
	if(!self->fg)
	{
		self->fg = ANSICODE_FG_DEFAULT;
	}
	if(!self->bg)
	{
		self->bg = ANSICODE_BG_DEFAULT;
	}

	log_assert(
		ansicode_checkfg(self), 
		"'%s' is not a valid foreground", 
		self->fg->name
	);
	log_assert(
		ansicode_checkbg(self), 
		"'%s' is not a valid background", 
		self->bg->name
	);

	va_list vlist;
	va_start(vlist, fmt);

	fprintf(
		file,
		"%s%s%s%s", 
		ANSICODE_BEGIN, 
		self->fg->name, 
		ANSICODE_SEPARATOR, 
		self->bg->name
	);

	if(self->bold)
	{
		fprintf(file, "%s%s", ANSICODE_SEPARATOR, ANSICODE_BOLD);
	}

	if(self->italic)
	{
		fprintf(file, "%s%s", ANSICODE_SEPARATOR, ANSICODE_ITALIC);
	}

	if(self->underline)
	{
		fprintf(file, "%s%s", ANSICODE_SEPARATOR, ANSICODE_UNDERLINE);
	}

	fprintf(file, "%s", ANSICODE_END);
	vfprintf(file, fmt, vlist);
	fprintf(file, "%s%s%s", ANSICODE_BEGIN, ANSICODE_RESET, ANSICODE_END);

	va_end(vlist);
}

//Identical to ansicode_printf basically
void ansicode_vprintf(struct ANSICode* self, const char* fmt, va_list vlist)
{
	log_assert(self, "is NULL");
	log_assert(fmt, "is NULL");

	//If NULL or unknown use default
	if(!self->fg)
	{
		self->fg = ANSICODE_FG_DEFAULT;
	}
	if(!self->bg)
	{
		self->bg = ANSICODE_BG_DEFAULT;
	}

	log_assert(
		ansicode_checkfg(self), 
		"'%s' is not a valid foreground", 
		self->fg->name
	);
	log_assert(
		ansicode_checkbg(self), 
		"'%s' is not a valid background", 
		self->bg->name
	);

	printf(
		"%s%s%s%s", 
		ANSICODE_BEGIN, 
		self->fg->name, 
		ANSICODE_SEPARATOR, 
		self->bg->name
	);

	if(self->bold)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_BOLD);
	}

	if(self->italic)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_ITALIC);
	}

	if(self->underline)
	{
		printf("%s%s", ANSICODE_SEPARATOR, ANSICODE_UNDERLINE);
	}

	printf("%s", ANSICODE_END);
	vprintf(fmt, vlist);
	printf("%s%s%s", ANSICODE_BEGIN, ANSICODE_RESET, ANSICODE_END);
}


