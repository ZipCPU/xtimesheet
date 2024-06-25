////////////////////////////////////////////////////////////////////////////////
//
// Filename:	sw/thisweek.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
// {{{
// Purpose:	To count up the hours put into a particular project over the
//		course of the current week.
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
static const char *cpyright = "(C) 2022 Gisselquist Technology, LLC: " __FILE__;
#include <stdio.h>
#include <time.h>

#include "timecard.h"

#define	MXLEN	512

int main(int argc, char **argv) {
	TIMECARD	tc;
	time_t		midnight = 0, window_begin = 0, window_end = 0,
			acc = 0;
	char	line[MXLEN], *home;

	{
		time_t	when;
		struct	tm	datev;

		when = tc.get_midnight(time(NULL));
		localtime_r(&when, &datev);
		when -= datev.tm_wday * 24 * 3600; // Beginning of week
		window_begin = when;
		window_end = window_begin + 7 * 24 * 3600;

		localtime_r(&when, &datev);
		printf("Week begins: %04d/%02d/%02d\n",
			datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);
	}
		
	home = getenv("HOME");
	if (!home) {
		fprintf(stderr, "No $HOME environment variable defined\n");
		exit(EXIT_FAILURE);
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
						// printf("%04d/%02d/%02d is outside of our window\n", datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);
					}
				}
			}

			fclose(fp);
			// }}}
		} else if (NULL != home && argv[argn][0] == '%') {
			// {{{
			FILE	*fcfg, *fp;
			char	cfg_task[128], cfg_file[128];

			strcpy(cfg_file, home);
			strcat(cfg_file, "/.xtimesheet");
			fcfg = fopen(cfg_file, "r");
			if (fcfg) {
				while(fgets(cfg_task, sizeof(cfg_task), fcfg)) {
				// acc += tc.hours_between(cfg_task, window_begin, window_end);
				while(isspace(cfg_task[strlen(cfg_task)-1]))
					cfg_task[strlen(cfg_task)-1] = '\0';
				fp = fopen(cfg_task, "r");
				if (NULL != fp) {
					while(fgets(line, MXLEN, fp)) {
						time_t	lnstart, lnstop;
						if (tc.parse(line, lnstart, lnstop)) {
							if (lnstart > 24*3600) {
								midnight = tc.get_midnight(lnstart);
							} else {
								lnstart += midnight;
								lnstop  += midnight;
							}

							if ((lnstart >= window_begin)&&(lnstop < window_end)) {
								assert(lnstop >= lnstart);
								acc += lnstop - lnstart;
							} else {
								struct tm	datev;
								localtime_r(&lnstop, &datev);
							}
						}
					}

					fclose(fp);
				}
				} fclose(fcfg);
			}
			// }}}
		} else if (tc.digitstr(argv[argn],4)) { // Specify the week
			// {{{
			time_t	when;
			struct	tm	datev;

			when = tc.get_midnight(argv[argn]);
			localtime_r(&when, &datev);
			when -= datev.tm_wday * 24 * 3600; // Beginning of week
			if (window_begin != when)
				acc = 0;
			window_begin = when;
			window_end = window_begin + 7 * 24 * 3600;

			localtime_r(&when, &datev);
			printf("Week begins: %04d/%02d/%02d\n",
			datev.tm_year+1900, datev.tm_mon+1, datev.tm_mday);
			// }}}
		} else
			fprintf(stderr, "Unknown arg, %s\n", argv[argn]);
	}

	long	hour_tenths = (long)(acc + 180)/60/6;
	printf("%.1f Hours\n", (double)hour_tenths/10.0);
}

