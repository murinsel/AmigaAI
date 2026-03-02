# YAM 2 — ARexx-Schnittstelle

## Allgemeines

YAM (Yet Another Mailer) stellt einen ARexx-Port bereit, über den alle wichtigen Funktionen ferngesteuert werden können.

**Port-Name:** `YAM`

### Parameter-Typen

| Kürzel | Bedeutung |
|--------|-----------|
| `/A` | Pflichtparameter (required) |
| `/K` | Keyword — muss mit Schlüsselwort eingeleitet werden |
| `/N` | Numerischer Wert |
| `/S` | Schalter (Switch, boolean) |
| `/M` | Mehrfachwert (Liste mit null oder mehr Elementen) |

### Ergebnisrückgabe

Befehle mit Rückgabewerten unterstützen drei Varianten:

**Direkt in RESULT:**
```rexx
FOLDERINFO
/* RESULT = "0 Eingang incoming 10 2 4 23030 1" */
```

**In benannte Variable (VAR):**
```rexx
FOLDERINFO VAR fi
/* fi = "0 Eingang incoming 10 2 4 23030 1" */
```

**In Stem-Variable (STEM) — strukturierter Zugriff:**
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

**Mehrfachwerte (/M) mit STEM:**
```rexx
ADDRFIND STEM found. "Marcel Beck" NAMEONLY
/* found.alias.count = 2  */
/* found.alias.0 = "Mars" */
/* found.alias.1 = "mbe"  */
```

### Strings mit Leerzeichen

Parameter mit Leerzeichen müssen in Anführungszeichen übergeben werden:
```rexx
/* FALSCH: */
sub = 'Hallo Welt'
'WRITESUBJECT' sub

/* RICHTIG: */
'WRITESUBJECT "'sub'"'
/* oder: */
'WRITESUBJECT "Hallo Welt"'
```

---

## Adressbuch

### ADDRNEW

Erstellt einen neuen Adressbucheintrag unterhalb des aktuellen Eintrags.

```
ADDRNEW [VAR/K] [STEM/K] [TYPE] [ALIAS] [NAME] [EMAIL] => ALIAS
```

| Parameter | Beschreibung |
|-----------|-------------|
| `TYPE` | `G` = Gruppe, `L` = Verteilerliste, Standard = Person |
| `ALIAS` | Alias des neuen Eintrags |
| `NAME` | Voller Name |
| `EMAIL` | E-Mail-Adresse |

Mindestens eines der Felder ALIAS, NAME oder EMAIL muss angegeben werden. Gibt den Alias des erstellten Eintrags zurück.

```rexx
'ADDRNEW ALIAS "jd" NAME "John Doe" EMAIL "john@example.com"'
```

### ADDREDIT

Bearbeitet die Felder des aktuellen Adressbucheintrags.

```
ADDREDIT [ALIAS] [NAME] [EMAIL] [PGP] [HOMEPAGE] [STREET] [CITY]
         [COUNTRY] [PHONE] [COMMENT] [BIRTHDATE/N] [IMAGE]
         [MEMBER/M] [ADD/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `ALIAS` | Alias ändern |
| `NAME` | Voller Name |
| `EMAIL` | E-Mail-Adresse |
| `PGP` | PGP-Schlüssel-ID |
| `HOMEPAGE` | Homepage-URL |
| `STREET` | Straße |
| `CITY` | Stadt |
| `COUNTRY` | Land |
| `PHONE` | Telefon |
| `COMMENT` | Kommentar |
| `BIRTHDATE/N` | Geburtsdatum im Format TTMMJJJJ (z.B. 13091969) |
| `IMAGE` | Pfad zum Portrait-Bild |
| `MEMBER/M` | Mitgliederliste (für Verteilerlisten) |
| `ADD/S` | Mitglieder hinzufügen statt ersetzen |

```rexx
'ADDRGOTO "jd"'
'ADDREDIT PHONE "+49 123 456" CITY "Berlin" COUNTRY "Germany"'
```

### ADDRDELETE

Löscht einen Adressbucheintrag.

```
ADDRDELETE [ALIAS]
```

Wird kein Alias angegeben, wird der aktuelle Eintrag gelöscht.

### ADDRGOTO

Macht den angegebenen Eintrag zum aktuellen.

```
ADDRGOTO ALIAS/A
```

### ADDRINFO

Gibt Informationen zu einem Adressbucheintrag zurück.

```
ADDRINFO [VAR/K] [STEM/K] ALIAS/A
  => TYPE, NAME, EMAIL, PGP, HOMEPAGE, STREET, CITY, COUNTRY,
     PHONE, COMMENT, BIRTHDATE/N, IMAGE, MEMBERS/M
