# PageStream — ARexx-Schnittstelle

## Allgemeines

PageStream ist ein professionelles Desktop-Publishing-Programm fuer AmigaOS. Es bietet eine umfangreiche ARexx-Schnittstelle mit ueber 400 Befehlen.

**Port-Name:** `PAGESTREAM` (oder benutzerdefiniert)

Quelle: https://www.pagestream.org/?action=Documents&id=630

### Parameter-Typen

| Kuerzel | Bedeutung |
|---------|-----------|
| `/i` | Integer (Ganzzahl) |
| `/d` | Decimal (Dezimalzahl, z.B. Koordinaten in Zoll/cm) |
| `/s` | String (Zeichenkette) |
| `/k` | Keyword (Schluesselwort aus vorgegebener Liste) |
| `/S` | Switch (Schalter) |
| `/a` | Array/Stem-Variable |

### Allgemeine Parameter

Viele Befehle akzeptieren diese optionalen Zielangaben:
- `PAGE number` — Zielseite (Format: `document:...-pagenum` oder einfach Seitennummer)
- `MPG name` — Masterpage-Name und -Seite
- `WINDOW name` — Zielfenster (Format: `document~window`)
- `DOCUMENT name` — Zieldokument
- `CHAPTER name` — Zielkapitel (Format: `document:chapter`)
- `OBJECTID number` — Zielobjekt (numerische ID)

### Gemeinsame Transformations-Parameter

Viele Zeichen- und Objekt-Befehle teilen sich diese Parameter:
- `ROTATE angle` — Drehwinkel in Grad (Standard: 0)
- `SKEW slantangle twistangle` — Neigung und Verdrehung (Standard: 0)
- `SLANT angle` — Nur Neigung (Standard: 0)
- `TWIST angle` — Nur Verdrehung (Standard: 0)
- `ABOUT pointx pointy` — Benutzerdefinierter Drehpunkt
- `ABOUTCENTER` — Drehung um Objektmitte (Standard)
- `CONSTRAIN` / `FREE` — Seitenverhaeltnis sperren / frei (Standard: FREE)
- `PRINT` / `NOPRINT` — Druckbar oder nicht (Standard: PRINT)
- `INFRONT` / `INBACK` — Stapelreihenfolge (Standard: INFRONT)

### Rueckgabewerte

Ergebnisse werden in der ARexx-Variable `RESULT` zurueckgegeben.
Viele Befehle geben Objekt-Handles (numerische IDs) zurueck, die fuer weitere Operationen verwendet werden koennen.

---

## Datei-Befehle (File Commands)

### OpenDocument

Oeffnet ein Dokument.

```
opendocument [ASK | FILE "file"] [FILTER filtername] [STATUS | NOSTATUS]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `ASK` | /S | Oeffnet Dateiauswahl-Dialog |
| `FILE` | /s | Dateiname und Pfad |
| `FILTER` | /k | Filter: `IFFDOC`, `PAGESTREAM2`, `PROPAGE`, `WORDWORTHDOC` |
| `STATUS` / `NOSTATUS` | /k | Fortschrittsanzeige (Standard: NOSTATUS) |

```rexx
ADDRESS PAGESTREAM
'opendocument file "Work:Documents/Newsletter.doc"'
```

### Open

Oeffnet ein Dokument und erstellt ein Fenster.

```
open [file] [FILTER ""] [optional module parameters]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `file` | /s | Dateipfad (ohne = Dateiauswahl-Dialog, ab v5.0.3.4) |
| `FILTER` | /s | Filter-Spezifikation |

```rexx
'open "PageStream:documents/Project.doc"'
```

### CloseDocument

Schliesst alle Ansichten eines Dokuments und das Dokument selbst.

