# Workbench — ARexx Interface

## Overview

The Workbench provides an ARexx port for remote control of windows, icons, menus, and other functions.

**Port Name:** `WORKBENCH`

**Requirements:** rexxsyslib.library and RexxMast must be running.

### Window References

Many commands accept a `WINDOW` parameter:
- `ROOT` — The main Workbench window
- Full path — e.g. `Work:` or `SYS:Utilities`

### Error Codes

On error, RC is set to 10. The error message can be retrieved via `GETATTR APPLICATION.LASTERROR` or the `FAULT` command.

---

## Window Management

### WINDOW

Opens, closes, and manages windows.

```
WINDOW WINDOWS/M/A [OPEN/S] [CLOSE/S] [SNAPSHOT/S] [ACTIVATE/S]
       [MIN/S] [MAX/S] [FRONT/S] [BACK/S] [CYCLE/K]
```

| Parameter | Description |
|-----------|-------------|
| `WINDOWS/M/A` | Window names (ROOT or paths) |
| `OPEN/S` | Open window |
| `CLOSE/S` | Close window |
| `SNAPSHOT/S` | Save position and size |
| `ACTIVATE/S` | Activate window |
| `MIN/S` | Minimize to smallest size |
| `MAX/S` | Maximize to largest size |
| `FRONT/S` | Bring to front |
| `BACK/S` | Send to back |
| `CYCLE/K` | Cycle through windows: `PREVIOUS` or `NEXT` |

```rexx
ADDRESS WORKBENCH
'WINDOW "Work:" OPEN'
'WINDOW "Work:" FRONT ACTIVATE'
'WINDOW root SNAPSHOT'
'WINDOW "SYS:Utilities" CLOSE'
```

### ACTIVATEWINDOW

Makes a window the active one.

```
ACTIVATEWINDOW [WINDOW]
```

```rexx
'ACTIVATEWINDOW "Work:"'
'ACTIVATEWINDOW root'
```

### CHANGEWINDOW

Changes position and size of a window simultaneously.

```
CHANGEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N] [WIDTH/N] [HEIGHT/N]
```

```rexx
'CHANGEWINDOW root LEFTEDGE 10 TOPEDGE 30 WIDTH 400 HEIGHT 300'
```

### MOVEWINDOW

Moves a window.

```
MOVEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N]
```

### SIZEWINDOW

Resizes a window.

```
SIZEWINDOW [WINDOW] [WIDTH/N] [HEIGHT/N]
```

### WINDOWTOFRONT / WINDOWTOBACK

Brings a window to front or sends it to back.

```
WINDOWTOFRONT [WINDOW]
WINDOWTOBACK [WINDOW]
```

### ZOOMWINDOW / UNZOOMWINDOW

Toggles between normal and alternative window size.

```
ZOOMWINDOW [WINDOW]
UNZOOMWINDOW [WINDOW]
```

### VIEW

Scrolls the visible area of a window.

```
VIEW [WINDOW] [PAGE/S] [PIXEL/S] [UP/S] [DOWN/S] [LEFT/S] [RIGHT/S]
```

| Parameter | Description |
|-----------|-------------|
| `PAGE/S` | Scroll by page |
| `PIXEL/S` | Scroll by pixel |
| `UP/S` `DOWN/S` `LEFT/S` `RIGHT/S` | Scroll direction |

```rexx
'VIEW root PAGE DOWN'
'VIEW "Work:" PIXEL RIGHT'
```

### LOCKGUI / UNLOCKGUI

Locks or unlocks the user interface. Nested — each LOCKGUI requires a matching UNLOCKGUI.

```
LOCKGUI
UNLOCKGUI
```

```rexx
'LOCKGUI'
/* ... operations ... */
'UNLOCKGUI'
```

---

## Icon Management

### ICON

Manipulates icons in a window.

