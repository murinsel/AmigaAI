# AmigaDOS Shell Commands Reference

Use these commands with the shell_command tool.

CRITICAL: AmigaDOS is NOT Unix. Do NOT use Unix syntax:
- Wildcard: use `#?` NOT `*` (e.g. `LIST SYS:Libs/#?.library`, NOT `*.library`)
- Directory separator: use `/` or `:` NOT `\`
- No `ls`, `grep`, `cat`, `rm` -- use LIST, SEARCH, TYPE, DELETE

CRITICAL: NEVER delete or rename files or directories without asking the user first.
Always confirm before using DELETE, DELETE ALL, or RENAME commands.

## File/Directory Listing
- `LIST <path>` -- List files with details (size, date, flags). Default: current dir.
  - `LIST <path> ALL` -- Recursive listing
  - `LIST <path> DIRS` -- Only directories
  - `LIST <path> FILES` -- Only files
  - `LIST <path> PAT=#?.txt` -- Pattern matching (AmigaDOS wildcards)
  - `LIST <path> LFORMAT "%s%s %l"` -- Custom format (%s=path, %s=name, %l=size)
- `DIR <path>` -- Brief directory listing (names only)

## File Operations
- `COPY <src> <dst>` -- Copy file or directory
  - `COPY <src> <dst> ALL` -- Copy recursively
  - `COPY <src> <dst> CLONE` -- Preserve dates/flags
- `DELETE <path>` -- Delete file or empty directory
  - `DELETE <path> ALL` -- Delete recursively (careful!)
- `RENAME <old> <new>` -- Rename/move file
- `MAKEDIR <path>` -- Create directory
- `PROTECT <file> <flags>` -- Set file permissions (rwed, +r, -w, etc.)

## File Content
- `TYPE <file>` -- Display file contents (text files)
- `TYPE <file> HEX` -- Hex dump
- `SEARCH <path> <string>` -- Search for text in files
  - `SEARCH <path> <string> ALL` -- Search recursively in subdirectories
  - `SEARCH <path> <string> FILE` -- Show only filenames (not matching lines)
  - `SEARCH <path> <string> NONUM` -- Don't show line numbers
  - `SEARCH <path> <string> QUIET` -- Only set return code, no output
  - `SEARCH <path> <string> CASE` -- Case-sensitive search (default: case-insensitive)
  NOTE: SEARCH expects the DIRECTORY first, then the search STRING.
  Example: `SEARCH SYS:S "SetPatch"` -- searches all files in SYS:S for "SetPatch"
  Example: `SEARCH WORK: "hello" ALL` -- searches recursively from WORK:
  Wrong: `SEARCH "hello" SYS:` -- this would look for file "hello" containing "SYS:"
- `SORT <from> <to>` -- Sort a text file

## System Information
- `VERSION` -- Show OS version
- `VERSION <file> FULL` -- Show version string of a file/library
- `ASSIGN` -- List all logical assignments
- `ASSIGN <name>: <path>` -- Create assignment
- `AVAIL` -- Show memory usage (chip/fast/total)
- `AVAIL FLUSH` -- Flush unused libraries/fonts from memory
- `STATUS` -- List running CLI processes
- `INFO` -- Show mounted volumes and free space

## Running Programs
- `RUN <command>` -- Run command in background (returns immediately)
  - `RUN >NIL: <command>` -- Background, discard output
- `EXECUTE <script>` -- Execute a script file
- `WHICH <command>` -- Find where a command lives
- `RESIDENT` -- Show resident commands

## Environment & Variables
- `GETENV <name>` -- Read ENV: variable
- `SETENV <name> <value>` -- Set ENV: variable (current session)
- `GET <name>` -- Read local shell variable
- `SET <name> <value>` -- Set local shell variable
- `ECHO <text>` -- Print text
- `CD <path>` -- Change/show current directory
- `CD` -- Show current directory (no args)

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

## Redirection
- `>file` -- Redirect output to file
- `>>file` -- Append output to file
- `>NIL:` -- Discard output
- `<file` -- Input from file
- `|` -- Pipe (requires pipe-handler, not always available)
