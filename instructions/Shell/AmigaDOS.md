# AmigaDOS Shell Commands Reference

Use these commands with the shell_command tool.

CRITICAL: AmigaDOS is NOT Unix. Do NOT use Unix syntax:
- Wildcard: use `#?` NOT `*` (e.g. `LIST SYS:Libs/#?.library`, NOT `*.library`)
- Directory separator: use `/` or `:` NOT `\`
- No `ls`, `grep`, `cat`, `rm` -- use LIST, SEARCH, TYPE, DELETE

CRITICAL: NEVER delete or rename files or directories without asking the user first.
Always confirm before using DELETE, DELETE ALL, or RENAME commands.

## File/Directory Listing
- `LIST [DIR] [PAT] [KEYS/S] [DATES/S] [NODATES/S] [TO] [SUB] [SINCE] [UPTO] [QUICK/S] [BLOCK/S] [NOHEAD/S] [FILES/S] [DIRS/S] [LFORMAT] [ALL/S]`
  - `LIST <path>` -- List files with details (size, date, flags). Default: current dir.
  - `LIST <path> ALL` -- Recursive listing
  - `LIST <path> DIRS` -- Only directories
  - `LIST <path> FILES` -- Only files
  - `LIST <path> PAT=#?.txt` -- Pattern matching (AmigaDOS wildcards)
  - `LIST <path> LFORMAT "%s%s %l"` -- Custom format (%s=path, %s=name, %l=size)
- `DIR [DIR] [OPT/K] [ALL/S] [DIRS/S] [FILES/S] [INTER/S]` -- Brief directory listing

## File Operations
- `COPY [FROM] {TO} [ALL/S] [QUIET/S] [BUF/K/N] [CLONE/S] [DATES/S] [NOPRO/S] [COM/S] [NOREQ/S]`
  - `COPY <src> <dst>` -- Copy file or directory
  - `COPY <src> <dst> ALL` -- Copy recursively
  - `COPY <src> <dst> CLONE` -- Preserve dates/flags
- `DELETE [FILE] [ALL/S] [QUIET/S] [FORCE/S]`
  - `DELETE <path>` -- Delete file or empty directory
  - `DELETE <path> ALL` -- Delete recursively (careful!)
  - `DELETE <path> ALL FORCE` -- Delete even protected files
- `RENAME [FROM] [TO/A] [QUIET/S]` -- Rename/move file
- `MAKEDIR NAME/A` -- Create directory
- `PROTECT [FILE] [FLAGS] [ADD/S] [SUB/S] [ALL/S] [QUIET/S]` -- Set file permissions (hsparwed)

## Finding Files by Name
- `LIST <path> PAT=<pattern> ALL` -- Find files by name recursively
  - Example: `LIST SYS: PAT=#?yam#? ALL` -- Find all files with "yam" in the name
  - Example: `LIST WORK: PAT=#?.png ALL FILES` -- Find all .png files recursively
  - Example: `LIST DH0: PAT=#?.config ALL LFORMAT "%s%s"` -- List full paths only

## File Content
- `TYPE [FROM/A/M] [TO] [OPT] [HEX/S] [NUMBER/S]`
  - `TYPE <file>` -- Display file contents
  - `TYPE <file> NUMBER` -- With line numbers
  - `TYPE <file> HEX` -- Hex dump
- `SEARCH [FROM/M] [SEARCH/A] [ALL/S] [NONUM/S] [QUIET/S] [QUICK/S] [FILE/S] [PATTERN/S] [CASE/S]`
  - `SEARCH <path> <string>` -- Search for text inside files
  - `SEARCH <path> <string> ALL` -- Search recursively
  - `SEARCH <path> <string> FILE` -- Show only filenames
  - `SEARCH <path> <string> CASE` -- Case-sensitive
  NOTE: SEARCH expects the DIRECTORY first, then the search STRING.
  Example: `SEARCH SYS:S "SetPatch"` -- searches all files in SYS:S for "SetPatch"
  Example: `SEARCH WORK: "hello" ALL` -- searches recursively from WORK:
  Wrong: `SEARCH "hello" SYS:` -- this would look for file "hello" containing "SYS:"
- `SORT [FROM/A] [TO/A] [COLSTART/K/N] [CASE/S] [NUMERIC/S]` -- Sort a text file

## System Information
- `VERSION [NAME] [FILE/S] [FULL/S] [RES/S] [UNIT/N]` -- Show version info
  - `VERSION` -- OS version
  - `VERSION <file> FULL` -- Version of a file/library
- `ASSIGN [NAME] [TARGET] [LIST/S] [EXISTS/S] [DISMOUNT/S] [DEFER/S] [PATH/S] [ADD/S] [REMOVE/S] [VOLS/S] [DIRS/S] [DEVICES/S]`
  - `ASSIGN` -- List all assignments
  - `ASSIGN <name>: <path>` -- Create assignment
  - `ASSIGN <name>: <path> ADD` -- Add path to existing assignment
  - `ASSIGN <name>: REMOVE` -- Remove assignment
