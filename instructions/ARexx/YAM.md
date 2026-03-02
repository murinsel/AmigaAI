# YAM ARexx Port (Yet Another Mailer)

Port name: YAM
Arguments: /A=required, /S=switch, /N=number, /K=keyword, /M=multiple results.
Results via: RESULT, VAR/K (named variable), or STEM/K (stem variable).

## Address Book
- `ADDRNEW [TYPE] [ALIAS] [NAME] [EMAIL]` -- Create new entry (TYPE: G=group, L=list, default=person). Returns ALIAS.
- `ADDREDIT [ALIAS] [NAME] [EMAIL] [PGP] [HOMEPAGE] [STREET] [CITY] [COUNTRY] [PHONE] [COMMENT] [BIRTHDATE/N] [IMAGE] [MEMBER/M] [ADD/S]` -- Edit current entry fields
- `ADDRDELETE [ALIAS]` -- Delete entry (current if no alias given)
- `ADDRGOTO ALIAS/A` -- Make entry the current one
- `ADDRINFO ALIAS/A` -- Get entry info. Returns TYPE,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBERS/M
- `ADDRFIND PATTERN/A [NAMEONLY/S] [EMAILONLY/S]` -- Search address book. Returns ALIAS/M
- `ADDRRESOLVE ALIAS/A` -- Resolve alias/name to email address(es). Returns EMAIL/M
- `ADDRLOAD FILENAME/A` -- Load address book from file
- `ADDRSAVE [FILENAME]` -- Save address book (current filename if omitted)

## Folder Operations
- `SETFOLDER FOLDER/A` -- Switch to folder (by name or number)
- `FOLDERINFO [FOLDER]` -- Get folder info. Returns NUMBER/N,NAME,PATH,TOTAL/N,NEW/N,UNREAD/N,SIZE/N,TYPE/N. TYPE: 0=normal, 1=incoming, 2=outgoing, 3=sent+received, 4=deleted, 5=sent+incoming, 6=sent
- `NEWMAILFILE [FOLDER]` -- Get unused mail filename for folder. Returns FILENAME

## Message Selection & Navigation
- `SETMAIL NUM/N/A` -- Select message by index (0-based)
- `SETMAILFILE MAILFILE/A` -- Select message by filename
- `LISTSELECT MODE/A` -- Select messages: N=none, A=all, T=toggle, or numeric position
- `GETSELECTED` -- Get positions of selected messages. Returns NUM/N/M

## Message Info
- `MAILINFO [INDEX/N]` -- Get message info. Returns INDEX/N,STATUS,FROM,TO,REPLYTO,SUBJECT,FILENAME,SIZE/N,DATE,FLAGS,MSGID/N. STATUS: N=new,O=read,U=unread,R=replied,F=forwarded,S=sent,W=waiting,H=held,E=error

## Message Actions
- `MAILREAD [WINDOW/N] [QUIET/S]` -- Open message in read window. Returns WINDOW/N
- `MAILREPLY [QUIET/S]` -- Reply to selected message. Returns WINDOW/N
- `MAILFORWARD [QUIET/S]` -- Forward selected message. Returns WINDOW/N
- `MAILBOUNCE [QUIET/S]` -- Bounce/redirect selected message. Returns WINDOW/N
- `MAILEDIT [QUIET/S]` -- Edit selected message. Returns WINDOW/N
- `MAILCOPY FOLDER/A` -- Copy selected messages to folder
- `MAILMOVE FOLDER/A` -- Move selected messages to folder
- `MAILDELETE [ATONCE/S] [FORCE/S]` -- Delete selected messages (ATONCE=skip trash, FORCE=no confirmation)
- `MAILSTATUS STATUS` -- Change status: O=read, U=unread, H=held, W=waiting
- `MAILCHANGESUBJECT SUBJECT/A` -- Change subject of selected message(s)
- `MAILSEND [ALL/S]` -- Send selected or all outgoing messages
- `MAILSENDALL` -- Send all messages in outbox
- `MAILFILTER [ALL/S]` -- Filter messages. Returns CHECKED/N,BOUNCED/N,FORWARDED/N,REPLIED/N,EXECUTED/N,MOVED/N,DELETED/N
- `MAILUPDATE` -- Rebuild index of current folder
- `MAILEXPORT FILENAME/A [ALL/S] [APPEND/S]` -- Export messages to file
- `MAILIMPORT FILENAME/A [WAIT/S]` -- Import messages from file
- `SETFLAG [VOL/K/N] [PER/K/N]` -- Set volatile/permanent user flags (0-7) on message

