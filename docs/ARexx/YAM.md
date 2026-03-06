# YAM 2 — ARexx Interface

## Overview

YAM (Yet Another Mailer) provides an ARexx port through which all important functions can be controlled remotely.

**Port name:** `YAM`

### Parameter Types

| Abbreviation | Meaning |
|--------|-----------|
| `/A` | Required parameter |
| `/K` | Keyword — must be preceded by a keyword |
| `/N` | Numeric value |
| `/S` | Switch (boolean) |
| `/M` | Multiple values (list with zero or more elements) |

### Return Values

Commands with return values support three variants:

**Directly in RESULT:**
```rexx
FOLDERINFO
/* RESULT = "0 Eingang incoming 10 2 4 23030 1" */
```

**In a named variable (VAR):**
```rexx
FOLDERINFO VAR fi
/* fi = "0 Eingang incoming 10 2 4 23030 1" */
```

**In a stem variable (STEM) — structured access:**
```rexx
FOLDERINFO STEM fi.
/* fi.number = 0        */
/* fi.name = "Eingang"  */
/* fi.path = "incoming" */
/* fi.total = 10        */
/* fi.new = 2           */
/* fi.unread = 4        */
/* fi.size = 23030      */
/* fi.type = 1          */
```

**Multiple values (/M) with STEM:**
```rexx
ADDRFIND STEM found. "Marcel Beck" NAMEONLY
/* found.alias.count = 2  */
/* found.alias.0 = "Mars" */
/* found.alias.1 = "mbe"  */
```

### Strings with Spaces

Parameters containing spaces must be passed in quotes:
```rexx
/* WRONG: */
sub = 'Hallo Welt'
'WRITESUBJECT' sub

/* CORRECT: */
'WRITESUBJECT "'sub'"'
/* or: */
'WRITESUBJECT "Hallo Welt"'
```

---

## Address Book

### ADDRNEW

Creates a new address book entry below the current entry.

```
ADDRNEW [VAR/K] [STEM/K] [TYPE] [ALIAS] [NAME] [EMAIL] => ALIAS
```

| Parameter | Description |
|-----------|-------------|
| `TYPE` | `G` = Group, `L` = Mailing list, Default = Person |
| `ALIAS` | Alias of the new entry |
| `NAME` | Full name |
| `EMAIL` | Email address |

At least one of the fields ALIAS, NAME, or EMAIL must be specified. Returns the alias of the created entry.

```rexx
'ADDRNEW ALIAS "jd" NAME "John Doe" EMAIL "john@example.com"'
```

### ADDREDIT

Edits the fields of the current address book entry.

```
ADDREDIT [ALIAS] [NAME] [EMAIL] [PGP] [HOMEPAGE] [STREET] [CITY]
         [COUNTRY] [PHONE] [COMMENT] [BIRTHDATE/N] [IMAGE]
         [MEMBER/M] [ADD/S]
```

| Parameter | Description |
|-----------|-------------|
| `ALIAS` | Change alias |
| `NAME` | Full name |
| `EMAIL` | Email address |
| `PGP` | PGP key ID |
| `HOMEPAGE` | Homepage URL |
| `STREET` | Street |
| `CITY` | City |
| `COUNTRY` | Country |
| `PHONE` | Phone |
| `COMMENT` | Comment |
| `BIRTHDATE/N` | Birth date in format DDMMYYYY (e.g. 13091969) |
| `IMAGE` | Path to portrait image |
| `MEMBER/M` | Member list (for mailing lists) |
| `ADD/S` | Add members instead of replacing |

```rexx
'ADDRGOTO "jd"'
'ADDREDIT PHONE "+49 123 456" CITY "Berlin" COUNTRY "Germany"'
```

### ADDRDELETE

Deletes an address book entry.

```
ADDRDELETE [ALIAS]
```

If no alias is specified, the current entry is deleted.

### ADDRGOTO

Makes the specified entry the current one.

```
ADDRGOTO ALIAS/A
```

### ADDRINFO

Returns information about an address book entry.