```
ICON [WINDOW] NAMES/M [OPEN/S] [MAKEVISIBLE/S] [SELECT/S] [UNSELECT/S]
     [UP/N] [DOWN/N] [LEFT/N] [RIGHT/N] [X/N] [Y/N]
     [ACTIVATE/K] [CYCLE/K] [MOVE/K]
```

| Parameter | Description |
|-----------|-------------|
| `NAMES/M` | Icon names |
| `OPEN/S` | Open icon (launch program / open drawer) |
| `MAKEVISIBLE/S` | Make icon visible (scroll into view) |
| `SELECT/S` | Select icon |
| `UNSELECT/S` | Deselect icon |
| `UP/N` `DOWN/N` `LEFT/N` `RIGHT/N` | Move icon by N pixels |
| `X/N` `Y/N` | Set icon to absolute position |
| `ACTIVATE/K` | Activate next icon: `UP`, `DOWN`, `LEFT`, `RIGHT` |
| `CYCLE/K` | Cycle through icons: `PREVIOUS`, `NEXT` |
| `MOVE/K` | Navigate hierarchy: `IN` (enter), `OUT` (leave) |

```rexx
ADDRESS WORKBENCH
/* Select icons */
'ICON WINDOW root NAMES Workbench Work SELECT'

/* Move icon */
'ICON WINDOW "Work:" NAMES "MyProgram" X 100 Y 50'

/* Open icon (launch program) */
'ICON WINDOW "SYS:Utilities" NAMES MultiView OPEN'
```

### INFO

Opens the information dialog for a file, drawer, or volume.

```
INFO NAME/A
```

```rexx
'INFO NAME "SYS:"'
'INFO NAME "Work:MyFile"'
```

---

## File Operations

### NEWDRAWER

Creates a new drawer.

```
NEWDRAWER NAME/A
```

NAME must be an absolute path.

```rexx
'NEWDRAWER "RAM:NewDrawer"'
'NEWDRAWER "Work:Projects/New"'
```

### DELETE

Deletes files or drawers.

```
DELETE NAME/A [ALL/S]
```

| Parameter | Description |
|-----------|-------------|
| `NAME/A` | Absolute path of the file/drawer |
| `ALL/S` | Delete contents recursively |

```rexx
'DELETE "RAM:TempFile"'
'DELETE "RAM:TempFolder" ALL'
```

### RENAME

Renames a file, drawer, or volume.

```
RENAME OLDNAME/A NEWNAME/A
```

OLDNAME must be an absolute path. NEWNAME is just the new name (without path).

```rexx
'RENAME "RAM:OldName" "NewName"'
```

---

## Queries

### GETATTR

Queries information from the Workbench database.

```
GETATTR OBJECT/A [NAME/K] [STEM/K] [VAR/K]
```

### Queryable Objects

**Application:**

| Object | Description |
|--------|-------------|
| `APPLICATION.VERSION` | Workbench version |
| `APPLICATION.SCREEN` | Screen name |
| `APPLICATION.AREXX` | ARexx port name |
| `APPLICATION.LASTERROR` | Last error message |
| `APPLICATION.ICONBORDER` | Icon border active? |

**Fonts:**

| Object | Description |
|--------|-------------|
| `APPLICATION.FONT.SCREEN.NAME` | Screen font name |
| `APPLICATION.FONT.SCREEN.SIZE` | Screen font size |
| `APPLICATION.FONT.ICON.NAME` | Icon font name |
| `APPLICATION.FONT.ICON.SIZE` | Icon font size |
| `APPLICATION.FONT.SYSTEM.NAME` | System font name |
| `APPLICATION.FONT.SYSTEM.SIZE` | System font size |

**Windows:**

| Object | Description |
|--------|-------------|
| `WINDOWS.COUNT` | Number of open windows |
| `WINDOWS.ACTIVE` | Path of the active window |
| `WINDOWS.0` ... `WINDOWS.N` | Path of the Nth window |

**Window Details (with NAME=window path):**

