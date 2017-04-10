////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	mkglade.cpp
//
// Project:	Xtimesheet, a very simple text-based timesheet tracking program
//
// Purpose:	Turns a glade (xml) file into a C-source code file that can then
//		be compiled into an application, so the application doesn't need
//	to search to find its resource files, but rather is an independent and
//	stand alone application.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017, Gisselquist Technology, LLC
//
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
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

//
// putout
//
// Add escapes to any characters that need to be escaped within C strings.
//
void	putout(FILE *fp, const char *str) {
	const char	*ptr = str;

	while(*ptr) {
		switch(*ptr) {
		case '\'':	fprintf(fp, "\\\'"); break;
		case '\"':	fprintf(fp, "\\\""); break;
		default:	fprintf(fp, "%c", *ptr);	break;
		} ptr++;
	} fprintf(fp, "\\\n");
}

//
// main
//
int main(int argc, char **argv) {
	FILE	*fpin, *fpout;
	char	buf[4096], glade_caps[128];
	char	*glade_xml, *glade_obj;

	if (argc != 3) {
		printf("Usage: mkglade <glade-file.glade> <source>\n");
		printf("\n");
		printf("\tTurns the glade file into two files: source.cpp,\n");
		printf("\tand source.h, containing the string source[] and\n");
		printf("\tdepending upon the #def SOURCE.\n");
		exit(-2);
	}

	glade_xml = argv[1]; assert(access(glade_xml, R_OK)==0);
	glade_obj = argv[2]; strcpy(glade_caps, glade_obj);

	fpin = fopen(glade_xml,"r");
	strcpy(buf, glade_obj); strcat(buf, ".cpp");
	fpout = fopen(buf, "w");

	assert(fpin);
	assert(fpout);

	for(char *ptr = glade_caps; *ptr; ptr++) {
		*ptr = toupper(*ptr);
	}

	fprintf(fpout, "#include \"%s.h\"\n", glade_obj);
	fprintf(fpout, "\n");
	fprintf(fpout, "\n");
	fprintf(fpout, "const char %s[] = \"\\\n", glade_obj);
	while(fgets(buf, sizeof(buf), fpin)) {
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';
		if (buf[strlen(buf)-1] == '\r')
			buf[strlen(buf)-1] = '\0';
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';
		putout(fpout, buf);
		// fprintf(fpout, "%s\\\n", buf);
	}
	fprintf(fpout, "\";\n");
	fclose(fpout);
	fclose(fpin);

	strcpy(buf, glade_obj); strcat(buf, ".h");
	if (access(buf, R_OK)!=0) { // If the file does not exist, then ...
		// build it.
		fpout = fopen(buf, "w");
		fprintf(fpout, "#ifndef %s_H\n", glade_caps);
		fprintf(fpout, "#define %s_H\n", glade_caps);
		fprintf(fpout, "\n");
		fprintf(fpout, "extern const char %s[];\n", glade_obj);
		fprintf(fpout, "\n");
		fprintf(fpout, "#endif\n");
		fclose(fpout);
	}
}