```

| Rückgabe | Beschreibung |
|----------|-------------|
| `TYPE` | `P` = Person, `L` = Verteilerliste, `G` = Gruppe |
| `BIRTHDATE` | Datum im Format JJJJMMTT |
| `IMAGE` | Dateipfad des Portraits |
| `MEMBERS` | Mitglieder einer Verteilerliste |

```rexx
'ADDRINFO STEM ai. "jd"'
say "Name:" ai.name
say "Email:" ai.email
say "Typ:" ai.type
```

### ADDRFIND

Sucht im Adressbuch nach Einträgen.

```
ADDRFIND [VAR/K] [STEM/K] PATTERN/A [NAMEONLY/S] [EMAILONLY/S] => ALIAS/M
```

| Parameter | Beschreibung |
|-----------|-------------|
| `PATTERN/A` | Suchbegriff |
| `NAMEONLY/S` | Nur im Feld "Voller Name" suchen |
| `EMAILONLY/S` | Nur im Feld "Adresse" suchen |

```rexx
'ADDRFIND STEM res. "example.com" EMAILONLY'
do i = 0 to res.alias.count - 1
    say res.alias.i
end
```

### ADDRRESOLVE

Wandelt einen Alias oder Namen in E-Mail-Adresse(n) um.

```
ADDRRESOLVE [VAR/K] [STEM/K] ALIAS/A => EMAIL/M
```

### ADDRLOAD / ADDRSAVE

Adressbuch laden und speichern.

```
ADDRLOAD FILENAME/A
ADDRSAVE [FILENAME]
```

Bei ADDRSAVE ohne Dateiname wird der aktuelle verwendet.

---

## Postfach-Verwaltung

### SETFOLDER

Wechselt in ein Postfach.

```
SETFOLDER FOLDER/A
```

FOLDER ist entweder die Nummer oder der Name des Postfachs.

```rexx
'SETFOLDER "Eingang"'
'SETFOLDER 0'          /* Posteingang (Index 0) */
```

### FOLDERINFO

Gibt Informationen zum Postfach zurück.

```
FOLDERINFO [VAR/K] [STEM/K] [FOLDER]
  => NUMBER/N, NAME, PATH, TOTAL/N, NEW/N, UNREAD/N, SIZE/N, TYPE/N
```

Ohne FOLDER-Parameter wird das aktuelle Postfach verwendet.

| Rückgabe | Beschreibung |
|----------|-------------|
| `NUMBER` | Interne Nummer (0 = Posteingang) |
| `NAME` | Anzeigename |
| `PATH` | Verzeichnisname |
| `TOTAL` | Gesamtzahl der Nachrichten |
| `NEW` | Anzahl neuer Nachrichten |
| `UNREAD` | Anzahl ungelesener Nachrichten |
| `SIZE` | Größe in Bytes |
| `TYPE` | 0=normal, 1=Eingang, 2=Ausgang, 3=Gesendet (Standard), 4=Gelöscht, 5=Gesendet+Empfang, 6=Gesendet |

```rexx
'FOLDERINFO STEM fi.'
say "Postfach:" fi.name "—" fi.total "Nachrichten," fi.new "neu"
```

### NEWMAILFILE

Liefert einen unbenutzten Dateinamen für eine neue Nachricht.

```
NEWMAILFILE [VAR/K] [STEM/K] [FOLDER] => FILENAME
```

---

## Nachrichten-Auswahl & Navigation

### SETMAIL

Wählt eine Nachricht im aktuellen Postfach nach Position (0-basiert).

```
SETMAIL NUM/N/A
```

### SETMAILFILE

Wählt eine Nachricht nach Dateiname.

```
SETMAILFILE MAILFILE/A
```

### LISTSELECT

Wählt Nachrichten im aktuellen Postfach aus.

```
LISTSELECT MODE/A
```

| MODE | Beschreibung |
|------|-------------|
| `N` | Keine auswählen (Auswahl aufheben) |
| `A` | Alle auswählen |
| `T` | Auswahl umschalten |
| numerisch | Nachricht an Position hinzufügen |

```rexx
'LISTSELECT A'    /* alle auswählen */
'LISTSELECT N'    /* Auswahl aufheben */
'LISTSELECT 5'    /* Nachricht 5 zur Auswahl hinzufügen */
```

### GETSELECTED

Gibt die Positionen der markierten Nachrichten zurück.

```
GETSELECTED [VAR/K] [STEM/K] => NUM/N/M
```

```rexx
'GETSELECTED STEM sel.'
do i = 0 to sel.num.count - 1
    say "Markiert:" sel.num.i
