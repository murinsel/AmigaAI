# IBROWSE ARexx Port

Arguments: /A=required, /S=switch, /N=number. BROWSERNR is the [ID] shown in the window title.

## Standard MUI Commands
- `QUIT [FORCE/S]` -- Exit IBrowse (FORCE skips confirmation)
- `SHOW` / `ACTIVATE` -- Uniconify, bring to front
- `HIDE` / `DEACTIVATE` -- Iconify
- `INFO ITEM/A` -- Get info. ITEM: TITLE, AUTHOR, COPYRIGHT, DESCRIPTION, VERSION, BASE, SCREEN
- `HELP FILE/A` -- Write command list to file

## Navigation
- `GOTOURL URL/A [BROWSERNR/N] [SAVE/S] [RELOAD/S] [MIME]` -- Load URL in browser
- `BACK [BROWSERNR/N]` -- Navigate back
- `FORWARD [BROWSERNR/N]` -- Navigate forward
- `HOME [BROWSERNR/N]` -- Load homepage
- `STOP [BROWSERNR/N]` -- Stop loading
- `RELOAD [BROWSERNR/N] [ALL/S] [FRAMES/S] [IMAGES/S]` -- Reload page/frames/images
- `LOADIMAGES [BROWSERNR/N]` -- Load unloaded images

## Windows & Browsers
- `NEW [URL]` -- Open new window or browser (returns RESULT=ID)
- `NEWWINDOW [URL]` -- Open new window (returns RESULT=ID)
- `NEWBROWSER [URL] [BROWSEWINDOWNR/N]` -- Open new browser tab (returns RESULT=ID)
- `CLOSEBROWSER [BROWSERNR/N]` -- Close browser
- `SCREENTOFRONT` / `SCREENTOBACK` -- Screen depth arrange

## Query
- `QUERY ITEM/A [BROWSERNR/N]` -- Get info. ITEM: URL, TITLE, ACTIVEBROWSERNR, ACTIVEWINDOWNR

## Bookmarks
- `ADDHOTLIST TITLE/A [GROUP/S] [URL] [SHORTCUT] [SHOWINMENU/N] [INSERTGROUP] [ACTIVATE/S]` -- Add bookmark
- `ADDFASTLINK TITLE/A [URL] [PROMPTTITLE/S]` -- Add fastlink button
- `OPENHOTLIST [CLOSE/S]` -- Open/close Hotlist Manager

## Other Windows
- `OPENHISTORY [BROWSERNR/N] [CLOSE/S]` -- Open/close browser history
- `OPENCACHEBROWSER [CLOSE/S]` -- Open/close cache explorer
- `OPENINFOWINDOW [CLOSE/S]` -- Open/close info window

## Flush
- `FLUSH [BROWSERNR/N] [HISTORY/S] [ALLIMAGES/S] [CACHEDIMAGES/S] [IMAGES/S]` -- Free memory

## Error Codes (RC)
-2=Out of memory, -3=Unknown command, -4=Syntax error, -5=Unknown item, -6=No browser active
