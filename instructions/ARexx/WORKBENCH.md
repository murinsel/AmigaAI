# WORKBENCH ARexx Port

Arguments: /A=required, /S=switch, /N=number, /K=keyword, /M=multiple, /F=rest of line.
WINDOW argument accepts: ROOT (desktop), ACTIVE (focused window), or absolute drawer path.

## Window Management
- `WINDOW WINDOWS/M/A [OPEN/S] [CLOSE/S] [SNAPSHOT/S] [ACTIVATE/S] [MIN/S] [MAX/S] [FRONT/S] [BACK/S] [CYCLE PREVIOUS|NEXT]` -- Open/close/manage windows
- `ACTIVATEWINDOW [WINDOW]` -- Focus a window
- `CHANGEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N] [WIDTH/N] [HEIGHT/N]` -- Move and resize
- `MOVEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N]` -- Move window
- `SIZEWINDOW [WINDOW] [WIDTH/N] [HEIGHT/N]` -- Resize window
- `WINDOWTOFRONT [WINDOW]` -- Bring to front
- `WINDOWTOBACK [WINDOW]` -- Send to back
- `ZOOMWINDOW [WINDOW]` -- Toggle zoom
- `UNZOOMWINDOW [WINDOW]` -- Restore from zoom
- `VIEW [WINDOW] [PAGE/S] [PIXEL/S] [UP/S] [DOWN/S] [LEFT/S] [RIGHT/S]` -- Scroll window content
- `LOCKGUI` / `UNLOCKGUI` -- Lock/unlock all drawer windows

## Icon Manipulation
- `ICON [WINDOW] [NAMES/M] [OPEN/S] [MAKEVISIBLE/S] [SELECT/S] [UNSELECT/S] [UP/N] [DOWN/N] [LEFT/N] [RIGHT/N] [X/N] [Y/N] [ACTIVATE UP|DOWN|LEFT|RIGHT] [CYCLE PREVIOUS|NEXT] [MOVE IN|OUT]` -- Manipulate icons

## File Operations
- `DELETE NAME/A [ALL/S]` -- Delete file/drawer (ALL=recursive). Requires absolute path
- `RENAME OLDNAME/A NEWNAME/A` -- Rename (NEWNAME is name only, no path)
- `NEWDRAWER NAME/A` -- Create directory. Requires absolute path
- `INFO NAME/A` -- Open info requester for file/drawer

## Query Information
- `GETATTR OBJECT/A [NAME/K] [STEM/K] [VAR/K]` -- Query Workbench database (see objects below)
- `FAULT CODE/A/N` -- Get error description for error code
- `HELP [COMMAND/K] [MENUS/S] [PROMPT/S]` -- Get command help

## ARexx Execution
- `RX [CONSOLE/S] [ASYNC/S] CMD/A/F` -- Execute ARexx script or inline command

## Menu & Keyboard
- `MENU [WINDOW/K] [INVOKE] [NAME/K] [TITLE/K] [SHORTCUT/K] [ADD/S] [REMOVE/S] [CMD/K/F]` -- Invoke or manage custom menu items
- `KEYBOARD NAME/A [ADD/S] [REMOVE/S] [KEY] [CMD/F]` -- Bind ARexx commands to keys

## GETATTR Objects
- `APPLICATION.VERSION|SCREEN|AREXX|LASTERROR` -- App info
- `APPLICATION.FONT.SCREEN|ICON|SYSTEM.NAME|SIZE` -- Font info
- `APPLICATION.ICONBORDER.LEFT|TOP|RIGHT|BOTTOM` -- Icon border
- `WINDOWS.COUNT|ACTIVE` / `WINDOWS.n` -- Open window list
- `WINDOW.LEFT|TOP|WIDTH|HEIGHT` -- Window geometry (use NAME=path)
- `WINDOW.MIN.WIDTH|HEIGHT` / `WINDOW.MAX.WIDTH|HEIGHT` -- Window limits
- `WINDOW.VIEW.LEFT|TOP` -- Scroll position
- `WINDOW.SCREEN.NAME|WIDTH|HEIGHT` -- Screen info
- `WINDOW.ICONS.ALL|SELECTED|UNSELECTED.COUNT` -- Icon counts
- `WINDOW.ICONS.ALL|SELECTED|UNSELECTED.n.NAME|LEFT|TOP|WIDTH|HEIGHT|TYPE|STATUS` -- Icon details
- Icon types: DISK, DRAWER, TOOL, PROJECT, GARBAGE, DEVICE, KICK, APPICON

## MENU INVOKE Items
Workbench: WORKBENCH.BACKDROP|EXECUTE|REDRAWALL|UPDATEALL|LASTMESSAGE|ABOUT|QUIT
Window: WINDOW.NEWDRAWER|OPENPARENT|CLOSE|UPDATE|SELECTCONTENTS|CLEARSELECTION|RESIZETOFIT
Window cleanup: WINDOW.CLEANUPBY.COLUMN|NAME|DATE|SIZE|TYPE
Window snapshot: WINDOW.SNAPSHOT.WINDOW|ALL
Window show: WINDOW.SHOW.ONLYICONS|ALLFILES
Window view: WINDOW.VIEWBY.ICON|NAME|DATE|SIZE|TYPE
Icons: ICONS.OPEN|COPY|RENAME|INFORMATION|SNAPSHOT|UNSNAPSHOT|LEAVEOUT|PUTAWAY|DELETE|FORMATDISK|EMPTYTRASH
Tools: TOOLS.RESETWB
