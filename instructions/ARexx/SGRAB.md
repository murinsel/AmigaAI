# SGRAB ARexx Port

Port name: SGRAB. Requires easyrexx.library. SGrab must be running for the port to be available.

## Commands
- `GRABSCREEN [FILE <path>] [PNG] [JPEG] [X=<x>] [Y=<y>] [W=<w>] [H=<h>]` -- Capture screen to file
  - FILE: Output path (required)
  - PNG: Save as PNG format
  - JPEG: Save as JPEG format
  - X/Y/W/H: Capture region (optional, default: full screen)
