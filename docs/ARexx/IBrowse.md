# IBrowse 2 — ARexx Interface

## Overview

IBrowse is a web browser for AmigaOS with comprehensive ARexx control.

**Port Name:** `IBROWSE`

### Parameter Types

| Abbreviation | Meaning |
|---------|-----------|
| `/A` | Required parameter |
| `/S` | Switch (boolean) |
| `/N` | Numeric value |

### BROWSERNR

Many commands accept `BROWSERNR/N` — the browser ID shown in the title bar as `[ID]`. If omitted, the active browser is used.

### Error Codes (RC)

| RC | Meaning |
|----|---------|
| -2 | Out of memory |
| -3 | Unknown ARexx command |
| -4 | Syntax error |
| -5 | Unknown item |
| -6 | No browser active or invalid browser number |

---

## Standard MUI Commands

### QUIT

Exits IBrowse.

```
QUIT [FORCE/S]
```

| Parameter | Description |
|-----------|-------------|
| `FORCE/S` | Skip confirmation |

```rexx
ADDRESS IBROWSE
'QUIT FORCE'
```

### SHOW / ACTIVATE

Restores IBrowse from iconified state and brings it to the front.

```
SHOW
ACTIVATE
```

### HIDE / DEACTIVATE

Iconifies IBrowse.

```
HIDE
DEACTIVATE
```

### INFO

Returns program information.

```
INFO ITEM/A
```

| ITEM | Description |
|------|-------------|
| `TITLE` | Program name |
| `AUTHOR` | Author |
| `COPYRIGHT` | Copyright notice |
| `DESCRIPTION` | Description |
| `VERSION` | Version number |
| `BASE` | Base name |
| `SCREEN` | Screen name |

RC = -5 if ITEM is unknown.

```rexx
OPTIONS RESULTS
ADDRESS IBROWSE
'INFO ITEM=TITLE'
SAY RESULT
```

### HELP

Writes a list of all ARexx commands to a file.

```
HELP FILE/A
```

```rexx
ADDRESS IBROWSE
'HELP FILE=RAM:ibrowse_arexx.txt'
```

---

## Navigation

### GOTOURL

Loads a URL in the browser.

```
GOTOURL URL/A [BROWSERNR/N] [SAVE/S] [RELOAD/S] [MIME]
```

| Parameter | Description |
|-----------|-------------|
| `URL/A` | The URL to load |
| `BROWSERNR/N` | Target browser (default: active browser) |
| `SAVE/S` | Download file instead of displaying |
| `RELOAD/S` | Bypass cache, reload |
| `MIME` | Override MIME type manually |

RC = -6 if no browser is active.

```rexx
ADDRESS IBROWSE
'GOTOURL "http://www.amiga.org" BROWSERNR=1'
```

```rexx
/* Download a file */
'GOTOURL "http://example.com/file.lha" SAVE'
```

### BACK

Navigates back in history.

```
BACK [BROWSERNR/N]
```

### FORWARD

Navigates forward in history.

```
FORWARD [BROWSERNR/N]
```

### HOME

Loads the homepage (from settings).

```
HOME [BROWSERNR/N]
```

### STOP

Stops the current loading operation.

```
STOP [BROWSERNR/N]
```

### RELOAD

Reloads the current page.

```
RELOAD [BROWSERNR/N] [ALL/S] [FRAMES/S] [IMAGES/S]
```

| Parameter | Description |
|-----------|-------------|
| `ALL/S` | Reload everything |
| `FRAMES/S` | Reload frames only |
| `IMAGES/S` | Reload images only |

### LOADIMAGES

Loads images that have not been loaded yet.

```
LOADIMAGES [BROWSERNR/N]
```

---

## Windows and Browsers

### NEW

Opens a new window or browser (depending on settings).

```
NEW [URL]
```

Returns the number of the new browser/window in RESULT.

### NEWWINDOW

Opens a new window explicitly.

```
NEWWINDOW [URL]
```

Returns the new window number in RESULT.

### NEWBROWSER

Opens a new browser tab in an existing window.

```
NEWBROWSER [URL] [BROWSEWINDOWNR/N]
```

| Parameter | Description |
|-----------|-------------|
| `URL` | URL to load |
| `BROWSEWINDOWNR/N` | Target window (default: active window) |

Returns the new browser number in RESULT. RC = -6 if window number is invalid.

### CLOSEBROWSER

Closes a browser.

```
CLOSEBROWSER [BROWSERNR/N]
```

### SCREENTOFRONT / SCREENTOBACK

