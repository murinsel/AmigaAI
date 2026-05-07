# SGRAB ARexx Port

Port name: SGRAB. Requires easyrexx.library. SGrab must be running as a Commodity for the port to be available.

## Launching sgrab so the ARexx port is registered

Start sgrab from a Shell with a CX_POPKEY argument so it installs itself as a Commodity and stays resident:

```
sgrab CX_POPKEY="shift ctrl numericpad *"
```

Without `CX_POPKEY` sgrab does not register its ARexx port. The shell escape `**` in scripts produces a literal `*`. Once started, `Status` lists it and the SGRAB port answers ARexx commands.

## Quoting rules

The quoting depends on HOW you send the command:

- **arexx_command tool:** send the command WITHOUT enclosing single quotes. The tool sends the string directly to the port. Example: `GRABSCREEN SCREEN DeluxeMusic FILE "RAM:shot.png" PNG`
  - If you wrap it in single quotes here, SGRAB returns a misleading `error 20/0` (the grab may still happen, but the result reporting is wrong).
- **ARexx script (.rexx file):** every command line MUST be enclosed in single quotes so the ARexx interpreter sends the literal text instead of treating words as variables. Example:
  ```rexx
  ADDRESS 'SGRAB'
  'GRABSCREEN SCREEN DeluxeMusic FILE "RAM:shot.png" PNG'
  ```

In both cases, use double quotes around string arguments that contain spaces or path characters (e.g. FILE paths).
The SCREEN public-screen name is given WITHOUT quotes (e.g. `SCREEN DeluxeMusic`, not `SCREEN "DeluxeMusic"`).

## Commands

- `GRABSCREEN [SCREEN <pubname>] [FILE <path>] [PNG] [JPEG] [X=<x>] [Y=<y>] [W=<w>] [H=<h>]` -- Capture screen to file
  - SCREEN: Public-screen name to capture (e.g. `DeluxeMusic`, `Workbench`). Name unquoted. If omitted, the frontmost screen is grabbed. Use `list_screens` (or check intuition) to find the public name.
  - FILE: Output path (required, quoted if it contains spaces or punctuation)
  - PNG: Save as PNG format
  - JPEG: Save as JPEG format
  - X/Y/W/H: Capture region (optional, default: full screen)

## Examples

Grab the DeluxeMusic public screen as PNG (via arexx_command):
```
GRABSCREEN SCREEN DeluxeMusic FILE "RAM:dmusic.png" PNG
```

Grab a 320x200 region of the Workbench screen as JPEG (via arexx_command):
```
GRABSCREEN SCREEN Workbench X=0 Y=0 W=320 H=200 FILE "RAM:region.jpg" JPEG
```

Same command from an ARexx script:
```rexx
ADDRESS 'SGRAB'
'GRABSCREEN SCREEN DeluxeMusic FILE "RAM:dmusic.png" PNG'
```
