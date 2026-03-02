# PageStream — ARexx Interface

## Overview

PageStream is a professional desktop publishing application for AmigaOS. It provides an extensive ARexx interface with over 400 commands.

**Port Name:** `PAGESTREAM` (or user-defined)

Source: https://www.pagestream.org/?action=Documents&id=630

### Parameter Types

| Abbreviation | Meaning |
|---------|-----------|
| `/i` | Integer |
| `/d` | Decimal (e.g. coordinates in inches/cm) |
| `/s` | String |
| `/k` | Keyword (from a predefined list) |
| `/S` | Switch |
| `/a` | Array/Stem variable |

### Common Parameters

Many commands accept these optional target specifiers:
- `PAGE number` — Target page (format: `document:...-pagenum` or simply page number)
- `MPG name` — Master page name and page
- `WINDOW name` — Target window (format: `document~window`)
- `DOCUMENT name` — Target document
- `CHAPTER name` — Target chapter (format: `document:chapter`)
- `OBJECTID number` — Target object (numeric ID)

### Common Transformation Parameters

Many drawing and object commands share these parameters:
- `ROTATE angle` — Rotation angle in degrees (default: 0)
- `SKEW slantangle twistangle` — Slant and twist (default: 0)
- `SLANT angle` — Slant only (default: 0)
- `TWIST angle` — Twist only (default: 0)
- `ABOUT pointx pointy` — Custom rotation point
- `ABOUTCENTER` — Rotate around object center (default)
- `CONSTRAIN` / `FREE` — Lock / free aspect ratio (default: FREE)
- `PRINT` / `NOPRINT` — Printable or not (default: PRINT)
- `INFRONT` / `INBACK` — Stacking order (default: INFRONT)

### Return Values

Results are returned in the ARexx variable `RESULT`.
Many commands return object handles (numeric IDs) that can be used for further operations.

---

## File Commands

### OpenDocument

Opens a document.

```
opendocument [ASK | FILE "file"] [FILTER filtername] [STATUS | NOSTATUS]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `ASK` | /S | Opens file selection dialog |
| `FILE` | /s | File name and path |
| `FILTER` | /k | Filter: `IFFDOC`, `PAGESTREAM2`, `PROPAGE`, `WORDWORTHDOC` |
| `STATUS` / `NOSTATUS` | /k | Progress indicator (default: NOSTATUS) |

```rexx
ADDRESS PAGESTREAM
'opendocument file "Work:Documents/Newsletter.doc"'
```

### Open

Opens a document and creates a window.

```
open [file] [FILTER ""] [optional module parameters]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `file` | /s | File path (omit = file selection dialog, since v5.0.3.4) |
| `FILTER` | /s | Filter specification |

```rexx
'open "PageStream:documents/Project.doc"'
```

### CloseDocument

Closes all views of a document and the document itself.