```
closedocument [mode] [DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `mode` | /k | `FORCE` (ohne Speicherabfrage), `ALERT` (Dialog bei Aenderungen), `QUIET` (Standard: still, Fehler bei ungespeicherten Aenderungen) |
| `DOCUMENT` | /s | Dokumentname (Standard: aktuell) |
| `WINDOW` | /s | Fenstername (Standard: aktuell) |

```rexx
'closedocument force document "project.doc"'
```

### CloseWindow

Schliesst eine einzelne Ansicht. Das Dokument bleibt offen.

```
closewindow [WINDOW name]
```

```rexx
'closewindow window "Untitled~View.1"'
```

### SaveDocument

Speichert ein offenes Dokument.

```
savedocument [ASK | FILE "file" | DEFAULT] [FILTER filtername]
            [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
            [TEMPLATE] [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `ASK` | /S | Dateiauswahl-Dialog |
| `FILE` | /s | Dateiname und Pfad |
| `DEFAULT` | /S | Standard-Speicherort |
| `FILTER` | /k | Filter (Standard: IFFDOC) |
| `STATUS` / `NOSTATUS` | /k | Fortschrittsanzeige |
| `QUIET` / `FORCE` / `ALERT` | /k | Ueberschreib-Verhalten |
| `TEMPLATE` | /S | Als Vorlage speichern |

```rexx
'savedocument file "Work:Documents/Project.doc" force'
```

### RevertDocument

Laedt die letzte gespeicherte Version eines Dokuments.

```
revertdocument [STATUS | NOSTATUS] [FORCE | ALERT]
              [FILTER filtername]
              [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `STATUS` / `NOSTATUS` | /k | Fortschrittsanzeige (Standard: NOSTATUS) |
| `FORCE` / `ALERT` | /k | Bestaetigungs-Modus (Standard: ALERT) |
| `FILTER` | /k | Filter: `IFFDOC`, `PAGESTREAM2`, `PROPAGE`, `WORDWORTHDOC` |

```rexx
'revertdocument status quiet'
```

### NewDocument

Erstellt ein neues Dokument.

```
newdocument [name]
```

Gibt den Dokumentnamen in RESULT zurueck. Erstellt keine Standard-Masterpage und oeffnet kein Fenster.

```rexx
'newdocument "Mein Projekt"'
docname = RESULT
```

### CollectOutput

Sammelt alle Dateien fuer die Ausgabe (ab PageStream 4.0).

```
collectoutput [ASK | FILE "file"] [STATUS | NOSTATUS]
             [QUIET | FORCE | ALERT]
             [DOCUMENT document | WINDOW windowspec]
```

### CheckExternalLinks

Prueft externe Verknuepfungen im Dokument.

```
checkexternallinks
```

---

### Import/Export

#### PlaceGraphic

Platziert eine Grafikdatei in einem Dokument.

```
placegraphic [FILE "file"] [FILTER filtername]
            [STATUS | NOSTATUS]
            [ASIS | AT x y | USER]
            [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `FILE` | /s | Dateiname und Pfad (ohne = Dateiauswahl-Dialog) |
| `FILTER` | /k | `PICT`, `TIFF`, `IFFILBM`, `GIF`, `BMP`, `IFFDR2D`, `IFFILUS`, `JPEG`, `MACPAINT`, `PRODRAW`, `PCX`, `ILLUSTRATOREPS`, `FREEHANDEPS`, `ARTEXPRESSIONEPS`, `EPS`, `PNG` |
| `STATUS` / `NOSTATUS` | /k | Fortschrittsanzeige (Standard: NOSTATUS) |
| `ASIS` | /S | Originalgroesse und -position |
| `AT x y` | /d | An Position zentrieren (Standard: Fenstermitte) |
| `USER` | /S | Benutzer platziert manuell |

```rexx
'placegraphic file "Work:Pictures/Logo.iff" filter iffilbm at 4.0 2.0'
```

#### InsertText

Fuegt Text aus einer Datei an der Cursor-Position ein.

```
inserttext [FILE filepath] [FILTER name] [progress] [charset]
          [CONVERTQUOTE toggle] [CONVERTDASH toggle]
          [LINEHASLF toggle] [RETAINFORMAT toggle]
          [TEXTCODE code]
          [DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `FILE` | /s | Pfad zur Textdatei |
| `FILTER` | /k | `ASCII`, `IFFCTXT`, `IFFFTXT`, `WORDWORTH` |
| `CONVERTQUOTE` | /k | Typographische Anfuehrungszeichen konvertieren |
| `CONVERTDASH` | /k | Gedankenstriche konvertieren |
| `LINEHASLF` | /k | Zeilenumbruch-Behandlung |
| `RETAINFORMAT` | /k | Formatierung beibehalten |

#### ExportGraphic

Exportiert ein Grafikobjekt.

```
exportgraphic [FILE "file"] [FILTER filtername]
             [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
             [DOCUMENT document | WINDOW windowspec | ID id]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `FILE` | /s | Dateiname (ohne = Dateiauswahl-Dialog) |
| `FILTER` | /k | `PICT`, `TIFF`, `IFFILBM`, `GIF`, `BMP`, `IFFDR2D`, `IFFILUS`, `JPEG`, `PNG`, `JPEG2000` |
| `STATUS` / `NOSTATUS` | /k | Fortschrittsanzeige |
| `QUIET` / `FORCE` / `ALERT` | /k | Ueberschreib-Verhalten |
| `ID` | /i | Objekt-ID (Standard: aktuell) |

```rexx
'exportgraphic file "Ram:Snapshot" filter iffilbm status'
```

#### ExportText

Exportiert den ausgewaehlten Text.

```
exporttext [FILE "file"] [FILTER filtername]
          [STATUS | NOSTATUS] [QUIET | FORCE | ALERT]
          [DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `FILTER` | /k | `ASCII`, `IFFFTXT`, `IFFCTXT`, `WORDWORTH`, `RTF` |

#### ExportPDF

Exportiert als PDF.

```
exportpdf [FILE "file"] [DOCUMENT document | WINDOW windowspec]
```

---

### Pfad-Einstellungen

#### SetDocumentPath

Setzt den Standard-Dokumentpfad.

```
setdocumentpath [filepath] [SAVE]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `filepath` | /s | Der Dateipfad |
| `SAVE` | /S | In Einstellungsdatei speichern |

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

#### SetScriptPath (nur Amiga)

```
setscriptpath [filepath] [SAVE]
```

```rexx
'setscriptpath "Rexx:"'
```

#### SetBackup

Konfiguriert Backup- und Autosave-Einstellungen.

```
setbackup [AUTOBACKUP status [COUNT number]]
         [AUTOSAVE status [INTERVAL time]]
         [PATH filepath] [SAVE]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `AUTOBACKUP` | /k | `ON`, `OFF`, `TOGGLE` |
| `COUNT` | /i | Anzahl Backup-Kopien |
| `AUTOSAVE` | /k | `ON`, `OFF`, `TOGGLE` |
| `INTERVAL` | /i | Autosave-Intervall in Minuten |
| `PATH` | /s | Backup-Verzeichnis |
| `SAVE` | /S | In Einstellungsdatei speichern |

Hinweis: `TOGGLE` darf nicht zusammen mit `SAVE` verwendet werden.

```rexx
'setbackup autobackup on autosave on interval 5 save'
```

#### SetAutoColumns

```
setautocolumns [ON | OFF | TOGGLE] [LIKE MASTERPAGE | PREVIOUS] [SAVE]
```

#### SelectOnPaste

Waehlt importierten/eingefuegten Text automatisch aus.

```
selectonpaste [status] [SAVE]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `status` | /k | `ON`, `OFF`, `TOGGLE` |
| `SAVE` | /S | In Einstellungsdatei speichern |

#### UseScratch

```
usescratch [TRUE | FALSE] [SAVE]
```

#### FileFilter

```
filefilter [TRUE | FALSE] [SAVE]
```

---

### Datei-Abfragen

#### GetDocumentPath

```
getdocumentpath
```

Gibt den aktuellen Dokumentpfad in RESULT zurueck.

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

Gibt `ON` oder `OFF` in RESULT zurueck. `LIKE` empfaengt `MASTERPAGE` oder `PREVIOUS`.

#### GetSelectOnPaste

```
getselectonpaste
```

Gibt `ON` oder `OFF` in RESULT zurueck.

#### GetPasteInCenter

```
getpasteincenter
```

#### GetWindows

```
getwindows stem [DOCUMENT document | WINDOW windowspec]
```

Gibt Fensteranzahl in RESULT zurueck. Fensternamen in `stem.0`, `stem.1`, etc.

---

## Zeichen-Befehle (Drawing Commands)

### DrawBox

Zeichnet ein neues Rechteck.

```
drawbox <left top right bottom> [type]
       [CORNER radius | ECORNER radiusx radiusy]
       [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
       [ABOUT pointx pointy | ABOUTCENTER]
       [constraint] [printable] [stack]
       [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Box-Koordinaten (Pflicht) |
| `type` | /k | `NORMAL`, `ROUND`, `SCALLOP`, `BEVEL`, `INSET` |
| `CORNER` | /d | Eckenradius (Standard: 0) |
| `ECORNER` | /d | Elliptischer Eckenradius (radiusx radiusy, Standard: 0) |

Gibt Objekt-Handle in RESULT zurueck.

```rexx
'drawbox 1.75 1.5 2.75 2.5'
'drawbox 3 3 6 6 inback page "project.doc~8"'
'drawbox 6 7 8 9 ecorner 0.1 0.2 skew 0 45'
```

### DrawEllipse

Zeichnet eine neue Ellipse.

```
drawellipse <centerx centery radiusx radiusy> [type]
           [ANGLES startangle endangle]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `centerx centery` | /d | Mittelpunkt-Koordinaten (Pflicht) |
| `radiusx radiusy` | /d | Radius-Abmessungen (Pflicht) |
| `type` | /k | `ELLIPSE` (Standard), `PIE`, `ARC` |
| `ANGLES` | /d | Start- und Endwinkel (fuer PIE/ARC) |

```rexx
'drawellipse 3 3 1.5 1.5'
'drawellipse 5.25 6.125 1.5 2.5 pie angles 45 90 print'
```

### DrawLine

Zeichnet eine neue Linie.

```
drawline <left top right bottom>
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable] [stack]
        [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Linienendpunkt-Koordinaten (Pflicht) |

```rexx
'drawline 1 1 5 5'
'drawline 2 1.25 8.5 11 page "project.doc~6" noprint'
```

### DrawPolygon

Zeichnet ein regelmaessiges Polygon.

```
drawpolygon <centerx centery radiusx radiusy> [type]
           [SIDES number] [OFFSETANGLE angle]
           [DEFLECTION amount] [ANGLEDEFLECTION amount]
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `centerx centery` | /d | Mittelpunkt (Pflicht) |
| `radiusx radiusy` | /d | Polygon-Radius (Pflicht) |
| `type` | /k | `NORMAL` (Standard), `STAR`, `PUFFY`, `SCALLOP`, `WAVY` |
| `SIDES` | /i | Seitenanzahl (Standard: 5, muss >2 sein) |
| `OFFSETANGLE` | /d | Vor-Drehungswinkel (Standard: 0) |
| `DEFLECTION` | /d | Zackenrtiefe (Standard: 40%) |
| `ANGLEDEFLECTION` | /d | Zackenwinkel (Standard: 0%) |

```rexx
'drawpolygon 4.0 5.0 2.0 2.0 STAR SIDES 6'
'drawpolygon 0.3 1.8 1.25 1.5 sides 8 puffy'
```

### DrawSpiral

Zeichnet eine Spirale.

```
drawspiral x1 y1 ROUNDS n SPACING dist STARTANGLE angle
          [frame] [CONTENTOFFSET x y] [CONTENTSCALE x y]
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `x1 y1` | /d | Startkoordinaten |
| `ROUNDS` | /i | Anzahl Umdrehungen |
| `SPACING` | /d | Abstand zwischen Spiralarmen |
| `STARTANGLE` | /d | Startwinkel |

### DrawGrid

Erstellt ein Rasterobjekt.

```
drawgrid <pointx1 pointy1 pointx2 pointy2 |
         POINTS pointx1 pointy1 pointx2 pointy2 pointx3 pointy3 pointx4 pointy4>
        [DIVISIONS horz vert]
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable] [stack]
        [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `pointx1 pointy1 pointx2 pointy2` | /d | Rechteckiges Raster (2 Eckpunkte) |
| `POINTS` | /d | Nicht-rechteckiges Raster (4 Eckpunkte, gegen den Uhrzeigersinn) |
| `DIVISIONS` | /i | Rasterunterteilungen horizontal/vertikal (Standard: 4x4) |

```rexx
'drawgrid 2 2 6 6 divisions 5 10'
'drawgrid points 1 1 1 3 5 6 2 3 page "MyDoc~6"'
```

### DrawTable

Erstellt ein Tabellenobjekt.

```
drawtable <left top right bottom> [ROWS rows] [COLUMNS columns]
         [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
         [ABOUT pointx pointy | ABOUTCENTER]
         [constraint] [printable] [stack]
         [PAGE pagespec | MPG masterpagespec | DOCUMENT document | WINDOW windowspec]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Tabellen-Koordinaten (Pflicht) |
| `ROWS` | /i | Zeilenanzahl |
| `COLUMNS` | /i | Spaltenanzahl |

### DrawColumn

Zeichnet einen neuen Textrahmen mit einer oder mehreren Spalten.

```
drawcolumn <left top right bottom> [COLUMNS number] [GUTTER space]
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Rahmen-Koordinaten (Pflicht) |
| `COLUMNS` | /i | Spaltenanzahl |
| `GUTTER` | /d | Spaltenabstand |

Hinweis: Fuer mehrseitige oder groeßenabhaengige Spalten `CreateColumns` verwenden.

```rexx
'drawcolumn 1 1 7.5 10 columns 2 gutter 0.25'
```

### DrawTextObj

Erstellt einen neuen Textblock.

```
drawtextobj <left top>
           [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
           [ABOUT pointx pointy | ABOUTCENTER]
           [constraint] [printable] [stack]
           [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top` | /d | Linke obere Ecke des Textblocks (Pflicht) |

```rexx
'drawtextobj 1 1'
'drawtextobj 1 1 rotate 45 inback page "project.doc~8"'
```

### DrawPicture

Platziert einen Bildrahmen.

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Rahmen-Koordinaten (Pflicht) |
| `CONTENTOFFSET` | /d | Versatz innerhalb des Rahmens (Standard: 0,0) |
| `CONTENTSCALE` | /d | Skalierung innerhalb des Rahmens (Standard: 100%) |
| `DPI` | /d | Aufloesung (xdpi ydpi) |
| `frame` | /k | `FRAMED` oder `FRAMELESS` |
| `stored` | /k | `INTERNAL` (Standard) oder `EXTERNAL` |
| `FILE` | /s | Bilddatei-Pfad |
| `FPO` | /k | Aufloesung: `DEFAULT`, `FINE`, `MEDIUM`, `COARSE`, `CUSTOM xdpi ydpi` |

```rexx
'drawpicture 0.5 0.5 2.5 3 dpi 150 150 external'
```

### DrawEPS

Platziert einen EPS-Rahmen.

```
draweps <left top right bottom>
       [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
       [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
       [ABOUT pointx pointy | ABOUTCENTER]
       [constraint] [printable] [stack] [frame] [stored]
       [FILE filepath]
       [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `left top right bottom` | /d | Rahmen-Koordinaten (Pflicht) |
| `CONTENTOFFSET` | /d | Versatz (Standard: 0,0) |
| `CONTENTSCALE` | /d | Skalierung (Standard: 100%) |
| `frame` | /k | `FRAMED` oder `FRAMELESS` |
| `stored` | /k | `INTERNAL` (Standard) oder `EXTERNAL` |
| `FILE` | /s | EPS-Dateipfad |

```rexx
'draweps 1.75 1.5 2.75 2.5'
'draweps 3 3 6 6 offset -2 2 contentscale 80 constrain'
```

---

### Pfad-Befehle

#### CreatePath

Erstellt einen neuen Pfad.

```
createpath <left top right bottom>
          [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
          [ABOUT pointx pointy | ABOUTCENTER]
          [constraint] [printable] [stack]
          [PAGE number | WINDOW name]
```

Gibt Objekt-Handle in RESULT zurueck.

```rexx
'createpath 1.5 2.5 3.25 6.457'
```

#### AddPoint

Fuegt einen Punkt zum aktuellen Pfad hinzu.

```
addpoint <MOVETO pointx pointy [join] |
         LINETO pointx pointy [join] |
         CURVETO curvex1 curvey1 curvex2 curvey2 pointx pointy [join] |
         CLOSEPATH>
        [PAGE number | WINDOW name | OBJECTID number]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `MOVETO` | /d | Erster Punkt eines neuen Pfads/Unterpfads |
| `LINETO` | /d | Linie zum angegebenen Punkt |
| `CURVETO` | /d | Bezier-Kurve (Kontrollpunkt1, Kontrollpunkt2, Endpunkt) |
| `CLOSEPATH` | /S | Schliesst den offenen Pfad |
| `join` | /k | `CORNERJOIN` (Standard) oder `SMOOTHJOIN` |

Hinweis: Der Join-Typ beeinflusst nur die Interaktion mit dem Reshape-Tool, nicht die Koordinaten.

```rexx
'addpoint moveto 1.0 2.0'
'addpoint lineto 5.0 2.0'
'addpoint curveto 5.0 4.0 3.0 6.0 1.0 4.0 smoothjoin'
'addpoint closepath'
```

#### CreatePoint

Fuegt einen Punkt zu einem existierenden Pfad an der naechstgelegenen Stelle hinzu.

```
createpoint <NEAR x y> [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'createpoint near 5.1 6.25'
```

#### EditPoint

Bearbeitet die Koordinaten eines Pfadpunkts.

```
editpoint [ANCHOR x y] [CONTROLBEFORE x y] [CONTROLAFTER x y]
         [join] [POINTINDEX point]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `ANCHOR` | /d | Punkt-Koordinaten |
| `CONTROLBEFORE` | /d | Kurvengriff vor dem Punkt |
| `CONTROLAFTER` | /d | Kurvengriff nach dem Punkt |
| `join` | /k | `CORNERJOIN` oder `SMOOTHJOIN` |
| `POINTINDEX` | /i | Punktnummer (Standard: ausgewaehlt). Nummerierung ab 0 vom ersten MOVETO. |

#### MovePoint

Verschiebt ausgewaehlte Pfadpunkte.

```
movepoint [OFFSET x y] [POINTINDEX point]
         [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### NudgePoint

Verschiebt Pfadpunkte um den eingestellten Nudge-Wert.

```
nudgepoint <[horz] [vert]> [TAP] [POINTINDEX point]
          [DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `horz` | /k | `LEFT` oder `RIGHT` |
| `vert` | /k | `UP` oder `DOWN` |
| `TAP` | /S | 1/10 des normalen Nudge-Werts |

#### DeletePoint

Loescht den ausgewaehlten Pfadpunkt.

```
deletepoint [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SplitPoint

Teilt einen Pfad an einem Punkt. Bei offenem Pfad: zwei Unterpfade; bei geschlossenem: offener Pfad.

```
splitpoint [POINTINDEX point] [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'splitpoint pointindex 23'
```

#### ClosePath

Schliesst den ausgewaehlten offenen Pfad.

```
closepath [PAGE number | WINDOW name | OBJECTID number]
```

#### MergeSubPaths

Fuegt Unterpfade innerhalb eines Pfadobjekts zusammen.

```
mergesubpaths [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### SnapToGrid

Aendert den Rastereinrast-Status.

```
snaptogrid [status] [WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `status` | /k | `ON`, `OFF`, `TOGGLE` |

Hinweis: Rastereinrastung beeinflusst nur die Benutzeroberflaeche, nicht Skript-Befehle.

---

### Medien-Befehle

#### EditPicture

Setzt Koordinaten und Optionen eines Bildrahmens.

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

Setzt Koordinaten und Optionen eines EPS-Rahmens.

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

Setzt Koordinaten einer Linie.

```
editline [POSITION x1 y1 x2 y2 [ADJUST]]
        [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
        [ABOUT pointx pointy | ABOUTCENTER]
        [constraint] [printable]
        [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

#### DissolveEPS

Trennt ein EPS-Objekt und seine Bitmap-Vorschau in zwei separate Objekte.

```
dissolveeps [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SetEPS

Setzt Name und Beschreibung eines EPS-Objekts.

```
seteps [NAME name] [DESC description]
      [DOCUMENT name | WINDOW name | OBJECTID number]
```

```rexx
'seteps name "CorporateLogo"'
```

#### GetEPS (Abfrage)

Fragt Eigenschaften eines EPS-Objekts ab.

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

| Ergebnis | Beschreibung |
|----------|-------------|
| `POSITION` | stem: .LEFT, .TOP, .RIGHT, .BOTTOM |
| `FILEINFO` | stem: .MODE (INTERNAL\|EXTERNAL), .FILE, .FORMAT, .DATE, .TIME |
| `FRAME` | `ON` oder `OFF` |
| `CONTENTOFFSET` | stem: .X, .Y |
| `CONTENTSCALE` | stem: .H, .V |
| `ROTATION` | stem: .SLANT, .TWIST |
| `ABOUT` | stem: .MODE (POINT\|CENTER), .X, .Y |

---

### Zusammengesetzte Objekte

#### CreateCompound

Fuegt ausgewaehlte Formen zu einem Verbundobjekt zusammen.

```
createcompound [POSITION left top right bottom]
              [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
              [ABOUT pointx pointy | ABOUTCENTER]
              [constraint] [printable] [stack]
              [DOCUMENT name | WINDOW name]
```

Gibt Objekt-Handle in RESULT zurueck.

```rexx
'createcompound'
'createcompound position 1.5 2.5 3.25 6.4 constrain'
```

#### CreateDrawing

Erstellt ein neues Zeichnungsobjekt.

```
createdrawing [POSITION left top right bottom]
             [CONTENTOFFSET offsetx offsety] [CONTENTSCALE scalex scaley]
             [ROTATE angle | SKEW slantangle twistangle | SLANT angle | TWIST angle]
             [ABOUT pointx pointy | ABOUTCENTER]
             [constraint] [printable] [stack] [frame]
             [DOCUMENT name | WINDOW name]
             [DRAWING | LAYER] [VISIBLE | INVISIBLE] [EDITABLE | NONEDITABLE]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `DRAWING` / `LAYER` | /k | Typ der Zeichnung |
| `VISIBLE` / `INVISIBLE` | /k | Ebenensichtbarkeit (wenn LAYER) |
| `EDITABLE` / `NONEDITABLE` | /k | Ebenenbearbeitbarkeit (wenn LAYER) |

```rexx
'createdrawing'
'createdrawing position 1.5 2.5 3.25 6.457 contentscale 80.5 constrain'
```

#### ApplyAttributes

Wendet aufgenommene Attribute auf einen Absatz oder ein Objekt an.

```
applyattributes <AT pointx pointy [stack] | RANGE left top right bottom [stack]>
               [PAGE number | MPG name | DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `AT` | /d | Attribute auf Absatz an Position anwenden |
| `RANGE` | /d | Attribute auf alle Absaetze im Bereich anwenden |
| `stack` | /k | `FRONTMOST` (Standard) oder `BACKMOST` |

```rexx
'applyattributes at 1.25 3.3 backmost'
'applyattributes range 1.25 3.3 1.75 5.25'
```

---

## Dokument-Befehle (Document Commands)

### Seiten

#### InsertPage

Fuegt Seite(n) ein.

```
insertpage [where] [PAGE number | DOCUMENT name | WINDOW name]
          [COUNT number] [INSPREAD [type]]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `where` | /k | `BEFORE` (Standard) oder `AFTER` |
| `COUNT` | /i | Anzahl Seiten (Standard: 1) |
| `INSPREAD` | /k | `DEFAULT`, `HORIZONTAL`, `VERTICAL` |

#### DeletePage

Loescht Seite(n).

```
deletepage [PAGE number] [TO number] [mode]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `PAGE` | /s | Erste zu loeschende Seite (Standard: aktuell) |
| `TO` | /s | Letzte Seite eines Bereichs |
| `mode` | /k | `FORCE`, `ALERT`, `QUIET` (Standard) |

```rexx
'deletepage page 5 to 7 force'
```

#### MovePage

Verschiebt Seite(n) innerhalb eines Dokuments.

```
movepage [PAGE number | DOCUMENT name | WINDOW name]
        [TO pagenum] <BEFORE document:...-pagenum | AFTER document:...-pagenum>
        [INSPREAD [DEFAULT | HORIZONTAL | VERTICAL]]
```

Hinweis: Seiten koennen nicht ausserhalb ihres Kapitels oder Dokuments verschoben werden.

#### MakePageSpread

Erstellt einen Seitenverbund aus zwei oder mehr Seiten.

```
makepagespread [PAGE number | DOCUMENT name | WINDOW name]
              [TO number] [INSPREAD [HORIZONTAL | VERTICAL]]
```

Zum Aufloesen eines Spreads `TO` weglassen.

```rexx
'makepagespread page 1 to 3 vertical'
'makepagespread page 1'  /* loest Spread auf */
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

### Kapitel

#### NewChapter

```
newchapter chaptername chapter# [PAGESFROMDOCUMENT]
          [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

Gibt den neuen Kapitelnamen in RESULT zurueck.

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

### Masterpages

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

Weist eine Masterpage einer Seite oder einem Seitenbereich zu.

```
selectmasterpage [NAME masterpagename] [PAGE number] [stack] [status]
                [RIPPLE | TO number]
                [DOCUMENT document | WINDOW document-window]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `NAME` | /s | Masterpage-Name |
| `stack` | /k | `INFRONT` oder `INBACK` |
| `status` | /k | `SHOW` oder `HIDE` |
| `RIPPLE` | /S | Auf alle folgenden Seiten anwenden |
| `TO` | /s | Bis zu Seite |

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

### Seitenabmessungen und Layout

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `inside` | /d | Innen-/Links-Rand |
| `outside` | /d | Aussen-/Rechts-Rand |
| `top` | /d | Oberer Rand |
| `bottom` | /d | Unterer Rand |

```rexx
'setmarginguides 1 0.75 1 1.25 masterpage "Layout2"'
```

---

### Hilfslinien (Guides)

#### AddGuides

```
addguides [type] <AT position> [MPG name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `type` | /k | `HORIZONTAL`, `VERTICAL`, `BOTH` |
| `AT` | /d | Position(en) der Hilfslinie(n) |
| `MPG` | /s | Masterpage-Name und -Seite |

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

#### SetGuide (Einrast-Einstellungen)

```
setguide [SNAPALL | SNAPRANGE rangex rangey] [MASTERPAGE name]
```

#### SnapToGuides

```
snaptoguides [status] [PAGE status] [RULER status] [WINDOW name]
```

---

### Raster und Anzeige

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `status` | /k | `SHOW`, `HIDE`, `TOGGLE` (Pflicht) |
| `stack` | /k | `INFRONT` oder `INBACK` (Standard: INBACK) |
| `COLOR` | /i | RGB-Werte, 0-255 |

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `status` | /k | `SHOW`, `HIDE`, `TOGGLE` (Pflicht) |
| `OFFSET` | /d | Lineal-Offset in Pixeln |
| `ZERO` | /d | Nullpunkt-Koordinaten |
| `MSYS` | /k | `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPICAS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `FEET`, `METERS`, `SAMEAS` |
| `DIRECTION` | /k | Horizontal: `LEFT`\|`RIGHT`, Vertikal: `UP`\|`DOWN` |

```rexx
'displayruler show offset 30 120 msys inches inches'
```

---

### Revisions-Verwaltung

#### LogRevision

```
logrevision [DESCRIPTION text] [VERSION major minor | BUMPREV]
           [USER username] [CHAPTER name | PAGE number | MASTERPAGE name]
```

```rexx
'logrevision bumprev'
'logrevision description "Fixed colors" user "Colleen"'
```

#### SetRevision (ab v5.0.3.4)

```
setrevision [DESCRIPTION description] [BUMPMINOR | BUMPMAJOR | VERSION major minor]
           [CREATED | MODIFIED] [USER username] [REVISION revh]
```

#### DeleteRevision (ab v5.0.3.4)

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

### Bildschirm-Aktualisierung

#### Refresh

Schaltet Bildschirmaktualisierungen um fuer schnellere Skript-Ausfuehrung.

```
refresh [ON | OFF | WAIT | CONTINUE] [ALL | DOCUMENT document | WINDOW document-window]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `OFF` | /k | Aktualisierung aus, ohne Aenderungen zu merken |
| `WAIT` | /k | Aktualisierung aus, merkt sich was aktualisiert werden muss |
| `ON` | /k | Aktualisierung ein, verwirft alle gemerkten Aktualisierungen |
| `CONTINUE` | /k | Aktualisierung ein, fuehrt gemerkte Aktualisierungen durch |

Typischerweise zusammen mit `LockInterface` fuer bessere Performance.

---

### Dokument-Abfragen

#### CurrentDocument / CurrentWindow / CurrentPage / CurrentChapter / CurrentMasterpage

```
currentdocument [WINDOW name]
currentwindow [WINDOW name]
currentpage [WINDOW name]
currentchapter [DOCUMENT name | WINDOW name]
currentmasterpage [DOCUMENT name | WINDOW name]
```

Alle geben den Namen/die Nummer in RESULT zurueck.

Die `Path`-Varianten geben den vollstaendigen Pfad zurueck:
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

Gibt Dokumentanzahl in RESULT zurueck. Namen in `stem.0`, `stem.1`, etc.

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

Gibt `changed` oder `unchanged` in RESULT zurueck.

#### GetChapters / GetChapterDesc / GetChapterNumber / GetChapterNumbering

```
getchapters stem [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
getchapterdesc [DOCUMENT name | CHAPTER name]
getchapternumber [DOCUMENT name | CHAPTER name]
getchapternumbering stem [DOCUMENT name | CHAPTER name | WINDOW name]
```

GetChapterNumbering Ergebnisse: `stem.startmode`, `stem.start`, `stem.format`, `stem.language`, `stem.prefix`

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

Ergebnisse: `stem.startmode`, `stem.start`, `stem.lengthmode`, `stem.length`, `stem.masterpage`, `stem.format`, `stem.language`, `stem.prefix`

#### GetDimensions

```
getdimensions stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Ergebnisse: `stem.width`, `stem.height`, `stem.orientation` (PORTRAIT\|LANDSCAPE), `stem.spreads` (FACING\|INDIVIDUAL), `stem.sides` (SINGLE\|DOUBLE)

#### GetDefaultPageSizes

```
getdefaultpagesizes namestem [measurement] [custom]
                   [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `measurement` | /k | `STANDARD` (Standard) oder `USER` |
| `custom` | /k | `NOCUSTOM` (Standard) oder `CUSTOM` |

Ergebnisse: Jeder Eintrag hat `.name`, `.width`, `.height`.

#### GetBleed

```
getbleed stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Ergebnisse: `stem.H`, `stem.V`

#### GetColumnGuides

```
getcolumnguides stem [MASTERPAGE name]
```

Ergebnisse: `stem.count`, `stem.gutter`

#### GetMarginGuides

```
getmarginguides stem [MASTERPAGE name]
```

Ergebnisse: `stem.inside`, `stem.outside`, `stem.top`, `stem.bottom`

```rexx
'getmarginguides info'
SAY 'Inside margin:  ' || info.inside
SAY 'Outside margin: ' || info.outside
```

#### GetGrid

```
getgrid stem [MASTERPAGE document:...-masterpage | DOCUMENT document | WINDOW document-window]
```

Ergebnisse: `stem.h`, `stem.v`, `stem.x`, `stem.y`, `stem.snap` (ALL\|RANGE), `stem.rangeh`, `stem.rangev`, `stem.displayh`, `stem.displayv`, `stem.displayx`, `stem.displayy`

#### GetGridDisplay / GetGridSnap

```
getgriddisplay [DEPTH layer] [COLOR stem] [WINDOW name]
getgridsnap [WINDOW name]
```

#### GetGuide / GetGuides / GetGuideDisplay / GetGuideSnap

```
getguide stem [MASTERPAGE name]         /* stem.snap, stem.rangeh, stem.rangev */
getguides stem [type] [MPG name]        /* Guide-Positionen in stem.0, stem.1, ... */
getguidedisplay [DEPTH layer] [PAGE flag] [PAGECOLOR stem] [RULER flag] [RULERCOLOR stem] [WINDOW name]
getguidesnap [WINDOW name]
```

#### GetRulerDisplay

```
getrulerdisplay [OFFSET stem] [ZERO stem] [MSYS stem] [DIRECTION stem] [WINDOW name]
```

Ergebnisse: OFFSET: `.x`, `.y`; ZERO: `.x`, `.y`; MSYS: `.h`, `.v`; DIRECTION: `.h` (LEFT\|RIGHT), `.v` (UP\|DOWN)

#### GetRevisions / GetRevision (ab v5.0.3.4)

```
getrevisions revhlist [DOCUMENT document | CHAPTER document:... | PAGE ... | MASTERPAGE ... | WINDOW ...]
getrevision [DESCRIPTION &desc] [VERSION &version] [TYPE &type] [USER &username] [REVISION revh]
```

---

## Objekt-Befehle (Object Commands)

### Auswahl und Navigation

#### SelectObject

Waehlt ein Objekt aus.

```
selectobject <AT pointx pointy [stack] | RANGE left top right bottom [stack] |
             ALL | NONE | INVERT | NEXT | PREVIOUS |
             LINKS | CHAINFIRST | CHAINLAST>
            [PAGE number | MPG name | DOCUMENT name | WINDOW name | OBJECTID number]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `AT` | /d | Objekt an Position |
| `RANGE` | /d | Alle Objekte im Bereich |
| `ALL` | /S | Alle Objekte |
| `NONE` | /S | Auswahl aufheben |
| `INVERT` | /S | Auswahl umkehren |
| `NEXT` / `PREVIOUS` | /S | Naechstes/vorheriges Objekt |
| `LINKS` | /S | Verkettete Objekte |
| `CHAINFIRST` / `CHAINLAST` | /S | Erstes/letztes Element der Kette |
| `stack` | /k | `FRONTMOST` (Standard) oder `BACKMOST` |

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

Gibt Anzahl in RESULT zurueck. Objekt-IDs in `stem.0`, `stem.1`, etc.

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `horz` | /k | `LEFT` oder `RIGHT` |
| `vert` | /k | `UP` oder `DOWN` |
| `TAP` | /S | 1/10 des normalen Nudge-Werts |

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

Allgemeine Transformation.

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `horz` | /k | `LEFTALIGN`, `CENTERALIGN`, `RIGHTALIGN` |
| `vert` | /k | `TOPALIGN`, `MIDDLEALIGN`, `BOTTOMALIGN` |
| `scope` | /k | `TOPAGE` (Standard), `TOMARGINS`, `TOSELECTION` |

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

### Ebenen-Reihenfolge

```
bringtofront [DOCUMENT name | WINDOW name | OBJECTID number]
bringforward [DOCUMENT name | WINDOW name | OBJECTID number]
sendtoback [DOCUMENT name | WINDOW name | OBJECTID number]
sendbackward [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Bearbeitung

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

### Gruppen und Verbundobjekte

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

### Pfad-Operationen

```
converttopath [DOCUMENT name | WINDOW name | OBJECTID number]
mergepaths [DOCUMENT name | WINDOW name]
splitpaths [DOCUMENT name | WINDOW name | OBJECTID number]
flattenpath [DOCUMENT name | WINDOW name | OBJECTID number]
smoothpath [DOCUMENT name | WINDOW name | OBJECTID number]
simplifypath [DOCUMENT name | WINDOW name | OBJECTID number]
reversepath [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### Boolesche Pfad-Operationen

```
andpaths [DOCUMENT name | WINDOW name]     /* Schnittmenge */
orpaths [DOCUMENT name | WINDOW name]      /* Vereinigung */
subpaths [DOCUMENT name | WINDOW name]     /* Subtraktion */
xorpaths [DOCUMENT name | WINDOW name]     /* Exklusiv-Oder */
```

---

### Masken

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

### Textumfluss

#### SetTextWrap / GetTextWrap

```
settextwrap [WRAP mode] [OFFSET offset]
           [DOCUMENT name | WINDOW name | OBJECTID number]
gettextwrap [WRAP &mode] [OFFSET &offset]
           [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Tabellen

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

### Objekt-Abfragen

#### GetObject

```
getobject [TYPE &type] [POSITION &coord] [NAME &name]
         [DOCUMENT document | WINDOW windowspec | OBJECTID objectid]
```

Gibt Objekttyp, Position (stem: .LEFT, .TOP, .RIGHT, .BOTTOM) und Name zurueck.

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

## Text-Befehle (Text Commands)

### Text-Bearbeitung

#### EditText

Setzt den Cursor in einen Textrahmen.

```
edittext [AT pointx pointy [stack] | OBJECTID number]
        [PAGE number | DOCUMENT name | WINDOW name]
```

#### Insert

Fuegt Text an der Cursorposition ein.

```
insert <text> [CHARACTERSET csetname]
      [DOCUMENT document | WINDOW document~window]
```

```rexx
'edittext'
'insert "Hallo PageStream!"'
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

Positioniert den Textcursor.

```
textcursor <direction> [SELECT]
          [DOCUMENT name | WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `direction` | /k | `LEFT`, `RIGHT`, `UP`, `DOWN`, `WORDLEFT`, `WORDRIGHT`, `LINESTART`, `LINEEND`, `PARASTART`, `PARAEND`, `CHAPTERSTART`, `CHAPTEREND`, `ARTICLESTART`, `ARTICLEEND` |
| `SELECT` | /S | Text waehrend der Bewegung auswaehlen |

---

### Suchen und Ersetzen

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

### Zeichenformatierung

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

Entfernt alle Textauszeichnungen.

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

#### Zeichen-Abfragen

```
getfont [DOCUMENT name | WINDOW name]
gettypesize [DOCUMENT name | WINDOW name]
gettypewidth [DOCUMENT name | WINDOW name]
getbaseline [DOCUMENT name | WINDOW name]
```

---

### Absatzformatierung

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

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `CLEAR` | /S | Alle Tabulatoren loeschen |
| `position` | /d | Tab-Position |
| `type` | /k | `LEFT`, `RIGHT`, `CENTER`, `DECIMAL` |
| `FILL` | /s | Fuellzeichen |
| `ALIGN` | /s | Ausrichtungszeichen (fuer DECIMAL) |

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

### Spezialelemente einfuegen

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

### Textverkettung

#### BreakTextRouting

```
breaktextrouting [DOCUMENT name | WINDOW name | OBJECTID number]
```

#### SetTextRouting

```
settextrouting [DOCUMENT name | WINDOW name | OBJECTID number]
```

---

### Verzeichnisse generieren

```
generateindex [DOCUMENT document | WINDOW document-window]
generatetoc [DOCUMENT document | WINDOW document-window]
generatetof [DOCUMENT document | WINDOW document-window]
```

---

## Farb-Befehle (Color Commands)

### SetColor

```
setcolor colorname [ALL | FILL | STROKENUMBER n]
        [OBJECT | TEXT | SHADOW | OUTLINE | REVERSE | UNDERLINE |
         RULEABOVE | RULEBELOW | CELLFILL | CELLTOP | CELLBOTTOM |
         CELLLEFT | CELLRIGHT | DROPCAP | BULLET | NUMBERED]
        [DOCUMENT name | WINDOW name | OBJECTID number | STYLETAG name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `colorname` | /s | Farbname |
| `ALL` | /S | Fuellung und Kontur (Standard) |
| `FILL` | /S | Nur Fuellung |
| `STROKENUMBER` | /i | Bestimmte Kontur (ab 0) |
| `what` | /k | Zielattribut |

```rexx
'setcolor "Red" fill text'
```

### GetColors

```
getcolors [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

Gibt Farbanzahl und -namen zurueck.

---

## Stil-Befehle (Style Commands)

### NewStyleTag

```
newstyletag name type [NEXTSTYLETAG tagname]
           [DOCUMENT name | WINDOW name | CHAPTER name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `name` | /s | Name des neuen Stils |
| `type` | /k | `CHARACTER`, `PARAGRAPH`, `OBJECT`, `COLOR`, `FPATTERN`, `LPATTERN` |
| `NEXTSTYLETAG` | /s | Nachfolgender Stil |

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

Beginnt/beendet die Modifikation eines Stils. Muessen paarweise verwendet werden.

```
beginstyletag [STYLETAG name]
endstyletag [STYLETAG name]
```

### AppendStyleTags

Laedt eine Stildatei und haengt deren Stile an.

```
appendstyletags <FILE filepath | ASK> [type] [progress]
               [DOCUMENT name | WINDOW name | CHAPTER name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `FILE` | /s | Stildatei-Pfad |
| `ASK` | /S | Dateiauswahl-Dialog |
| `type` | /k | `ALL` (Standard), `TEXT`, `PARAGRAPH`, `CHARACTER`, `OBJECT`, `COLOR` |
| `progress` | /k | `STATUS` oder `NOSTATUS` |

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

Hinweis: STROKENUMBER zaehlt ab 0 (im Gegensatz zur UI, die ab 1 zaehlt).

### Stil-Abfragen

#### GetStyleTags

```
getstyletagss liststem [mode] [DOCUMENT document | CHAPTER document:... | WINDOW document-window]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `mode` | /k | `ALL` (Standard), `TEXT`, `PARAGRAPH`, `CHARACTER`, `OBJECT`, `COLOR` |

Gibt Anzahl zurueck. Stilnamen in der Liste.

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

Gibt Stilnamen zurueck oder `MIXED` wenn mehrere Stile verwendet werden.

---

## System-Befehle (System Commands)

### Fenster und Ansicht

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

Aendert Seite, Zoom und Offset einer Ansicht.

```
display [PAGE <number | shortcut>] [SCALE <custom | preset>]
       [OFFSET offsetx offsety] [WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `PAGE` | /i oder /k | Seitennummer oder: `LASTUSED`, `PREVIOUS`, `NEXT`, `UP`, `DOWN`, `START`, `END`; oder MPG-Name |
| `SCALE` | /d oder /k | Prozent oder: `FULLPAGE`, `FULLPAGEWIDTH`, `FULLPAGEHEIGHT`, `FULLPASTEBOARD`, `FULLPASTEBOARDWIDTH`, `FULLPASTEBOARDHEIGHT`, `LASTUSED`, `ZOOMIN`, `ZOOMOUT` |
| `OFFSET` | /d | Horizontaler und vertikaler Versatz |

```rexx
'display page 7 scale fullpage'
'display page previous'
'display page "MyDoc~7" scale 85 offset 5.5 "-3"'
```

#### SetToolMode

```
settoolmode tool [WINDOW name]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `tool` | /k | `OBJECT`, `RESHAPE`, `CROP`, `MAGNIFY`, `TEXT`, `EYEDROPPER`, `COLUMN`, `LINE`, `BOX`, `RBOX`, `ELLIPSE`, `ARC`, `PEN`, `FREEHAND`, `GRID`, `ROUTETEXT`, `LASTUSED`, `PREVIOUS`, `NEXT` |

#### MainToolbox / SetToolbar / ColorPalette

```
maintoolbox [OPEN | CLOSE]
settoolbar [SHOW | HIDE | TOGGLE] [WINDOW name]
colorpalette [OPEN | CLOSE]
```

### Einstellungen

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

Masseinheiten: `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `METRICPOINTS`, `FEET`, `METERS`

---

### Drucken

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

### Rueckgaengig / Wiederholen

```
undo [DOCUMENT name | WINDOW name]
redo [DOCUMENT name | WINDOW name]
```

### Beenden

```
quit [mode]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `mode` | /k | `FORCE` (ohne Speichern), `ALERT` (Standard, Benutzer fragen), `QUIET` (still beenden) |

---

### System-Abfragen

#### GetVersion

```
getversion
```

Gibt Versionsstring in RESULT zurueck.

#### GetDisplay

```
getdisplay stem [WINDOW name]
```

Ergebnisse: `stem.page`, `stem.mode`, `stem.scale`, `stem.left`, `stem.top`

#### GetScreenDPI (nur Amiga)

```
getscreendpi stem
```

Ergebnisse: `stem.x`, `stem.y`

#### GetScreenName (nur Amiga)

```
getscreenname
```

Gibt den Public-Screen-Namen in RESULT zurueck.

#### GetPortname (nur Amiga)

```
getportname
```

Gibt den ARexx-Port-Namen in RESULT zurueck.

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

Gibt das aktuelle Werkzeug zurueck: `OBJECT`, `RESHAPE`, `CROP`, `TEXT`, `MAGNIFY`, `EYEDROPPER`, `COLUMN`, `LINE`, `BOX`, `RBOX`, `ELLIPSE`, `ARC`, `POLYGON`, `PEN`, `FREEHAND`, `GRID`, `ROUTETEXT`, `ROTATE`, `SBOX`, `BBOX`, `IBOX`, `PIE`, `STAR`, `SCALLOP`, `PUFFY`, `WAVY`, `LASSO` (PGS4.0+), `TABLE` (PGS4.1+)

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

Gibt die letzte Fehlernummer bzw. den letzten Fehlertext zurueck.

---

### Koordinaten-Konvertierung

#### GetCoordFromString

```
getcoordfroms string defaultmeasurementsystem
```

Konvertiert einen Mess-String (z.B. "1in") in einen internen Koordinatenwert.

Masseinheiten: `INCHES`, `CENTIMETERS`, `MILLIMETERS`, `PICAS`, `POINTS`, `PRINTERPOINTS`, `CICEROS`, `DIDOTPOINTS`, `METRICPOINTS`, `FEET`, `METERS`

#### GetCmdStringFromCoord

```
getcmdstringfromcoord coord measurementtype
```

Konvertiert einen internen Koordinatenwert in einen Befehlsstring (z.B. "1in").

#### GetUIStringFromCoord

```
getuistringfromcoord coord measurementtype
```

Konvertiert in einen UI-formatierten String.

---

### Oberflaechen-Sperre

#### LockInterface

```
lockinterface <TRUE | FALSE>
```

**WARNUNG:** Wenn das Skript endet, bevor `lockinterface false` aufgerufen wird, bleibt die Oberflaeche gesperrt und PageStream muss zwangsbeendet werden!

```rexx
'lockinterface true'
/* ... Operationen ... */
'lockinterface false'
```

#### CheckLock

```
checklock
```

Prueft den aktuellen Sperrstatus.

---

### Skript-Ausfuehrung

#### RX

```
rx filepath [DOCUMENT name | WINDOW name]
```

Fuehrt ein externes ARexx-, Python- oder AppleScript-Skript aus.

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

### Einstellungs-Pufferung

#### BeginPrefCapture / EndPrefCapture

Puffert mehrere Einstellungsaenderungen fuer schnellere Ausfuehrung.

```
beginprefcapture [SAVE]
endprefcapture
```

### CharacterSet

```
characterset csetname [SAVE]
```

Setzt den Zeichensatz. Gibt den vorherigen zurueck.

### Beep (nur Amiga)

```
beep
```

---

## Dialog-Befehle (Dialog Box Commands)

PageStream ermoeglicht das Erstellen eigener Dialogboxen in ARexx-Skripten.

### AllocArexxRequester

Erstellt einen neuen Dialog.

```
allocarexxrequester name width height
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `name` | /s | Titel des Dialogs |
| `width` | /i | Innere Breite in Pixeln |
| `height` | /i | Innere Hoehe in Pixeln |

Gibt Dialog-Handle in RESULT zurueck. **Muss vor Skript-Ende mit `freearexxrequester` freigegeben werden!**

```rexx
'allocarexxrequester "Test Dialog Box" 400 220'
iTextBox = RESULT
```

### AllocArexxList

Erstellt eine Liste fuer Listen-Steuerelemente.

```
allocarexxlist
```

Gibt Listen-Handle in RESULT zurueck. Listen existieren unabhaengig von Dialogen. Eine Liste kann mehreren Steuerelementen zugewiesen werden. **Muss mit `freearexxlist` freigegeben werden!**

### AddArexxList

Fuegt ein Element zu einer Liste hinzu.

```
addarexxlist handle item
```

Elemente werden ab 0 indiziert. Keine maximale Anzahl.

```rexx
'addarexxlist choices "First choice"'
```

### AddArexxGadget

Fuegt ein Steuerelement zu einem Dialog hinzu.

```
addarexxgadget dbox type left top width [height]
              [LABEL name] [LABELPOS pos] [STRING text]
              [BORDER style] [CHECKED status]
              [LIST handle] [CURRENT value] [TOTAL value]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `dbox` | /i | Dialog-Handle |
| `type` | /k | `EXIT`, `STRING`, `MULTILINE`, `TEXT`, `CHECKBOX`, `POPUP`, `CYCLE`, `SCROLLIST`, `SLIDER` |
| `left` | /i | Linke Position |
| `top` | /i | Obere Position |
| `width` | /i | Breite |
| `height` | /i | Hoehe (fuer MULTILINE und SCROLLIST, Minimum: 36) |
| `LABEL` | /s | Beschriftung |
| `LABELPOS` | /k | Position: `LEFT`, `RIGHT`, `ABOVE`, `ABOVELEFT`, `ABOVERIGHT`, `BELOW`, `BELOWLEFT`, `BELOWRIGHT`, `CENTER`, `CENTERLEFT`, `CENTERRIGHT`, `LEFTABOVE`, `LEFTBELOW`, `RIGHTABOVE`, `RIGHTBELOW` |
| `STRING` | /s | Standard-Text |
| `BORDER` | /k | `NONE`, `RAISED`, `RECESSED`, `SHINE`, `SHADOW`, `TEXT` |
| `CHECKED` | /k | `TRUE` oder `FALSE` (fuer CHECKBOX) |
| `LIST` | /i | Listen-Handle (fuer POPUP, CYCLE, SCROLLIST) |
| `CURRENT` | /i | Standard-Listeneintrag oder Slider-Position |
| `TOTAL` | /d | Slider-Bereich |

Gibt Steuerelement-Handle in RESULT zurueck.

```rexx
'addarexxgadget' dboxid 'exit 16 114 70 label "Cancel"'
Cancel = RESULT

'addarexxgadget' dboxid 'string 16 20 280 label "Name:" string "Beispiel"'
nameField = RESULT

'addarexxgadget' dboxid 'scrollist 16 8 90 72'
Fonts = RESULT
```

### SetArexxGadget

Aendert Eigenschaften eines Steuerelements.

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

Fragt den Zustand eines Steuerelements ab.

```
getarexxgadget dbhandle ctlhandle attribute
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `attribute` | /s | `CHECKED` (gibt 0/1 zurueck), `STRING` (Text), `CURRENT` (ausgewaehltes Element) |

```rexx
'getarexxgadget' iGetName sNameCtl 'string'
sName = RESULT
```

### DoARexxRequester / DoRequester

Zeigt den Dialog an und wartet auf Benutzereingabe.

```
doarexxrequester dbhandle
dorequester dbhandle
```

Gibt den Handle des gedrueckten EXIT-Buttons in RESULT zurueck.

### FreeARexxRequester / FreeARexxList

Gibt Dialoge und Listen frei. **Immer aufrufen, um Speicherverlust zu vermeiden!**

```
freearexxrequester dbhandle
freearexxlist listhandle
```

### AlertRequester

Zeigt eine Warnmeldung an.

```
alertrequester message [BUTTON1 label] [BUTTON2 label] [BUTTON3 label]
```

---

### Busy-Requester

#### OpenBusyRequester

```
openbusyrequester [MESSAGE message] [ABORT <DISABLED|ENABLED>]
                 [THERMOMETER <DISABLED|ENABLED>]
                 [DELAYEDOPEN <DISABLED|ENABLED>]
                 [TOTAL total] [CURRENT current]
```

Gibt Handle in RESULT zurueck.

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

Gibt 0 zurueck wenn Stop nicht gedrueckt, 1 wenn gedrueckt.

#### CloseBusyRequester

```
closebusyrequester handle
```

```rexx
/* Fortschrittsanzeige */
'openbusyrequester message "Verarbeite..." thermometer enabled total 100 current 0'
bh = RESULT
DO i = 1 TO 100
    'getbusyrequester' bh
    IF RESULT = 1 THEN BREAK
    ELSE 'setbusyrequester' bh 'current' i
END
'closebusyrequester' bh
```

---

### Benutzereingabe-Dialoge

#### GetChoice (nur Amiga)

```
getchoice MESSAGE message BUTTON1 label [BUTTON2 label] [BUTTON3 label]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `MESSAGE` | /s | Nachricht (max. 44 Zeichen) |
| `BUTTON1` | /s | Standard-Button (max. 8 Zeichen) |
| `BUTTON2` | /s | Abbrechen-Button (mit Esc aktivierbar) |
| `BUTTON3` | /s | Dritter Button |

Gibt Button-Nummer in RESULT zurueck. Unterstrich vor Buchstabe = Tastenkuerzel (z.B. "\_OK").

```rexx
'getchoice message "Click on a button." button1 "\_OK" button2 "\_Cancel"'
buttonid = RESULT
```

#### GetFile

```
getfile [TITLE title] [mode] [PATH name] [FILE name]
       [POSBUTTON label] [NEGBUTTON label]
```

| Parameter | Typ | Beschreibung |
|-----------|-----|-------------|
| `mode` | /k | `LOAD` (Standard) oder `SAVE` |
| RC = 0 | | Erfolg, Dateipfad in RESULT |
| RC = 10 | | Abgebrochen |

```rexx
'getfile TITLE "Save a file" save path "ram:" posbutton "Save"'
```

#### GetFilePath

```
getfilepath [PATH path] [TITLE title] [POSBUTTON ok] [NEGBUTTON cancel]
```

#### GetString (nur Amiga)

```
getstring [STRING default] [TITLE label] [POSBUTTON label] [NEGBUTTON label]
```

RC = 0 + String in RESULT bei Erfolg; RC = 10 bei Abbruch.

```rexx
'getstring string "Erase this." title "\_Text" posbutton "\_Yes" negbutton "\_No"'
userstring = RESULT
```

#### GetRegion

```
getregion stem [MESSAGE message]
```

Benutzer zieht einen Bereich auf. Ergebnisse: `stem.x1`, `stem.y1`, `stem.x2`, `stem.y2`. RC = 0 bei Erfolg, RC = 10 bei Abbruch.

```rexx
'getregion coord message "Drag to define an area"'
IF RC = 0 THEN DO
    SAY coord.x1 coord.y1 coord.x2 coord.y2
END
```

---

## Skript-Befehle (Script Commands)

### BeginCommandCapture / EndCommandCapture

```
begincommandcapture
endcommandcapture
```

---

## Beispiel-Skripte

### Neues Dokument mit Text

```rexx
/* CreateDoc.rexx - Dokument mit Textrahmen */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'opendocument'
'drawtextobj 1.0 1.0 6.5 9.0'
textobj = RESULT
'edittext'
'setfont "Times"'
'settypesize 12'
'insert "Hallo PageStream!"'
```

### Mehrere Formen zeichnen

```rexx
/* DrawShapes.rexx - Verschiedene Formen */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'drawbox 1.0 1.0 3.0 2.0'
'drawellipse 5.0 1.5 1.5 1.0'
'drawpolygon 3.5 5.0 1.5 1.5 STAR SIDES 5'
'drawline 1.0 8.0 7.0 8.0'
```

### Benutzerdialog

```rexx
/* UserDialog.rexx - Einfacher Eingabe-Dialog */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'allocarexxrequester "Mein Dialog" 300 100'
dbox = RESULT

'addarexxgadget' dbox 'EXIT 10 60 80 LABEL "\_OK"'
ok = RESULT
'addarexxgadget' dbox 'EXIT 210 60 80 LABEL "\_Abbrechen"'
cancel = RESULT
'addarexxgadget' dbox 'STRING 10 20 280 LABEL "Name:" STRING "Beispiel"'
nameField = RESULT

'doarexxrequester' dbox
button = RESULT

IF button = ok THEN DO
    'getarexxgadget' dbox nameField 'string'
    SAY "Eingabe:" RESULT
END

'freearexxrequester' dbox
```

### Skript mit Fortschrittsanzeige

```rexx
/* Progress.rexx - Fortschrittsbalken */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'lockinterface true'
'refresh wait'

'openbusyrequester message "Verarbeite Seiten..." thermometer enabled total 10 current 0'
bh = RESULT

DO i = 1 TO 10
    'getbusyrequester' bh
    IF RESULT = 1 THEN LEAVE  /* Stop gedrueckt */
    /* ... Seitenoperationen ... */
    'setbusyrequester' bh 'current' i
END

'closebusyrequester' bh
'refresh continue'
'lockinterface false'
```

### Alle Dokumente auflisten

```rexx
/* ListDocs.rexx - Offene Dokumente auflisten */
OPTIONS RESULTS
ADDRESS PAGESTREAM

'getdocuments docs'
count = RESULT
SAY count "Dokument(e) offen:"
DO i = 0 TO count - 1
    SAY "  " docs.i
END
```
