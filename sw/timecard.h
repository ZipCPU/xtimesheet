////////////////////////////////////////////////////////////////////////////////
//
// Filename:	sw/timecard.h
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
// {{{
// Purpose:	To define the data types used to store timesheet information
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2024, Gisselquist Technology, LLC
// {{{
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory, run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
// }}}
// License:	GPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/gpl.html
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#ifndef	TIMECARD_H
#define	TIMECARD_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

extern long	timezone; // seconds west of UTC

class	TIMECARD {
public:
	TIMECARD(void) {}

	bool	istimecard(const char *fname);
	bool	parse(const char *line, time_t &lnstart, time_t &lnstop);
	void	log(const char *fname, time_t t_start, time_t t_stop);
	void	note_start(const char *fname, time_t t_start);
	time_t	get_midnight(const char *ln);
	time_t	get_midnight(time_t when);
	time_t	get_month(time_t when);	// Get first of month
	bool	digitstr(const char *str, int len);
	static	char	*trimtask(char *task_name);
};

#endif // TIMECARD_H
