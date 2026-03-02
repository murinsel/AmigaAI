# PAGESTREAM ARexx Port

Port name: PAGESTREAM (or user-defined). Over 400 commands. Detailed docs: AmigaAI:docs/ARexx/PageStream.md
Arguments: /i=integer, /d=decimal, /s=string, /k=keyword, /S=switch, /a=array/stem.
Common target params: PAGE number, WINDOW name, DOCUMENT name, CHAPTER name, OBJECTID number, MPG name.
Common transform params: ROTATE angle, SKEW slant twist, SLANT angle, TWIST angle, ABOUT x y | ABOUTCENTER, CONSTRAIN|FREE, PRINT|NOPRINT, INFRONT|INBACK.
Most draw/create commands return an object handle in RESULT.

## File Commands
- `opendocument [ASK | FILE "file"] [FILTER name] [STATUS|NOSTATUS]` -- Open document
- `open [file] [FILTER ""]` -- Open document and create window
- `closedocument [FORCE|ALERT|QUIET] [DOCUMENT name | WINDOW name]` -- Close document
- `closewindow [WINDOW name]` -- Close single view
- `savedocument [ASK|FILE "file"|DEFAULT] [FILTER name] [STATUS|NOSTATUS] [QUIET|FORCE|ALERT] [TEMPLATE]` -- Save document
- `revertdocument [STATUS|NOSTATUS] [FORCE|ALERT] [FILTER name]` -- Revert to last saved
- `newdocument [name]` -- Create new document (returns name)
- `placegraphic [FILE "file"] [FILTER name] [ASIS|AT x y|USER]` -- Place graphic. FILTER: PICT,TIFF,IFFILBM,GIF,BMP,JPEG,PNG,EPS,...
- `inserttext [FILE path] [FILTER name]` -- Insert text. FILTER: ASCII,IFFCTXT,IFFFTXT,WORDWORTH
- `exportgraphic [FILE "file"] [FILTER name] [STATUS|NOSTATUS]` -- Export graphic. FILTER: PICT,TIFF,IFFILBM,GIF,BMP,JPEG,PNG
- `exporttext [FILE "file"] [FILTER name]` -- Export text. FILTER: ASCII,IFFFTXT,IFFCTXT,WORDWORTH,RTF
- `exportpdf [FILE "file"]` -- Export as PDF
- `collectoutput [ASK|FILE "file"]` -- Collect output files
- `setdocumentpath [path] [SAVE]` / `setgraphicpath` / `settextpath` / `setscratchpath` / `setscriptpath` -- Set default paths
- `setbackup [AUTOBACKUP ON|OFF] [COUNT n] [AUTOSAVE ON|OFF] [INTERVAL mins] [PATH path] [SAVE]` -- Backup settings
- `selectonpaste [ON|OFF|TOGGLE] [SAVE]` -- Auto-select on paste

## Drawing Commands
- `drawbox <left top right bottom> [NORMAL|ROUND|SCALLOP|BEVEL|INSET] [CORNER r | ECORNER rx ry] [transform] [target]` -- Draw box
- `drawellipse <cx cy rx ry> [ELLIPSE|PIE|ARC] [ANGLES start end] [transform] [target]` -- Draw ellipse
- `drawline <x1 y1 x2 y2> [transform] [target]` -- Draw line
- `drawpolygon <cx cy rx ry> [NORMAL|STAR|PUFFY|SCALLOP|WAVY] [SIDES n] [OFFSETANGLE a] [DEFLECTION d] [transform] [target]` -- Draw polygon
- `drawspiral x1 y1 ROUNDS n SPACING d STARTANGLE a [transform] [target]` -- Draw spiral
- `drawgrid <x1 y1 x2 y2 | POINTS x1 y1 x2 y2 x3 y3 x4 y4> [DIVISIONS h v] [transform] [target]` -- Draw grid
- `drawtable <left top right bottom> [ROWS r] [COLUMNS c] [transform] [target]` -- Draw table
- `drawcolumn <left top right bottom> [COLUMNS n] [GUTTER space] [transform] [target]` -- Draw text column frame
- `drawtextobj <left top> [transform] [target]` -- Draw text block
- `drawpicture <left top right bottom> [CONTENTOFFSET ox oy] [CONTENTSCALE sx sy] [DPI xdpi ydpi] [FRAMED|FRAMELESS] [INTERNAL|EXTERNAL] [FILE path] [transform] [target]` -- Place picture frame
- `draweps <left top right bottom> [CONTENTOFFSET ox oy] [CONTENTSCALE sx sy] [FRAMED|FRAMELESS] [FILE path] [transform] [target]` -- Place EPS frame

