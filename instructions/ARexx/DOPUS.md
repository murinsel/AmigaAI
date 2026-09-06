# Directory Opus 5 / Magellan II

Directory Opus Magellan is the Workbench replacement used by CaffeineOS. It is
usually the main interface on the machine, so most desktop work goes through it.

Path: see Programs.md for where it is installed on this system.
ARexx port: DOPUS.1

The port is DOPUS.x where x is the invocation count. The first instance, which
is the usual one, is DOPUS.1. Confirm with list_ports before sending commands.

## Before sending commands

Scripts must include OPTIONS RESULTS near the top, otherwise RESULT stays empty.

Return values arrive in RESULT. Two exceptions return data in DOPUSRC instead.
Error codes arrive in RC.

## Two command families

Commands starting with dopus act on the program as a whole: screen, desktop,
icons, internal command registration, requesters.

Commands starting with lister act on a single lister window. Most take a handle
as their first argument, obtained from 'dopus getlister' or 'lister new'.

## dopus commands

ADDAPPICON, ADDTRAP, BACK, CHECKDESKTOP, CLEAR, COMMAND, DESKTOPPOPUP,
ERROR, FRONT, GETDESKTOP, GETFILETYPE, GETSTRING, MATCHDESKTOP, PROGRESS,
QUERY, QUERY BACKGROUND, QUERY FONT, QUERY PALETTE, QUERY PENS,
QUERY SOUND, READ, REFRESH, REFRESH ALL, REFRESH BACKGROUND, REFRESH ICONS,
REFRESH LISTER, REMAPPICON, REMTRAP, REQUEST, SCREEN, SCRIPT, SEND, SET,
SET BACKGROUND, SET FONT, SET PALETTE, SET PENS, SET SOUND, SETAPPICON,
VERSION

## lister commands

ADD, ADDSTEM, CLEAR, CLEAR VALUE, CLEARCACHES, CLOSE, COPY, EMPTY,
FINDCACHE, GETSTRING, ICONIFY, NEW, QUERY, QUERY ABORT, QUERY ACTIVE,
QUERY ALL, QUERY BUSY, QUERY CASE, QUERY COMMENTLENGTH, QUERY DEST,
QUERY DIRS, QUERY DISPLAY, QUERY ENTRIES, QUERY ENTRY, QUERY FILES,
QUERY FIRSTSEL, QUERY FLAGS, QUERY HANDLER, QUERY HEADER, QUERY HIDE,
QUERY LABEL, QUERY LOCK, QUERY MODE, QUERY NAMELENGTH, QUERY NUMDIRS,
QUERY NUMENTRIES, QUERY NUMFILES, QUERY NUMSELDIRS, QUERY NUMSELENTRIES,
QUERY NUMSELFILES, QUERY PATH, QUERY POSITION, QUERY PROC, QUERY SELDIRS,
QUERY SELENTRIES, QUERY SELFILES, QUERY SEPARATE, QUERY SHOW, QUERY SORT,
QUERY SOURCE, QUERY TITLE, QUERY TOOLBAR, QUERY VALUE, QUERY VISIBLE,
QUERY WINDOW, READ, REFRESH, RELOAD, REMOVE, REQUEST, SELECT, SET,
SET BUSY, SET CASE, SET COMMENTLENGTH, SET DEST, SET DISPLAY, SET FIELD,
SET FLAGS, SET HANDLER, SET HEADER, SET HIDE, SET LABEL, SET LOCK,
SET MODE, SET NAMELENGTH, SET NEWPROGRESS, SET OFF, SET PATH, SET POSITION,
SET PROGRESS, SET SEPARATE, SET SHOW, SET SORT, SET SOURCE, SET TITLE,
SET TOOLBAR, SET VALUE, SET VISIBLE, WAIT

## Getting the detail

Do not guess arguments. Use arexx_help with program DOPUS and the full command
name to get syntax, parameters and return values, for example:

  arexx_help program="DOPUS" command="lister set"
  arexx_help program="DOPUS" command="dopus command"

## Custom handlers

Opus can push events to an external public message port, so a script can react
to clicks, drops and edits inside a lister. That system is documented in the
guide this file was derived from and is not covered here.
