# IBrowse 2 — ARexx-Schnittstelle

## Allgemeines

IBrowse ist ein Webbrowser fuer AmigaOS mit umfangreicher ARexx-Steuerung.

**Port-Name:** `IBROWSE`

### Parameter-Typen

| Kuerzel | Bedeutung |
|---------|-----------|
| `/A` | Pflichtparameter (required) |
| `/S` | Schalter (Switch, boolean) |
| `/N` | Numerischer Wert |

### BROWSERNR

Viele Befehle akzeptieren `BROWSERNR/N` — die Browser-ID, die in der Titelzeile als `[ID]` angezeigt wird. Ohne Angabe wird der aktive Browser verwendet.

### Fehlercodes (RC)

| RC | Bedeutung |
|----|-----------|
| -2 | Nicht genuegend Speicher (Out of memory) |
| -3 | Unbekannter ARexx-Befehl |
| -4 | Syntaxfehler |
| -5 | Unbekanntes Item |
| -6 | Kein Browser aktiv oder Browsernummer ungueltig |

---

## Standard-MUI-Befehle

### QUIT

Beendet IBrowse.

```
QUIT [FORCE/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `FORCE/S` | Bestaetigung ueberspringen |

```rexx
ADDRESS IBROWSE
'QUIT FORCE'
```

### SHOW / ACTIVATE

Stellt IBrowse aus dem ikonifizierten Zustand wieder her und bringt es in den Vordergrund.

```
SHOW
ACTIVATE
```

### HIDE / DEACTIVATE

Ikonifiziert IBrowse.

```
HIDE
DEACTIVATE
```

### INFO

Gibt Programminformationen zurueck.

```
INFO ITEM/A
```

| ITEM | Beschreibung |
|------|-------------|
| `TITLE` | Programmname |
| `AUTHOR` | Autor |
| `COPYRIGHT` | Copyright-Hinweis |
| `DESCRIPTION` | Beschreibung |
| `VERSION` | Versionsnummer |
| `BASE` | Basisname |
| `SCREEN` | Bildschirmname |

RC = -5 wenn ITEM unbekannt.

```rexx
OPTIONS RESULTS
ADDRESS IBROWSE
'INFO ITEM=TITLE'
SAY RESULT
```

### HELP

Schreibt eine Liste aller ARexx-Befehle in eine Datei.

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

Laedt eine URL im Browser.

```
GOTOURL URL/A [BROWSERNR/N] [SAVE/S] [RELOAD/S] [MIME]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `URL/A` | Die zu ladende URL |
| `BROWSERNR/N` | Zielbrowser (Standard: aktiver Browser) |
| `SAVE/S` | Datei herunterladen statt anzeigen |
| `RELOAD/S` | Cache umgehen, neu laden |
| `MIME` | MIME-Typ manuell ueberschreiben |

RC = -6 wenn kein Browser aktiv.

```rexx
ADDRESS IBROWSE
'GOTOURL "http://www.amiga.org" BROWSERNR=1'
```

```rexx
/* Datei herunterladen */
'GOTOURL "http://example.com/file.lha" SAVE'
```

### BACK

Navigiert zurueck in der History.

```
BACK [BROWSERNR/N]
```

### FORWARD

Navigiert vorwaerts in der History.

```
FORWARD [BROWSERNR/N]
```

### HOME

Laedt die Homepage (aus den Einstellungen).

```
HOME [BROWSERNR/N]
```

### STOP

Stoppt den aktuellen Ladevorgang.

```
STOP [BROWSERNR/N]
```

### RELOAD

Laedt die aktuelle Seite neu.

```
RELOAD [BROWSERNR/N] [ALL/S] [FRAMES/S] [IMAGES/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `ALL/S` | Alles neu laden |
| `FRAMES/S` | Nur Frames neu laden |
| `IMAGES/S` | Nur Bilder neu laden |

### LOADIMAGES

Laedt noch nicht geladene Bilder nach.

```
LOADIMAGES [BROWSERNR/N]
```

---

## Fenster und Browser

### NEW

Oeffnet ein neues Fenster oder einen neuen Browser (je nach Voreinstellung).

```
NEW [URL]
```

Gibt in RESULT die Nummer des neuen Browsers/Fensters zurueck.

### NEWWINDOW

Oeffnet explizit ein neues Fenster.

```
NEWWINDOW [URL]
```

Gibt in RESULT die neue Fensternummer zurueck.

### NEWBROWSER

Oeffnet einen neuen Browser-Tab in einem bestehenden Fenster.

```
NEWBROWSER [URL] [BROWSEWINDOWNR/N]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `URL` | URL die geladen werden soll |
| `BROWSEWINDOWNR/N` | Zielfenster (Standard: aktives Fenster) |

Gibt in RESULT die neue Browsernummer zurueck. RC = -6 wenn Fensternummer ungueltig.

### CLOSEBROWSER

Schliesst einen Browser.

```
CLOSEBROWSER [BROWSERNR/N]
```

### SCREENTOFRONT / SCREENTOBACK