## Path Commands
- `createpath <left top right bottom> [transform] [target]` -- Create new path (returns handle)
- `addpoint <MOVETO x y | LINETO x y | CURVETO cx1 cy1 cx2 cy2 x y | CLOSEPATH> [CORNERJOIN|SMOOTHJOIN] [target]` -- Add point to path
- `createpoint <NEAR x y> [target]` -- Add point at nearest path position
- `editpoint [ANCHOR x y] [CONTROLBEFORE x y] [CONTROLAFTER x y] [CORNERJOIN|SMOOTHJOIN] [POINTINDEX n] [target]` -- Edit point
- `movepoint [OFFSET x y] [POINTINDEX n] [target]` -- Move point
- `nudgepoint [LEFT|RIGHT] [UP|DOWN] [TAP] [POINTINDEX n] [target]` -- Nudge point
- `deletepoint [target]` / `splitpoint [POINTINDEX n] [target]` / `closepath [target]` / `mergesubpaths [target]`

## Document Commands
- `insertpage [BEFORE|AFTER] [PAGE n] [COUNT n]` / `deletepage [PAGE n] [TO n] [FORCE|ALERT|QUIET]` / `movepage`
- `newchapter name chapter# [PAGESFROMDOCUMENT]` / `deletechapter [CHAPTER name] [ALERT]` / `movechapter`
- `newmasterpage name w h [PORTRAIT|LANDSCAPE] [SINGLE|DOUBLE] [FACING|INDIVIDUAL]` / `deletemasterpage` / `movemasterpage`
- `selectmasterpage [NAME name] [PAGE n] [INFRONT|INBACK] [SHOW|HIDE] [RIPPLE|TO n]` -- Assign masterpage to page(s)
- `setdimensions w h [PORTRAIT|LANDSCAPE] [SINGLE|DOUBLE] [FACING|INDIVIDUAL] [SAVE]` -- Set page dimensions
- `setbleed h v [SAVE]` / `setcolumnguides cols gutter [SAVE]` / `setmarginguides in out top bottom [SAVE]`
- `addguides [HORIZONTAL|VERTICAL|BOTH] <AT pos> [MPG name]` / `deleteguides` / `setguides` / `setguide`
- `setgrid [SNAP h v] [SNAPOFFSET x y] [DISPLAYINTERVAL h v] [SAVE]` / `snaptogrid [ON|OFF|TOGGLE]` / `snaptoguides`
- `displaygrid <SHOW|HIDE|TOGGLE> [INFRONT|INBACK] [COLOR r g b]` / `displayguides` / `displayruler`
- `refresh [ON|OFF|WAIT|CONTINUE]` -- Toggle screen refresh for performance
- `logrevision [DESCRIPTION text] [VERSION major minor | BUMPREV] [USER name]` -- Add revision log entry

## Document Queries
- `currentdocument` / `currentwindow` / `currentpage` / `currentchapter` / `currentmasterpage` -- Returns name/number
- `getdocuments stem` / `getchapters stem` / `getmasterpages stem` / `getwindows stem` -- Returns count, names in stem
- `getdimensions stem` -- stem.width, stem.height, stem.orientation, stem.sides, stem.spreads
- `getmarginguides stem` -- stem.inside, stem.outside, stem.top, stem.bottom
- `getcolumnguides stem` -- stem.count, stem.gutter
- `getpagenumbering stem` / `getchapternumbering stem` / `getbleed stem` / `getgrid stem`
- `getdocumentstatus` -- Returns "changed" or "unchanged"

