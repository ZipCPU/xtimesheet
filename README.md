# XTimesheet

After keeping track of the hours I spent on a project by hand for some time,
I quickly realized that a computerized timesheet application could be just
as good, perhaps even more reliable, and even easier to use than my notes.

My particular problem comes from the fact that I work at home.  Various family
memory thus believe they can come and interrupt my work at any time.  It just
never felt right to bill the times that were interrupted to a customer, and it
was *very* inconvenient to write the start and stop times down.

This application thus helps me with that.

# Data Format

xtimesheet requires a timesheet text file to start.  The first line in that
file should start with "Project: ".  The rest of the line identifies the 
project that file is tracking.  The second line in the file starts with "Rate: "
and outlines the hourly rate used for the project.  After that, lines in the
file are arranged by date and time worked: YYYY/MM/DD HHMMSS -- HHMMSS, followed
by the total time in each time interval in tenths of hours.

This format is quite robust, however.  Comments may be placed on the ends of 
lines, or even anywhere in the file.  XTimesheet just looks for lines that look
like clock lines and ignores all other lines.

# Status

I've now used this for some time, and I like it.  However, the program has a
HUGE flaw in it.  It cannot generate a timesheet file from scratch, nor can it
query the user for which timesheet to open if no timesheet is given on the
command line.  Hence, to use it, you'll need to build a text file with the
first two lines (Project and Rate) already within it, and only then start
XTimesheet on that file.

# License

Gisselquist Technology, LLC, is pleased to provide you with this project
under the GPLv3 license.  Should you wish to fund and thereby direct any
further development of this project, please contact us.