- `AVAIL [CHIP/S] [FAST/S] [TOTAL/S] [FLUSH/S]` -- Show memory usage
- `STATUS [PROCESS/N] [FULL/S] [TCB/S] [CLI/N] [COM/K] [ALL/S]` -- List running processes
- `INFO [DEVICE] [VOLS/S] [DISKS/S] [GOODONLY/S]` -- Show mounted volumes and free space

## Running Programs
- `RUN [COMMAND/F]` -- Run command in background
  - `RUN >NIL: <command>` -- Background, discard output
- `EXECUTE [FILE/A]` -- Execute a script file
- `WHICH [FILE/A] [NORES/S] [RES/S] [ALL/S]` -- Find where a command lives
- `RESIDENT [NAME] [FILE] [REMOVE/S] [ADD/S] [REPLACE/S] [PURE/S] [SYSTEM/S]` -- Manage resident commands
- `STACK [SIZE/N]` -- Show or set stack size

## Environment & Variables
- `GETENV [NAME/A]` -- Read ENV: variable
- `SETENV [NAME/A] [STRING/F]` -- Set ENV: variable (current session)
- `GET [NAME/A]` -- Read local shell variable
- `SET [NAME/A] [STRING/F]` -- Set local shell variable
- `ECHO [STRING/M] [NOLINE/S] [FIRST/N] [LEN/N] [TO/K]` -- Print text
- `CD [DIR]` -- Change or show current directory

## Date & Time
- `DATE [DATE] [TIME] [TO/K]` -- Show or set system date/time
- `WAIT [TIME] [SEC/S] [MIN/S] [UNTIL/K]` -- Wait (e.g. `WAIT 5 SEC`, `WAIT UNTIL 14:00`)

## Scripting
- `IF [NOT/S] [WARN/S] [ERROR/S] [FAIL/S] [EQ/K] [GT/K] [GE/K] [VAL/S] [EXISTS/K]` -- Conditional
- `ELSE` / `ENDIF` -- Conditional blocks
- `SKIP [LABEL/A] [BACK/S]` / `LAB [LABEL/A]` -- Jump to label
- `FAILAT [RCLIM/N]` -- Set error threshold
- `QUIT [RC/N]` -- Exit script with return code
- `ASK [PROMPT/A]` -- Ask yes/no question (sets WARN on yes)
- `REQUESTCHOICE [TITLE/A] [BODY/A] [GADGETS/A] [PUBSCREEN/K]` -- GUI choice dialog
- `REQUESTFILE [DRAWER] [FILE/K] [PATTERN/K] [TITLE/K] [POSITIVE/K] [NEGATIVE/K] [ACCEPTPATTERN/K] [REJECTPATTERN/K] [SAVEMODE/S] [MULTISELECT/S] [DRAWERSONLY/S] [NOICONS/S] [PUBSCREEN/K]` -- File requester

## Path Conventions
- `SYS:` -- System boot volume
- `WORK:` -- Secondary volume
- `RAM:` -- RAM disk
- `T:` -- Temporary directory (usually RAM:T)
- `S:` -- Scripts directory (SYS:S)
- `LIBS:` -- Libraries directory
- `DEVS:` -- Devices directory
- `C:` -- Commands directory
- `ENV:` / `ENVARC:` -- Environment variables (volatile / persistent)
- `AmigaAI:` -- This application's directory

## AmigaDOS Wildcards (NOT Unix!)
NEVER use `*` as wildcard. It does NOT work on AmigaDOS.
- `#?` -- Match any characters (this is the AmigaDOS equivalent of `*`)
- `?` -- Match single character
- `~` -- NOT (negate pattern)
- `|` -- OR (alternative patterns)
- `()` -- Grouping
- Example: `#?.txt` matches all .txt files (NOT *.txt!)
- Example: `#?.library` matches all .library files
- Example: `~(#?.info)` matches all files except .info files
- Example: `LIST WORK: PAT=#?.mod` matches all .mod files

## Screen Capture (SGrab)
- `sgrab FILE <path> PNG NOBEEP` -- Capture full screen as PNG
- `sgrab FILE <path> PNG NOBEEP X <x> Y <y> W <w> H <h>` -- Capture region
- `sgrab FILE <path> JPEG NOBEEP` -- Capture as JPEG
- Formats: ILBM (default), PNG, JPEG
- NOBEEP suppresses the screen flash during capture
- Use the screenshot tool instead of calling sgrab directly

## Redirection
- `>file` -- Redirect output to file
- `>>file` -- Append output to file
- `>NIL:` -- Discard output
- `<file` -- Input from file
- `|` -- Pipe (requires pipe-handler, not always available)