end
```

---

## Nachrichten-Informationen

### MAILINFO

Gibt detaillierte Informationen zu einer Nachricht zurück.

```
MAILINFO [VAR/K] [STEM/K] [INDEX/N]
  => INDEX/N, STATUS, FROM, TO, REPLYTO, SUBJECT, FILENAME,
     SIZE/N, DATE, FLAGS, MSGID/N
```

Ohne INDEX wird die aktuelle Nachricht verwendet.

| Rückgabe | Beschreibung |
|----------|-------------|
| `STATUS` | `N`=neu, `O`=gelesen, `U`=ungelesen, `R`=beantwortet, `F`=weitergeleitet, `S`=gesendet, `W`=wartend, `H`=gesperrt, `E`=Fehler |
| `FROM` | Absender |
| `TO` | Empfänger |
| `REPLYTO` | Antwortadresse |
| `SUBJECT` | Betreff |
| `FILENAME` | Dateipfad der Nachricht |
| `SIZE` | Größe in Bytes |
| `DATE` | Datum |
| `FLAGS` | 8-Byte-String im Format `"MARCS-00"` |
| `MSGID` | 32-Bit-komprimierte Message-ID |

**FLAGS-Format `"MARCS-00"`:**
- `M` = Mehrere Empfänger (Multipart)
- `A` = Anlagen (Attachments)
- `R` = Bericht (Report)
- `C` = Verschlüsselt (Crypted)
- `S` = Signiert (Signed)
- Letzte zwei Ziffern: benutzerdefinierbare Flags (siehe SETFLAG)

```rexx
'MAILINFO STEM mi. 0'
say "Von:" mi.from
say "Betreff:" mi.subject
say "Status:" mi.status
say "Größe:" mi.size "Bytes"
```

---

## Nachrichten-Aktionen

### MAILREAD

Öffnet die aktuelle Nachricht im Lesefenster.

```
MAILREAD [VAR/K] [STEM/K] [WINDOW/N] [QUIET/S] => WINDOW/N
```

| Parameter | Beschreibung |
|-----------|-------------|
| `WINDOW/N` | Bereits geöffnetes Fenster aktivieren |
| `QUIET/S` | Fenster unsichtbar öffnen |

### MAILREPLY

Öffnet ein Schreibfenster zum Beantworten der selektierten Nachricht.

```
MAILREPLY [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILFORWARD

Öffnet ein Schreibfenster zum Weiterleiten.

```
MAILFORWARD [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILBOUNCE

Öffnet ein Schreibfenster zum Umleiten (Redirect).

```
MAILBOUNCE [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILEDIT

Öffnet die Nachricht zum Bearbeiten.

```
MAILEDIT [VAR/K] [STEM/K] [QUIET/S] => WINDOW/N
```

### MAILCOPY

Kopiert die selektierten Nachrichten in ein anderes Postfach.

```
MAILCOPY FOLDER/A
```

FOLDER ist Name oder Nummer des Zielpostfachs.

### MAILMOVE

Verschiebt die selektierten Nachrichten.

```
MAILMOVE FOLDER/A
```

### MAILDELETE

Löscht die selektierten Nachrichten.

```
MAILDELETE [ATONCE/S] [FORCE/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `ATONCE/S` | Sofort löschen (nicht in Papierkorb) |
| `FORCE/S` | Keine Sicherheitsabfrage |

### MAILSTATUS

Ändert den Status der selektierten Nachrichten.

```
MAILSTATUS STATUS
```

| STATUS | Bedeutung |
|--------|-----------|
| `O` | Gelesen (Old) |
| `U` | Ungelesen (Unread) |
| `H` | Gesperrt (Hold) |
| `W` | Bereit zum Versand (Waiting) |

### MAILCHANGESUBJECT

Ändert den Betreff der selektierten Nachricht(en).

```
MAILCHANGESUBJECT SUBJECT/A
```

### MAILSEND

Versendet selektierte oder alle Nachrichten im Postausgang.

```
MAILSEND [ALL/S]
```

### MAILSENDALL

Versendet alle Nachrichten im Postausgang.

```
MAILSENDALL
```

### MAILFILTER

Filtert Nachrichten im aktuellen Postfach.

```
MAILFILTER [VAR/K] [STEM/K] [ALL/S]
  => CHECKED/N, BOUNCED/N, FORWARDED/N, REPLIED/N,
     EXECUTED/N, MOVED/N, DELETED/N
```

Ohne ALL werden nur neue Nachrichten gefiltert.

### MAILUPDATE

Erneuert den Index des aktuellen Postfachs.

```
MAILUPDATE
```

### MAILEXPORT

Exportiert Nachrichten in eine Datei.

```
MAILEXPORT FILENAME/A [ALL/S] [APPEND/S]
```

### MAILIMPORT

Importiert Nachrichten aus einer Datei.

```
MAILIMPORT FILENAME/A [WAIT/S]
```

Mit WAIT kann der Benutzer eine Vorauswahl treffen.

### SETFLAG

Setzt benutzerdefinierbare Flags an einer Nachricht (Werte 0-7).

```
SETFLAG [VOL/K/N] [PER/K/N]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `VOL` | Flüchtiges Flag (geht beim Neustart verloren) |
| `PER` | Permanentes Flag (im Dateikommentar gespeichert) |

---

## Post abholen

### MAILCHECK

Prüft auf neue E-Mails und lädt sie herunter.

```
MAILCHECK [VAR/K] [STEM/K] [POP/K/N] [MANUAL/S]
  => DOWNLOADED/N, ONSERVER/N, DUPSKIPPED/N, DELETED/N
```

| Parameter | Beschreibung |
|-----------|-------------|
| `POP/N` | Nur bestimmten POP3-Server abfragen (Nummer) |
| `MANUAL/S` | Benutzer kann Nachrichten vorauswählen |

```rexx
'MAILCHECK STEM mc.'
say mc.downloaded "neue Nachrichten heruntergeladen"
say mc.onserver "auf dem Server"
```

---

## Lesefenster

### READCLOSE

Schließt das aktive Lesefenster.

```
READCLOSE
```

### READINFO

Liefert Informationen über die Anlagen einer Nachricht im aktiven Lesefenster.

```
READINFO [VAR/K] [STEM/K] => FILENAME/M, FILETYPE/M, FILESIZE/N/M, TEMPFILE/M
```

```rexx
'READINFO STEM ri.'
do i = 0 to ri.filename.count - 1
    say "Anlage:" ri.filename.i "(" ri.filetype.i "," ri.filesize.i "Bytes)"
end
```

### READPRINT

Druckt eine Nachricht oder Anlage.

```
READPRINT [PART/N]
```

Ohne PART wird die sichtbare Nachricht gedruckt. Mit PART-Nummer wird die entsprechende Anlage gedruckt.

### READSAVE

Speichert eine Nachricht oder Anlage in eine Datei.

```
READSAVE [PART/N] [FILENAME/K] [OVERWRITE/S]
```

---

## Schreibfenster

### MAILWRITE

Öffnet ein neues Schreibfenster.

```
MAILWRITE [VAR/K] [STEM/K] [WINDOW/N] [QUIET/S] => WINDOW/N
```

### WRITETO

Setzt das "An"-Feld.

```
WRITETO ADDRESS/A/M [ADD/S]
```

Ohne ADD werden bestehende Adressen überschrieben.

### WRITECC

Setzt das "Kopie an"-Feld (CC).

```
WRITECC ADDRESS/A/M [ADD/S]
```

### WRITEBCC

Setzt das "Versteckte Kopie"-Feld (BCC).

```
WRITEBCC ADDRESS/A/M [ADD/S]
```

### WRITEMAILTO

Schreibt Adressen in das "To"-Feld und entfernt vorherige Einträge.

```
WRITEMAILTO ADDRESS/A/M
```

### WRITEFROM

Setzt das "Von"-Feld.

```
WRITEFROM ADDRESS/A
```

### WRITEREPLYTO

Setzt das "Antwort an"-Feld.

```
WRITEREPLYTO ADDRESS/A
```

### WRITESUBJECT

Setzt den Betreff.

```
WRITESUBJECT SUBJECT/A
```

```rexx
'WRITESUBJECT "Betreff mit Leerzeichen"'
```

### WRITEATTACH

Fügt eine Datei als Anlage hinzu.

```
WRITEATTACH FILE/A [DESC] [ENCMODE] [CTYPE]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `FILE/A` | Dateipfad |
| `DESC` | Beschreibung der Anlage |
| `ENCMODE` | `uu` (UUencode) oder `b64` (Base64) |
| `CTYPE` | MIME Content-Type |

```rexx
'WRITEATTACH "WORK:Bilder/foto.jpg" DESC "Urlaubsfoto" ENCMODE b64 CTYPE "image/jpeg"'
```

### WRITEOPTIONS

Setzt Optionen für die neue Nachricht.

```
WRITEOPTIONS [DELETE/S] [RECEIPT/S] [NOTIF/S] [ADDINFO/S]
             [IMPORTANCE/N] [SIG/N] [SECURITY/N]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `DELETE/S` | Nachricht nach Versand löschen |
| `RECEIPT/S` | Empfangsbestätigung anfordern |
| `NOTIF/S` | Nachrichtverfolgung (Notification) |
| `ADDINFO/S` | Persönliche Daten hinzufügen |
| `IMPORTANCE/N` | 0=hoch, 1=normal, 2=niedrig |
| `SIG/N` | Signatur: 0=keine, 1-3=Signatur 1-3 |
| `SECURITY/N` | 0=normal, 1-5=Sicherheitsstufen |

### WRITEEDITOR

Gibt einen ARexx-Befehl an den internen TextEditor (TextEditor.mcc) weiter.

```
WRITEEDITOR COMMAND/A
```

Siehe TextEditor.mcc-Dokumentation für verfügbare Befehle.

### WRITESEND

Sendet die Nachricht sofort.

```
WRITESEND
```

### WRITEQUEUE

Stellt die Nachricht in die Warteschlange.

```
WRITEQUEUE [HOLD/S]
```

Mit HOLD wird die Nachricht als "gesperrt" markiert und nicht automatisch versendet.

---

## Dialoge

### REQUEST

Öffnet ein Abfragefenster mit frei definierbaren Knöpfen.

```
REQUEST [VAR/K] [STEM/K] BODY/A GADGETS/A => RESULT/N
```

Rückgabewert: 0=rechter Knopf, 1=linker, 2=zweiter von links, usw.

```rexx
'REQUEST BODY "Nachricht wirklich löschen?" GADGETS "Ja|Nein|Abbrechen"'
if RESULT = 1 then say "Ja gewählt"
if RESULT = 2 then say "Nein gewählt"
if RESULT = 0 then say "Abbrechen gewählt"
```

### REQUESTFOLDER

Öffnet einen Postfach-Auswahldialog.

```
REQUESTFOLDER [VAR/K] [STEM/K] BODY/A [EXCLUDEACTIVE/S] => FOLDER
```

RC=1 wenn der Benutzer abbricht.

### REQUESTSTRING

Öffnet einen Texteingabe-Dialog.

```
REQUESTSTRING [VAR/K] [STEM/K] BODY/A [STRING/K] [SECRET/S] => STRING
```

| Parameter | Beschreibung |
|-----------|-------------|
| `STRING/K` | Vorgabetext |
| `SECRET/S` | Eingabe verbergen (für Passwörter) |

RC=1 wenn der Benutzer abbricht.

---

## Anwendungssteuerung

### APPBUSY / APPNOBUSY

Setzt/beendet den "Busy"-Zustand von YAM.

```
APPBUSY [VAR/K] [STEM/K] [TEXT]
APPNOBUSY
```

Im Busy-Zustand sind keine Benutzereingaben möglich. Der Text wird in der Statuszeile angezeigt. RC=1 wenn YAM bereits beschäftigt ist.

```rexx
'APPBUSY TEXT "Bitte warten..."'
/* ... Operationen ... */
'APPNOBUSY'
```

### INFO

Gibt Programminformationen zurück.

```
INFO [VAR/K] [STEM/K] [ITEM] => VALUE
```

| ITEM | Beschreibung |
|------|-------------|
| `TITLE` | Programmname |
| `AUTHOR` | Autor |
| `COPYRIGHT` | Copyright-Hinweis |
| `DESCRIPTION` | Beschreibung |
| `VERSION` | Version |
| `BASE` | Basisname |
| `SCREEN` | Bildschirmname |

### USERINFO

Gibt Informationen zum angemeldeten Benutzer zurück.

```
USERINFO [VAR/K] [STEM/K] => USERNAME, EMAIL, REALNAME, CONFIG, MAILDIR, FOLDERS/N
```

```rexx
'USERINFO STEM ui.'
say "Benutzer:" ui.username
say "E-Mail:" ui.email
say "Postfächer:" ui.folders
```

### ISONLINE

Prüft ob YAM gerade E-Mails sendet oder empfängt.

```
ISONLINE
```

RC=1 wenn YAM online ist (sendet/empfängt).

### HIDE / SHOW

Ikonifiziert YAM oder stellt es wieder her.

```
HIDE
SHOW
```

### SCREENTOFRONT / SCREENTOBACK

Bringt den YAM-Bildschirm nach vorne/hinten.

```
SCREENTOFRONT
SCREENTOBACK
```

### HELP

Gibt eine Liste aller unterstützten ARexx-Befehle aus.

```
HELP [FILE]
```

Ohne FILE wird in das Shell-Fenster ausgegeben.

### QUIT

Beendet YAM.

```
QUIT [FORCE/S]
```

Mit FORCE werden Sicherheitsabfragen unterdrückt.

---

## Netzwerk

### GETURL

Lädt eine Datei per HTTP aus dem Internet.

```
GETURL URL/A FILENAME/A
```

| RC | Bedeutung |
|----|-----------|
| 0 | Erfolgreich |
| 5 | Keine Online-Verbindung |
| 10 | URL nicht gefunden |

```rexx
'GETURL "http://example.com/datei.txt" "RAM:datei.txt"'
```

---

## Beispiel-Skripte

### Neue E-Mail senden

```rexx
/* SendMail.rexx - E-Mail über YAM senden */
ADDRESS YAM

'MAILWRITE QUIET'
window = RESULT

'WRITETO "empfaenger@example.com"'
'WRITESUBJECT "Test-Nachricht"'
'WRITEATTACH "WORK:dokument.pdf" DESC "Dokument" ENCMODE b64'
'WRITESEND'
```

### Postfach-Statistik

```rexx
/* FolderStats.rexx - Alle Postfächer anzeigen */
ADDRESS YAM

'USERINFO STEM ui.'
do i = 0 to ui.folders - 1
    'FOLDERINFO STEM fi.' i
    say fi.name': ' fi.total 'Nachrichten (' fi.new 'neu,' fi.unread 'ungelesen)'
end
```

### Ungelesene Nachrichten auflisten

```rexx
/* ListUnread.rexx - Ungelesene Nachrichten im Eingang */
ADDRESS YAM

'SETFOLDER 0'
'FOLDERINFO STEM fi.'

do i = 0 to fi.total - 1
    'MAILINFO STEM mi.' i
    if mi.status = 'U' | mi.status = 'N' then
        say mi.from ':' mi.subject
end
```
