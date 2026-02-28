# AmigaAI

Native Claude AI agent for AmigaOS 3.x (68020+).

## Dependencies
- AmigaOS 3.x with Roadshow TCP/IP stack
- AmiSSL v5 (from Aminet)
- json-c (from Aminet)
- MUI 3.8+ (Magic User Interface)

## Build (Cross-Compilation)
```
./build.sh
```
Uses Docker image `kareandersen/amiga-gcc` to cross-compile with m68k-amigaos-gcc.

## Build (Native on Amiga with vbcc)
```
make -f Makefile.amiga
```

## Configuration
Set API key: `echo "sk-ant-your-key" > ENV:AmigaAI/api_key`
Save permanently: `copy ENV:AmigaAI ENVARC:AmigaAI ALL`

## ARexx
Port name: AMIGAAI
Commands: ASK, GETLAST, CLEAR, SETMODEL, SETSYSTEM, QUIT
