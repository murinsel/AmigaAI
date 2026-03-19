# Deluxe Music 2.0 ARexx Port

Global port name: DMUSIC. However, the global port only works while a DMusic window is active. If another window becomes active (e.g. Workbench), messages to the global port will fail. Always address a specific document through its document port instead. Document ports are named DMUSIC.0, DMUSIC.1, DMUSIC.2, etc.

File formats: CMUS (native, recommended for saving), DMCS (legacy), Simple MIDI (Type 0 for <=8 tracks, Type 1 for >8 tracks).

Parameter syntax: /A = required, /S = switch (no value), /K = keyword, /N = numeric, /M = multiple, /F = rest of line.

## Program Control
- `UNDO` -- Undo the last operation
- `QUIT FORCE/S` -- Quit the program. FORCE skips save confirmation

## File/Document Operations
- `NEW` -- Open a new blank score
- `OPEN FILENAME,FORCE/S` -- Open an existing score file. FORCE skips save confirmation
- `CLOSE FORCE/S` -- Close the current score
- `SAVE` -- Save the current score
- `SAVEAS FILENAME,FORMAT/K,EMBEDSAMPLES/K,FORCE/S` -- Save with options (FORMAT: CMUS, DMCS, MIDI)
- `REVERT FORCE/S` -- Revert to previously saved version
- `PRINT OPTIONS/S` -- Print the score. OPTIONS opens print dialog

## Clipboard
- `COPY` -- Copy selected items to the clipboard
- `CUT` -- Cut selected items to the clipboard
- `PASTE` -- Paste from clipboard into the score

## Playback
- `PLAY SECTION/S` -- Play score. SECTION plays only the selected section
- `STOP` -- Stop playback
- `RESUME` -- Resume playback from where it was last stopped

## GUI Control
- `CHANGEWINDOW LEFT/N,TOP/N,WIDTH/N,HEIGHT/N` -- Move and resize the current window
- `SIZEWINDOW WIDTH/N,HEIGHT/N` -- Resize the main document window
- `WINDOW NAMES/M,OPEN/S,CLOSE/S,SNAPSHOT/S,ACTIVATE/S,MIN/S,MAX/S,FRONT/S,BACK/S,PREV/S,NEXT/S` -- Control command windows
- `MOVEWINDOW LEFT/N,TOP/N` -- Move a window
- `SCREENTOFRONT` -- Move the DeluxeMusic screen to the front
- `SCREENTOBACK` -- Move the DeluxeMusic screen to the back
- `LOCKDISPLAY` -- Inhibit screen refreshes
- `UNLOCKDISPLAY` -- Undo a LOCKDISPLAY command
- `UPDATEDISPLAY FORCE/S` -- Refresh the display
- `LOCKGUI` -- Inhibit all keyboard and mouse input
- `UNLOCKGUI` -- Undo a LOCKGUI command
- `BEEPSCREEN` -- Beep or flash the screen
- `SETSTATUSBAR TITLE` -- Place a message in the program's title bar
- `SAVESETTINGS` -- Save the GUI settings

## Tool Window
- `SETTOOL TOOL/A` -- Select a particular tool
- `SETACCIDENTAL VALUE/A,UPDATE/S` -- Change the accidental settings
- `SETDIVISION VALUE/A,UPDATE/S` -- Change the note duration settings
- `SETDOT VALUE/A,UPDATE/S` -- Change the dotted note setting
- `SETDYNAMIC VALUE/A,UPDATE/S` -- Change the dynamics settings
- `SETTUPLET VALUE/A,UPDATE/S` -- Change the tuplet settings

## User Prompts
- `REQUESTNOTIFY TITLE/K,PROMPT` -- Open a 1-button requester
- `REQUESTRESPONSE TITLE/K,PROMPT,DEFAULT/K/N,BUTTONS/K` -- Open a 2-button requester
- `REQUESTNUM TITLE/K,PROMPT,DEFAULT/K/N,BUTTONS/K` -- Prompt the user for a number
- `REQUESTSTRING TITLE/K,PROMPT,DEFAULT/K,BUTTONS/K` -- Prompt the user for a string
- `REQUESTFILE TITLE/K,PATH/K,FILE/K,PATTERN/K` -- Prompt the user for a filename

## Cursor Movement
- `GOTO TRACK/K/N,STAFF/K/N,MEASURE/K/N,CHORD/K/N,CLOCK/K/N,HIDDEN/S` -- Move cursor to a specified location
- `POSITION SOF/S,EOF/S,SOM/S,EOM/S,HIDDEN/S` -- Move cursor to start/end of measure (SOM/EOM) or score (SOF/EOF)
- `NEXT EVENT/S,NOTE/S,INTERVAL/S,CHORD/S,MEASURE/S,STAFF/S,TRACK/S,SELECTED/N,HIDDEN/S,COUNT/N` -- Move to next item
- `PREVIOUS EVENT/S,NOTE/S,INTERVAL/S,CHORD/S,MEASURE/S,STAFF/S,TRACK/S,SELECTED/N,HIDDEN/S,COUNT/N` -- Move to previous item

## Selecting Data
- `SELECTITEM ADD/S,REMOVE/S,TOGGLE/S` -- Select the item after the cursor
- `SELECTALL` -- Select all items in the score
- `DESELECTALL` -- Deselect all selected items

