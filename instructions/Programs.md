# Available Amiga Programs

This file lists programs installed on this Amiga. Use this to know what software is available, where it is located, and how to control it via ARexx.

**You can update this file!** When you discover a new program (e.g. via list_ports, shell_command, or identify_file), add it below using write_file on `AmigaAI:instructions/Programs.md`. Keep the format consistent.

## System Utilities

| Program | Path | ARexx Port | Description |
|---------|------|------------|-------------|
| MultiView | SYS:Utilities/MultiView | MULTIVIEW.1 | Universal file viewer (pictures, text, AmigaGuide, sound). Uses DataTypes. |
| Ed | C:Ed | ED | Standard AmigaOS text editor |
| Installer | SYS:Utilities/Installer | — | Amiga Installer for .install scripts |
| IconEdit | SYS:Tools/IconEdit | — | Icon editor for Workbench icons |
| Calculator | SYS:Tools/Calculator | — | Simple calculator |
| Clock | SYS:Tools/Clock | — | Clock display |

## Communication

| Program | Path | ARexx Port | Description |
|---------|------|------------|-------------|
| YAM | YAM:YAM | YAM | E-mail client (POP3/SMTP). Startup takes up to 60 seconds — be patient. ARexx docs: AmigaAI:instructions/ARexx/YAM.md |
| IBrowse | IBrowse:IBrowse | IBROWSE | Web browser (HTTP/HTTPS). ARexx docs: AmigaAI:instructions/ARexx/IBrowse.md |

## Graphics & DTP

| Program | Path | ARexx Port | Description |
|---------|------|------------|-------------|
| PageStream | PageStream:PageStream | PAGESTREAM | Desktop publishing. ARexx docs: AmigaAI:instructions/ARexx/PageStream.md |
| SGrab | SYS:Tools/SGrab | SGRAB | Screenshot tool (PNG output). ARexx docs: AmigaAI:instructions/ARexx/SGRAB.md |

## System Services

| Program | Path | ARexx Port | Description |
|---------|------|------------|-------------|
| RexxMast | SYS:System/RexxMast | REXX | ARexx interpreter/server. Must be running for ARexx scripts. |
| Workbench | — | WORKBENCH | Workbench desktop. ARexx docs: AmigaAI:instructions/ARexx/WORKBENCH.md |
| AmigaAI | AmigaAI:AmigaAI | AMIGAAI | This application (you!). ARexx docs: AmigaAI:instructions/ARexx/AMIGAAI.md |

## How to Discover More Programs

- Use `list_ports` tool to see currently active ARexx ports
- Use `shell_command` with `List SYS:Tools` or `List SYS:Utilities` to find installed programs
- Check common locations: `SYS:Tools/`, `SYS:Utilities/`, `SYS:System/`, `C:`, `Work:`
- When you find a new program, add it to this file!
