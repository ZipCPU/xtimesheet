////////////////////////////////////////////////////////////////////////////////
//
// Filename:	sw/thismonth.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
// {{{
// Purpose:	To quickly count up the number of hours put into this particular
//		project over the course of the month, given a set of timesheet
//	files containing work windows.
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
__attribute__((unused))
static const char *cpyright = "(C) 2023 Gisselquist Technology, LLC: " __FILE__;
#include <stdio.h>
#include <time.h>

#include "timecard.h"

#define	MXLEN	512

void	usage(void) {
	fprintf(stderr, "Usage: thismonth [month|[startdate enddate]] timesheet.txt [*]\n");
}

int main(int argc, char **argv) {
	TIMECARD	tc;
	time_t		midnight = 0, window_begin = 0, window_end = 0,
			acc = 0;
	char	line[MXLEN];

	if (argc <= 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	{
		time_t	when;
		struct	tm	datev;

		when = tc.get_midnight(time(NULL));
		localtime_r(&when, &datev);
		when -= (datev.tm_mday-1) * 24 * 3600; // Beginning of month
		window_begin = when;

		datev.tm_mday = 1;
		datev.tm_mon += 1;
		if (datev.tm_mon >= 12) {
			datev.tm_year += 1;
			datev.tm_mon   = 0;
		}
		window_end = mktime(&datev);
	}


	for(int argn=1; argn<argc; argn++) {
		if (access(argv[argn], R_OK)==0) {
			// {{{
			FILE	*fp = fopen(argv[argn], "r");
			while(fgets(line, MXLEN, fp)) {
				time_t	lnstart, lnstop;
				if (tc.parse(line, lnstart, lnstop)) {
					if (lnstart > 24*3600) {
						midnight = tc.get_midnight(lnstart);
					} else {
						lnstart += midnight;
						lnstop  += midnight;
					}

					if ((lnstart > window_begin)&&(lnstop < window_end)) {
						assert(lnstop >= lnstart);
						acc += lnstop - lnstart;
					} else {
						struct tm	datev;
						localtime_r(&lnstop, &datev);
//						printf("%04d/%02d/%02d is outside of our window\n", datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);
					}
				}
			}

			fclose(fp);
			// }}}
		} else if (argv[argn][0] == '%') {
			// {{{
			FILE	*fcfg;
			char	*home, cfg_file[128], task_line[128],
				*cfg_task;

			home = getenv("HOME");
			strcpy(cfg_file, home);
			strcat(cfg_file, "/.xtimesheet");
			if (home && NULL != (fcfg = fopen(cfg_file, "r"))) {
				while(fgets(task_line, sizeof(task_line), fcfg)) {
					FILE	*fp;
					cfg_task = strtok(task_line, " \r\n");
					if(NULL != (fp=fopen(cfg_task, "r"))) {
						while(fgets(line, MXLEN, fp)) {
							time_t	lnstart, lnstop;
							if (tc.parse(line, lnstart, lnstop)) {
								if (lnstart > 24*3600) {
									midnight = tc.get_midnight(lnstart);
								} else {
									lnstart += midnight;
									lnstop  += midnight;
								}

								if ((lnstart > window_begin)&&(lnstop < window_end)) {
									assert(lnstop >= lnstart);
									acc += lnstop - lnstart;
								} else {
									struct tm	datev;
									localtime_r(&lnstop, &datev);
								}
							}
						} fclose(fp);
					}
				} fclose(fcfg);
			}
			// }}}
		} else if (tc.digitstr(argv[argn],4)) {
			// {{{
			time_t	when;
			struct	tm	datev;
			char	datestr[32];
			strncpy(datestr, argv[argn], sizeof(datestr));
			if ((tc.digitstr(datestr, 6))&&(!tc.digitstr(datestr, 8)))
				strcat(datestr, "01");

			printf("Making time from %s\n", datestr);

			when = tc.get_midnight(datestr);
			localtime_r(&when, &datev);
			when -= (datev.tm_mday-1) * 24 * 3600; // Beginning of month
			window_begin = when;

			datev.tm_mday = 1;
			datev.tm_mon += 1;
			if (datev.tm_mon >= 12) {
				datev.tm_year += 1;
				datev.tm_mon   = 0;
			}
			window_end = mktime(&datev);

			localtime_r(&window_begin, &datev);
			printf("Month begins: %04d/%02d/%02d\n",
			datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);
			localtime_r(&window_end, &datev);
			printf("Month ends: %04d/%02d/%02d\n",
			datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);

			if (acc != 0)
				fprintf(stderr, "WARNING: times updated after hours already calculated\n");
			// }}}
		}
	}

	long	hour_tenths = (long)(acc + 180)/60/6;
	printf("%.1f Hours\n", (double)hour_tenths/10.0);
}