```
ADDRINFO [VAR/K] [STEM/K] ALIAS/A
  => TYPE, NAME, EMAIL, PGP, HOMEPAGE, STREET, CITY, COUNTRY,
     PHONE, COMMENT, BIRTHDATE/N, IMAGE, MEMBERS/M
```

| Return value | Description |
|----------|-------------|
| `TYPE` | `P` = Person, `L` = Mailing list, `G` = Group |
| `BIRTHDATE` | Date in format YYYYMMDD |
| `IMAGE` | File path of the portrait |
| `MEMBERS` | Members of a mailing list |

```rexx
'ADDRINFO STEM ai. "jd"'
say "Name:" ai.name
say "Email:" ai.email
say "Type:" ai.type
```

### ADDRFIND

Searches the address book for entries.

```
ADDRFIND [VAR/K] [STEM/K] PATTERN/A [NAMEONLY/S] [EMAILONLY/S] => ALIAS/M
```

| Parameter | Description |
|-----------|-------------|
| `PATTERN/A` | Search term |
| `NAMEONLY/S` | Search only in the "Full name" field |
| `EMAILONLY/S` | Search only in the "Address" field |

```rexx
'ADDRFIND STEM res. "example.com" EMAILONLY'
do i = 0 to res.alias.count - 1
    say res.alias.i
end
```

### ADDRRESOLVE

Resolves an alias or name to email address(es).

```
ADDRRESOLVE [VAR/K] [STEM/K] ALIAS/A => EMAIL/M
```

### ADDRLOAD / ADDRSAVE

Load and save the address book.

```
ADDRLOAD FILENAME/A
ADDRSAVE [FILENAME]
```

For ADDRSAVE without a filename, the current one is used.

---

## Mailbox Management

### SETFOLDER

Switches to a mailbox.

```
SETFOLDER FOLDER/A
```

FOLDER is either the number or the name of the mailbox.

```rexx
'SETFOLDER "Eingang"'
'SETFOLDER 0'          /* Inbox (index 0) */
```

### FOLDERINFO

Returns information about the mailbox.

```
FOLDERINFO [VAR/K] [STEM/K] [FOLDER]
  => NUMBER/N, NAME, PATH, TOTAL/N, NEW/N, UNREAD/N, SIZE/N, TYPE/N
```

Without a FOLDER parameter, the current mailbox is used.

| Return value | Description |
|----------|-------------|
| `NUMBER` | Internal number (0 = Inbox) |
| `NAME` | Display name |
| `PATH` | Directory name |
| `TOTAL` | Total number of messages |
| `NEW` | Number of new messages |
| `UNREAD` | Number of unread messages |
| `SIZE` | Size in bytes |
| `TYPE` | 0=normal, 1=Inbox, 2=Outbox, 3=Sent (default), 4=Deleted, 5=Sent+Received, 6=Sent |

```rexx
'FOLDERINFO STEM fi.'
say "Mailbox:" fi.name "—" fi.total "messages," fi.new "new"
```

### NEWMAILFILE

Returns an unused filename for a new message.

```
NEWMAILFILE [VAR/K] [STEM/K] [FOLDER] => FILENAME
```

---

## Message Selection & Navigation

### SETMAIL

Selects a message in the current mailbox by position (0-based).

```
SETMAIL NUM/N/A
```

### SETMAILFILE

Selects a message by filename.

```
SETMAILFILE MAILFILE/A
```

### LISTSELECT

Selects messages in the current mailbox.

```
LISTSELECT MODE/A
```

| MODE | Description |
|------|-------------|
| `N` | Select none (clear selection) |
| `A` | Select all |
| `T` | Toggle selection |
| numeric | Add message at position |

```rexx
'LISTSELECT A'    /* select all */
'LISTSELECT N'    /* clear selection */
'LISTSELECT 5'    /* add message 5 to selection */
```

### GETSELECTED

Returns the positions of the selected messages.

```
GETSELECTED [VAR/K] [STEM/K] => NUM/N/M
```

```rexx
'GETSELECTED STEM sel.'
do i = 0 to sel.num.count - 1
    say "Selected:" sel.num.i
end
```

---

## Message Information

### MAILINFO

Returns detailed information about a message.