Bringt den IBrowse-Bildschirm nach vorne bzw. nach hinten.

```
SCREENTOFRONT
SCREENTOBACK
```

---

## Abfragen

### QUERY

Fragt Informationen zum aktuellen Browser ab.

```
QUERY ITEM/A [BROWSERNR/N]
```

| ITEM | Beschreibung |
|------|-------------|
| `URL` | Aktuelle URL des Browsers |
| `TITLE` | Seitentitel |
| `ACTIVEBROWSERNR` | Nummer des aktiven Browsers |
| `ACTIVEWINDOWNR` | Nummer des aktiven Fensters |

RC = -5 wenn ITEM unbekannt, RC = -6 wenn kein Browser aktiv.

```rexx
OPTIONS RESULTS
ADDRESS IBROWSE
'QUERY ITEM=URL BROWSERNR=2'
SAY "URL:" RESULT

'QUERY ITEM=TITLE'
SAY "Titel:" RESULT
```

---

## Lesezeichen (Hotlist)

### ADDHOTLIST

Fuegt einen Eintrag in den Hotlist Manager ein.

```
ADDHOTLIST [GROUP/S] TITLE/A [URL] [SHORTCUT] [SHOWINMENU/N]
           [INSERTGROUP] [ACTIVATE/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `GROUP/S` | Gruppe erstellen (statt Lesezeichen) |
| `TITLE/A` | Name des Eintrags |
| `URL` | URL (nicht noetig fuer Gruppen) |
| `SHORTCUT` | Tastenkuerzel |
| `SHOWINMENU/N` | 0=nicht im Menue, >0=im Menue anzeigen |
| `INSERTGROUP` | Zielgruppe (Gross-/Kleinschreibung egal) |
| `ACTIVATE/S` | Hotlist Manager oeffnen und Eintrag anzeigen |

```rexx
ADDRESS IBROWSE
'ADDHOTLIST TITLE="Amiga News" URL="http://www.amiga.org" INSERTGROUP="Amiga"'
```

```rexx
/* Neue Gruppe erstellen */
'ADDHOTLIST GROUP TITLE="Meine Links"'
```

### ADDFASTLINK

Fuegt einen Fastlink-Button hinzu.

```
ADDFASTLINK TITLE/A [URL] [PROMPTTITLE/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `TITLE/A` | Titel des Buttons |
| `URL` | URL |
| `PROMPTTITLE/S` | Eingabedialog anzeigen |

### OPENHOTLIST

Oeffnet oder schliesst den Hotlist Manager.

```
OPENHOTLIST [CLOSE/S]
```

---

## Weitere Fenster

### OPENHISTORY

Oeffnet oder schliesst das History-Fenster.

```
OPENHISTORY [BROWSERNR/N] [CLOSE/S]
```

### OPENCACHEBROWSER

Oeffnet oder schliesst den Cache Explorer.

```
OPENCACHEBROWSER [CLOSE/S]
```

### OPENINFOWINDOW

Oeffnet oder schliesst das Info-Fenster.

```
OPENINFOWINDOW [CLOSE/S]
```

---

## Speicherverwaltung

### FLUSH

Gibt Speicher frei.

```
FLUSH [BROWSERNR/N] [HISTORY/S] [ALLIMAGES/S] [CACHEDIMAGES/S] [IMAGES/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `BROWSERNR/N` | Zielbrowser (nur fuer HISTORY und IMAGES relevant) |
| `HISTORY/S` | Browser-History loeschen |
| `ALLIMAGES/S` | Alle Bilder aus dem Speicher entfernen (ignoriert BROWSERNR) |
| `CACHEDIMAGES/S` | Gecachte Bilder entfernen (ignoriert BROWSERNR) |
| `IMAGES/S` | Bilder des Browsers entfernen |

```rexx
ADDRESS IBROWSE
'FLUSH HISTORY BROWSERNR=2'
'FLUSH ALLIMAGES'
```

---

## Beispiel-Skripte

### URL im Hintergrund laden

```rexx
/* LoadURL.rexx - URL in IBrowse laden */
PARSE ARG url
IF url = '' THEN DO
    SAY 'Usage: rx LoadURL.rexx <url>'
    EXIT 5
END

ADDRESS IBROWSE
'GOTOURL "'url'"'
IF RC ~= 0 THEN DO
    SAY 'Fehler: Kein Browser aktiv, oeffne neues Fenster...'
    'NEWWINDOW "'url'"'
END
```

### Alle offenen URLs auflisten

```rexx
/* ListURLs.rexx - URLs aller offenen Browser anzeigen */
OPTIONS RESULTS
ADDRESS IBROWSE

DO i = 1 TO 20
    'QUERY ITEM=URL BROWSERNR='i
    IF RC = 0 THEN
        SAY 'Browser' i': ' RESULT
END
```

### Seite herunterladen

```rexx
/* Download.rexx - Datei herunterladen */
PARSE ARG url
ADDRESS IBROWSE
'GOTOURL "'url'" SAVE'
```