## Mail Check
- `MAILCHECK [POP/K/N] [MANUAL/S]` -- Check for new mail. Returns DOWNLOADED/N,ONSERVER/N,DUPSKIPPED/N,DELETED/N

## Read Window
- `READCLOSE` -- Close active read window
- `READINFO` -- Get attachment info. Returns FILENAME/M,FILETYPE/M,FILESIZE/N/M,TEMPFILE/M
- `READPRINT [PART/N]` -- Print message or attachment
- `READSAVE [PART/N] [FILENAME/K] [OVERWRITE/S]` -- Save message or attachment

## Write Window
- `MAILWRITE [WINDOW/N] [QUIET/S]` -- Open new write window. Returns WINDOW/N
- `WRITETO ADDRESS/A/M [ADD/S]` -- Set "To" field (ADD=append)
- `WRITECC ADDRESS/A/M [ADD/S]` -- Set "CC" field (ADD=append)
- `WRITEBCC ADDRESS/A/M [ADD/S]` -- Set "BCC" field (ADD=append)
- `WRITEFROM ADDRESS/A` -- Set "From" field
- `WRITEREPLYTO ADDRESS/A` -- Set "Reply-To" field
- `WRITESUBJECT SUBJECT/A` -- Set subject
- `WRITEMAILTO ADDRESS/A/M` -- Set "To" field (replaces existing)
- `WRITEATTACH FILE/A [DESC] [ENCMODE] [CTYPE]` -- Add attachment (ENCMODE: uu or b64)
- `WRITEOPTIONS [DELETE/S] [RECEIPT/S] [NOTIF/S] [ADDINFO/S] [IMPORTANCE/N] [SIG/N] [SECURITY/N]` -- Set options (IMPORTANCE: 0=high,1=normal,2=low; SIG: 0=none,1-3; SECURITY: 0=normal,1-5)
- `WRITEEDITOR COMMAND/A` -- Send ARexx command to internal TextEditor
- `WRITESEND` -- Send the message
- `WRITEQUEUE [HOLD/S]` -- Queue message (HOLD=mark as held)

## User Interface
- `REQUEST BODY/A GADGETS/A` -- Show requester dialog. Returns RESULT/N (0=rightmost button, 1=leftmost, etc.)
- `REQUESTFOLDER BODY/A [EXCLUDEACTIVE/S]` -- Show folder selection dialog. Returns FOLDER
- `REQUESTSTRING BODY/A [STRING/K] [SECRET/S]` -- Show string input dialog. Returns STRING
- `HIDE` -- Iconify YAM
- `SHOW` -- Deiconify YAM
- `SCREENTOFRONT` -- Bring YAM screen to front
- `SCREENTOBACK` -- Send YAM screen to back

## Application
- `APPBUSY [TEXT]` -- Set busy state with status text. RC=1 if already busy
- `APPNOBUSY` -- Clear busy state. RC=1 if still busy
- `INFO [ITEM]` -- Get app info (ITEM: TITLE,AUTHOR,COPYRIGHT,DESCRIPTION,VERSION,BASE,SCREEN)
- `USERINFO` -- Get user info. Returns USERNAME,EMAIL,REALNAME,CONFIG,MAILDIR,FOLDERS/N
- `ISONLINE` -- Check if YAM is sending/receiving. RC=1 if online
- `HELP [FILE]` -- List supported ARexx commands
- `QUIT [FORCE/S]` -- Exit YAM (FORCE=no confirmation)

## Network
- `GETURL URL/A FILENAME/A` -- Download file via HTTP. RC=5 if offline, RC=10 if not found