| Object | Description |
|--------|-------------|
| `WINDOW.LEFT` | Left position |
| `WINDOW.TOP` | Top position |
| `WINDOW.WIDTH` | Width |
| `WINDOW.HEIGHT` | Height |
| `WINDOW.MIN.WIDTH` | Minimum width |
| `WINDOW.MIN.HEIGHT` | Minimum height |
| `WINDOW.MAX.WIDTH` | Maximum width |
| `WINDOW.MAX.HEIGHT` | Maximum height |
| `WINDOW.VIEW.LEFT` | Visible area left |
| `WINDOW.VIEW.TOP` | Visible area top |
| `WINDOW.ICONS.COUNT` | Number of icons in window |
| `WINDOW.ICONS.0` ... | Icon names |

```rexx
OPTIONS RESULTS
ADDRESS WORKBENCH

/* Workbench version */
'GETATTR APPLICATION.VERSION'
SAY "Version:" RESULT

/* Number of open windows */
'GETATTR WINDOWS.COUNT'
SAY "Open windows:" RESULT

/* Active window */
'GETATTR WINDOWS.ACTIVE'
SAY "Active:" RESULT

/* Window position */
'GETATTR WINDOW.LEFT NAME "Work:"'
SAY "Left:" RESULT

/* All icons in root window */
'GETATTR WINDOW.ICONS.COUNT NAME ROOT'
count = RESULT
DO i = 0 TO count - 1
    'GETATTR WINDOW.ICONS.'i' NAME ROOT'
    SAY "Icon:" RESULT
END
```

### FAULT

Returns a human-readable error message for an error code.

```
FAULT CODE/A/N
```

```rexx
'FAULT 205'
SAY RESULT   /* "Object not found" */
```

### HELP

Displays help information.

```
HELP [COMMAND/K] [MENUS/S] [PROMPT/S]
```

| Parameter | Description |
|-----------|-------------|
| `COMMAND/K` | Show syntax of a specific command |
| `MENUS/S` | List available menu items |
| `PROMPT/S` | Invoke help system |

```rexx
'HELP COMMAND GETATTR'
SAY RESULT   /* Shows the command syntax */

'HELP MENUS'
SAY RESULT   /* List of all menu items */
```

---

## Menu Management

### MENU

Invokes menu items or creates custom menus.

```
MENU [WINDOW/K] [INVOKE] [NAME/K] [TITLE/K] [SHORTCUT/K]
     [ADD/S] [REMOVE/S] [CMD/K/F]
```

| Parameter | Description |
|-----------|-------------|
| `INVOKE` | Execute menu item |
| `NAME/K` | Unique identifier |
| `TITLE/K` | Display text |
| `SHORTCUT/K` | Keyboard shortcut (one character) |
| `ADD/S` | Add menu item |
| `REMOVE/S` | Remove menu item |
| `CMD/K/F` | Associated ARexx command |

### Standard Menu Items

| Menu Item | Description |
|-----------|-------------|
| `WORKBENCH.BACKDROP` | Toggle backdrop |
| `WORKBENCH.EXECUTE` | Execute command |
| `WORKBENCH.REDRAWALL` | Redraw all |
| `WORKBENCH.UPDATEALL` | Update all |
| `WORKBENCH.LASTMESSAGE` | Show last message |
| `WORKBENCH.ABOUT` | About Workbench |
| `WORKBENCH.QUIT` | Quit Workbench |
| `WINDOW.NEWDRAWER` | New drawer |
| `WINDOW.OPENPARENT` | Open parent directory |
| `WINDOW.CLOSE` | Close window |
| `WINDOW.UPDATE` | Update window |
| `WINDOW.SELECTALL` | Select all |
| `WINDOW.CLEANUPBY.NAME` | Clean up by name |
| `WINDOW.CLEANUPBY.DATE` | Clean up by date |
| `WINDOW.CLEANUPBY.SIZE` | Clean up by size |
| `WINDOW.CLEANUPBY.TYPE` | Clean up by type |
| `WINDOW.VIEWBY.ICON` | View by icons |
| `WINDOW.VIEWBY.NAME` | View by name |
| `WINDOW.VIEWBY.DATE` | View by date |
| `WINDOW.VIEWBY.SIZE` | View by size |
| `WINDOW.VIEWBY.TYPE` | View by type |
| `WINDOW.SHOWONLY.ALL` | Show all |
| `ICONS.OPEN` | Open icon |
| `ICONS.COPY` | Copy icon |
| `ICONS.RENAME` | Rename icon |
| `ICONS.INFORMATION` | Icon information |
| `ICONS.SNAPSHOT` | Snapshot icon position |
| `ICONS.UNSNAPSHOT` | Unsnapshot icon position |
| `ICONS.LEAVEOUT` | Leave out icon on Workbench |
| `ICONS.PUTAWAY` | Put away icon |
| `ICONS.DELETE` | Delete icon |
| `ICONS.FORMATDISK` | Format disk |
| `ICONS.EMPTYTRASH` | Empty trash |
| `TOOLS.RESETWB` | Reset Workbench |

