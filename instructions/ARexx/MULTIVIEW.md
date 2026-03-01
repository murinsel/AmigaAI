# MULTIVIEW ARexx Port

Port name: MULTIVIEW.1 (incremented per instance: .2, .3, ...). Custom via PORTNAME CLI arg.
Arguments: /A=required, /S=switch, /N=number, /K=keyword.

## File Operations
- `OPEN [NAME/K] [CLIPBOARD/S] [CLIPUNIT/K/N]` -- Open file or clipboard contents
- `RELOAD` -- Reload current object
- `SAVEAS [NAME/K] [IFF/S]` -- Save to disk (IFF=force IFF format instead of RAW)
- `PRINT` -- Print current object

## Clipboard
- `MARK` -- Start mark/selection mode
- `COPY` -- Copy selection (or whole object) to clipboard
- `PASTE [UNIT/N]` -- Open clipboard unit contents
- `CLEARSELECTED` -- Deselect current block

## Navigation (AmigaGuide)
- `NEXT` -- Jump to next node
- `PREVIOUS` -- Jump to previous node

## Trigger Methods
- `DOTRIGGERMETHOD METHOD/A` -- Perform a trigger method on current object
- `GETTRIGGERINFO [VAR/S] [STEM/K]` -- List supported trigger methods (stem.count, stem.n.label/command/method)

## Object Info
- `GETOBJECTINFO [VAR/S] [STEM/K]` -- Get object info (stem.filename/name/basename/group/id)
- `GETFILEINFO` -- Get file info for current object
- `GETCURRENTDIR` -- Get current directory of object
- `ABOUT` -- Show info requester

## Window & Screen
- `MINIMUMSIZE` / `NORMALSIZE` / `MAXIMUMSIZE` -- Resize window
- `WINDOWTOFRONT` / `WINDOWTOBACK` -- Window depth
- `ACTIVATEWINDOW` -- Activate window
- `SCREENTOFRONT` / `SCREENTOBACK` -- Screen depth
- `BEEPSCREEN` -- Flash screen
- `SCREEN TRUE/S|FALSE/S` -- Toggle own screen
- `PUBSCREEN NAME/A` -- Move to public screen
- `SNAPSHOT` -- Save window position to ENVARC:MultiView/

## Application
- `QUIT` -- Exit MultiView