```
closedocument [mode] [DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `mode` | /k | `FORCE` (without save prompt), `ALERT` (dialog if changes exist), `QUIET` (default: silent, error on unsaved changes) |
| `DOCUMENT` | /s | Document name (default: current) |
| `WINDOW` | /s | Window name (default: current) |

```rexx
'closedocument force document "project.doc"'
```

### CloseWindow

Closes a single view. The document remains open.

```
closewindow [WINDOW name]
```

```rexx
'closewindow window "Untitled~View.1"'
```

### SaveDocument

Saves an open document.

```
savedocument [ASK | FILE "file" | DEFAULT] [FILTER filtername]
            [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
            [TEMPLATE] [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `ASK` | /S | File selection dialog |
| `FILE` | /s | File name and path |
| `DEFAULT` | /S | Default save location |
| `FILTER` | /k | Filter (default: IFFDOC) |
| `STATUS` / `NOSTATUS` | /k | Progress indicator |
| `QUIET` / `FORCE` / `ALERT` | /k | Overwrite behavior |
| `TEMPLATE` | /S | Save as template |

```rexx
'savedocument file "Work:Documents/Project.doc" force'
```

### RevertDocument

Reloads the last saved version of a document.

```
revertdocument [STATUS | NOSTATUS] [FORCE | ALERT]
              [FILTER filtername]
              [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `STATUS` / `NOSTATUS` | /k | Progress indicator (default: NOSTATUS) |
| `FORCE` / `ALERT` | /k | Confirmation mode (default: ALERT) |
| `FILTER` | /k | Filter: `IFFDOC`, `PAGESTREAM2`, `PROPAGE`, `WORDWORTHDOC` |

```rexx
'revertdocument status quiet'
```

### NewDocument

Creates a new document.

```
newdocument [name]
```

Returns the document name in RESULT. Does not create a default master page and does not open a window.

```rexx
'newdocument "My Project"'
docname = RESULT
```

### CollectOutput

Collects all files for output (since PageStream 4.0).

```
collectoutput [ASK | FILE "file"] [STATUS | NOSTATUS]
             [QUIET | FORCE | ALERT]
             [DOCUMENT document | WINDOW windowspec]
```

### CheckExternalLinks

Checks external links in the document.

```
checkexternallinks
```

---

### Import/Export

#### PlaceGraphic

Places a graphic file in a document.

```
placegraphic [FILE "file"] [FILTER filtername]
            [STATUS | NOSTATUS]
            [ASIS | AT x y | USER]
            [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `FILE` | /s | File name and path (omit = file selection dialog) |
| `FILTER` | /k | `PICT`, `TIFF`, `IFFILBM`, `GIF`, `BMP`, `IFFDR2D`, `IFFILUS`, `JPEG`, `MACPAINT`, `PRODRAW`, `PCX`, `ILLUSTRATOREPS`, `FREEHANDEPS`, `ARTEXPRESSIONEPS`, `EPS`, `PNG` |
| `STATUS` / `NOSTATUS` | /k | Progress indicator (default: NOSTATUS) |
| `ASIS` | /S | Original size and position |
| `AT x y` | /d | Center at position (default: window center) |
| `USER` | /S | User places manually |

```rexx
'placegraphic file "Work:Pictures/Logo.iff" filter iffilbm at 4.0 2.0'
```

#### InsertText

Inserts text from a file at the cursor position.

```
inserttext [FILE filepath] [FILTER name] [progress] [charset]
          [CONVERTQUOTE toggle] [CONVERTDASH toggle]
          [LINEHASLF toggle] [RETAINFORMAT toggle]
          [TEXTCODE code]
          [DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `FILE` | /s | Path to text file |
| `FILTER` | /k | `ASCII`, `IFFCTXT`, `IFFFTXT`, `WORDWORTH` |
| `CONVERTQUOTE` | /k | Convert typographic quotation marks |
| `CONVERTDASH` | /k | Convert em dashes |
| `LINEHASLF` | /k | Line break handling |
| `RETAINFORMAT` | /k | Retain formatting |

#### ExportGraphic

Exports a graphic object.

```
exportgraphic [FILE "file"] [FILTER filtername]
             [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
             [DOCUMENT document | WINDOW windowspec | ID id]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `FILE` | /s | File name (omit = file selection dialog) |
| `FILTER` | /k | `PICT`, `TIFF`, `IFFILBM`, `GIF`, `BMP`, `IFFDR2D`, `IFFILUS`, `JPEG`, `PNG`, `JPEG2000` |
| `STATUS` / `NOSTATUS` | /k | Progress indicator |
| `QUIET` / `FORCE` / `ALERT` | /k | Overwrite behavior |
| `ID` | /i | Object ID (default: current) |

```rexx
'exportgraphic file "Ram:Snapshot" filter iffilbm status'
```

#### ExportText

Exports the selected text.

```
exporttext [FILE "file"] [FILTER filtername]
          [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
          [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `FILTER` | /k | `ASCII`, `IFFFTXT`, `IFFCTXT`, `WORDWORTH`, `RTF` |

#### ExportPDF

Exports as PDF.

```
exportpdf [FILE "file"] [DOCUMENT document | WINDOW windowspec]
```

---

### Path Settings

#### SetDocumentPath

Sets the default document path.

```
setdocumentpath [filepath] [SAVE]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `filepath` | /s | The file path |
| `SAVE` | /S | Save to settings file |

```rexx
'setdocumentpath "PageStream3:Documents"'
```

#### SetGraphicPath

```
setgraphicpath [graphicpath] [SAVE]
```

```rexx
'setgraphicpath "Brilliance:Pictures"'
```

#### SetTextPath

```
settextpath [filepath] [SAVE]
```

```rexx
'settextpath "Work:Wordworth5/Documents"'
```

#### SetScratchPath

```
setscratchpath [scratchpath] [SAVE]
```

#### SetScriptPath (Amiga only)

```
setscriptpath [filepath] [SAVE]
```

```rexx
'setscriptpath "Rexx:"'
```

#### SetBackup

Configures backup and autosave settings.

```
setbackup [AUTOBACKUP status [COUNT number]]
         [AUTOSAVE status [INTERVAL time]]
         [PATH filepath] [SAVE]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `AUTOBACKUP` | /k | `ON`, `OFF`, `TOGGLE` |
| `COUNT` | /i | Number of backup copies |
| `AUTOSAVE` | /k | `ON`, `OFF`, `TOGGLE` |
| `INTERVAL` | /i | Autosave interval in minutes |
| `PATH` | /s | Backup directory |
| `SAVE` | /S | Save to settings file |

Note: `TOGGLE` must not be used together with `SAVE`.

```rexx
'setbackup autobackup on autosave on interval 5 save'
```

#### SetAutoColumns

```
setautocolumns [ON | OFF | TOGGLE] [LIKE MASTERPAGE | PREVIOUS] [SAVE]
```

#### SelectOnPaste

Automatically selects imported/pasted text.

```
selectonpaste [status] [SAVE]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `status` | /k | `ON`, `OFF`, `TOGGLE` |
| `SAVE` | /S | Save to settings file |

#### UseScratch

```
usescratch [TRUE | FALSE] [SAVE]
```

#### FileFilter

```
filefilter [TRUE | FALSE] [SAVE]
```

---

### File Queries

#### GetDocumentPath

```
getdocumentpath
```

Returns the current document path in RESULT.

#### GetGraphicPath

```
getgraphicpath
```

#### GetTextPath

```
gettextpath
```

#### GetScratchPath

```
getscratchpath
```

#### GetScriptPath

```
getscriptpath
```

#### GetAutoColumns

```
getautocolumns [LIKE &flag]
```

Returns `ON` or `OFF` in RESULT. `LIKE` receives `MASTERPAGE` or `PREVIOUS`.

#### GetSelectOnPaste

```
getselectonpaste
```

Returns `ON` or `OFF` in RESULT.

#### GetPasteInCenter

```
getpasteincenter
```

#### GetWindows

```
getwindows stem [DOCUMENT document | WINDOW windowspec]
```

Returns window count in RESULT. Window names in `stem.0`, `stem.1`, etc.

---

## Drawing Commands

### DrawBox

Draws a new rectangle.

```
drawbox <left top right bottom> [type]
       [CORNER radius | ECORNER radiusx radiusy]
       [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
       [ABOUT pointx pointy | ABOUTCENTER]
       [constraint] [printable] [stack]
       [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Box coordinates (required) |
| `type` | /k | `NORMAL`, `ROUND`, `SCALLOP`, `BEVEL`, `INSET` |
| `CORNER` | /d | Corner radius (default: 0) |
| `ECORNER` | /d | Elliptical corner radius (radiusx radiusy, default: 0) |

Returns object handle in RESULT.

```rexx
'drawbox 1.75 1.5 2.75 2.5'
'drawbox 3 3 6 6 inback page "project.doc~8"'
'drawbox 6 7 8 9 ecorner 0.1 0.2 skew 0 45'
```

### DrawEllipse

Draws a new ellipse.

```
drawellipse <centerx centery radiusx radiusy> [type]
           [ANGLES startangle endangle]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `centerx centery` | /d | Center coordinates (required) |
| `radiusx radiusy` | /d | Radius dimensions (required) |
| `type` | /k | `ELLIPSE` (default), `PIE`, `ARC` |
| `ANGLES` | /d | Start and end angles (for PIE/ARC) |

```rexx
'drawellipse 3 3 1.5 1.5'
'drawellipse 5.25 6.125 1.5 2.5 pie angles 45 90 print'
```

### DrawLine

Draws a new line.

```
drawline <left top right bottom>
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable] [stack]
        [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Line endpoint coordinates (required) |

```rexx
'drawline 1 1 5 5'
'drawline 2 1.25 8.5 11 page "project.doc~6" noprint'
```

### DrawPolygon

Draws a regular polygon.

```
drawpolygon <centerx centery radiusx radiusy> [type]
           [SIDES number] [OFFSETANGLE angle]
           [DEFLECTION amount] [ANGLEDEFLECTION amount]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `centerx centery` | /d | Center point (required) |
| `radiusx radiusy` | /d | Polygon radius (required) |
| `type` | /k | `NORMAL` (default), `STAR`, `PUFFY`, `SCALLOP`, `WAVY` |
| `SIDES` | /i | Number of sides (default: 5, must be >2) |
| `OFFSETANGLE` | /d | Pre-rotation angle (default: 0) |
| `DEFLECTION` | /d | Spike depth (default: 40%) |
| `ANGLEDEFLECTION` | /d | Spike angle (default: 0%) |

```rexx
'drawpolygon 4.0 5.0 2.0 2.0 STAR SIDES 6'
'drawpolygon 0.3 1.8 1.25 1.5 sides 8 puffy'
```

### DrawSpiral

Draws a spiral.

```
drawspiral x1 y1 ROUNDS n SPACING dist STARTANGLE angle
          [frame] [CONTENTOFFSET x y] [CONTENTSCALE x y]
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `x1 y1` | /d | Start coordinates |
| `ROUNDS` | /i | Number of revolutions |
| `SPACING` | /d | Spacing between spiral arms |
| `STARTANGLE` | /d | Start angle |

### DrawGrid

Creates a grid object.

```
drawgrid <pointx1 pointy1 pointx2 pointy2 |
         POINTS pointx1 pointy1 pointx2 pointy2 pointx3 pointy3 pointx4 pointy4>
        [DIVISIONS horz vert]
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable] [stack]
        [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `pointx1 pointy1 pointx2 pointy2` | /d | Rectangular grid (2 corner points) |
| `POINTS` | /d | Non-rectangular grid (4 corner points, counterclockwise) |
| `DIVISIONS` | /i | Grid subdivisions horizontal/vertical (default: 4x4) |

```rexx
'drawgrid 2 2 6 6 divisions 5 10'
'drawgrid points 1 1 1 3 5 6 2 3 page "MyDoc~6"'
```

### DrawTable

Creates a table object.

```
drawtable <left top right bottom> [ROWS rows] [COLUMNS columns]
         [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
         [ABOUT pointx pointy | ABOUTCENTER]
         [constraint] [printable] [stack]
         [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Table coordinates (required) |
| `ROWS` | /i | Number of rows |
| `COLUMNS` | /i | Number of columns |

### DrawColumn

Draws a new text frame with one or more columns.

```
drawcolumn <left top right bottom> [COLUMNS number] [GUTTER space]
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Frame coordinates (required) |
| `COLUMNS` | /i | Number of columns |
| `GUTTER` | /d | Column spacing |

Note: For multi-page or size-dependent columns, use `CreateColumns`.

```rexx
'drawcolumn 1 1 7.5 10 columns 2 gutter 0.25'
```

### DrawTextObj

Creates a new text block.

```
drawtextobj <left top>
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top` | /d | Top-left corner of the text block (required) |

```rexx
'drawtextobj 1 1'
'drawtextobj 1 1 rotate 45 inback page "project.doc~8"'
```

### DrawPicture

Places a picture frame.

```
drawpicture <left top right bottom>
           [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [DPI xdpi ydpi] [frame] [stored]
           [FILE filepath]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
           [FPO {DEFAULT|FINE|MEDIUM|COARSE|CUSTOM xdpi ydpi}]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Frame coordinates (required) |
| `CONTENTOFFSET` | /d | Offset within the frame (default: 0,0) |
| `CONTENTSCALE` | /d | Scale within the frame (default: 100%) |
| `DPI` | /d | Resolution (xdpi ydpi) |
| `frame` | /k | `FRAMED` or `FRAMELESS` |
| `stored` | /k | `INTERNAL` (default) or `EXTERNAL` |
| `FILE` | /s | Image file path |
| `FPO` | /k | Resolution: `DEFAULT`, `FINE`, `MEDIUM`, `COARSE`, `CUSTOM xdpi ydpi` |

```rexx
'drawpicture 0.5 0.5 2.5 3 dpi 150 150 external'
```

### DrawEPS

Places an EPS frame.

```
draweps <left top right bottom>
       [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
       [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
       [ABOUT pointx pointy | ABOUTCENTER]
       [constraint] [printable] [stack] [frame] [stored]
       [FILE filepath]
       [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `left top right bottom` | /d | Frame coordinates (required) |
| `CONTENTOFFSET` | /d | Offset (default: 0,0) |
| `CONTENTSCALE` | /d | Scale (default: 100%) |
| `frame` | /k | `FRAMED` or `FRAMELESS` |
| `stored` | /k | `INTERNAL` (default) or `EXTERNAL` |
| `FILE` | /s | EPS file path |

```rexx
'draweps 1.75 1.5 2.75 2.5'
'draweps 3 3 6 6 offset -2 2 contentscale 80 constrain'
```

---

### Path Commands

#### CreatePath

Creates a new path.

```
createpath <left top right bottom>
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE number | WINDOW name]
```

Returns object handle in RESULT.

```rexx
'createpath 1.5 2.5 3.25 6.457'
```

#### AddPoint

Adds a point to the current path.

```
addpoint <MOVETO pointx pointy [join] |
         LINETO pointx pointy [join] |
         CURVETO curvex1 curvey1 curvex2 curvey2 pointx pointy [join] |
         CLOSEPATH>
        [PAGE number | WINDOW name | OBJECTID number]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `MOVETO` | /d | First point of a new path/subpath |
| `LINETO` | /d | Line to the specified point |
| `CURVETO` | /d | Bezier curve (control point 1, control point 2, endpoint) |
| `CLOSEPATH` | /S | Closes the open path |
| `join` | /k | `CORNERJOIN` (default) or `SMOOTHJOIN` |

Note: The join type only affects interaction with the Reshape tool, not the coordinates.

```rexx
'addpoint moveto 1.0 2.0'
'addpoint lineto 5.0 2.0'
'addpoint curveto 5.0 4.0 3.0 6.0 1.0 4.0 smoothjoin'
'addpoint closepath'
```

#### CreatePoint

Adds a point to an existing path at the nearest location.

```
createpoint <NEAR x y> [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'createpoint near 5.1 6.25'
```

#### EditPoint

Edits the coordinates of a path point.

```
editpoint [ANCHOR x y] [CONTROLBEFORE x y] [CONTROLAFTER x y]
         [join] [POINTINDEX point]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `ANCHOR` | /d | Point coordinates |
| `CONTROLBEFORE` | /d | Curve handle before the point |
| `CONTROLAFTER` | /d | Curve handle after the point |
| `join` | /k | `CORNERJOIN` or `SMOOTHJOIN` |
| `POINTINDEX` | /i | Point number (default: selected). Numbering starts at 0 from the first MOVETO. |

#### MovePoint

Moves selected path points.

```
movepoint [OFFSET x y] [POINTINDEX point]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### NudgePoint

Moves path points by the configured nudge value.

```
nudgepoint <[horz] [vert]> [TAP] [POINTINDEX point]
          [DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `horz` | /k | `LEFT` or `RIGHT` |
| `vert` | /k | `UP` or `DOWN` |
| `TAP` | /S | 1/10 of the normal nudge value |

#### DeletePoint

Deletes the selected path point.

```
deletepoint [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SplitPoint

Splits a path at a point. For open paths: two subpaths; for closed paths: an open path.

```
splitpoint [POINTINDEX point] [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'splitpoint pointindex 23'
```

#### ClosePath

Closes the selected open path.

```
closepath [PAGE number | WINDOW name | OBJECTID number]
```

#### MergeSubPaths

Merges subpaths within a path object.

```
mergesubpaths [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### SnapToGrid

Changes the grid snap status.

```
snaptogrid [status] [WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `status` | /k | `ON`, `OFF`, `TOGGLE` |

Note: Grid snapping only affects the user interface, not script commands.

---

### Media Commands

#### EditPicture

Sets coordinates and options of a picture frame.

```
editpicture [POSITION left top right bottom [SCALECONTENT]]
           [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [frame] [stored]
           [FILE filepath] [DPI xdpi ydpi]
           [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### EditEPS

Sets coordinates and options of an EPS frame.

```
editeps [POSITION left top right bottom [SCALECONTENT]]
       [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
       [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
       [ABOUT pointx pointy | ABOUTCENTER]
       [constraint] [printable] [frame] [stored]
       [FILE filepath]
       [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'editeps position 1.75 1.5 2.75 2.5'
'editeps offset -2 -2 contentscale 80 constrain'
```

#### EditLine

Sets coordinates of a line.

```
editline [POSITION x1 y1 x2 y2 [ADJUST]]
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable]
        [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### DissolveEPS

Separates an EPS object and its bitmap preview into two separate objects.

```
dissolveeps [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SetEPS

Sets the name and description of an EPS object.

```
seteps [NAME name] [DESC description]
      [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'seteps name "CorporateLogo"'
```

#### GetEPS (Query)

Queries properties of an EPS object.

```
geteps [POSITION &coord] [NAME &name] [DESC &desc]
      [AUTHOR &author] [COPYRIGHT &copyright]
      [FILEINFO &filestem] [FRAME &flag]
      [CONTENTOFFSET &offset] [CONTENTSCALE &scale]
      [CONTENTROTATION &rotation] [ROTATION &rotation]
      [ABOUT &mode] [CONSTRAIN &flag] [PRINT &flag]
      [DRAWPAGE &n] [DRAWPAGECOUNT &n]
      [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

| Result | Description |
|----------|-------------|
| `POSITION` | stem: .LEFT, .TOP, .RIGHT, .BOTTOM |
| `FILEINFO` | stem: .MODE (INTERNAL\|EXTERNAL), .FILE, .FORMAT, .DATE, .TIME |
| `FRAME` | `ON` or `OFF` |
| `CONTENTOFFSET` | stem: .X, .Y |
| `CONTENTSCALE` | stem: .H, .V |
| `ROTATION` | stem: .SLANT, .TWIST |
| `ABOUT` | stem: .MODE (POINT\|CENTER), .X, .Y |

---

### Compound Objects

#### CreateCompound

Combines selected shapes into a compound object.

```
createcompound [POSITION left top right bottom]
              [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
              [ABOUT pointx pointy | ABOUTCENTER]
              [constraint] [printable] [stack]
              [DOCUMENT name | WINDOW name]
```

Returns object handle in RESULT.

```rexx
'createcompound'
'createcompound position 1.5 2.5 3.25 6.4 constrain'
```

#### CreateDrawing

Creates a new drawing object.

```
createdrawing [POSITION left top right bottom]
             [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
             [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
             [ABOUT pointx pointy | ABOUTCENTER]
             [constraint] [printable] [stack] [frame]
             [DOCUMENT name | WINDOW name]
             [DRAWING | LAYER] [VISIBLE | INVISIBLE] [EDITABLE | NONEDITABLE]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `DRAWING` / `LAYER` | /k | Drawing type |
| `VISIBLE` / `INVISIBLE` | /k | Layer visibility (when LAYER) |
| `EDITABLE` / `NONEDITABLE` | /k | Layer editability (when LAYER) |

```rexx
'createdrawing'
'createdrawing position 1.5 2.5 3.25 6.457 contentscale 80.5 constrain'
```

#### ApplyAttributes

Applies recorded attributes to a paragraph or object.

```
applyattributes <AT pointx pointy [stack] | RANGE left top right bottom [stack]>
               [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `AT` | /d | Apply attributes to paragraph at position |
| `RANGE` | /d | Apply attributes to all paragraphs in range |
| `stack` | /k | `FRONTMOST` (default) or `BACKMOST` |

```rexx
'applyattributes at 1.25 3.3 backmost'
'applyattributes range 1.25 3.3 1.75 5.25'
```

---

## Document Commands

### Pages

#### InsertPage

Inserts page(s).

```
insertpage [where] [PAGE number | DOCUMENT name | WINDOW name]
          [COUNT number] [INSPREAD [type]]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `where` | /k | `BEFORE` (default) or `AFTER` |
| `COUNT` | /i | Number of pages (default: 1) |
| `INSPREAD` | /k | `DEFAULT`, `HORIZONTAL`, `VERTICAL` |

#### DeletePage

Deletes page(s).

```
deletepage [PAGE number] [TO number] [mode]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `PAGE` | /s | First page to delete (default: current) |
| `TO` | /s | Last page of a range |
| `mode` | /k | `FORCE`, `ALERT`, `QUIET` (default) |

```rexx
'deletepage page 5 to 7 force'
```

#### MovePage

Moves page(s) within a document.

```
movepage [PAGE number | DOCUMENT name | WINDOW name]
        [TO pagenum] <BEFORE document:...-pagenum | AFTER document:...-pagenum>
        [INSPREAD [DEFAULT | HORIZONTAL | VERTICAL]]
```

Note: Pages cannot be moved outside their chapter or document.

#### MakePageSpread

Creates a page spread from two or more pages.

```
makepagespread [PAGE number | DOCUMENT name | WINDOW name]
              [TO number] [INSPREAD [HORIZONTAL | VERTICAL]]
```

To dissolve a spread, omit `TO`.

```rexx
'makepagespread page 1 to 3 vertical'
'makepagespread page 1'  /* dissolves the spread */
```

#### SetPageDesc / SetPageName / SetPageNumbering

```
setpagedesc text [PAGE number]
setpagename newname [PAGE name]
setpagenumbering [START] [MASTERPAGE name] [FORMAT type] [LANGUAGE name]
                [PREFIX string] [DOCUMENT name | CHAPTER name | WINDOW name]
```

FORMAT: `DEFAULT`, `SLONG`, `ARABIC`, `ROMANUPPER`, `ROMANLOWER`, `ALPHAUPPER`, `ALPHALOWER`
LANGUAGE: `DEFAULT`, `AMERICAN`, `ENGLISH`, `FRANCAIS`, `DEUTSCH`

---

### Chapters

#### NewChapter

```
newchapter chaptername chapter# [PAGESFROMDOCUMENT]
          [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

Returns the new chapter name in RESULT.

```rexx
'newchapter "Part Three" 7'
```

#### DeleteChapter

```
deletechapter [CHAPTER name | WINDOW name] [ALERT]
```

#### MoveChapter

```
movechapter [CHAPTER name | WINDOW name] <BEFORE newchap# | AFTER newchap# | ATEND>
           [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

#### SetChapterDesc / SetChapterName / SetChapterNumbering

```
setchapterdesc newdesc [CHAPTER document:... | WINDOW document-window]
setchaptername name [CHAPTER name | WINDOW name]
setchapternumbering [START] [MASTERPAGE name] [FORMAT type] [LANGUAGE name]
                   [PREFIX string] [DOCUMENT name | CHAPTER name | WINDOW name]
```

---

### Master Pages

#### NewMasterPage

```
newmasterpage masterpagename pagew pageh [PORTRAIT | LANDSCAPE]
            [SINGLE | DOUBLE] [FACING | INDIVIDUAL]
            [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

```rexx
'newmasterpage "Columnar" 8.5 11 landscape double'
```

#### DeleteMasterpage

```
deletemasterpage [MASTERPAGE name]
```

#### MoveMasterpage

```
movemasterpage [MASTERPAGE name] [CHAPTER name]
```

#### SelectMasterpage

Assigns a master page to a page or page range.

```
selectmasterpage [NAME masterpagename] [PAGE number] [stack] [status]
                [RIPPLE | TO number]
                [DOCUMENT document | WINDOW document-window]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `NAME` | /s | Master page name |
| `stack` | /k | `INFRONT` or `INBACK` |
| `status` | /k | `SHOW` or `HIDE` |
| `RIPPLE` | /S | Apply to all following pages |
| `TO` | /s | Up to page |

```rexx
'selectmasterpage name "Default MPage" page 5 inback'
'selectmasterpage name "2 Column Layout" ripple'
```

#### SetMasterpageDesc / SetMasterpageName

```
setmasterpagedesc text [MASTERPAGE name]
setmasterpagename newname [MASTERPAGE oldname]
```

---

### Page Dimensions and Layout

#### SetDimensions

```
setdimensions pagew pageh [PORTRAIT | LANDSCAPE] [SINGLE | DOUBLE]
             [FACING | INDIVIDUAL] [SAVE]
             [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

#### SetBleed

```
setbleed bleedh bleedv [SAVE]
        [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

#### SetColumnGuides

```
setcolumnguides columns gutter [SAVE] [MASTERPAGE name]
```

```rexx
'setcolumnguides 3 0.25 masterpage "3 Column Layout"'
```

#### SetMarginGuides

```
setmarginguides inside outside top bottom [SAVE] [MASTERPAGE name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `inside` | /d | Inside/left margin |
| `outside` | /d | Outside/right margin |
| `top` | /d | Top margin |
| `bottom` | /d | Bottom margin |

```rexx
'setmarginguides 1 0.75 1 1.25 masterpage "Layout2"'
```

---

### Guides

#### AddGuides

```
addguides [type] <AT position> [MPG name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `type` | /k | `HORIZONTAL`, `VERTICAL`, `BOTH` |
| `AT` | /d | Position(s) of the guide(s) |
| `MPG` | /s | Master page name and page |

```rexx
'addguides vertical at 2.5 mpg left'
'addguides horizontal at 2.5 3.5'
```

#### DeleteGuides

```
deleteguides [type] <AT position> [MPG name]
```

#### SetGuides

```
setguides <VERTICAL | HORIZONTAL | BOTH> [MPG name] AT position1 position2 ...
```

#### SetGuide (Snap Settings)

```
setguide [SNAPALL | SNAPRANGE rangex rangey] [MASTERPAGE name]
```

#### SnapToGuides

```
snaptoguides [status] [PAGE status] [RULER status] [WINDOW name]
```

---

### Grid and Display

#### SetGrid

```
setgrid [SNAP gridh gridv] [SNAPOFFSET gridxoffset gridyoffset]
       [SNAPALL | SNAPRANGE rangex rangey]
       [DISPLAYINTERVAL gridh gridv] [DISPLAYOFFSET gridxoffset gridyoffset]
       [SAVE]
       [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

#### DisplayGrid

```
displaygrid <status> [stack] [COLOR red green blue] [WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `status` | /k | `SHOW`, `HIDE`, `TOGGLE` (required) |
| `stack` | /k | `INFRONT` or `INBACK` (default: INBACK) |
| `COLOR` | /i | RGB values, 0-255 |

```rexx
'displaygrid show infront'
'displaygrid color 0 255 255'
```

#### DisplayGuides

```
displayguides <status> [stack] [RULER status] [RULERCOLOR r g b]
             [PAGE status] [PAGECOLOR r g b] [WINDOW name]
```

#### DisplayRuler

```
displayruler <status> [OFFSET offsetx offsety] [ZERO zerox zeroy]
            [MSYS horzunits vertunits] [DIRECTION horz vert] [WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `status` | /k | `SHOW`, `HIDE`, `TOGGLE` (required) |
| `OFFSET` | /d | Ruler offset in pixels |
| `ZERO` | /d | Zero point coordinates |
| `MSYS` | /k | `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPICAS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `FEET`, `METERS`, `SAMEAS` |
| `DIRECTION` | /k | Horizontal: `LEFT`\|`RIGHT`, Vertical: `UP`\|`DOWN` |

```rexx
'displayruler show offset 30 120 msys inches inches'
```

---

### Revision Management

#### LogRevision

```
logrevision [DESCRIPTION text] [VERSION major minor | BUMPREV]
           [USER username] [CHAPTER name | PAGE number | MASTERPAGE name]
```

```rexx
'logrevision bumprev'
'logrevision description "Fixed colors" user "Colleen"'
```

#### SetRevision (since v5.0.3.4)

```
setrevision [DESCRIPTION description] [BUMPMINOR | BUMPMAJOR | VERSION major minor]
           [CREATED | MODIFIED] [USER username] [REVISION revh]
```

#### DeleteRevision (since v5.0.3.4)

```
deleterevision [REVISION revh]
```

#### SetRevisionTracking

```
setrevisiontracking [ON | OFF | TOGGLE] [SAVE]
```

#### SetDocumentStatus

```
setdocumentstatus [CHANGED | UNCHANGED] [DOCUMENT document | WINDOW document-window]
```

---

### Screen Refresh

#### Refresh

Toggles screen refresh for faster script execution.

```
refresh [ON | OFF | WAIT | CONTINUE] [ALL | DOCUMENT document | WINDOW document-window]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `OFF` | /k | Refresh off, without tracking changes |
| `WAIT` | /k | Refresh off, tracks what needs to be refreshed |
| `ON` | /k | Refresh on, discards all tracked refreshes |
| `CONTINUE` | /k | Refresh on, performs tracked refreshes |

Typically used together with `LockInterface` for better performance.

---

### Document Queries

#### CurrentDocument / CurrentWindow / CurrentPage / CurrentChapter / CurrentMasterpage

```
currentdocument [WINDOW name]
currentwindow [WINDOW name]
currentpage [WINDOW name]
currentchapter [DOCUMENT name | WINDOW name]
currentmasterpage [DOCUMENT name | WINDOW name]
```

All return the name/number in RESULT.

The `Path` variants return the full path:
```
currentwindowpath       /* Format: documentname~windowname */
currentpagepath         /* Format: document[:chapter]~pagenum */
currentchapterpath      /* Format: document[:chapter[:subchapter]] */
currentmasterpagepath   /* Format: document[:chapter~]masterpage */
```

#### GetDocuments

```
getdocuments stem
```

Returns document count in RESULT. Names in `stem.0`, `stem.1`, etc.

#### GetDocumentDesc / GetDocumentAuthor / GetDocumentCopyright

```
getdocumentdesc [DOCUMENT name]
getdocumentauthor [DOCUMENT document | WINDOW document-window]
getdocumentcopyright [DOCUMENT document | WINDOW document-window]
```

#### GetDocumentStatus

```
getdocumentstatus [DOCUMENT name]
```

Returns `changed` or `unchanged` in RESULT.

#### GetChapters / GetChapterDesc / GetChapterNumber / GetChapterNumbering

```
getchapters stem [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
getchapterdesc [DOCUMENT name | CHAPTER name]
getchapternumber [DOCUMENT name | CHAPTER name]
getchapternumbering stem [DOCUMENT name | CHAPTER name | WINDOW name]
```

GetChapterNumbering results: `stem.startmode`, `stem.start`, `stem.format`, `stem.language`, `stem.prefix`

#### GetMasterpages / GetMasterpageDesc

```
getmasterpages stem [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
getmasterpagedesc [MASTERPAGE name]
```

#### GetPageDesc / GetPageName / GetPageNumber / GetPageMasterpage

```
getpagedesc [PAGE number]
getpagename [PAGE number]
getpagenumber [PAGE document:...-pagenum | DOCUMENT document | WINDOW document-window]
getpagemasterpage [MASTERPAGE name] [SIDE name] [DEPTH level]
                 [PAGE number | DOCUMENT name | WINDOW name]
```

#### GetPageNumbering

```
getpagenumbering stem [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

Results: `stem.startmode`, `stem.start`, `stem.lengthmode`, `stem.length`, `stem.masterpage`, `stem.format`, `stem.language`, `stem.prefix`

#### GetDimensions

```
getdimensions stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Results: `stem.width`, `stem.height`, `stem.orientation` (PORTRAIT\|LANDSCAPE), `stem.spreads` (FACING\|INDIVIDUAL), `stem.sides` (SINGLE\|DOUBLE)

#### GetDefaultPageSizes

```
getdefaultpagesizes namestem [measurement] [custom]
                   [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `measurement` | /k | `STANDARD` (default) or `USER` |
| `custom` | /k | `NOCUSTOM` (default) or `CUSTOM` |

Results: Each entry has `.name`, `.width`, `.height`.

#### GetBleed

```
getbleed stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Results: `stem.H`, `stem.V`

#### GetColumnGuides

```
getcolumnguides stem [MASTERPAGE name]
```

Results: `stem.count`, `stem.gutter`

#### GetMarginGuides

```
getmarginguides stem [MASTERPAGE name]
```

Results: `stem.inside`, `stem.outside`, `stem.top`, `stem.bottom`

```rexx
'getmarginguides info'
SAY 'Inside margin:  ' || info.inside
SAY 'Outside margin: ' || info.outside
```

#### GetGrid

```
getgrid stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Results: `stem.h`, `stem.v`, `stem.x`, `stem.y`, `stem.snap` (ALL\|RANGE), `stem.rangeh`, `stem.rangev`, `stem.displayh`, `stem.displayv`, `stem.displayx`, `stem.displayy`

#### GetGridDisplay / GetGridSnap

```
getgriddisplay [DEPTH layer] [COLOR stem] [WINDOW name]
getgridsnap [WINDOW name]
```

#### GetGuide / GetGuides / GetGuideDisplay / GetGuideSnap

```
getguide stem [MASTERPAGE name]         /* stem.snap, stem.rangeh, stem.rangev */
getguides stem [type] [MPG name]        /* Guide positions in stem.0, stem.1, ... */
getguidedisplay [DEPTH layer] [PAGE flag] [PAGECOLOR stem] [RULER flag] [RULERCOLOR stem] [WINDOW name]
getguidesnap [WINDOW name]
```

#### GetRulerDisplay

```
getrulerdisplay [OFFSET stem] [ZERO stem] [MSYS stem] [DIRECTION stem] [WINDOW name]
```

Results: OFFSET: `.x`, `.y`; ZERO: `.x`, `.y`; MSYS: `.h`, `.v`; DIRECTION: `.h` (LEFT\|RIGHT), `.v` (UP\|DOWN)

#### GetRevisions / GetRevision (since v5.0.3.4)

```
getrevisions revhlist [DOCUMENT document | CHAPTER document:... | PAGE ... | MASTERPAGE ... | WINDOW ...]
getrevision [DESCRIPTION &desc] [VERSION &version] [TYPE &type] [USER &username] [REVISION revh]
```

---

## Object Commands

### Selection and Navigation

#### SelectObject

Selects an object.

```
selectobject <AT pointx pointy [stack] | RANGE left top right bottom [stack] |
             ALL | NONE | INVERT | NEXT | PREVIOUS |
             LINKS | CHAINFIRST | CHAINLAST>
            [PAGE number | MPG name | DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `AT` | /d | Object at position |
| `RANGE` | /d | All objects in range |
| `ALL` | /S | All objects |
| `NONE` | /S | Deselect |
| `INVERT` | /S | Invert selection |
| `NEXT` / `PREVIOUS` | /S | Next/previous object |
| `LINKS` | /S | Linked objects |
| `CHAINFIRST` / `CHAINLAST` | /S | First/last element of the chain |
| `stack` | /k | `FRONTMOST` (default) or `BACKMOST` |

```rexx
'selectobject at 3.5 5.0'
'selectobject range 1 1 7 10'
'selectobject all'
```

#### SelectPoint

```
selectpoint <AT pointx pointy | ALL | NONE | INVERT | NEXT | PREVIOUS>
           [POINTINDEX point]
           [PAGE number | DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SelectCell

```
selectcell [ROW row] [COLUMN column]
          [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### SelectActiveLayer

```
selectactivelayer [layerindex | layername]
                 [DOCUMENT name | WINDOW name]
```

#### GetSelectedObjects

```
getselectedobjects stem [DOCUMENT document | WINDOW document-window]
```

Returns count in RESULT. Object IDs in `stem.0`, `stem.1`, etc.

---

### Transformation

#### Move

```
move <OFFSET offsetx offsety | TO x y>
    [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Nudge

```
nudge <[horz] [vert]> [TAP]
     [DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `horz` | /k | `LEFT` or `RIGHT` |
| `vert` | /k | `UP` or `DOWN` |
| `TAP` | /S | 1/10 of the normal nudge value |

#### Rotate

```
rotate angle [ABOUT pointx pointy | ABOUTCENTER]
            [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### ScaleObject

```
scaleobject <SCALE scalex scaley | SIZE newwidth newheight>
           [ABOUT pointx pointy | ABOUTCENTER]
           [CONSTRAIN | FREE]
           [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Skew

```
skew <SLANT angle | TWIST angle | SKEW slant twist>
    [ABOUT pointx pointy | ABOUTCENTER]
    [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Transform

General transformation.

```
transform [POSITION left top right bottom]
         [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
         [ABOUT pointx pointy | ABOUTCENTER]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Align

```
align [horz] [vert] [scope]
     [DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `horz` | /k | `LEFTALIGN`, `CENTERALIGN`, `RIGHTALIGN` |
| `vert` | /k | `TOPALIGN`, `MIDDLEALIGN`, `BOTTOMALIGN` |
| `scope` | /k | `TOPAGE` (default), `TOMARGINS`, `TOSELECTION` |

#### Distribute

```
distribute [HORZEVEN | HORZSPACE space | HORZNONE]
          [VERTEVEN | VERTSPACE space | VERTNONE]
          [DOCUMENT name | WINDOW name]
```

#### Duplicate

```
duplicate [OFFSET offsetx offsety] [COPIES number]
         [ROTATE angle] [SCALE scalex scaley]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### MoveToPage

```
movetopage [PAGE number | DOCUMENT name | WINDOW name]
```

---

### Stacking Order

```
bringtofront [DOCUMENT name | WINDOW name | OBJECTID number]
bringforward [DOCUMENT name | WINDOW name | OBJECTID number]
sendtoback [DOCUMENT name | WINDOW name | OBJECTID number]
sendbackward [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Editing

#### CopyObject / CutObject / PasteObject / DeleteObject

```
copyobject [DOCUMENT name | WINDOW name | OBJECTID number]
cutobject [DOCUMENT name | WINDOW name | OBJECTID number]
pasteobject [PAGE number | MPG name | DOCUMENT name | WINDOW name]
deleteobject [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### EditObject

```
editobject [POSITION left top right bottom [SCALECONTENT | ADJUST]]
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable]
          [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Lock / Unlock

```
lock [DOCUMENT name | WINDOW name | OBJECTID number]
unlock [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Groups and Compound Objects

#### CreateGroup / DissolveGroup

```
creategroup [DOCUMENT name | WINDOW name]
dissolvegroup [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### CreateColumns

```
createcolumns [COLUMNS number] [GUTTER space]
             [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### EditCompound / DissolveCompound / DissolveObject / DissolveDrawing

```
editcompound [DOCUMENT name | WINDOW name | OBJECTID number]
dissolvecompound [DOCUMENT name | WINDOW name | OBJECTID number]
dissolveobject [DOCUMENT name | WINDOW name | OBJECTID number]
dissolvedrawing [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Path Operations

```
converttopath [DOCUMENT name | WINDOW name | OBJECTID number]
mergepaths [DOCUMENT name | WINDOW name]
splitpaths [DOCUMENT name | WINDOW name | OBJECTID number]
flattenpath [DOCUMENT name | WINDOW name | OBJECTID number]
smoothpath [DOCUMENT name | WINDOW name | OBJECTID number]
simplifypath [DOCUMENT name | WINDOW name | OBJECTID number]
reversepath [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Boolean Path Operations

```
andpaths [DOCUMENT name | WINDOW name]     /* Intersection */
orpaths [DOCUMENT name | WINDOW name]      /* Union */
subpaths [DOCUMENT name | WINDOW name]     /* Subtraction */
xorpaths [DOCUMENT name | WINDOW name]     /* Exclusive Or */
```

---

### Masks

```
makemask [DOCUMENT name | WINDOW name | OBJECTID number]
releasemask [DOCUMENT name | WINDOW name | OBJECTID number]
clearmask [DOCUMENT name | WINDOW name | OBJECTID number]
```

```
makedrawingmask [DOCUMENT name | WINDOW name | OBJECTID number]
releasedrawingmask [...]
cleardrawingmask [...]
```

```
makepicturemask [DOCUMENT name | WINDOW name | OBJECTID number]
releasepicturemask [...]
clearpicturemask [...]
```

```
makeepsmask [DOCUMENT name | WINDOW name | OBJECTID number]
releaseepsmask [...]
clearepsmask [...]
```

```
generatemask [DOCUMENT name | WINDOW name | OBJECTID number]
generatepicturemask [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Text Wrap

#### SetTextWrap / GetTextWrap

```
settextwrap [WRAP mode] [OFFSET offset]
           [DOCUMENT name | WINDOW name | OBJECTID number]
gettextwrap [WRAP &mode] [OFFSET &offset]
           [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Tables

#### SetTable / GetTable

```
settable [ROWS rows] [COLUMNS columns] [CELLGUTTER gutter]
        [DOCUMENT name | WINDOW name | OBJECTID number]
gettable [ROWS &rows] [COLUMNS &columns] [CELLGUTTER &gutter]
        [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SetCell / GetCell

```
setcell [ROW row] [COLUMN column] [COLSPAN columns] [ROWSPAN rows]
       [DOCUMENT name | WINDOW name | OBJECTID number]
getcell [ROW row] [COLUMN column] [COLSPAN &columns] [ROWSPAN &rows]
       [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### InsertCellColumn / DeleteCellColumn / InsertCellRow / DeleteCellRow

```
insertcellcolumn [COLUMN column] [DOCUMENT name | WINDOW name | OBJECTID number]
deletecellcolumn [COLUMN column] [DOCUMENT name | WINDOW name | OBJECTID number]
insertcellrow [ROW row] [DOCUMENT name | WINDOW name | OBJECTID number]
deletecellrow [ROW row] [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### JoinCell / BreakCell / ClearCell

```
joincell [DOCUMENT name | WINDOW name | OBJECTID number]
breakcell [DOCUMENT name | WINDOW name | OBJECTID number]
clearcell [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Object Queries

#### GetObject

```
getobject [TYPE &type] [POSITION &coord] [NAME &name]
         [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

Returns object type, position (stem: .LEFT, .TOP, .RIGHT, .BOTTOM) and name.

#### GetPath / GetPathPoint / GetPathPoints

```
getpath [POINTS &count] [SUBPATHS &count] [CLOSED &flag]
       [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getpathpoint stem [POINTINDEX point]
            [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getpathpoints stem [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### GetBox / GetEllipse / GetLine / GetPolygon / GetGrid

```
getbox [TYPE &type] [CORNER &radius] [ECORNER &stem]
      [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getellipse [TYPE &type] [ANGLES &stem] [CENTER &stem] [RADIUS &stem]
          [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getline [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getpolygon [TYPE &type] [SIDES &sides] [OFFSETANGLE &angle]
          [DEFLECTION &amount] [ANGLEDEFLECTION &amount]
          [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getgridobject [ROWS &rows] [COLUMNS &columns]
             [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### GetPicture / GetDrawing / GetGroup / GetCompound

```
getpicture [POSITION &coord] [FILEINFO &stem] [DPI &stem] [FRAME &flag]
          [CONTENTOFFSET &stem] [CONTENTSCALE &stem] [FPO &stem]
          [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getdrawing [POSITION &coord] [CONTENTOFFSET &stem] [CONTENTSCALE &stem]
          [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getgroup [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getcompound [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### GetColumn / GetTextObj / GetTextFrame

```
getcolumn [COLUMNS &n] [GUTTER &gutter]
         [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
gettextobj [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
gettextframe [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### GetRotation / GetObjectLock

```
getrotation [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
getobjectlock [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

---

## Text Commands

### Text Editing

#### EditText

Places the cursor in a text frame.

```
edittext [AT pointx pointy [stack] | OBJECTID number]
        [PAGE number | DOCUMENT name | WINDOW name]
```

#### Insert

Inserts text at the cursor position.

```
insert <text> [CHARACTERSET csetname]
      [DOCUMENT document | WINDOW document~window]
```

```rexx
'edittext'
'insert "Hello PageStream!"'
```

#### CopyText / CutText / PasteText / DeleteText

```
copytext [DOCUMENT name | WINDOW name]
cuttext [DOCUMENT name | WINDOW name]
pastetext [DOCUMENT name | WINDOW name]
deletetext [DOCUMENT name | WINDOW name]
```

#### DropText

```
droptext [DOCUMENT name | WINDOW name]
```

#### SelectText

```
selecttext <AT pointx pointy [stack] | WORD | LINE | PARAGRAPH | CHAPTER | ALL | NONE>
          [DOCUMENT name | WINDOW name]
```

#### TextCursor

Positions the text cursor.

```
textcursor <direction> [SELECT]
          [DOCUMENT name | WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `direction` | /k | `LEFT`, `RIGHT`, `UP`, `DOWN`, `WORDLEFT`, `WORDRIGHT`, `LINESTART`, `LINEEND`, `PARASTART`, `PARAEND`, `CHAPTERSTART`, `CHAPTEREND`, `ARTICLESTART`, `ARTICLEEND` |
| `SELECT` | /S | Select text during movement |

---

### Find and Replace

#### FindText

```
findtext <text> [CASESENSITIVE] [WHOLEWORD] [WRAPAROUND]
        [DOCUMENT name | WINDOW name]
```

#### ReplaceText

```
replacetext <text> <replacement> [CASESENSITIVE] [WHOLEWORD] [WRAPAROUND] [ALL]
           [DOCUMENT name | WINDOW name]
```

---

### Character Formatting

#### SetFont

```
setfont fontname [DOCUMENT name | WINDOW name | STYLETAG name]
```

```rexx
'setfont "Times"'
```

#### SetTypeSize

```
settypesize size [DOCUMENT name | WINDOW name | STYLETAG name]
```

```rexx
'settypesize 12'
```

#### SetTypeWidth

```
settypewidth width [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetTypeAngle

```
settypeangle angle [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetBold / SetItalic / SetUnderline / SetOutline / SetShadow / SetReverse

```
setbold [ON | OFF | TOGGLE] [DOCUMENT name | WINDOW name | STYLETAG name]
setitalic [ON | OFF | TOGGLE] [...]
setunderline [ON | OFF | TOGGLE] [...]
setoutline [ON | OFF | TOGGLE] [...]
setshadow [ON | OFF | TOGGLE] [...]
setreverse [ON | OFF | TOGGLE] [...]
```

#### SetNormal

Removes all text styles.

```
setnormal [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetSmallCaps / SetSuperscript / SetSubscript / SetScript / SetCase

```
setsmallcaps [ON | OFF | TOGGLE] [...]
setsuperscript [...]
setsubscript [...]
setscript [NONE | SUPERSCRIPT | SUBSCRIPT] [...]
setcase [NORMAL | UPPER | LOWER | TITLE] [...]
```

#### SetKerning / SetTracking / SetBaseline

```
setkerning amount [DOCUMENT name | WINDOW name | STYLETAG name]
settracking amount [DOCUMENT name | WINDOW name | STYLETAG name]
setbaseline amount [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### Character Queries

```
getfont [DOCUMENT name | WINDOW name]
gettypesize [DOCUMENT name | WINDOW name]
gettypewidth [DOCUMENT name | WINDOW name]
getbaseline [DOCUMENT name | WINDOW name]
```

---

### Paragraph Formatting

#### SetAlignment

```
setalignment [LEFT | RIGHT | CENTER | JUSTIFY | FORCELEFT | FORCERIGHT | FORCECENTER | FORCEJUSTIFY]
            [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetLeading / SetBaselineLeading / SetParagraphLeading

```
setleading <FIXED amount | AUTO percent | RELATIVE percent>
          [DOCUMENT name | WINDOW name | STYLETAG name]
setbaselineleading <FIXED amount | AUTO percent | RELATIVE percent>
                  [DOCUMENT name | WINDOW name | STYLETAG name]
setparagraphleading <BEFORE amount> <AFTER amount>
                   [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetIndent / SetFirstLine / SetHanging

```
setindent left right [DOCUMENT name | WINDOW name | STYLETAG name]
setfirstline amount [DOCUMENT name | WINDOW name | STYLETAG name]
sethanging amount [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetTabRuler

```
settabruler [CLEAR | TAB position [type] [FILL fillchar] [ALIGN alignchar]]
           [DOCUMENT name | WINDOW name | STYLETAG name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `CLEAR` | /S | Delete all tab stops |
| `position` | /d | Tab position |
| `type` | /k | `LEFT`, `RIGHT`, `CENTER`, `DECIMAL` |
| `FILL` | /s | Fill character |
| `ALIGN` | /s | Alignment character (for DECIMAL) |

#### SetHyphenation

```
sethyphenation [ON | OFF | TOGGLE] [MINWORD length] [MINBEFORE length]
              [MINAFTER length] [MAXROWS rows]
              [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetWidowOrphan

```
setwidoworphan [WIDOW lines] [ORPHAN lines]
              [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetLastLineFlush

```
setlastlineflush [ON | OFF | TOGGLE]
                [DOCUMENT name | WINDOW name | STYLETAG name]
```

#### SetBullet / SetNumbered / SetDropCap

```
setbullet [ON | OFF | TOGGLE] [CHAR character] [...]
setnumbered [ON | OFF | TOGGLE] [...]
setdropcap [ON | OFF | TOGGLE] [LINES lines] [...]
```

---

### Inserting Special Elements

#### InsertBreak

```
insertbreak [COLUMN | PAGE | SECTION | FRAME]
           [DOCUMENT name | WINDOW name]
```

#### InsertChar

```
insertchar charcode [DOCUMENT name | WINDOW name]
```

#### InsertControl

```
insertcontrol [PAGENUMBER | CHAPTERSTART | SECTIONSTART | ...]
             [DOCUMENT name | WINDOW name]
```

#### InsertDash

```
insertdash [EM | EN | DISCRETIONARY | NONBREAKING]
          [DOCUMENT name | WINDOW name]
```

#### InsertDate / InsertTime

```
insertdate [FORMAT format] [DOCUMENT name | WINDOW name]
inserttime [FORMAT format] [DOCUMENT name | WINDOW name]
```

#### InsertBookmark / DeleteBookmark / EditBookmark

```
insertbookmark name [DOCUMENT name | WINDOW name]
deletebookmark name [DOCUMENT name | WINDOW name]
editbookmark oldname newname [DOCUMENT name | WINDOW name]
```

#### InsertFigureMark / InsertIndexMark / InsertInlineObject

```
insertfiguremark [DOCUMENT name | WINDOW name]
insertindexmark topic [DOCUMENT name | WINDOW name]
insertinlineobject [DOCUMENT name | WINDOW name]
```

#### InsertNonBreaking

```
insertnonbreaking [SPACE | HYPHEN]
                 [DOCUMENT name | WINDOW name]
```

---

### Text Chaining

#### BreakTextRouting

```
breaktextrouting [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SetTextRouting

```
settextrouting [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Generating Indexes

```
generateindex [DOCUMENT document | WINDOW document-window]
generatetoc [DOCUMENT document | WINDOW document-window]
generatetof [DOCUMENT document | WINDOW document-window]
```

---

## Color Commands

### SetColor

```
setcolor colorname [ALL | FILL | STROKENUMBER n]
        [OBJECT | TEXT | SHADOW | OUTLINE | REVERSE | UNDERLINE |
         RULEABOVE | RULEBELOW | CELLFILL | CELLTOP | CELLBOTTOM |
         CELLLEFT | CELLRIGHT | DROPCAP | BULLET | NUMBERED]
        [DOCUMENT name | WINDOW name | OBJECTID number | STYLETAG name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `colorname` | /s | Color name |
| `ALL` | /S | Fill and stroke (default) |
| `FILL` | /S | Fill only |
| `STROKENUMBER` | /i | Specific stroke (starting at 0) |
| `what` | /k | Target attribute |

```rexx
'setcolor "Red" fill text'
```

### GetColors

```
getcolors [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

Returns color count and names.

---

## Style Commands

### NewStyleTag

```
newstyletag name type [NEXTSTYLETAG tagname]
           [DOCUMENT name | WINDOW name | CHAPTER name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `name` | /s | Name of the new style |
| `type` | /k | `CHARACTER`, `PARAGRAPH`, `OBJECT`, `COLOR`, `FPATTERN`, `LPATTERN` |
| `NEXTSTYLETAG` | /s | Following style |

### DeleteStyleTag

```
deletestyletag [REPLACEWITH name] <STYLETAG name>
```

```rexx
'deletestyletag replacewith "SubHeadline" styletag "Headline"'
```

### SetStyleTag

```
setstyleTag [NEXTSTYLETAG nexttagname] [UNLOCKED | LOCKED]
           [INTERNAL | DRAWING] [NOTOC | TOC]
           [STYLETAG document:...-tagname]
```

### SetStyleTagName

```
setstyletagname name [STYLETAG name]
```

```rexx
'setstyletagname "Body Text" styletag "paragraph text"'
```

### BeginStyleTag / EndStyleTag

Begins/ends the modification of a style. Must be used in pairs.

```
beginstyletag [STYLETAG name]
endstyletag [STYLETAG name]
```

### AppendStyleTags

Loads a style file and appends its styles.

```
appendstyletags <FILE filepath | ASK> [type] [progress]
               [DOCUMENT name | WINDOW name | CHAPTER name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `FILE` | /s | Style file path |
| `ASK` | /S | File selection dialog |
| `type` | /k | `ALL` (default), `TEXT`, `PARAGRAPH`, `CHARACTER`, `OBJECT`, `COLOR` |
| `progress` | /k | `STATUS` or `NOSTATUS` |

```rexx
'appendstyletags file "Work:Newsletter10" text'
```

### SaveStyleTags

```
savestyletags [ASK | FILE "file"] [ALL | TEXT | PARAGRAPH | CHARACTER | OBJECT | COLOR]
             [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
             [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

### SetObjectStyle / SetCharacterStyle / SetParagraphStyle

```
setobjectstyle style [DOCUMENT name | WINDOW name | OBJECTID number | STYLETAG name]
setcharacterstyle style [DOCUMENT name | WINDOW name | STYLETAG name]
setparagraphstyle tagname [DOCUMENT document | WINDOW document-window | STYLETAG document:...-tagname]
```

```rexx
'setparagraphstyle "Body Text"'
```

### SetStrokeStyle

```
setstrokestyle dash [ALL | FILL | STROKENUMBER number] [what]
              [DOCUMENT name | WINDOW name | OBJECTID number | STYLETAG name]
```

### SetFPatternStyle

```
setfpatternstyle tagname [ALL | FILL | STROKENUMBER n] [what]
               [DOCUMENT name | WINDOW name | ID id | STYLETAG document:...-tagname]
```

Note: STROKENUMBER counts from 0 (unlike the UI, which counts from 1).

### Style Queries

#### GetStyleTags

```
getstyletagss liststem [mode] [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `mode` | /k | `ALL` (default), `TEXT`, `PARAGRAPH`, `CHARACTER`, `OBJECT`, `COLOR` |

Returns count. Style names in the list.

#### GetStyleTag

```
getstyletag [TYPE &type] [NEXTSTYLETAG &nexttagname]
           [LOCKED &flag] [USAGE &flag] [INTOC &flag]
           [STYLETAG document:...-tagname]
```

#### GetStyleTagData

```
getstyletagdata [TYPEFONT &font] [TYPESIZE &size] [TYPEWIDTH &width]
               [RGBCOLOR &r &g &b] [CMYKCOLOR &c &m &y &k]
               [STYLETAG document:...-tagname]
```

#### GetCharacterStyle / GetParagraphStyle

```
getcharacterstyle [DOCUMENT document | WINDOW document-window]
getparagraphstyle [DOCUMENT document | WINDOW document-window]
```

Returns style name or `MIXED` if multiple styles are used.

---

## System Commands

### Windows and Views

#### OpenWindow

```
openwindow [DOCUMENT name]
```

#### HideWindow / RevealWindow

```
hidewindow [WINDOW name]
revealwindow [WINDOW name]
```

#### SetWindowPos

```
setwindowpos [AT x y] [SIZE width height] [WINDOW name]
```

```rexx
'setwindowpos at 0 -1 size 640 10000'
```

#### Display

Changes page, zoom, and offset of a view.

```
display [PAGE <number | shortcut>] [SCALE <custom | preset>]
       [OFFSET offsetx offsety] [WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `PAGE` | /i or /k | Page number or: `LASTUSED`, `PREVIOUS`, `NEXT`, `UP`, `DOWN`, `START`, `END`; or MPG name |
| `SCALE` | /d or /k | Percentage or: `FULLPAGE`, `FULLPAGEWIDTH`, `FULLPAGEHEIGHT`, `FULLPASTEBOARD`, `FULLPASTEBOARDWIDTH`, `FULLPASTEBOARDHEIGHT`, `LASTUSED`, `ZOOMIN`, `ZOOMOUT` |
| `OFFSET` | /d | Horizontal and vertical offset |

```rexx
'display page 7 scale fullpage'
'display page previous'
'display page "MyDoc~7" scale 85 offset 5.5 "-3"'
```

#### SetToolMode

```
settoolmode tool [WINDOW name]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `tool` | /k | `OBJECT`, `RESHAPE`, `CROP`, `MAGNIFY`, `TEXT`, `EYEDROPPER`, `COLUMN`, `LINE`, `BOX`, `RBOX`, `ELLIPSE`, `ARC`, `PEN`, `FREEHAND`, `GRID`, `ROUTETEXT`, `LASTUSED`, `PREVIOUS`, `NEXT` |

#### MainToolbox / SetToolbar / ColorPalette

```
maintoolbox [OPEN | CLOSE]
settoolbar [SHOW | HIDE | TOGGLE] [WINDOW name]
colorpalette [OPEN | CLOSE]
```

### Settings

#### LoadSettings / SaveSettings

```
loadsettings [FILE "file" | ASK]
savesettings [FILE "file" | ASK]
```

#### LoadFontPrefs / SaveFontPrefs

```
loadfontprefs [FILE "file" | ASK]
savefontprefs [FILE "file" | ASK]
```

#### LoadPrintPrefs / SavePrintPrefs

```
loadprintprefs [FILE "file" | ASK]
saveprintprefs [FILE "file" | ASK]
```

#### SetMeasurements

```
setmeasurements [COORDINATE hmsys vmsys] [RELATIVE rmsys] [TEXT tmsys]
               [FROM <PAGE|SPREAD>] [SAVE]
```

Measurement units: `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `METRICPOINTS`, `FEET`, `METERS`

---

### Printing

#### PrintDocument

```
printdocument [SIDES <BOTH|EVEN|ODD>] [SCALE <ACTUAL|FULLPAGE|scale>]
             [COPIES copies] [THUMBNAILS <ON|OFF>]
             [TILING <ON [TILINGOVERLAP amount]|OFF>]
             [PICTURES <ON|OFF>] [DRAWINGS <ON|OFF>]
             [COLLATE <ON|OFF>] [REVERSEORDER <ON|OFF>]
             [PRINTBLANK <ON|OFF>] [PRINTERMARKS <ON|OFF>]
             [MIRROR <ON|OFF>] [NEGATIVE <ON|OFF>]
             [OUTPUT <GRAYSCALE|COLOR|SEPARATIONS|PROCESS|SPOT>]
             [PAGE page | MPG masterpage | PAGERANGE "page,from-to,..."]
             [DOCUMENT document | CHAPTER document:...]
```

---

### Undo / Redo

```
undo [DOCUMENT name | WINDOW name]
redo [DOCUMENT name | WINDOW name]
```

### Quit

```
quit [mode]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `mode` | /k | `FORCE` (without saving), `ALERT` (default, ask user), `QUIET` (quit silently) |

---

### System Queries

#### GetVersion

```
getversion
```

Returns version string in RESULT.

#### GetDisplay

```
getdisplay stem [WINDOW name]
```

Results: `stem.page`, `stem.mode`, `stem.scale`, `stem.left`, `stem.top`

#### GetScreenDPI (Amiga only)

```
getscreendpi stem
```

Results: `stem.x`, `stem.y`

#### GetScreenName (Amiga only)

```
getscreenname
```

Returns the public screen name in RESULT.

#### GetPortname (Amiga only)

```
getportname
```

Returns the ARexx port name in RESULT.

#### GetWindowPos

```
getwindowpos [AT stem] [SIZE stem] [WINDOW name]
```

```rexx
'getwindowpos at coords size coords'
SAY 'Left: ' || coords.x
SAY 'Width: ' || coords.w
```

#### GetToolMode

```
gettoolmode [WINDOW document-window]
```

Returns the current tool: `OBJECT`, `RESHAPE`, `CROP`, `TEXT`, `MAGNIFY`, `EYEDROPPER`, `COLUMN`, `LINE`, `BOX`, `RBOX`, `ELLIPSE`, `ARC`, `POLYGON`, `PEN`, `FREEHAND`, `GRID`, `ROUTETEXT`, `ROTATE`, `SBOX`, `BBOX`, `IBOX`, `PIE`, `STAR`, `SCALLOP`, `PUFFY`, `WAVY`, `LASSO` (PGS4.0+), `TABLE` (PGS4.1+)

#### GetMeasurements

```
getmeasurements [DOCUMENT document | WINDOW document-window]
```

#### GetUndo / GetRedo

```
getundo [DOCUMENT name | WINDOW name]
getredo [DOCUMENT name | WINDOW name]
```

#### GetErrNum / GetErrStr

```
geterrnum
geterrstr
```

Returns the last error number or error text respectively.

---

### Coordinate Conversion

#### GetCoordFromString

```
getcoordfroms string defaultmeasurementsystem
```

Converts a measurement string (e.g. "1in") to an internal coordinate value.

Measurement units: `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `METRICPOINTS`, `FEET`, `METERS`

#### GetCmdStringFromCoord

```
getcmdstringfromcoord coord measurementtype
```

Converts an internal coordinate value to a command string (e.g. "1in").

#### GetUIStringFromCoord

```
getuistringfromcoord coord measurementtype
```

Converts to a UI-formatted string.

---

### Interface Lock

#### LockInterface

```
lockinterface <TRUE | FALSE>
```

**WARNING:** If the script ends before `lockinterface false` is called, the interface remains locked and PageStream must be force-quit!

```rexx
'lockinterface true'
/* ... operations ... */
'lockinterface false'
```

#### CheckLock

```
checklock
```

Checks the current lock status.

---

### Script Execution

#### RX

```
rx filepath [DOCUMENT name | WINDOW name]
```

Executes an external ARexx, Python, or AppleScript script.

```rexx
'rx "PageStream:Scripts/Test"'
```

#### Execute / Play

```
execute filepath
play filepath
```

#### StartRecording / StopRecording

```
startrecording
stoprecording
```

---

### Preference Buffering

#### BeginPrefCapture / EndPrefCapture

Buffers multiple preference changes for faster execution.

```
beginprefcapture [SAVE]
endprefcapture
```

### CharacterSet

```
characterset csetname [SAVE]
```

Sets the character set. Returns the previous one.

### Beep (Amiga only)

```
beep
```

---

## Dialog Box Commands

PageStream allows creating custom dialog boxes in ARexx scripts.

### AllocArexxRequester

Creates a new dialog.

```
allocarexxrequester name width height
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `name` | /s | Dialog title |
| `width` | /i | Inner width in pixels |
| `height` | /i | Inner height in pixels |

Returns dialog handle in RESULT. **Must be freed before script ends with `freearexxrequester`!**

```rexx
'allocarexxrequester "Test Dialog Box" 400 220'
iTextBox = RESULT
```

### AllocArexxList

Creates a list for list controls.

```
allocarexxlist
```

Returns list handle in RESULT. Lists exist independently of dialogs. A list can be assigned to multiple controls. **Must be freed with `freearexxlist`!**

### AddArexxList

Adds an item to a list.

```
addarexxlist handle item
```

Items are indexed starting at 0. No maximum count.

```rexx
'addarexxlist choices "First choice"'
```

### AddArexxGadget

Adds a control to a dialog.

```
addarexxgadget dbox type left top width [height]
              [LABEL name] [LABELPOS pos] [STRING text]
              [BORDER style] [CHECKED status]
              [LIST handle] [CURRENT value] [TOTAL value]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `dbox` | /i | Dialog handle |
| `type` | /k | `EXIT`, `STRING`, `MULTILINE`, `TEXT`, `CHECKBOX`, `POPUP`, `CYCLE`, `SCROLLIST`, `SLIDER` |
| `left` | /i | Left position |
| `top` | /i | Top position |
| `width` | /i | Width |
| `height` | /i | Height (for MULTILINE and SCROLLIST, minimum: 36) |
| `LABEL` | /s | Label text |
| `LABELPOS` | /k | Position: `LEFT`, `RIGHT`, `ABOVE`, `ABOVELEFT`, `ABOVERIGHT`, `BELOW`, `BELOWLEFT`, `BELOWRIGHT`, `CENTER`, `CENTERLEFT`, `CENTERRIGHT`, `LEFTABOVE`, `LEFTBELOW`, `RIGHTABOVE`, `RIGHTBELOW` |
| `STRING` | /s | Default text |
| `BORDER` | /k | `NONE`, `RAISED`, `RECESSED`, `SHINE`, `SHADOW`, `TEXT` |
| `CHECKED` | /k | `TRUE` or `FALSE` (for CHECKBOX) |
| `LIST` | /i | List handle (for POPUP, CYCLE, SCROLLIST) |
| `CURRENT` | /i | Default list entry or slider position |
| `TOTAL` | /d | Slider range |

Returns control handle in RESULT.

```rexx
'addarexxgadget' dboxid 'exit 16 114 70 label "Cancel"'
Cancel = RESULT

'addarexxgadget' dboxid 'string 16 20 280 label "Name:" string "Example"'
nameField = RESULT

'addarexxgadget' dboxid 'scrollist 16 8 90 72'
Fonts = RESULT
```

### SetArexxGadget

Changes properties of a control.

```
setarexxgadget reqhandle objid [HIDDEN <TRUE|FALSE>] [DISABLED <TRUE|FALSE>]
              [CHECKED <TRUE|FALSE>] [HIGHLABEL <TRUE|FALSE>]
              [READONLY <TRUE|FALSE>] [ESCAPE <TRUE|FALSE>]
              [DEFAULT <TRUE|FALSE>] [LABEL string] [STRING string]
              [LEFT num] [TOP num] [WIDTH num] [HEIGHT num]
              [CURRENT num] [TOTAL num] [LABELPOS position]
              [BORDER border_type] [ALIGNMENT <LEFT|CENTER|RIGHT>]
              [ORIENTATION <HORIZONTAL|VERTICAL>] [LIST listhandle]
```

### GetARexxGadget

Queries the state of a control.

```
getarexxgadget dbhandle ctlhandle attribute
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `attribute` | /s | `CHECKED` (returns 0/1), `STRING` (text), `CURRENT` (selected item) |

```rexx
'getarexxgadget' iGetName sNameCtl 'string'
sName = RESULT
```

### DoARexxRequester / DoRequester

Displays the dialog and waits for user input.

```
doarexxrequester dbhandle
dorequester dbhandle
```

Returns the handle of the pressed EXIT button in RESULT.

### FreeARexxRequester / FreeARexxList

Frees dialogs and lists. **Always call to avoid memory leaks!**

```
freearexxrequester dbhandle
freearexxlist listhandle
```

### AlertRequester

Displays a warning message.

```
alertrequester message [BUTTON1 label] [BUTTON2 label] [BUTTON3 label]
```

---

### Busy Requester

#### OpenBusyRequester

```
openbusyrequester [MESSAGE message] [ABORT <DISABLED|ENABLED>]
                 [THERMOMETER <DISABLED|ENABLED>]
                 [DELAYEDOPEN <DISABLED|ENABLED>]
                 [TOTAL total] [CURRENT current]
```

Returns handle in RESULT.

```rexx
'openbusyrequester message "Thinking..." thermometer enabled total 100 current 0'
bh = RESULT
```

#### SetBusyRequester

```
setbusyrequester handle [MESSAGE text] [THERMOMETER status]
                       [ABORT status] [TOTAL value] [CURRENT value]
```

#### GetBusyRequester

```
getbusyrequester handle
```

Returns 0 if Stop was not pressed, 1 if pressed.

#### CloseBusyRequester

```
closebusyrequester handle
```

```rexx
/* Progress indicator */
'openbusyrequester message "Processing..." thermometer enabled total 100 current 0'
bh = RESULT
DO i = 1 TO 100
    'getbusyrequester' bh
    IF RESULT = 1 THEN BREAK
    ELSE 'setbusyrequester' bh 'current' i
END
'closebusyrequester' bh
```

---

### User Input Dialogs

#### GetChoice (Amiga only)

```
getchoice MESSAGE message BUTTON1 label [BUTTON2 label] [BUTTON3 label]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `MESSAGE` | /s | Message (max. 44 characters) |
| `BUTTON1` | /s | Default button (max. 8 characters) |
| `BUTTON2` | /s | Cancel button (activatable with Esc) |
| `BUTTON3` | /s | Third button |

Returns button number in RESULT. Underscore before a letter = keyboard shortcut (e.g. "\_OK").

```rexx
'getchoice message "Click on a button." button1 "\_OK" button2 "\_Cancel"'
buttonid = RESULT
```

#### GetFile

```
getfile [TITLE title] [mode] [PATH name] [FILE name]
       [POSBUTTON label] [NEGBUTTON label]
```

| Parameter | Type | Description |
|-----------|-----|-------------|
| `mode` | /k | `LOAD` (default) or `SAVE` |
| RC = 0 | | Success, file path in RESULT |
| RC = 10 | | Cancelled |

```rexx
'getfile TITLE "Save a file" save path "ram:" posbutton "Save"'
```

#### GetFilePath

```
getfilepath [PATH path] [TITLE title] [POSBUTTON ok] [NEGBUTTON cancel]
```

#### GetString (Amiga only)

```
getstring [STRING default] [TITLE label] [POSBUTTON label] [NEGBUTTON label]
```

RC = 0 + string in RESULT on success; RC = 10 on cancel.

```rexx
'getstring string "Erase this." title "\_Text" posbutton "\_Yes" negbutton "\_No"'
userstring = RESULT
```

#### GetRegion

```
getregion stem [MESSAGE message]
```

User drags a region. Results: `stem.x1`, `stem.y1`, `stem.x2`, `stem.y2`. RC = 0 on success, RC = 10 on cancel.

```rexx
'getregion coord message "Drag to define an area"'
IF RC = 0 THEN DO
    SAY coord.x1 coord.y1 coord.x2 coord.y2
END
```

---

## Script Commands

### BeginCommandCapture / EndCommandCapture

```
begincommandcapture
endcommandcapture
```

---

## Example Scripts

### New Document with Text

```rexx
/* CreateDoc.rexx - Document with text frame */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'opendocument'
'drawtextobj 1.0 1.0 6.5 9.0'
textobj = RESULT
'edittext'
'setfont "Times"'
'settypesize 12'
'insert "Hello PageStream!"'
```

### Drawing Multiple Shapes

```rexx
/* DrawShapes.rexx - Various shapes */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'drawbox 1.0 1.0 3.0 2.0'
'drawellipse 5.0 1.5 1.5 1.0'
'drawpolygon 3.5 5.0 1.5 1.5 STAR SIDES 5'
'drawline 1.0 8.0 7.0 8.0'
```

### User Dialog

```rexx
/* UserDialog.rexx - Simple input dialog */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'allocarexxrequester "My Dialog" 300 100'
dbox = RESULT

'addarexxgadget' dbox 'EXIT 10 60 80 LABEL "\_OK"'
ok = RESULT
'addarexxgadget' dbox 'EXIT 210 60 80 LABEL "\_Cancel"'
cancel = RESULT
'addarexxgadget' dbox 'STRING 10 20 280 LABEL "Name:" STRING "Example"'
nameField = RESULT

'doarexxrequester' dbox
button = RESULT

IF button = ok THEN DO
    'getarexxgadget' dbox nameField 'string'
    SAY "Input:" RESULT
END

'freearexxrequester' dbox
```

### Script with Progress Indicator

```rexx
/* Progress.rexx - Progress bar */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'lockinterface true'
'refresh wait'

'openbusyrequester message "Processing pages..." thermometer enabled total 10 current 0'
bh = RESULT

DO i = 1 TO 10
    'getbusyrequester' bh
    IF RESULT = 1 THEN LEAVE  /* Stop pressed */
    /* ... page operations ... */
    'setbusyrequester' bh 'current' i
END

'closebusyrequester' bh
'refresh continue'
'lockinterface false'
```

### Listing All Documents

```rexx
/* ListDocs.rexx - List open documents */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'getdocuments docs'
count = RESULT
SAY count "document(s) open:"
DO i = 0 TO count - 1
    SAY "  " docs.i
END
```
