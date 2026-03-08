# SGrab - Shell Reference

Path: `SYS:Tools/SGrab` — Screen/window grabber (ILBM, PNG, JPEG output).

## Key Options

```
SGrab FILE PNG/S DELAY/K/N WINDOW/S WINDOWCONTENTS/S MARK/S X/N/K Y/N/K W/N/K H/N/K NOBEEP/S
```

| Option | Description |
|--------|-------------|
| `FILE` | Output filename |
| `PNG` | Save as PNG (default: ILBM) |
| `DELAY` | Seconds to wait before grab. Default: 5 |
| `WINDOW` | Grab active window instead of screen |
| `WINDOWCONTENTS` | Grab window without borders |
| `MARK` | Select area with mouse |
| `X/Y/W/H` | Capture region coordinates (ignored with WINDOW/MARK) |
| `NOBEEP` | Don't flash screen after grab |
| `NUMBER` | Auto-number output files |

## Filename Keywords

Use in FILE: `{number}`, `{width}`, `{height}`, `{depth}`, `{size}`, `{title}`, `{type}`, `{time}`, `{date}`

## Examples

```
SGrab FILE RAM:screenshot.png PNG DELAY 2
SGrab FILE RAM:window.png PNG WINDOWCONTENTS
SGrab FILE RAM:region.png PNG X 50 Y 50 W 100 H 100
```