## Object Commands
- `selectobject <AT x y | RANGE l t r b | ALL | NONE | INVERT | NEXT | PREVIOUS> [FRONTMOST|BACKMOST] [target]` -- Select object(s)
- `getselectedobjects stem` -- Returns count, IDs in stem
- `move <OFFSET ox oy | TO x y> [target]` / `nudge [LEFT|RIGHT] [UP|DOWN] [TAP] [target]`
- `rotate angle [ABOUT x y | ABOUTCENTER] [target]` / `scaleobject <SCALE sx sy | SIZE w h> [target]` / `skew`
- `align [LEFTALIGN|CENTERALIGN|RIGHTALIGN] [TOPALIGN|MIDDLEALIGN|BOTTOMALIGN] [TOPAGE|TOMARGINS|TOSELECTION]`
- `distribute [HORZEVEN|HORZSPACE s|HORZNONE] [VERTEVEN|VERTSPACE s|VERTNONE]`
- `duplicate [OFFSET ox oy] [COPIES n] [ROTATE a] [SCALE sx sy]` / `movetopage [PAGE n]`
- `bringtofront` / `bringforward` / `sendtoback` / `sendbackward` -- Stack order
- `copyobject` / `cutobject` / `pasteobject` / `deleteobject` -- Clipboard
- `editobject [POSITION l t r b [SCALECONTENT|ADJUST]] [transform] [target]` -- Edit object properties
- `lock` / `unlock` -- Lock/unlock objects
- `creategroup` / `dissolvegroup` / `createcompound` / `dissolvecompound` / `dissolveobject`
- `converttopath` / `mergepaths` / `splitpaths` / `flattenpath` / `smoothpath` / `reversepath`
- `andpaths` / `orpaths` / `subpaths` / `xorpaths` -- Boolean path operations
- `makemask` / `releasemask` / `clearmask` / `generatemask` -- Object masks (also: drawingmask, picturemask, epsmask variants)
- `settextwrap [WRAP mode] [OFFSET offset]` / `gettextwrap`
- `settable [ROWS r] [COLUMNS c] [CELLGUTTER g]` / `setcell` / `insertcellcolumn` / `insertcellrow` / `joincell` / `breakcell`
- `getobject [TYPE &t] [POSITION &p] [NAME &n]` / `getpath` / `getbox` / `getellipse` / `getpicture` / `getrotation`

## Text Commands
- `edittext [AT x y [FRONTMOST|BACKMOST] | OBJECTID n]` -- Enter text editing mode
- `insert <text> [CHARACTERSET name]` -- Insert text at cursor
- `copytext` / `cuttext` / `pastetext` / `deletetext` / `droptext` -- Text clipboard
- `selecttext <AT x y | WORD | LINE | PARAGRAPH | CHAPTER | ALL | NONE>` -- Select text
- `textcursor <LEFT|RIGHT|UP|DOWN|WORDLEFT|WORDRIGHT|LINESTART|LINEEND|PARASTART|PARAEND|...> [SELECT]` -- Move cursor
- `findtext <text> [CASESENSITIVE] [WHOLEWORD] [WRAPAROUND]` / `replacetext <text> <replacement> [ALL]`
- `setfont name` / `settypesize size` / `settypewidth w` / `settypeangle a` -- Character formatting
- `setbold [ON|OFF|TOGGLE]` / `setitalic` / `setunderline` / `setoutline` / `setshadow` / `setreverse` / `setnormal`
- `setsmallcaps` / `setsuperscript` / `setsubscript` / `setkerning amount` / `settracking amount` / `setbaseline amount`
- `setalignment [LEFT|RIGHT|CENTER|JUSTIFY|FORCELEFT|FORCERIGHT|FORCECENTER|FORCEJUSTIFY]` -- Paragraph alignment
- `setleading <FIXED a | AUTO p | RELATIVE p>` / `setparagraphleading <BEFORE a> <AFTER a>` -- Line spacing
- `setindent left right` / `setfirstline amount` / `sethanging amount` -- Indentation
- `settabruler [CLEAR | TAB pos [LEFT|RIGHT|CENTER|DECIMAL] [FILL char]]` -- Tab stops
- `sethyphenation [ON|OFF] [MINWORD n] [MINBEFORE n] [MINAFTER n] [MAXROWS n]`
- `setwidoworphan [WIDOW n] [ORPHAN n]` / `setbullet` / `setnumbered` / `setdropcap [LINES n]`
- `insertbreak [COLUMN|PAGE|SECTION|FRAME]` / `insertchar code` / `insertdash [EM|EN|DISCRETIONARY|NONBREAKING]`
- `insertdate [FORMAT f]` / `inserttime` / `insertbookmark name` / `insertindexmark topic`
- `generateindex` / `generatetoc` / `generatetof` -- Generate index/TOC/TOF

