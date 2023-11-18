////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	timecard.cpp
// {{{
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	This file supports the non-GUI portions of reading timesheets.
//		Indeed, the goal of this file is to support the reading and
//	comprehension of the timesheets themselves.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2017-2022, Gisselquist Technology, LLC
// {{{
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
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
__attribute__((unused))
static const char *cpyright = "(C) 2022 Gisselquist Technology, LLC: " __FILE__;

#include "timecard.h"

const bool	DEBUG = false;

bool	TIMECARD::digitstr(const char *str, int len) {
	for(int i=0; i<len; i++)
		if (!isdigit(str[i]))
			return false;
	return true;
}

time_t	TIMECARD::get_midnight(const char *ln) {
	struct	tm	datev;
	unsigned	v;
	const	char	*ptr = ln;

	memset(&datev, 0, sizeof(datev));

	v =          (ptr[0]-'0');
	v = v * 10 + (ptr[1]-'0');
	v = v * 10 + (ptr[2]-'0');
	v = v * 10 + (ptr[3]-'0');
	datev.tm_year = v - 1900;

	ptr += 4; if (*ptr == '/') ptr++;

	v  = (ptr[0]-'0')*10;
	v += (ptr[1]-'0');
	datev.tm_mon = v - 1;

	ptr += 2; if (*ptr == '/') ptr++;

	v  = (ptr[0]-'0')*10;
	v += (ptr[1]-'0');
	datev.tm_mday = v;

	return mktime(&datev);
}

time_t	TIMECARD::get_midnight(time_t when) {
	struct	tm	datev;
	localtime_r(&when, &datev);

	/*
	printf("1. LOCALTIME: %04d/%02d/%02d %02d:%02d:%02d --> %ld (%ld)\n",
		datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday,
		datev.tm_hour, datev.tm_min, datev.tm_sec,
		mktime(&datev), mktime(&datev)-when);
	*/
	datev.tm_hour = 0;
	datev.tm_min = 0;
	datev.tm_sec = 0;
	datev.tm_isdst = 0;

	/*
	printf("2. LOCALTIME: %04d/%02d/%02d %02d:%02d:%02d --> %ld (%ld)\n",
		datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday,
		datev.tm_hour, datev.tm_min, datev.tm_sec,
		mktime(&datev), mktime(&datev)-when);
	*/

	return mktime(&datev);
}