## Inserting Data
- `INSERTITEM TYPE,TRACK/K/N,DURATION/K,CLOCKS/K/N,DOTS/K/N,TUPLET/K/N,ACCIDENTAL/K,OCTAVE/K/N,LINE/K/N,PITCH/K/N,LETTER/K,NOTESTEM/K,LOUDNESS/K/N,FONT/K,RATE/K/N,SELECTED/N,HIDDEN/S,NOFIX/S` -- Add an item to the score
- `INSERTMEASURE` -- Add a new measure

## Deleting Data
- `CLEAR FORCE/S` -- Delete all data from a score
- `ERASE` -- Delete all selected items
- `CLEARMOD` -- Remove all accidentals/tuplets from selected items
- `DELBACKWD` -- Delete the item before the cursor
- `DELFWD` -- Delete the item after the cursor
- `DELETEMEASURE COUNT/N` -- Delete measure(s)

## Changing Data
- `CHANGEITEM TYPE,TRACK/K/N,DURATION/K,CLOCKS/K/N,DOTS/K/N,TUPLET/K/N,ACCIDENTAL/K,OCTAVE/K/N,LINE/K/N,PITCH/K/N,LETTER/K,NOTESTEM/K,LOUDNESS/K/N,FONT/K,RATE/K/N,SELECTED/N,HIDDEN/S,NOFIX/S` -- Modify the item after the cursor. NOFIX disables re-sorting during batch edits
- `CHANGESELECTED TYPE,TRACK/K/N,DURATION/K,CLOCKS/K/N,DOTS/K/N,TUPLET/K/N,ACCIDENTAL/K,OCTAVE/K/N,LINE/K/N,PITCH/K/N,LETTER/K,NOTESTEM/K,LOUDNESS/K/N,FONT/K,RATE/K/N,SELECTED/N,HIDDEN/S,NOFIX/S` -- Modify the currently selected items. NOFIX disables re-sorting during batch edits
- `CHANGETONE HALFSTEP/K/N,LEVEL/K/N,OCTAVE/K/N` -- Change pitch of selected notes
- `FLIPNOTESTEM DIRECTION` -- Flip the direction of selected note stems
- `GROUPITEM ACTION/A,TYPE/A,DIRECTION` -- Add or remove grouping (ties, beams, 8va's, etc.)
- `DOUBLETIME` -- Double the duration of selected notes
- `HALFTIME` -- Halve the duration of selected notes
- `JOINMEASURE` -- Join two measures into one
- `SPLITMEASURE` -- Split one measure into two
- `REALIGNMEASURE` -- Realign the current measure
- `SETBARATTR BEGINSECTION/S,ENDSECTION/S,ENDING1/K,ENDING2/K,BEGINREPEAT/K,ENDREPEAT/K,DOUBLEBAR/K` -- Change bar lines
- `SETCLEF TYPE/A,STAFF/N/A,START/N,END/N` -- Change a clef
- `SETKEYSIGNATURE TYPE/A,STAFF/N/A,START/N,END/N,TRANSPOSE/K` -- Change the key signature
- `SETTIMESIGNATURE BEATS/N/A,DURATION/N/A,MEASURE/N` -- Change the time signature
- `SETSTAFF STAFF/N/A,ATTR/A,VALUE/A` -- Change staff attributes

## Program Information
- `GETATTR OBJECT/A,NAME,FIELD` -- Get information about DeluxeMusic's status
- `GETERRORINFO ERROR/N/A` -- Give information about an error number
- `GETPOSITION FIELD` -- Get current cursor position
- `GETITEMATTR FIELD` -- Get information about the item after the cursor

## Musical Information
- `GETCLEF STAFF/N/A,MEASURE/N,EFFECTIVE/S` -- Get clef info. EFFECTIVE returns the clef in effect at that measure
- `GETKEYSIGNATURE STAFF/N/A,MEASURE/N,EFFECTIVE/S` -- Get key signature info
- `GETSTAFF STAFF/N/A,ATTR/A` -- Get staff attributes
- `GETTIMESIGNATURE MEASURE/N,EFFECTIVE/S` -- Get time signature info

## Batch Editing
- `RESORTEVENTS` -- Re-sort notes after batch editing with NOFIX. Recommended pattern: LOCKGUI, LOCKDISPLAY, edit with NOFIX, RESORTEVENTS, UNLOCKDISPLAY, UNLOCKGUI

## ARexx Control
- `RX CONSOLE/S,ASYNC/S,COMMAND/F` -- Execute an ARexx script. ASYNC runs without waiting
- `RXS CONSOLE/S,ASYNC/S,COMMAND/F` -- Execute an ARexx string. ASYNC runs without waiting
- `ABORTSCRIPTS` -- Abort all currently executing ARexx scripts
- `CMDSHELL` -- Open the ARexx command shell
- `HELP COMMAND` -- Display usage information for an ARexx command
- `LEARN FILE/K,STOP/S` -- Start/stop recording a macro. FILE specifies output file
- `PLAYMACRO` -- Execute the last recorded macro
- `SAVEMACRO FILE/K` -- Save last recorded macro

## Notes
- Predefined ARexx macros are in Rexx: directory. Assign macros to function keys via Macros menu > Assign Macro, then Save Settings to persist as tooltypes.
- For best print results, set Score width (Settings > Score Setup) equal to Print width (Project > Print As...).
- Set Tempo requester has a "Set Default Tempo" button for setting a global default tempo for the entire score.
- Loading old DMCS format scores into DMusic converts them to CMUS. First load is slower; subsequent loads of the saved CMUS file are faster.
- MIDI file I/O: complex scores with multiple tracks and complex rhythms may not convert perfectly.
- DMPlayer is included for non-commercial playback distribution. Save scores as CMUS with embedded instruments for distribution.
