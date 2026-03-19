# Deluxe Music 2.0 ARexx Port

Global port name: DMUSIC. However, the global port only works while a DMusic window is active. If another window becomes active (e.g. Workbench), messages to the global port will fail. Always address a specific document through its document port instead. Document ports are named DMUSIC.0, DMUSIC.1, DMUSIC.2, etc.

File formats: CMUS (native, recommended for saving), DMCS (legacy), Simple MIDI (Type 0 for <=8 tracks, Type 1 for >8 tracks).

For detailed parameter docs and examples, use the arexx_help tool: arexx_help program=DMusic command=INSERTITEM

## Program Control
- `UNDO` -- Undo the last operation
- `QUIT` -- Quit the program (FORCE/S)

## File/Document Operations
- `NEW` -- Open a new blank score
- `OPEN` -- Open a score file (FILENAME, FORCE/S)
- `CLOSE` -- Close the current score (FORCE/S)
- `SAVE` -- Save the current score
- `SAVEAS` -- Save with options (FILENAME, FORMAT/K, EMBEDSAMPLES/K, FORCE/S)
- `REVERT` -- Revert to saved version (FORCE/S)
- `PRINT` -- Print the score (OPTIONS/S)

## Clipboard
- `COPY` / `CUT` / `PASTE`

## Playback
- `PLAY` -- Play score (SECTION/S)
- `STOP` -- Stop playback
- `RESUME` -- Resume playback

## GUI Control
- `CHANGEWINDOW` -- Move/resize window (LEFT/N, TOP/N, WIDTH/N, HEIGHT/N)
- `SIZEWINDOW` -- Resize (WIDTH/N, HEIGHT/N)
- `WINDOW` -- Control named windows (NAMES/M, OPEN/S, CLOSE/S, ACTIVATE/S, ...)
- `MOVEWINDOW` -- Move (LEFT/N, TOP/N)
- `SCREENTOFRONT` / `SCREENTOBACK`
- `LOCKDISPLAY` / `UNLOCKDISPLAY` -- Inhibit/resume screen refresh
- `UPDATEDISPLAY` -- Refresh display (FORCE/S)
- `LOCKGUI` / `UNLOCKGUI` -- Inhibit/resume user input
- `BEEPSCREEN` -- Flash screen
- `SETSTATUSBAR` -- Set title bar text (TITLE)
- `SAVESETTINGS` -- Save GUI settings

## Tool Window
- `SETTOOL` -- Select tool (TOOL/A)
- `SETACCIDENTAL` / `SETDIVISION` / `SETDOT` / `SETDYNAMIC` / `SETTUPLET` -- Change settings (VALUE/A, UPDATE/S)

## User Prompts
- `REQUESTNOTIFY` -- 1-button message (TITLE/K, PROMPT)
- `REQUESTRESPONSE` -- 2-button question (TITLE/K, PROMPT, DEFAULT/K/N, BUTTONS/K)
- `REQUESTNUM` -- Ask for number (TITLE/K, PROMPT, DEFAULT/K/N, BUTTONS/K)
- `REQUESTSTRING` -- Ask for string (TITLE/K, PROMPT, DEFAULT/K, BUTTONS/K)
- `REQUESTFILE` -- Ask for file (TITLE/K, PATH/K, FILE/K, PATTERN/K)

## Cursor Movement
- `GOTO` -- Move to position (TRACK/K/N, STAFF/K/N, MEASURE/K/N, CHORD/K/N, CLOCK/K/N)
- `POSITION` -- Move to boundary (SOF/S, EOF/S, SOM/S, EOM/S)
- `NEXT` / `PREVIOUS` -- Move to next/previous item (EVENT/S, NOTE/S, CHORD/S, MEASURE/S, STAFF/S, TRACK/S, COUNT/N)

## Selecting Data
- `SELECTITEM` -- Select item at cursor (ADD/S, REMOVE/S, TOGGLE/S)
- `SELECTALL` / `DESELECTALL`

## Inserting Data
- `INSERTITEM` -- Add item (TYPE, DURATION/K, LETTER/K, OCTAVE/K/N, PITCH/K/N, LOUDNESS/K/N, RATE/K/N, NOFIX/S, ...)
- `INSERTMEASURE` -- Add empty measure

## Deleting Data
- `CLEAR` -- Delete all (FORCE/S)
- `ERASE` -- Delete selected items
- `CLEARMOD` -- Remove accidentals/tuplets from selected
- `DELBACKWD` / `DELFWD` -- Delete before/after cursor
- `DELETEMEASURE` -- Delete measure(s) (COUNT/N)

## Changing Data
- `CHANGEITEM` / `CHANGESELECTED` -- Modify item/selection (same params as INSERTITEM)
- `CHANGETONE` -- Transpose (HALFSTEP/K/N, LEVEL/K/N, OCTAVE/K/N)
- `FLIPNOTESTEM` -- Flip stems (DIRECTION)
- `GROUPITEM` -- Add/remove ties, beams, etc. (ACTION/A, TYPE/A, DIRECTION)
- `DOUBLETIME` / `HALFTIME` -- Double/halve duration
- `JOINMEASURE` / `SPLITMEASURE` / `REALIGNMEASURE`
- `SETBARATTR` -- Bar lines (BEGINREPEAT/K, ENDREPEAT/K, ENDING1/K, ENDING2/K, DOUBLEBAR/K)
- `SETCLEF` -- Change clef (TYPE/A, STAFF/N/A, START/N, END/N)
- `SETKEYSIGNATURE` -- Change key (TYPE/A, STAFF/N/A, START/N, END/N, TRANSPOSE/K)
- `SETTIMESIGNATURE` -- Change time (BEATS/N/A, DURATION/N/A, MEASURE/N)
- `SETSTAFF` -- Change staff attributes (STAFF/N/A, ATTR/A, VALUE/A)

## Program Information
- `GETATTR` -- Query status (OBJECT/A, NAME, FIELD)
- `GETERRORINFO` -- Error description (ERROR/N/A)
- `GETPOSITION` -- Cursor position (FIELD)
- `GETITEMATTR` -- Item info (FIELD)

## Musical Information
- `GETCLEF` / `GETKEYSIGNATURE` / `GETTIMESIGNATURE` -- Query musical state (STAFF/N/A, MEASURE/N, EFFECTIVE/S)
- `GETSTAFF` -- Staff info (STAFF/N/A, ATTR/A)

## Batch Editing
- `RESORTEVENTS` -- Re-sort after NOFIX edits. Pattern: LOCKGUI, LOCKDISPLAY, edit with NOFIX, RESORTEVENTS, UNLOCKDISPLAY, UNLOCKGUI

## ARexx Control
- `RX` / `RXS` -- Execute script/string (CONSOLE/S, ASYNC/S, COMMAND/F)
- `ABORTSCRIPTS` -- Abort running scripts
- `CMDSHELL` -- Open ARexx shell
- `HELP` -- Command help (COMMAND)
- `LEARN` -- Record macro (FILE/K, STOP/S)
- `PLAYMACRO` / `SAVEMACRO` -- Play/save macro (FILE/K)

## Notes
- Predefined ARexx macros are in Rexx: directory. Assign to function keys via Macros menu.
- For best print results, set Score width equal to Print width.
- Set Tempo requester has a "Set Default Tempo" button for global tempo.
- Loading old DMCS scores converts to CMUS. First load is slower.
- MIDI I/O: complex scores may not convert perfectly.
- DMPlayer: for non-commercial distribution. Save as CMUS with embedded instruments.
