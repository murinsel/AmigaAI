# AmigaAI

A native AI assistant for the Commodore Amiga, powered by Anthropic's Claude API.

AmigaAI brings modern AI capabilities to AmigaOS 3.x, running natively on 68020+ hardware. It communicates with the Claude API over HTTPS using the Amiga's own TCP/IP stack and TLS libraries — no emulation, no bridging, just pure Amiga.

## Features

- **MUI-based chat interface** with real-time conversation display
- **Tool use** — Claude can execute AmigaDOS commands, send ARexx messages, read/write files, identify file types, and control mouse/keyboard
- **Input simulation** — Mouse positioning, clicks (left/right/middle), and keyboard input via input.device
- **Persistent memory** across sessions
- **Localization** — English (built-in) and German via locale.library catalogs
- **ARexx port** (AMIGAAI) for scripting and automation
- **Window control** — Move, resize, iconify via ARexx commands
- **FileType** standalone CLI command for DataType-based file identification
- **Configurable** model, system prompt, and API key via AmigaOS ENV: system
- **Single instance** — Only one copy of AmigaAI runs at a time
- **API logging** for debugging (CLI and ToolType activation)

## Requirements

- AmigaOS 3.x (68020 CPU or higher)
- [Roadshow](http://roadshow.apc-tcp.de/index-en.php) TCP/IP stack
- [AmiSSL](https://aminet.net/package/comm/net/AmiSSL-v5) v5
- [MUI](https://aminet.net/package/util/libs/mui38usr) 3.8+ with TextEditor.mcc
- [json-c](https://aminet.net/package/dev/lib/json-c-amiga) (JSON parsing library)

## Building

### Cross-compilation (recommended)

```
./build.sh
```

Uses [Bebbo's amiga-gcc](https://github.com/bebbo/amiga-gcc) toolchain. The build script auto-detects a native `m68k-amigaos-gcc` installation or falls back to Docker.

### Native on Amiga

```
make -f Makefile.amiga
```

Requires Bebbo's GCC installed on the Amiga.

## Configuration

Set your Anthropic API key:

```
echo "sk-ant-your-key" > ENV:AmigaAI/api_key
```

Save permanently:

```
copy ENV:AmigaAI ENVARC:AmigaAI ALL
```

## Command Line Arguments

```
AmigaAI [CREATEICON] [APILOG <file>]
```

| Argument | Description |
|----------|-------------|
| `CREATEICON` | Create a Workbench icon (.info file) for AmigaAI and exit |
| `APILOG <file>` | Log all API requests and responses to the specified file |

Example:

```
AmigaAI APILOG RAM:api.log
```

## ToolTypes

When launched from Workbench, AmigaAI reads ToolTypes from its icon (.info file):

| ToolType | Description |
|----------|-------------|
| `APILOG=<file>` | Log all API requests and responses to the specified file |

Example icon ToolType entry: `APILOG=RAM:api.log`

## Tools

AmigaAI provides Claude with the following tools:

| Tool | Description |
|------|-------------|
| `shell_command` | Execute AmigaDOS commands |
| `arexx_command` | Send ARexx commands to running applications |
| `read_file` | Read file contents |
| `write_file` | Write to files |
| `list_ports` | List active ARexx message ports |
| `identify_file` | Identify file types using the DataType system |
| `mouse_move` | Move mouse pointer to screen coordinates |
| `mouse_click` | Click mouse button (left, right, middle) |
| `key_press` | Send a raw keyboard event |
| `type_text` | Type a string via keyboard simulation |

## FileType

A standalone CLI command for identifying file types:

```
FileType SYS:Utilities/MultiView
FileType WORK:Images FILTER picture ALL
FileType DH0: FILTER ILBM ALL MAXFILES 20
```

## ARexx Interface

Port name: `AMIGAAI`

| Command | Description |
|---------|-------------|
| `ASK <question>` | Send a question to Claude |
| `GETLAST` | Get the last response |
| `CLEAR` | Clear conversation history |
| `SETMODEL <model>` | Change the Claude model |
| `SETSYSTEM <prompt>` | Set system prompt |
| `MEMADD <text>` | Add a persistent memory entry |
| `MEMCLEAR` | Clear all memory entries |
| `MEMCOUNT` | Return number of memory entries |
| `MEMORY` | Return all memory entries |
| `MOVE <left> <top>` | Move window to position |
| `RESIZE <width> <height>` | Resize window |
| `WINDOWPOS` | Get window position and size |
| `WINDOWTOFRONT` | Bring window to front |
| `WINDOWTOBACK` | Send window to back |
| `MOUSEMOVE <x> <y>` | Move mouse to screen coordinates |
| `MOUSECLICK <button>` | Click mouse (LEFT, RIGHT, MIDDLE) |
| `KEYPRESS <code> [<qual>]` | Send raw key event |
| `TYPETEXT <text>` | Type text via keyboard simulation |
| `HIDE` | Iconify the application |
| `SHOW` | Deiconify the application |
| `QUIT` | Exit AmigaAI |

## Localization

AmigaAI uses AmigaOS locale.library for localization. English is built-in, German is included as a catalog file.

To use German: Set your system language to "deutsch" in the Locale preferences, and copy `Catalogs/Deutsch/AmigaAI.catalog` to `LOCALE:Catalogs/Deutsch/`.

## License

This project is provided as-is for the Amiga community.