```rexx
ADDRESS WORKBENCH
/* Open About dialog */
'MENU INVOKE WORKBENCH.ABOUT'

/* Clean up icons by name */
'MENU WINDOW "Work:" INVOKE WINDOW.CLEANUPBY.NAME'

/* Add custom menu item */
'MENU ADD NAME "MyTool" TITLE "My Tool" SHORTCUT "M" CMD "rx MyScript.rexx"'

/* Remove menu item */
'MENU REMOVE NAME "MyTool"'
```

---

## Keyboard Bindings

### KEYBOARD

Binds ARexx commands to keyboard shortcuts.

```
KEYBOARD NAME/A [ADD/S] [REMOVE/S] [KEY] [CMD/F]
```

| Parameter | Description |
|-----------|-------------|
| `NAME/A` | Unique identifier |
| `ADD/S` | Add new binding |
| `REMOVE/S` | Remove binding |
| `KEY` | Key combination (Commodities format) |
| `CMD/F` | ARexx script or command |

```rexx
ADDRESS WORKBENCH
/* Bind Ctrl+A to a script */
'KEYBOARD ADD NAME "MyHotkey" KEY "ctrl a" CMD "rx MyScript.rexx"'

/* Remove binding */
'KEYBOARD REMOVE NAME "MyHotkey"'
```

---

## Script Execution

### RX

Executes ARexx scripts and commands.

```
RX [CONSOLE/S] [ASYNC/S] CMD/A/F
```

| Parameter | Description |
|-----------|-------------|
| `CONSOLE/S` | Console for I/O |
| `ASYNC/S` | Execute asynchronously |
| `CMD/A/F` | Script name or command |

```rexx
ADDRESS WORKBENCH
'RX CMD "test.wb"'
'RX ASYNC CMD "long_script.rexx"'
```

---

## Example Scripts

### List All Open Windows

```rexx
/* ListWindows.rexx */
OPTIONS RESULTS
ADDRESS WORKBENCH

'GETATTR WINDOWS.COUNT'
count = RESULT
SAY count "windows open:"
SAY ""

DO i = 0 TO count - 1
    'GETATTR WINDOWS.'i
    window = RESULT
    'GETATTR WINDOW.WIDTH NAME "'window'"'
    w = RESULT
    'GETATTR WINDOW.HEIGHT NAME "'window'"'
    h = RESULT
    SAY window "(" w "x" h ")"
END
```

### Clean Up Root Window Icons by Name

```rexx
/* CleanupRoot.rexx */
ADDRESS WORKBENCH
'MENU WINDOW root INVOKE WINDOW.CLEANUPBY.NAME'
```

### Open and Position a Window

```rexx
/* OpenAndPosition.rexx */
ADDRESS WORKBENCH
'WINDOW "Work:" OPEN'
'CHANGEWINDOW "Work:" LEFTEDGE 50 TOPEDGE 50 WIDTH 500 HEIGHT 400'
'WINDOW "Work:" FRONT ACTIVATE'
```
