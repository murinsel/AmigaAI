# Deluxe Music 2.0 ARexx Port

Global port name: DMUSIC. However, the global port only works while a DMusic window is active. If another window becomes active (e.g. Workbench), messages to the global port will fail. Always address a specific document through its document port instead.

File formats: CMUS (native, recommended for saving), DMCS (legacy), Simple MIDI (Type 0 for <=8 tracks, Type 1 for >8 tracks).

## Program Control
- `UNDO` -- Undo the last operation
- `QUIT` -- Quit the program

## File/Document Operations
- `NEW` -- Open a new blank score
- `OPEN` -- Open an existing score file
- `CLOSE` -- Close the current score
- `SAVE` -- Save the current score
- `SAVEAS` -- Save the current score and prompt for a filename
- `REVERT` -- Revert to previously saved version of the score
- `PRINT` -- Print the score

## Clipboard
- `COPY` -- Copy selected items to the clipboard
- `CUT` -- Cut selected items to the clipboard
- `PASTE` -- Paste from clipboard into the score

## Playback
- `PLAY` -- Play a score
- `STOP` -- Stop playback
- `RESUME` -- Resume playback from where it was last stopped

## GUI Control
- `CHANGEWINDOW` -- Move and resize the current window
- `SIZEWINDOW` -- Resize the main document window
- `WINDOW` -- Open/close/activate/resize command windows
- `MOVEWINDOW` -- Move a window
- `SCREENTOFRONT` -- Move the DeluxeMusic screen to the front
- `SCREENTOBACK` -- Move the DeluxeMusic screen to the back
- `LOCKDISPLAY` -- Inhibit screen refreshes
- `UNLOCKDISPLAY` -- Undo a LOCKDISPLAY command
- `UPDATEDISPLAY` -- Refresh the display
- `LOCKGUI` -- Inhibit all keyboard and mouse input
- `UNLOCKGUI` -- Undo a LOCKGUI command
- `BEEPSCREEN` -- Beep or flash the screen
- `SETSTATUSBAR` -- Place a message in the program's title bar
- `SAVESETTINGS` -- Save the GUI settings

## Tool Window
- `SETTOOL` -- Select a particular tool
- `SETACCIDENTAL` -- Change the accidental settings
- `SETDIVISION` -- Change the note duration settings
- `SETDOT` -- Change the dotted note setting
- `SETDYNAMIC` -- Change the dynamics settings
- `SETTUPLET` -- Change the tuplet settings

## User Prompts
- `REQUESTNOTIFY` -- Open a 1-button requester
- `REQUESTRESPONSE` -- Open a 2-button requester
- `REQUESTNUM` -- Prompt the user for a number
- `REQUESTSTRING` -- Prompt the user for a string
- `REQUESTFILE` -- Prompt the user for a filename

## Cursor Movement
- `GOTO` -- Move the cursor to a specified location
- `POSITION` -- Move the cursor to start/end of measure or score
- `NEXT` -- Move the cursor to the next item in the score
- `PREVIOUS` -- Move the cursor to the previous item in the score

## Selecting Data
- `SELECTITEM` -- Select the item after the cursor
- `SELECTALL` -- Select all items in the score
- `DESELECTALL` -- Deselect all selected items

## Inserting Data
- `INSERTITEM` -- Add an item to the score
- `INSERTMEASURE` -- Add a new measure

## Deleting Data
- `CLEAR` -- Delete all data from a score
- `ERASE` -- Delete all selected items
- `CLEARMOD` -- Remove all accidentals/tuplets from selected items
- `DELBACKWD` -- Delete the item before the cursor
- `DELFWD` -- Delete the item after the cursor
- `DELETEMEASURE` -- Delete the current measure

## Changing Data
- `CHANGEITEM` -- Modify the item after the cursor. Supports NOFIX parameter to disable re-sorting during batch edits
- `CHANGESELECTED` -- Modify the currently selected items. Supports NOFIX parameter to disable re-sorting during batch edits
- `CHANGETONE` -- Change the pitch of the selected notes
- `FLIPNOTESTEM` -- Flip the direction of selected note stems
- `GROUPITEM` -- Add or remove grouping (ties, beams, 8va's, etc.)
- `DOUBLETIME` -- Double the duration of selected notes
- `HALFTIME` -- Halve the duration of selected notes
- `JOINMEASURE` -- Join two measures into one
- `SPLITMEASURE` -- Split one measure into two
- `REALIGNMEASURE` -- Realign the current measure
- `SETBARATTR` -- Change the bar lines (repeat, 2nd ending, etc.)
- `SETCLEF` -- Change a clef
- `SETKEYSIGNATURE` -- Change the key signature
- `SETTIMESIGNATURE` -- Change the time signature
- `SETSTAFF` -- Change staff attributes

## Program Information
- `GETATTR` -- Get information about DeluxeMusic's status
- `GETERRORINFO` -- Give information about an error number
- `GETPOSITION` -- Get current cursor position
- `GETITEMATTR` -- Get information about the item after the cursor

## Musical Information
- `GETCLEF` -- Get information about clefs
- `GETKEYSIGNATURE` -- Get information about key signatures
- `GETSTAFF` -- Get information about staves
- `GETTIMESIGNATURE` -- Get information about time signatures

## Batch Editing
- `RESORTEVENTS` -- Re-sort notes after batch editing with NOFIX. Recommended pattern: LOCKGUI, LOCKDISPLAY, edit with NOFIX, RESORTEVENTS, UNLOCKDISPLAY, UNLOCKGUI

## ARexx Control
- `RX` -- Execute an ARexx script
- `RXS` -- Execute an ARexx string
- `ABORTSCRIPTS` -- Abort all currently executing ARexx scripts
- `CMDSHELL` -- Open the ARexx command shell
- `HELP` -- Display usage information for ARexx commands
- `LEARN` -- Start/stop recording a macro
- `PLAYMACRO` -- Execute the last recorded macro
- `SAVEMACRO` -- Save last recorded macro

## Notes
- Predefined ARexx macros are in Rexx: directory. Assign macros to function keys via Macros menu > Assign Macro, then Save Settings to persist as tooltypes.
- For best print results, set Score width (Settings > Score Setup) equal to Print width (Project > Print As...).
- Set Tempo requester has a "Set Default Tempo" button for setting a global default tempo for the entire score.
- Loading old DMCS format scores into DMusic converts them to CMUS. First load is slower; subsequent loads of the saved CMUS file are faster.
- MIDI file I/O: complex scores with multiple tracks and complex rhythms may not convert perfectly.
- DMPlayer is included for non-commercial playback distribution. Save scores as CMUS with embedded instruments for distribution.
