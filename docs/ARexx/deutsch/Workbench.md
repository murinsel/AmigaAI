# Workbench — ARexx-Schnittstelle

## Allgemeines

Die Workbench stellt einen ARexx-Port bereit, ueber den Fenster, Icons, Menues und weitere Funktionen ferngesteuert werden koennen.

**Port-Name:** `WORKBENCH`

**Voraussetzungen:** rexxsyslib.library und RexxMast muessen laufen.

### Fenster-Referenzen

Viele Befehle akzeptieren einen `WINDOW`-Parameter:
- `ROOT` — Das Hauptfenster der Workbench
- Vollstaendiger Pfad — z.B. `Work:` oder `SYS:Utilities`

### Fehlercodes

Bei Fehlern wird RC auf 10 gesetzt. Die Fehlermeldung kann ueber `GETATTR APPLICATION.LASTERROR` oder den `FAULT`-Befehl abgefragt werden.

---

## Fenster-Verwaltung

### WINDOW

Oeffnet, schliesst und verwaltet Fenster.

```
WINDOW WINDOWS/M/A [OPEN/S] [CLOSE/S] [SNAPSHOT/S] [ACTIVATE/S]
       [MIN/S] [MAX/S] [FRONT/S] [BACK/S] [CYCLE/K]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `WINDOWS/M/A` | Fensternamen (ROOT oder Pfade) |
| `OPEN/S` | Fenster oeffnen |
| `CLOSE/S` | Fenster schliessen |
| `SNAPSHOT/S` | Position und Groesse speichern |
| `ACTIVATE/S` | Fenster aktivieren |
| `MIN/S` | Auf Minimalgroesse verkleinern |
| `MAX/S` | Auf Maximalgroesse vergroessern |
| `FRONT/S` | In den Vordergrund |
| `BACK/S` | In den Hintergrund |
| `CYCLE/K` | Fenster durchschalten: `PREVIOUS` oder `NEXT` |

```rexx
ADDRESS WORKBENCH
'WINDOW "Work:" OPEN'
'WINDOW "Work:" FRONT ACTIVATE'
'WINDOW root SNAPSHOT'
'WINDOW "SYS:Utilities" CLOSE'
```

### ACTIVATEWINDOW

Macht ein Fenster zum aktiven.

```
ACTIVATEWINDOW [WINDOW]
```

```rexx
'ACTIVATEWINDOW "Work:"'
'ACTIVATEWINDOW root'
```

### CHANGEWINDOW

Aendert Position und Groesse eines Fensters gleichzeitig.

```
CHANGEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N] [WIDTH/N] [HEIGHT/N]
```

```rexx
'CHANGEWINDOW root LEFTEDGE 10 TOPEDGE 30 WIDTH 400 HEIGHT 300'
```

### MOVEWINDOW

Verschiebt ein Fenster.

```
MOVEWINDOW [WINDOW] [LEFTEDGE/N] [TOPEDGE/N]
```

### SIZEWINDOW

Aendert die Groesse eines Fensters.

```
SIZEWINDOW [WINDOW] [WIDTH/N] [HEIGHT/N]
```

### WINDOWTOFRONT / WINDOWTOBACK

Bringt ein Fenster nach vorne bzw. hinten.

```
WINDOWTOFRONT [WINDOW]
WINDOWTOBACK [WINDOW]
```

### ZOOMWINDOW / UNZOOMWINDOW

Wechselt zwischen normaler und alternativer Fenstergroesse.

```
ZOOMWINDOW [WINDOW]
UNZOOMWINDOW [WINDOW]
```

### VIEW

Scrollt den sichtbaren Bereich eines Fensters.

```
VIEW [WINDOW] [PAGE/S] [PIXEL/S] [UP/S] [DOWN/S] [LEFT/S] [RIGHT/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `PAGE/S` | Seitenweise scrollen |
| `PIXEL/S` | Pixelweise scrollen |
| `UP/S` `DOWN/S` `LEFT/S` `RIGHT/S` | Scrollrichtung |

```rexx
'VIEW root PAGE DOWN'
'VIEW "Work:" PIXEL RIGHT'
```

### LOCKGUI / UNLOCKGUI

Sperrt bzw. entsperrt die Benutzeroberflaeche. Verschachtelt — jedes LOCKGUI braucht ein passendes UNLOCKGUI.

```
LOCKGUI
UNLOCKGUI
```

```rexx
'LOCKGUI'
/* ... Operationen ... */
'UNLOCKGUI'
```

---

## Icon-Verwaltung

### ICON

Manipuliert Icons in einem Fenster.

```
ICON [WINDOW] NAMES/M [OPEN/S] [MAKEVISIBLE/S] [SELECT/S] [UNSELECT/S]
     [UP/N] [DOWN/N] [LEFT/N] [RIGHT/N] [X/N] [Y/N]
     [ACTIVATE/K] [CYCLE/K] [MOVE/K]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `NAMES/M` | Icon-Namen |
| `OPEN/S` | Icon oeffnen (Programm starten / Schublade oeffnen) |
| `MAKEVISIBLE/S` | Icon sichtbar machen (in den Fensterausschnitt scrollen) |
| `SELECT/S` | Icon auswaehlen |
| `UNSELECT/S` | Auswahl aufheben |
| `UP/N` `DOWN/N` `LEFT/N` `RIGHT/N` | Icon um N Pixel verschieben |
| `X/N` `Y/N` | Icon an absolute Position setzen |
| `ACTIVATE/K` | Naechstes Icon aktivieren: `UP`, `DOWN`, `LEFT`, `RIGHT` |
| `CYCLE/K` | Durch Icons schalten: `PREVIOUS`, `NEXT` |
| `MOVE/K` | In Hierarchie navigieren: `IN` (hinein), `OUT` (heraus) |

```rexx
ADDRESS WORKBENCH
/* Programm starten */
'ICON WINDOW root NAMES Workbench Work SELECT'

/* Icon verschieben */
'ICON WINDOW "Work:" NAMES "MeinProgramm" X 100 Y 50'

/* Icon oeffnen (Programm starten) */
'ICON WINDOW "SYS:Utilities" NAMES MultiView OPEN'
```

### INFO

Oeffnet den Informations-Dialog fuer eine Datei, Schublade oder Volume.

```
INFO NAME/A
```

```rexx
'INFO NAME "SYS:"'
'INFO NAME "Work:MeineDatei"'
```

---

## Datei-Operationen

### NEWDRAWER

Erstellt eine neue Schublade.

```
NEWDRAWER NAME/A
```

NAME muss ein absoluter Pfad sein.

```rexx
'NEWDRAWER "RAM:NeueSchublade"'
'NEWDRAWER "Work:Projekte/Neu"'
```

### DELETE

Loescht Dateien oder Schubladen.

```
DELETE NAME/A [ALL/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `NAME/A` | Absoluter Pfad der Datei/Schublade |
| `ALL/S` | Inhalt rekursiv loeschen |

```rexx
'DELETE "RAM:TempDatei"'
'DELETE "RAM:TempOrdner" ALL'
```

### RENAME

Benennt eine Datei, Schublade oder Volume um.

```
RENAME OLDNAME/A NEWNAME/A
```

OLDNAME muss ein absoluter Pfad sein. NEWNAME ist nur der neue Name (ohne Pfad).

```rexx
'RENAME "RAM:AlterName" "NeuerName"'
```

---

## Abfragen

### GETATTR

Fragt Informationen aus der Workbench-Datenbank ab.

```
GETATTR OBJECT/A [NAME/K] [STEM/K] [VAR/K]
```

### Abfragbare Objekte

**Anwendung:**

| Objekt | Beschreibung |
|--------|-------------|
| `APPLICATION.VERSION` | Workbench-Version |
| `APPLICATION.SCREEN` | Bildschirmname |
| `APPLICATION.AREXX` | ARexx-Port-Name |
| `APPLICATION.LASTERROR` | Letzte Fehlermeldung |
| `APPLICATION.ICONBORDER` | Icon-Rahmen aktiv? |

**Schriften:**

| Objekt | Beschreibung |
|--------|-------------|
| `APPLICATION.FONT.SCREEN.NAME` | Bildschirmschrift Name |
| `APPLICATION.FONT.SCREEN.SIZE` | Bildschirmschrift Groesse |
| `APPLICATION.FONT.ICON.NAME` | Icon-Schrift Name |
| `APPLICATION.FONT.ICON.SIZE` | Icon-Schrift Groesse |
| `APPLICATION.FONT.SYSTEM.NAME` | Systemschrift Name |
| `APPLICATION.FONT.SYSTEM.SIZE` | Systemschrift Groesse |

**Fenster:**

| Objekt | Beschreibung |
|--------|-------------|
| `WINDOWS.COUNT` | Anzahl offener Fenster |
| `WINDOWS.ACTIVE` | Pfad des aktiven Fensters |
| `WINDOWS.0` ... `WINDOWS.N` | Pfad des N-ten Fensters |

**Fenster-Details (mit NAME=Fensterpfad):**

| Objekt | Beschreibung |
|--------|-------------|
| `WINDOW.LEFT` | Linke Position |
| `WINDOW.TOP` | Obere Position |
| `WINDOW.WIDTH` | Breite |
| `WINDOW.HEIGHT` | Hoehe |
| `WINDOW.MIN.WIDTH` | Minimale Breite |
| `WINDOW.MIN.HEIGHT` | Minimale Hoehe |
| `WINDOW.MAX.WIDTH` | Maximale Breite |
| `WINDOW.MAX.HEIGHT` | Maximale Hoehe |
| `WINDOW.VIEW.LEFT` | Sichtbarer Bereich links |
| `WINDOW.VIEW.TOP` | Sichtbarer Bereich oben |
| `WINDOW.ICONS.COUNT` | Anzahl Icons im Fenster |
| `WINDOW.ICONS.0` ... | Icon-Namen |

```rexx
OPTIONS RESULTS
ADDRESS WORKBENCH

/* Workbench-Version */
'GETATTR APPLICATION.VERSION'
SAY "Version:" RESULT

/* Anzahl offener Fenster */
'GETATTR WINDOWS.COUNT'
SAY "Offene Fenster:" RESULT

/* Aktives Fenster */
'GETATTR WINDOWS.ACTIVE'
SAY "Aktiv:" RESULT

/* Fensterposition */
'GETATTR WINDOW.LEFT NAME "Work:"'
SAY "Links:" RESULT

/* Alle Icons im Root-Fenster */
'GETATTR WINDOW.ICONS.COUNT NAME ROOT'
count = RESULT
DO i = 0 TO count - 1
    'GETATTR WINDOW.ICONS.'i' NAME ROOT'
    SAY "Icon:" RESULT
END
```

### FAULT

Gibt eine lesbare Fehlermeldung fuer einen Fehlercode zurueck.

```
FAULT CODE/A/N
```

```rexx
'FAULT 205'
SAY RESULT   /* "Object not found" */
```

### HELP

Zeigt Hilfeinformationen an.

```
HELP [COMMAND/K] [MENUS/S] [PROMPT/S]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `COMMAND/K` | Syntax eines bestimmten Befehls anzeigen |
| `MENUS/S` | Verfuegbare Menuepunkte auflisten |
| `PROMPT/S` | Hilfesystem aufrufen |

```rexx
'HELP COMMAND GETATTR'
SAY RESULT   /* Zeigt die Befehlssyntax */

'HELP MENUS'
SAY RESULT   /* Liste aller Menuepunkte */
```

---

## Menue-Verwaltung

### MENU

Ruft Menuepunkte auf oder erstellt benutzerdefinierte Menues.

```
MENU [WINDOW/K] [INVOKE] [NAME/K] [TITLE/K] [SHORTCUT/K]
     [ADD/S] [REMOVE/S] [CMD/K/F]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `INVOKE` | Menuepunkt ausfuehren |
| `NAME/K` | Eindeutiger Bezeichner |
| `TITLE/K` | Anzeigetext |
| `SHORTCUT/K` | Tastenkuerzel (ein Zeichen) |
| `ADD/S` | Menuepunkt hinzufuegen |
| `REMOVE/S` | Menuepunkt entfernen |
| `CMD/K/F` | Zugeordneter ARexx-Befehl |

### Standard-Menuepunkte

| Menuepunkt | Beschreibung |
|------------|-------------|
| `WORKBENCH.BACKDROP` | Backdrop umschalten |
| `WORKBENCH.EXECUTE` | Befehl ausfuehren |
| `WORKBENCH.REDRAWALL` | Alles neu zeichnen |
| `WORKBENCH.UPDATEALL` | Alles aktualisieren |
| `WORKBENCH.LASTMESSAGE` | Letzte Meldung anzeigen |
| `WORKBENCH.ABOUT` | Ueber Workbench |
| `WORKBENCH.QUIT` | Workbench beenden |
| `WINDOW.NEWDRAWER` | Neue Schublade |
| `WINDOW.OPENPARENT` | Uebergeordnetes Verzeichnis |
| `WINDOW.CLOSE` | Fenster schliessen |
| `WINDOW.UPDATE` | Fenster aktualisieren |
| `WINDOW.SELECTALL` | Alles auswaehlen |
| `WINDOW.CLEANUPBY.NAME` | Aufraumen nach Name |
| `WINDOW.CLEANUPBY.DATE` | Aufraumen nach Datum |
| `WINDOW.CLEANUPBY.SIZE` | Aufraumen nach Groesse |
| `WINDOW.CLEANUPBY.TYPE` | Aufraumen nach Typ |
| `WINDOW.VIEWBY.ICON` | Ansicht: Icons |
| `WINDOW.VIEWBY.NAME` | Ansicht: Namen |
| `WINDOW.VIEWBY.DATE` | Ansicht: Datum |
| `WINDOW.VIEWBY.SIZE` | Ansicht: Groesse |
| `WINDOW.VIEWBY.TYPE` | Ansicht: Typ |
| `WINDOW.SHOWONLY.ALL` | Alle anzeigen |
| `ICONS.OPEN` | Icon oeffnen |
| `ICONS.COPY` | Icon kopieren |
| `ICONS.RENAME` | Icon umbenennen |
| `ICONS.INFORMATION` | Icon-Information |
| `ICONS.SNAPSHOT` | Icon-Position speichern |
| `ICONS.UNSNAPSHOT` | Icon-Position loeschen |
| `ICONS.LEAVEOUT` | Icon auf Workbench legen |
| `ICONS.PUTAWAY` | Icon zuruecklegen |
| `ICONS.DELETE` | Icon loeschen |
| `ICONS.FORMATDISK` | Datentraeger formatieren |
| `ICONS.EMPTYTRASH` | Papierkorb leeren |
| `TOOLS.RESETWB` | Workbench zuruecksetzen |

```rexx
ADDRESS WORKBENCH
/* About-Dialog oeffnen */
'MENU INVOKE WORKBENCH.ABOUT'

/* Icons nach Name aufraumen */
'MENU WINDOW "Work:" INVOKE WINDOW.CLEANUPBY.NAME'

/* Eigenen Menuepunkt hinzufuegen */
'MENU ADD NAME "MeinTool" TITLE "Mein Tool" SHORTCUT "M" CMD "rx MeinScript.rexx"'

/* Menuepunkt entfernen */
'MENU REMOVE NAME "MeinTool"'
```

---

## Tastenbelegung

### KEYBOARD

Bindet ARexx-Befehle an Tastenkombinationen.

```
KEYBOARD NAME/A [ADD/S] [REMOVE/S] [KEY] [CMD/F]
```

| Parameter | Beschreibung |
|-----------|-------------|
| `NAME/A` | Eindeutiger Bezeichner |
| `ADD/S` | Neue Belegung hinzufuegen |
| `REMOVE/S` | Belegung entfernen |
| `KEY` | Tastenkombination (Commodities-Format) |
| `CMD/F` | ARexx-Skript oder -Befehl |

```rexx
ADDRESS WORKBENCH
/* Ctrl+A an ein Skript binden */
'KEYBOARD ADD NAME "MeinHotkey" KEY "ctrl a" CMD "rx MeinScript.rexx"'

/* Belegung entfernen */
'KEYBOARD REMOVE NAME "MeinHotkey"'
```

---

## Skript-Ausfuehrung

### RX

Fuehrt ARexx-Skripte und -Befehle aus.

```
RX [CONSOLE/S] [ASYNC/S] CMD/A/F
```

| Parameter | Beschreibung |
|-----------|-------------|
| `CONSOLE/S` | Konsole fuer Ein-/Ausgabe |
| `ASYNC/S` | Asynchron ausfuehren |
| `CMD/A/F` | Skriptname oder Befehl |

```rexx
ADDRESS WORKBENCH
'RX CMD "test.wb"'
'RX ASYNC CMD "langes_skript.rexx"'
```

---

## Beispiel-Skripte

### Alle offenen Fenster auflisten

```rexx
/* ListWindows.rexx */
OPTIONS RESULTS
ADDRESS WORKBENCH

'GETATTR WINDOWS.COUNT'
count = RESULT
SAY count "Fenster offen:"
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

### Icons im Root-Fenster nach Name aufraumen

```rexx
/* CleanupRoot.rexx */
ADDRESS WORKBENCH
'MENU WINDOW root INVOKE WINDOW.CLEANUPBY.NAME'
```

### Fenster oeffnen und positionieren

```rexx
/* OpenAndPosition.rexx */
ADDRESS WORKBENCH
'WINDOW "Work:" OPEN'
'CHANGEWINDOW "Work:" LEFTEDGE 50 TOPEDGE 50 WIDTH 500 HEIGHT 400'
'WINDOW "Work:" FRONT ACTIVATE'
```