Brings the IBrowse screen to front or sends it to back.

```
SCREENTOFRONT
SCREENTOBACK
```

---

## Queries

### QUERY

Queries information about the current browser.

```
QUERY ITEM/A [BROWSERNR/N]
```

| ITEM | Description |
|------|-------------|
| `URL` | Current URL of the browser |
| `TITLE` | Page title |
| `ACTIVEBROWSERNR` | Number of the active browser |
| `ACTIVEWINDOWNR` | Number of the active window |

RC = -5 if ITEM is unknown, RC = -6 if no browser is active.

```rexx
OPTIONS RESULTS
ADDRESS IBROWSE
'QUERY ITEM=URL BROWSERNR=2'
SAY "URL:" RESULT

'QUERY ITEM=TITLE'
SAY "Title:" RESULT
```

---

## Bookmarks (Hotlist)

### ADDHOTLIST

Adds an entry to the Hotlist Manager.

```
ADDHOTLIST [GROUP/S] TITLE/A [URL] [SHORTCUT] [SHOWINMENU/N]
           [INSERTGROUP] [ACTIVATE/S]
```

| Parameter | Description |
|-----------|-------------|
| `GROUP/S` | Create a group (instead of a bookmark) |
| `TITLE/A` | Name of the entry |
| `URL` | URL (not needed for groups) |
| `SHORTCUT` | Keyboard shortcut |
| `SHOWINMENU/N` | 0=not in menu, >0=show in menu |
| `INSERTGROUP` | Target group (case insensitive) |
| `ACTIVATE/S` | Open Hotlist Manager and show entry |

```rexx
ADDRESS IBROWSE
'ADDHOTLIST TITLE="Amiga News" URL="http://www.amiga.org" INSERTGROUP="Amiga"'
```

```rexx
/* Create a new group */
'ADDHOTLIST GROUP TITLE="My Links"'
```

### ADDFASTLINK

Adds a fastlink button.

```
ADDFASTLINK TITLE/A [URL] [PROMPTTITLE/S]
```

| Parameter | Description |
|-----------|-------------|
| `TITLE/A` | Button title |
| `URL` | URL |
| `PROMPTTITLE/S` | Show input dialog |

### OPENHOTLIST

Opens or closes the Hotlist Manager.

```
OPENHOTLIST [CLOSE/S]
```

---

## Other Windows

### OPENHISTORY

Opens or closes the history window.

```
OPENHISTORY [BROWSERNR/N] [CLOSE/S]
```

### OPENCACHEBROWSER

Opens or closes the cache explorer.

```
OPENCACHEBROWSER [CLOSE/S]
```

### OPENINFOWINDOW

Opens or closes the info window.

```
OPENINFOWINDOW [CLOSE/S]
```

---

## Memory Management

### FLUSH

Frees memory.

```
FLUSH [BROWSERNR/N] [HISTORY/S] [ALLIMAGES/S] [CACHEDIMAGES/S] [IMAGES/S]
```

| Parameter | Description |
|-----------|-------------|
| `BROWSERNR/N` | Target browser (only relevant for HISTORY and IMAGES) |
| `HISTORY/S` | Clear browser history |
| `ALLIMAGES/S` | Remove all images from memory (ignores BROWSERNR) |
| `CACHEDIMAGES/S` | Remove cached images (ignores BROWSERNR) |
| `IMAGES/S` | Remove images of the browser |

```rexx
ADDRESS IBROWSE
'FLUSH HISTORY BROWSERNR=2'
'FLUSH ALLIMAGES'
```

---

## Example Scripts

### Load a URL

```rexx
/* LoadURL.rexx - Load URL in IBrowse */
PARSE ARG url
IF url = '' THEN DO
    SAY 'Usage: rx LoadURL.rexx <url>'
    EXIT 5
END

ADDRESS IBROWSE
'GOTOURL "'url'"'
IF RC ~= 0 THEN DO
    SAY 'Error: No browser active, opening new window...'
    'NEWWINDOW "'url'"'
END
```

### List All Open URLs

```rexx
/* ListURLs.rexx - Display URLs of all open browsers */
OPTIONS RESULTS
ADDRESS IBROWSE

DO i = 1 TO 20
    'QUERY ITEM=URL BROWSERNR='i
    IF RC = 0 THEN
        SAY 'Browser' i': ' RESULT
END
```

### Download a File

```rexx
/* Download.rexx - Download a file */
PARSE ARG url
ADDRESS IBROWSE
'GOTOURL "'url'" SAVE'
```