## Color & Style Commands
- `setcolor colorname [ALL|FILL|STROKENUMBER n] [OBJECT|TEXT|SHADOW|OUTLINE|...]` -- Set color
- `getcolors` -- Get available colors
- `newstyletag name <CHARACTER|PARAGRAPH|OBJECT|COLOR|FPATTERN|LPATTERN> [NEXTSTYLETAG tag]` -- Create style
- `deletestyletag [REPLACEWITH name] <STYLETAG name>` / `setstyletagname newname [STYLETAG name]`
- `beginstyletag [STYLETAG name]` / `endstyletag` -- Begin/end style modification (must be paired)
- `appendstyletags <FILE path | ASK> [ALL|TEXT|PARAGRAPH|CHARACTER|OBJECT|COLOR]` / `savestyletags`
- `setobjectstyle style` / `setcharacterstyle style` / `setparagraphstyle tagname`
- `setstrokestyle dash` / `setfpatternstyle tagname`
- `getstyletagss stem [ALL|TEXT|PARAGRAPH|...]` / `getstyletag` / `getstyletagdata`

## System Commands
- `openwindow [DOCUMENT name]` / `hidewindow` / `revealwindow` / `setwindowpos [AT x y] [SIZE w h]`
- `display [PAGE n|PREVIOUS|NEXT|START|END] [SCALE n|FULLPAGE|ZOOMIN|ZOOMOUT] [OFFSET x y]` -- Change view
- `settoolmode <OBJECT|RESHAPE|CROP|TEXT|MAGNIFY|LINE|BOX|ELLIPSE|PEN|FREEHAND|GRID|...>`
- `setmeasurements [COORDINATE hmsys vmsys] [RELATIVE rmsys] [TEXT tmsys] [SAVE]`
- `printdocument [SIDES BOTH|EVEN|ODD] [SCALE ACTUAL|FULLPAGE|n] [COPIES n] [OUTPUT GRAYSCALE|COLOR|SEPARATIONS]`
- `loadsettings` / `savesettings` / `loadfontprefs` / `savefontprefs` / `loadprintprefs` / `saveprintprefs`
- `undo` / `redo` / `quit [FORCE|ALERT|QUIET]`
- `getversion` / `getdisplay stem` / `getscreendpi stem` / `getportname` / `gettoolmode` / `getwindowpos`
- `lockinterface <TRUE|FALSE>` -- Lock UI (WARNING: must unlock before script ends!)
- `rx filepath` -- Execute external script
- `beginprefcapture [SAVE]` / `endprefcapture` -- Buffer preference changes for speed

## Dialog Box Commands
- `allocarexxrequester name width height` -- Create dialog (returns handle, MUST free before exit)
- `allocarexxlist` -- Create list (returns handle, MUST free before exit)
- `addarexxlist handle item` -- Add item to list
- `addarexxgadget dbox <EXIT|STRING|MULTILINE|TEXT|CHECKBOX|POPUP|CYCLE|SCROLLIST|SLIDER> left top width [height] [LABEL name] [LABELPOS pos] [STRING text] [CHECKED TRUE|FALSE] [LIST handle] [CURRENT n] [TOTAL n]` -- Add control (returns handle)
- `setarexxgadget reqh objid [HIDDEN|DISABLED|CHECKED|...] [LABEL s] [STRING s] [CURRENT n]` -- Modify control
- `getarexxgadget dbh ctlh <CHECKED|STRING|CURRENT>` -- Query control state
- `doarexxrequester dbh` / `dorequester dbh` -- Show dialog, wait for input (returns exit button handle)
- `freearexxrequester dbh` / `freearexxlist listh` -- Free resources
- `openbusyrequester [MESSAGE text] [THERMOMETER ENABLED|DISABLED] [ABORT ENABLED|DISABLED] [TOTAL n]` -- Progress dialog
- `setbusyrequester h [MESSAGE text] [CURRENT n]` / `getbusyrequester h` (returns 1 if Stop pressed) / `closebusyrequester h`
- `getchoice MESSAGE text BUTTON1 label [BUTTON2 label] [BUTTON3 label]` -- Choice dialog (Amiga only)
- `getfile [TITLE t] [LOAD|SAVE] [PATH p] [FILE f]` -- File selector (RC=0+path or RC=10=cancel)
- `getstring [STRING default] [TITLE label]` -- String input (RC=0+string or RC=10=cancel)
- `getregion stem [MESSAGE text]` -- Region selector (stem.x1,y1,x2,y2; RC=0 or RC=10)
