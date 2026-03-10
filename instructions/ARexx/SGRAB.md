# SGRAB ARexx Port

Port name: SGRAB. Requires easyrexx.library. SGrab must be running for the port to be available.

**CRITICAL Quoting rule:** Every command sent to the SGRAB port MUST be enclosed in single quotes — both when using arexx_command and when writing ARexx scripts. Without single quotes, ARexx interprets the words as variable names instead of sending them as commands. Use double quotes inside for string arguments.
- arexx_command: `'GRABSCREEN FILE "RAM:shot.png" PNG'`
- ARexx script: `ADDRESS 'SGRAB'` then `'GRABSCREEN FILE "RAM:shot.png" PNG'`
- WRONG (will fail): `GRABSCREEN FILE "RAM:shot.png" PNG` (missing single quotes!)

## Commands
- `GRABSCREEN [FILE <path>] [PNG] [JPEG] [X=<x>] [Y=<y>] [W=<w>] [H=<h>]` -- Capture screen to file
  - FILE: Output path (required)
  - PNG: Save as PNG format
  - JPEG: Save as JPEG format
  - X/Y/W/H: Capture region (optional, default: full screen)