```
MAILINFO [VAR/K] [STEM/K] [INDEX/N]
  => INDEX/N, STATUS, FROM, TO, REPLYTO, SUBJECT, FILENAME,
     SIZE/N, DATE, FLAGS, MSGID/N
```

Without INDEX, the current message is used.

| Return value | Description |
|----------|-------------|
| `STATUS` | `N`=new, `O`=read, `U`=unread, `R`=replied, `F`=forwarded, `S`=sent, `W`=waiting, `H`=hold, `E`=error |
| `FROM` | Sender |
| `TO` | Recipient |
| `REPLYTO` | Reply-to address |
| `SUBJECT` | Subject |
| `FILENAME` | File path of the message |
| `SIZE` | Size in bytes |
| `DATE` | Date |
| `FLAGS` | 8-byte string in format `"MARCS-00"` |
| `MSGID` | 32-bit compressed message ID |

**FLAGS format `"MARCS-00"`:**
- `M` = Multiple recipients (Multipart)
- `A` = Attachments
- `R` = Report
- `C` = Encrypted (Crypted)
- `S` = Signed
- Last two digits: user-definable flags (see SETFLAG)

```rexx
'MAILINFO STEM mi. 0'
say "From:" mi.from
say "Subject:" mi.subject
say "Status:" mi.status
say "Size:" mi.size "bytes"
```

---

## Message Actions

### MAILREAD

Opens the current message in the read window.

```
MAILREAD [VAR/K] [STEM/K] [WINDOW/N] [QUIET/S] => WINDOW/N
```

| Parameter | Description |
|-----------|-------------|
| `WINDOW/N` | Activate an already open window |
| `QUIET/S` | Open window invisibly |

### MAILREPLY

Opens a compose window to reply to the selected message.