bool	TIMECARD::parse(const char *line, time_t &lnstart, time_t &lnstop) {
	if ((digitstr(line, 4))&&(line[4] == '/')
		&&(digitstr(&line[5], 2))&&(line[7] == '/')
		&&(digitstr(&line[8], 2))&&(isspace(line[10]))
		&&(digitstr(&line[11], 6))
		// &&(strncmp(&line[17], " -- ", 4)==0)
		&&(isspace(line[17]))
		&&(line[18] == '-')
		&&(line[19] == '-')
		&&(isspace(line[20]))
		&&(digitstr(&line[21], 6))
		) {
		// YYYY/MM/DD HHMMSS -- HHMMSS\n
		if (DEBUG)
			printf("MATCH YY/MM/DD HHMMSS: %s\n", line);
		time_t		midnight;
		midnight = get_midnight(line);

		// printf("MATCH-1: %s\n", line);

		unsigned sstart, sstop;
		sstart = (line[11]-'0')*10+line[12]-'0';
		sstart *= 60;
		sstart+= (line[13]-'0')*10+line[14]-'0';
		sstart *= 60;
		sstart+= (line[15]-'0')*10+line[16]-'0';

		sstop = (line[21]-'0')*10+line[22]-'0';
		sstop *= 60;
		sstop+= (line[23]-'0')*10+line[24]-'0';
		sstop *= 60;
		sstop+= (line[25]-'0')*10+line[26]-'0';

		lnstart = midnight + sstart;
		lnstop  = midnight + sstop;

		if (sstop < sstart)
			printf("FAIL: %s\n", line);
		assert(sstop >= sstart);
		if (false) {
			struct	tm	datev;
			localtime_r(&midnight, &datev);
			printf("%04d/%02d/%02d %02d:%02d:%02d : ",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
			localtime_r(&lnstart, &datev);
			printf("%04d/%02d/%02d %02d:%02d:%02d --",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
			localtime_r(&lnstop, &datev);
			printf(" %04d/%02d/%02d %02d:%02d:%02d\n",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
		}
		return true;
	} else if ((digitstr(line, 14))
		&&(digitstr(&line[18], 6))
		&&(isspace(line[14]))
		&&(line[15] == '-')
		&&(line[16] == '-')
		&&(isspace(line[27]))) {
		// YYYYMMDDHHMMSS -- HHMMSS\n
		time_t		midnight;
		midnight = get_midnight(line);

		if (DEBUG)
			printf("MATCH YYMMDD HHMMSS: %s\n", line);

		unsigned sstart, sstop;
		sstart = (line[ 8]-'0')*10+line[ 9]-'0';	// Hours
		sstart *= 60;
		sstart+= (line[10]-'0')*10+line[11]-'0';	// Minutes
		sstart *= 60;
		sstart+= (line[12]-'0')*10+line[13]-'0';	// Seconds

		sstop = (line[18]-'0')*10+line[19]-'0'; // Hours
		sstop *= 60;
		sstop+= (line[20]-'0')*10+line[21]-'0';	// Minutes
		sstop *= 60;
		sstop+= (line[22]-'0')*10+line[23]-'0';	// Seconds

		lnstart = midnight + sstart;
		lnstop  = midnight + sstop;
		assert(sstop >= sstart);

		if (false) {
			struct	tm	datev;
			localtime_r(&midnight, &datev);
			printf("%04d/%02d/%02d %02d:%02d:%02d : ",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
			localtime_r(&lnstart, &datev);
			printf("%04d/%02d/%02d %02d:%02d:%02d --",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
			localtime_r(&lnstop, &datev);
			printf(" %04d/%02d/%02d %02d:%02d:%02d\n",
				datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday, datev.tm_hour, datev.tm_min, datev.tm_sec);
		}

		return true;
	} else if ((line[0]=='2')&&(line[1]=='0')&&(digitstr(line, 8))) {
		// YYYYMMDD
		time_t		midnight;

		if (DEBUG)
			printf("MATCH YR: %s\n", line);

		midnight = get_midnight(line);
		lnstart = midnight;
		lnstop  = midnight;

		return true;
	} else if ((line[0] == '\t')&&(digitstr(&line[1], 4))
		&&(digitstr(&line[9], 4))
		&&(isspace(line[5]))
		&&(line[6] == '-')
		&&(line[7] == '-')
		&&(isspace(line[8]))) {

		if (DEBUG)
			printf("MATCH HHMM: %s\n", line);
		// \tHHMM -- HHMM
		unsigned sstart, sstop;
		sstart = (line[1]-'0')*10+line[2]-'0';
		sstart *= 60;
		sstart += (line[3]-'0')*10+line[4]-'0';
		sstart *= 60; // No seconds

		sstop = (line[9]-'0')*10+line[10]-'0';	// Hours
		sstop *= 60;
		sstop += (line[11]-'0')*10+line[12]-'0'; // Minutes
		sstop *= 60; // No seconds

		assert(sstop > sstart);

		lnstart = sstart;
		lnstop  = sstop;
		return true;
	} else if (DEBUG) {
		printf("No match: %s\n", line);
		if (line[0] != '\t') printf("\tNot tab\n");
		else if (!digitstr(&line[1],4))	printf("\tMissing first digitstr\n");
		else if (!digitstr(&line[9],4))	printf("\tMissing second digitstr\n");
		else if ((isspace(line[5]))
			&&(line[6] == '-')
			&&(line[7] == '-')
			&&(isspace(line[8]))) printf("\tNo match to dashes\n");
	}

	return false;
}

void	TIMECARD::log(const char *fname, time_t t_start, time_t t_stop) {
	FILE	*fp;
	struct	tm	tp_start, tp_stop;
	double	hrs;

	// Don't log anything less than a second of work
	if (t_start == t_stop)
		return;

	assert(get_midnight(t_start) == get_midnight(t_stop));

	hrs = (t_stop-t_start) / 3600.0;

	localtime_r(&t_start, &tp_start);
	localtime_r(&t_stop, &tp_stop);
	fp = fopen(fname, "a");

	/*
	fprintf(fp, "%04d%02d%02d%02d%02d%02d -- %02d%02d%02d (%4.1f)\n",
		tp_start.tm_year+1900, tp_start.tm_mon+1,
		tp_start.tm_mday,
		tp_start.tm_hour, tp_start.tm_min, tp_start.tm_sec,
		tp_stop.tm_hour, tp_stop.tm_min, tp_stop.tm_sec, hrs);
	*/

	fprintf(fp, "%04d/%02d/%02d %02d%02d%02d -- %02d%02d%02d (%4.1f)\n",
		tp_start.tm_year+1900, tp_start.tm_mon+1,
		tp_start.tm_mday,
		tp_start.tm_hour, tp_start.tm_min, tp_start.tm_sec,
		tp_stop.tm_hour, tp_stop.tm_min, tp_stop.tm_sec, hrs);

	fclose(fp);
}

void	TIMECARD::note_start(const char *fname, time_t t_start) {
	FILE	*fp;
	struct	tm	tp_start;

	localtime_r(&t_start, &tp_start);
	fp = fopen(fname, "a");

	fprintf(fp, "%04d/%02d/%02d %02d%02d%02d -- Start\n",
		tp_start.tm_year+1900, tp_start.tm_mon+1,
		tp_start.tm_mday,
		tp_start.tm_hour, tp_start.tm_min, tp_start.tm_sec);

	fclose(fp);
}


