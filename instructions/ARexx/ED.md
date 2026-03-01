# ED ARexx Port

Port name: ED (first instance), ED_1, ED_2, ... (subsequent instances).
Commands are Ed extended commands sent as strings. Numeric prefix repeats command (e.g. "4 D" deletes 4 lines).

## Cursor Movement
- `T` -- Top of file
- `B` -- Bottom of file
- `M n` -- Go to line n
- `N` -- Start of next line
- `P` -- Start of previous line
- `PD` -- Page down
- `PU` -- Page up
- `EP` -- End of page
- `CL` / `CR` -- Character left/right
- `CS` / `CE` -- Start/end of current line
- `WN` / `WP` -- Next/previous word
- `TB` -- Next tab stop

## Text Editing
- `I /text/` -- Insert text on line before cursor
- `A /text/` -- Insert text on line after cursor
- `D` -- Delete current line
- `DC` -- Delete character at cursor
- `DL` -- Delete character left of cursor
- `DW` -- Delete to end of word
- `EL` -- Delete to end of line
- `S` -- Split line at cursor
- `J` -- Join next line to current
- `FC` -- Flip case of character
- `U` -- Undo changes to current line

## Search & Replace
- `F /text/` -- Find next occurrence
- `BF /text/` -- Find backwards
- `E /old/new/` -- Replace next occurrence
- `RP /old/new/` -- Replace all occurrences
- `EQ /old/new/` -- Replace with confirmation
- `RPE /old/new/` -- Replace all with confirmation
- `UC` -- Case-insensitive search mode
- `LC` -- Case-sensitive search mode

## Block Operations
- `BS` -- Mark block start
- `BE` -- Mark block end
- `SB` -- Show block at screen top
- `IB` -- Insert copy of block after cursor
- `DB` -- Delete marked block
- `WB /filename/` -- Write block to file

## File Operations
- `SA` -- Save to current filename
- `SA /filename/` -- Save as
- `X` -- Save and exit
- `Q` -- Quit without saving
- `XQ` -- Exit (prompts if unsaved changes)
- `OP /filename/` -- Open file (replaces current)
- `NW /filename/` -- New file (replaces current)
- `IF /filename/` -- Insert file at cursor

## Settings
- `SL n` -- Set left margin
- `SR n` -- Set right margin
- `ST n` -- Set tab stop
- `EX` -- Extend margins for current line
- `SF n /text/` -- Set function key n
- `DF n` -- Display function key n

## Other
- `RV /stem/` -- Return editor state to ARexx stem variables
- `RE` -- Repeat last command
- `RF /filename/` -- Run command file
- `RX /script/` -- Run ARexx program
- `VW` -- Refresh display
- `SH` -- Show editor status
- `SM /text/` -- Show message in status line

## RV Stem Variables
stem.X/Y=cursor pos, stem.LINE=line number, stem.FILENAME=file name,
stem.CURRENT=current line text, stem.LEFT/RIGHT=margins, stem.TABSTOP=tab,
stem.LMAX=visible lines, stem.WIDTH=screen width, stem.BASE=window offset,
stem.FORCECASE=case flag, stem.LASTCMD=last command, stem.SEARCH=last search