```
MAILREPLY [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILFORWARD

Opens a compose window to forward.

```
MAILFORWARD [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILBOUNCE

Opens a compose window to redirect (bounce).

```
MAILBOUNCE [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILEDIT

Opens the message for editing.

```
MAILEDIT [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILCOPY

Copies the selected messages to another mailbox.

```
MAILCOPY FOLDER/A
```

FOLDER is the name or number of the destination mailbox.

### MAILMOVE

Moves the selected messages.

```
MAILMOVE FOLDER/A
```

### MAILDELETE

Deletes the selected messages.

```
MAILDELETE [ATONCE/S] [FORCE/S]
```

| Parameter | Description |
|-----------|-------------|
| `ATONCE/S` | Delete immediately (not to trash) |
| `FORCE/S` | No confirmation prompt |

### MAILSTATUS

Changes the status of the selected messages.

```
MAILSTATUS STATUS
```

| STATUS | Meaning |
|--------|-----------|
| `O` | Read (Old) |
| `U` | Unread |
| `H` | Hold |
| `W` | Ready to send (Waiting) |

### MAILCHANGESUBJECT

Changes the subject of the selected message(s).

```
MAILCHANGESUBJECT SUBJECT/A
```

### MAILSEND

Sends selected or all messages in the outbox.

```
MAILSEND [ALL/S]
```

### MAILSENDALL

Sends all messages in the outbox.

```
MAILSENDALL
```

### MAILFILTER

Filters messages in the current mailbox.

```
MAILFILTER [VAR/K] [STEM/K] [ALL/S]
  => CHECKED/N, BOUNCED/N, FORWARDED/N, REPLIED/N,
     EXECUTED/N, MOVED/N, DELETED/N
```

Without ALL, only new messages are filtered.

### MAILUPDATE

Refreshes the index of the current mailbox.

```
MAILUPDATE
```

### MAILEXPORT

Exports messages to a file.

```
MAILEXPORT FILENAME/A [ALL/S] [APPEND/S]
```

### MAILIMPORT

Imports messages from a file.

```
MAILIMPORT FILENAME/A [WAIT/S]
```

With WAIT, the user can make a pre-selection.

### SETFLAG

Sets user-definable flags on a message (values 0-7).

```
SETFLAG [VOL/K/N] [PER/K/N]
```

| Parameter | Description |
|-----------|-------------|
| `VOL` | Volatile flag (lost on restart) |
| `PER` | Permanent flag (stored in file comment) |

---

## Fetch Mail

### MAILCHECK

Checks for new emails and downloads them.

```
MAILCHECK [VAR/K] [STEM/K] [POP/K/N] [MANUAL/S]
  => DOWNLOADED/N, ONSERVER/N, DUPSKIPPED/N, DELETED/N
```

| Parameter | Description |
|-----------|-------------|
| `POP/N` | Query only a specific POP3 server (number) |
| `MANUAL/S` | User can pre-select messages |

```rexx
'MAILCHECK STEM mc.'
say mc.downloaded "new messages downloaded"
say mc.onserver "on server"
```

---

## Read Window

### READCLOSE

Closes the active read window.

```
READCLOSE
```

### READINFO

Returns information about the attachments of a message in the active read window.

```
READINFO [VAR/K] [STEM/K] => FILENAME/M, FILETYPE/M, FILESIZE/N/M, TEMPFILE/M
```

```rexx
'READINFO STEM ri.'
do i = 0 to ri.filename.count - 1
    say "Attachment:" ri.filename.i "(" ri.filetype.i "," ri.filesize.i "bytes)"
end
```

### READPRINT

Prints a message or attachment.

```
READPRINT [PART/N]
```

Without PART, the visible message is printed. With a PART number, the corresponding attachment is printed.

### READSAVE

Saves a message or attachment to a file.

```
READSAVE [PART/N] [FILENAME/K] [OVERWRITE/S]
```

---

## Compose Window

### MAILWRITE

Opens a new compose window.

```
MAILWRITE [VAR/K] [STEM/K] [WINDOW/N] [QUIET/S] => WINDOW/N
```

### WRITETO

Sets the "To" field.

```
WRITETO ADDRESS/A/M [ADD/S]
```

Without ADD, existing addresses are overwritten.

### WRITECC

Sets the "CC" (carbon copy) field.

```
WRITECC ADDRESS/A/M [ADD/S]
```

### WRITEBCC

Sets the "BCC" (blind carbon copy) field.

```
WRITEBCC ADDRESS/A/M [ADD/S]
```

### WRITEMAILTO

Writes addresses to the "To" field and removes previous entries.

```
WRITEMAILTO ADDRESS/A/M
```

### WRITEFROM

Sets the "From" field.

```
WRITEFROM ADDRESS/A
```

### WRITEREPLYTO

Sets the "Reply-To" field.

```
WRITEREPLYTO ADDRESS/A
```

### WRITESUBJECT

Sets the subject.

```
WRITESUBJECT SUBJECT/A
```

```rexx
'WRITESUBJECT "Subject with spaces"'
```

### WRITEATTACH

Adds a file as an attachment.

```
WRITEATTACH FILE/A [DESC] [ENCMODE] [CTYPE]
```

| Parameter | Description |
|-----------|-------------|
| `FILE/A` | File path |
| `DESC` | Description of the attachment |
| `ENCMODE` | `uu` (UUencode) or `b64` (Base64) |
| `CTYPE` | MIME Content-Type |

```rexx
'WRITEATTACH "WORK:Bilder/foto.jpg" DESC "Vacation photo" ENCMODE b64 CTYPE "image/jpeg"'
```

### WRITEOPTIONS

Sets options for the new message.

```
WRITEOPTIONS [DELETE/S] [RECEIPT/S] [NOTIF/S] [ADDINFO/S]
             [IMPORTANCE/N] [SIG/N] [SECURITY/N]
```

| Parameter | Description |
|-----------|-------------|
| `DELETE/S` | Delete message after sending |
| `RECEIPT/S` | Request read receipt |
| `NOTIF/S` | Message notification (tracking) |
| `ADDINFO/S` | Add personal information |
| `IMPORTANCE/N` | 0=high, 1=normal, 2=low |
| `SIG/N` | Signature: 0=none, 1-3=signature 1-3 |
| `SECURITY/N` | 0=normal, 1-5=security levels |

### WRITEEDITOR

Passes an ARexx command to the internal TextEditor (TextEditor.mcc).

```
WRITEEDITOR COMMAND/A
```

#### TextEditor.mcc ARexx Commands

These commands can be passed via `WRITEEDITOR`:

**Clipboard & Editing:**

| Command | Template | Description |
|---------|----------|-------------|
| `CLEAR` | — | Clear all editor contents |
| `CUT` | — | Cut selected text to clipboard |
| `COPY` | — | Copy selected text to clipboard |
| `PASTE` | — | Paste from clipboard at cursor |
| `ERASE` | — | Erase selected text (no clipboard) |
| `DELETE` | — | Delete character at cursor |
| `BACKSPACE` | — | Delete character before cursor |
| `KILLLINE` | — | Delete entire current line |
| `UNDO` | — | Undo last action |
| `REDO` | — | Redo last undone action |

**Navigation:**

| Command | Template | Description |
|---------|----------|-------------|
| `GOTOLINE` | `/N/A` | Go to line number |
| `GOTOCOLUMN` | `/N/A` | Go to column number |
| `CURSOR` | `Up/S,Down/S,Left/S,Right/S` | Move cursor one step |
| `NEXT` | `Word/S,Sentence/S,Paragraph/S,Page/S` | Move forward by unit |
| `PREVIOUS` | `Word/S,Sentence/S,Paragraph/S,Page/S` | Move backward by unit |
| `POSITION` | `SOF/S,EOF/S,SOL/S,EOL/S,SOW/S,EOW/S,SOV/S,EOV/S` | Jump to position |
| `SETBOOKMARK` | `/N/A` | Set bookmark (1-3) |
| `GOTOBOOKMARK` | `/N/A` | Jump to bookmark (1-3) |

Position abbreviations: SOF=Start Of File, EOF=End Of File, SOL=Start Of Line, EOL=End Of Line, SOW=Start Of Word, EOW=End Of Word, SOV=Start Of View, EOV=End Of View.

**Text & Query:**

| Command | Template | Description |
|---------|----------|-------------|
| `TEXT` | `/F` | Insert text at cursor position |
| `GETLINE` | — | Get current line text → RESULT |
| `LINE` | `/N/A` | Get/set current line number → RESULT |
| `COLUMN` | `/N/A` | Get/set current column → RESULT |
| `GETCURSOR` | `Line/S,Column/S` | Get cursor position → RESULT |
| `MARK` | `On/S,Off/S` | Start or stop text selection |

**Case Conversion:**

| Command | Template | Description |
|---------|----------|-------------|
| `TOUPPER` | — | Convert selected text to uppercase |
| `TOLOWER` | — | Convert selected text to lowercase |

#### Examples

```rexx
/* Insert text at cursor position */
WRITEEDITOR 'TEXT Hello World'

/* Go to beginning of file */
WRITEEDITOR 'POSITION SOF'

/* Go to line 10, column 5 */
WRITEEDITOR 'GOTOLINE 10'
WRITEEDITOR 'GOTOCOLUMN 5'

/* Select and copy a line */
WRITEEDITOR 'POSITION SOL'
WRITEEDITOR 'MARK ON'
WRITEEDITOR 'POSITION EOL'
WRITEEDITOR 'COPY'
WRITEEDITOR 'MARK OFF'

/* Get the current line text */
WRITEEDITOR 'GETLINE'
SAY RESULT

/* Get cursor position */
WRITEEDITOR 'GETCURSOR Line'
line = RESULT
WRITEEDITOR 'GETCURSOR Column'
col = RESULT
SAY 'Cursor at line' line 'column' col
```

### WRITESEND

Sends the message immediately.

```
WRITESEND
```

### WRITEQUEUE

Puts the message in the queue.

```
WRITEQUEUE [HOLD/S]
```

With HOLD, the message is marked as "hold" and not sent automatically.

---

## Dialogs

### REQUEST

Opens a requester window with freely definable buttons.

```
REQUEST [VAR/K] [STEM/K] BODY/A GADGETS/A => RESULT/N
```

Return value: 0=rightmost button, 1=leftmost, 2=second from left, etc.

```rexx
'REQUEST BODY "Really delete message?" GADGETS "Yes|No|Cancel"'
if RESULT = 1 then say "Yes selected"
if RESULT = 2 then say "No selected"
if RESULT = 0 then say "Cancel selected"
```

### REQUESTFOLDER

Opens a mailbox selection dialog.

```
REQUESTFOLDER [VAR/K] [STEM/K] BODY/A [EXCLUDEACTIVE/S] => FOLDER
```

RC=1 if the user cancels.

### REQUESTSTRING

Opens a text input dialog.

```
REQUESTSTRING [VAR/K] [STEM/K] BODY/A [STRING/K] [SECRET/S] => STRING
```

| Parameter | Description |
|-----------|-------------|
| `STRING/K` | Default text |
| `SECRET/S` | Hide input (for passwords) |

RC=1 if the user cancels.

---

## Application Control

### APPBUSY / APPNOBUSY

Sets/clears the "busy" state of YAM.

```
APPBUSY [VAR/K] [STEM/K] [TEXT]
APPNOBUSY
```

In busy state, no user input is possible. The text is displayed in the status bar. RC=1 if YAM is already busy.

```rexx
'APPBUSY TEXT "Please wait..."'
/* ... operations ... */
'APPNOBUSY'
```

### INFO

Returns program information.

```
INFO [VAR/K] [STEM/K] [ITEM] => VALUE
```

| ITEM | Description |
|------|-------------|
| `TITLE` | Program name |
| `AUTHOR` | Author |
| `COPYRIGHT` | Copyright notice |
| `DESCRIPTION` | Description |
| `VERSION` | Version |
| `BASE` | Base name |
| `SCREEN` | Screen name |

### USERINFO

Returns information about the logged-in user.

```
USERINFO [VAR/K] [STEM/K] => USERNAME, EMAIL, REALNAME, CONFIG, MAILDIR, FOLDERS/N
```

```rexx
'USERINFO STEM ui.'
say "User:" ui.username
say "Email:" ui.email
say "Mailboxes:" ui.folders
```

### ISONLINE

Checks whether YAM is currently sending or receiving emails.

```
ISONLINE
```

RC=1 if YAM is online (sending/receiving).

### HIDE / SHOW

Iconifies YAM or restores it.

```
HIDE
SHOW
```

### SCREENTOFRONT / SCREENTOBACK

Brings the YAM screen to the front/back.

```
SCREENTOFRONT
SCREENTOBACK
```

### HELP

Outputs a list of all supported ARexx commands.

```
HELP [FILE]
```

Without FILE, output goes to the shell window.

### QUIT

Quits YAM.

```
QUIT [FORCE/S]
```

With FORCE, confirmation prompts are suppressed.

---

## Network

### GETURL

Downloads a file via HTTP from the internet.

```
GETURL URL/A FILENAME/A
```

| RC | Meaning |
|----|-----------|
| 0 | Successful |
| 5 | No online connection |
| 10 | URL not found |

```rexx
'GETURL "http://example.com/datei.txt" "RAM:datei.txt"'
```

---

## Example Scripts

### Send a New Email

```rexx
/* SendMail.rexx - Send email via YAM */
ADDRESS YAM

'MAILWRITE QUIET'
window = RESULT

'WRITETO "recipient@example.com"'
'WRITESUBJECT "Test message"'
'WRITEATTACH "WORK:dokument.pdf" DESC "Document" ENCMODE b64'
'WRITESEND'
```

### Mailbox Statistics

```rexx
/* FolderStats.rexx - Display all mailboxes */
ADDRESS YAM

'USERINFO STEM ui.'
do i = 0 to ui.folders - 1
    'FOLDERINFO STEM fi.' i
    say fi.name': ' fi.total 'messages (' fi.new 'new,' fi.unread 'unread)'
end
```

### List Unread Messages

```rexx
/* ListUnread.rexx - Unread messages in inbox */
ADDRESS YAM

'SETFOLDER 0'
'FOLDERINFO STEM fi.'

do i = 0 to fi.total - 1
    'MAILINFO STEM mi.' i
    if mi.status = 'U' | mi.status = 'N' then
        say mi.from ':' mi.subject
end
```
