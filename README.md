# AmigaAI

A native AI assistant for the Commodore Amiga, powered by Anthropic's Claude API.

AmigaAI brings modern AI capabilities to AmigaOS 3.x, running natively on 68020+ hardware. It communicates with the Claude API over HTTPS using the Amiga's own TCP/IP stack and TLS libraries — no emulation, no bridging, just pure Amiga.

## Features

- **MUI-based chat interface** with real-time conversation display
- **Tool use** — Claude can execute AmigaDOS commands, send ARexx messages, read/write files, identify file types, and control mouse/keyboard
- **Drag & drop** — Drop files onto the chat area to send images or text to Claude, drop onto the input field to insert the path
- **Image conversion** — Amiga image formats (ILBM, etc.) are automatically converted to PNG via DataTypes before sending to the API
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
AmigaAI [CREATEICON] [APILOG <file>] [APIHOST <host>] [APIPORT <port>] [NOSSL]
        [APIPROVIDER <name>] [APIPATH <path>]
```

| Argument | Description |
|----------|-------------|
| `CREATEICON` | Create a Workbench icon (.info file) for AmigaAI and exit |
| `APILOG <file>` | Log all API requests and responses to the specified file |
| `APIHOST <host>` | API endpoint host (default: `api.anthropic.com`) |
| `APIPORT <port>` | API endpoint port (default: `443`) |
| `NOSSL` | Use plain HTTP instead of HTTPS |
| `APIPROVIDER <name>` | Request format: `anthropic` (default) or `openai` |
| `APIPATH <path>` | API endpoint path (e.g. `/v1/messages`, `/api/v1/chat/completions`) |

Example:

```
AmigaAI APILOG RAM:api.log
AmigaAI APIHOST=192.168.1.10 APIPORT=3456 NOSSL
```

## ToolTypes

When launched from Workbench, AmigaAI reads ToolTypes from its icon (.info file):

| ToolType | Description |
|----------|-------------|
| `APILOG=<file>` | Log all API requests and responses to the specified file |
| `APIHOST=<host>` | API endpoint host |
| `APIPORT=<port>` | API endpoint port |
| `NOSSL` | Use plain HTTP instead of HTTPS |
| `APIPROVIDER=<name>` | Request format (`anthropic` or `openai`) |
| `APIPATH=<path>` | API endpoint path |

Example icon ToolType entry: `APILOG=RAM:api.log`

## Using OpenRouter / Other Models

AmigaAI can also talk to OpenRouter, which exposes hundreds of models from
many vendors (Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, ...) behind
a single API key. Only models with **tool/function calling support** work
with AmigaAI's agent loop.

### Setup

1. Create an OpenRouter account and grab a key (`sk-or-v1-...`) at
   https://openrouter.ai/keys
2. In AmigaAI, open **Settings → Endpoint…** and click one of:
   - **OpenRouter** — Anthropic-compatible Messages API. Use this for
     Claude models on OpenRouter.
   - **OpenRouter (OpenAI)** — Chat Completions API. Use this for any
     non-Claude model (GPT-4o, Gemini, Llama, etc.).
3. Paste your `sk-or-v1-...` key into the API Key field. The key is
   stored separately from your Anthropic key (in
   `ENV:AmigaAI/api_key.openrouter`), so both can co-exist — just switch
   presets to swap.
4. Click **OK**, then open **Model…** to pick a model from the list.

### Configuration model

AmigaAI uses three independent settings to support multiple providers:

| Setting | Values | Meaning |
|---------|--------|---------|
| `api_provider` | `anthropic`, `openai` | Request body format on the wire |
| `api_auth` | `x-api-key`, `bearer` | HTTP auth scheme |
| `api_key_realm` | `anthropic`, `openrouter`, `openai` | Which key file to read |

This decoupling matters because OpenRouter speaks the **Anthropic Messages
format** but uses **Bearer auth**, and one OpenRouter key serves both formats.

### Image / screenshot tool with non-Claude models

Anthropic's API can return images directly inside a tool result, but the
OpenAI Chat Completions API can't. When using the OpenAI format, AmigaAI
automatically splits a screenshot result into a text-only tool message
(`Screenshot captured (see next message)`) plus a separate user message
carrying the image. The model sees the image either way; the user notices
nothing.

### Model availability

The Model dialog shows a different list per provider:

- **Anthropic native**: `claude-sonnet-4-6`, `claude-haiku-4-5-20251001`, `claude-opus-4-6`
- **OpenRouter (Anthropic format)**: `anthropic/claude-sonnet-4.6`, `anthropic/claude-haiku-4.5`, `anthropic/claude-opus-4.7`, …
- **OpenRouter (OpenAI format)**: `openai/gpt-4o`, `google/gemini-2.5-pro`, `meta-llama/llama-3.3-70b-instruct`, `mistralai/mistral-large`, `deepseek/deepseek-chat`, …

You can also type any model name into the Model dialog if it's not in the
preset list — OpenRouter will accept any model ID it recognises.

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
| `arexx_help` | Look up detailed ARexx command documentation on demand |
| `mouse_move` | Move mouse pointer to screen coordinates |
| `mouse_click` | Click mouse button (left, right, middle) |
| `key_press` | Send a raw keyboard event |
| `type_text` | Type a string via keyboard simulation |
| `screenshot` | Capture a screenshot (full screen or region) |

## ARexx Knowledge Base

AmigaAI includes detailed ARexx documentation for popular Amiga applications. When Claude detects a running ARexx port, it automatically loads the compact command overview. For detailed parameter info, it can query individual commands on demand via the `arexx_help` tool — keeping token usage low while providing full documentation depth.

Supported applications: **DMusic**, **IBrowse**, **PageStream**, **Workbench**, **YAM**

Additional applications can be supported by adding `.md` (overview) and `.help` (detailed reference) files to `instructions/ARexx/`.

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
