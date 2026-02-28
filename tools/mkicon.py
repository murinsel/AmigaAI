#!/usr/bin/env python3
"""Generate a custom Amiga .info icon for AmigaAI.

Creates an old-style (OS 1.x-3.x compatible) DiskObject icon with
a speech bubble + "AI" design, 2 bitplanes (4 colors).

Workbench 2/3 palette:
  Pen 0 = grey (background)
  Pen 1 = black (outlines)
  Pen 2 = white (highlights)
  Pen 3 = blue (fill)
"""

import struct
import sys
import os

# Icon dimensions
WIDTH = 48    # must be multiple of 16
HEIGHT = 44
DEPTH = 2     # 2 bitplanes = 4 colors

# Pen values
GREY  = 0
BLACK = 1
WHITE = 2
BLUE  = 3

def make_grid():
    """Create the pixel grid for the icon."""
    g = [[GREY] * WIDTH for _ in range(HEIGHT)]

    # --- Draw speech bubble outline and fill ---
    # Bubble body: rounded rectangle from rows 2-33, cols 3-42
    def ellipse_points(cx, cy, rx, ry):
        """Return set of (x,y) on the ellipse boundary."""
        pts = set()
        for a in range(3600):
            import math
            t = math.radians(a / 10.0)
            x = round(cx + rx * math.cos(t))
            y = round(cy + ry * math.sin(t))
            pts.add((x, y))
        return pts

    def fill_rect(g, x1, y1, x2, y2, color):
        for y in range(y1, y2 + 1):
            for x in range(x1, x2 + 1):
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    g[y][x] = color

    def fill_rounded_rect(g, x1, y1, x2, y2, r, fill_color, outline_color):
        """Draw a rounded rectangle with outline."""
        import math
        for y in range(y1, y2 + 1):
            for x in range(x1, x2 + 1):
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    # Check if inside rounded rect
                    inside = False
                    # Center area
                    if x1 + r <= x <= x2 - r and y1 <= y <= y2:
                        inside = True
                    elif x1 <= x <= x2 and y1 + r <= y <= y2 - r:
                        inside = True
                    else:
                        # Check corners
                        corners = [
                            (x1 + r, y1 + r),
                            (x2 - r, y1 + r),
                            (x1 + r, y2 - r),
                            (x2 - r, y2 - r),
                        ]
                        for cx, cy in corners:
                            dx = x - cx
                            dy = y - cy
                            if (x < x1 + r or x > x2 - r) and (y < y1 + r or y > y2 - r):
                                if dx * dx + dy * dy <= r * r:
                                    inside = True
                                    break

                    if inside:
                        # Check if on outline (within 1px of edge)
                        on_edge = False
                        for nx, ny in [(x-1,y),(x+1,y),(x,y-1),(x,y+1)]:
                            if nx < x1 or nx > x2 or ny < y1 or ny > y2:
                                on_edge = True
                                break
                            # Check corner rounding
                            if not on_edge:
                                for cx, cy in [(x1+r,y1+r),(x2-r,y1+r),(x1+r,y2-r),(x2-r,y2-r)]:
                                    dnx = nx - cx
                                    dny = ny - cy
                                    if (nx < x1+r or nx > x2-r) and (ny < y1+r or ny > y2-r):
                                        if dnx*dnx + dny*dny > r*r:
                                            on_edge = True
                                            break

                        if on_edge:
                            g[y][x] = outline_color
                        else:
                            g[y][x] = fill_color

    # Main bubble body
    fill_rounded_rect(g, 3, 2, 42, 33, 6, BLUE, BLACK)

    # Speech bubble tail (triangle pointing down-left)
    for row in range(34, 42):
        dy = row - 34
        x_start = 8 - dy
        x_end = 14 - dy
        if x_start < 2:
            x_start = 2
        for x in range(x_start, x_end + 1):
            if 0 <= x < WIDTH:
                g[row][x] = BLUE
        # Outline edges of tail
        if 0 <= x_start < WIDTH:
            g[row][x_start] = BLACK
        if 0 <= x_end < WIDTH and x_end != x_start:
            g[row][x_end] = BLACK
    # Tail tip
    if HEIGHT > 41:
        g[41][3] = BLACK
        g[41][4] = BLACK

    # Bottom outline of bubble connects to tail
    for x in range(15, 43):
        if g[33][x] == BLUE:
            g[33][x] = BLUE  # keep fill, outline already set

    # --- Add top-left highlight (white edge inside bubble) ---
    for y in range(4, 8):
        for x in range(6, 40):
            if g[y][x] == BLUE:
                g[y][x] = WHITE
                break  # only first blue pixel per row

    for x in range(8, 38):
        if g[4][x] == BLUE:
            g[4][x] = WHITE

    # --- Draw "AI" text ---
    # "A" character (rows 10-28, centered around col 13)
    A = [
        "  1111  ",
        " 111111 ",
        "111  111",
        "111  111",
        "111  111",
        "11111111",
        "11111111",
        "111  111",
        "111  111",
        "111  111",
        "111  111",
    ]

    # "I" character (rows 10-28, centered around col 31)
    I = [
        "11111111",
        "11111111",
        "   11   ",
        "   11   ",
        "   11   ",
        "   11   ",
        "   11   ",
        "   11   ",
        "   11   ",
        "11111111",
        "11111111",
    ]

    def draw_char(g, char_data, start_x, start_y, color):
        for dy, row in enumerate(char_data):
            for dx, ch in enumerate(row):
                if ch == '1':
                    x = start_x + dx
                    y = start_y + dy
                    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                        g[y][x] = color

    draw_char(g, A, 10, 11, WHITE)
    draw_char(g, I, 26, 11, WHITE)

    # --- Small sparkle/dot accent (top-right of bubble) ---
    g[6][38] = WHITE
    g[5][38] = WHITE
    g[6][39] = WHITE
    g[7][38] = WHITE
    g[6][37] = WHITE

    return g


def make_selected_grid(g):
    """Create selected (pressed) state - swap white/blue, darken."""
    g2 = [row[:] for row in g]
    for y in range(HEIGHT):
        for x in range(WIDTH):
            c = g2[y][x]
            if c == BLUE:
                g2[y][x] = WHITE
            elif c == WHITE:
                g2[y][x] = BLUE
    return g2


def grid_to_bitplanes(g):
    """Convert pixel grid to interleaved bitplane data (plane 0 then plane 1)."""
    words_per_row = WIDTH // 16
    plane0 = []
    plane1 = []

    for y in range(HEIGHT):
        for w in range(words_per_row):
            word0 = 0
            word1 = 0
            for bit in range(16):
                x = w * 16 + bit
                pen = g[y][x] if x < WIDTH else 0
                if pen & 1:
                    word0 |= (1 << (15 - bit))
                if pen & 2:
                    word1 |= (1 << (15 - bit))
            plane0.append(word0)
            plane1.append(word1)

    # Amiga format: all of plane 0, then all of plane 1
    data = b''
    for w in plane0:
        data += struct.pack('>H', w)
    for w in plane1:
        data += struct.pack('>H', w)
    return data


def write_icon(filename, grid, sel_grid, stack_size=131072):
    """Write an Amiga .info file."""
    img1_data = grid_to_bitplanes(grid)
    img2_data = grid_to_bitplanes(sel_grid)

    words_per_row = WIDTH // 16

    # Image structure (20 bytes each, on disk)
    def make_image_header():
        return struct.pack('>hhhhh',
            0,      # LeftEdge
            0,      # TopEdge
            WIDTH,  # Width
            HEIGHT, # Height
            DEPTH,  # Depth
        ) + struct.pack('>I', 0) + struct.pack('>BB', 0x03, 0x00) + struct.pack('>I', 0)
        # ImageData pointer (placeholder), PlanePick=0x03, PlaneOnOff=0x00, NextImage=NULL

    # Flags: GFLG_GADGIMAGE (0x0004) | GFLG_GADGHIMAGE (0x0002) = 0x0006
    # This means: use image for rendering, use alternate image for highlight
    gadget_flags = 0x0003  # GADGIMAGE | GADGHCOMP... let me use proper flags
    # Actually for two-image icon: Flags = GFLG_GADGHIMAGE (2) | GFLG_GADGIMAGE (4) = 6
    # But looking at real icons, typically Flags=3 means GADGHCOMP|GADGHIMAGE...
    # Let me use 0x0006 for two separate images
    gadget_flags = 0x0006

    with open(filename, 'wb') as f:
        # DiskObject header
        f.write(struct.pack('>HH', 0xE310, 1))  # Magic, Version

        # Embedded Gadget structure
        f.write(struct.pack('>I', 0))       # NextGadget = NULL
        f.write(struct.pack('>hh', 0, 0))   # LeftEdge, TopEdge
        f.write(struct.pack('>hh', WIDTH, HEIGHT))  # Width, Height
        f.write(struct.pack('>H', gadget_flags))  # Flags
        f.write(struct.pack('>H', 1))       # Activation
        f.write(struct.pack('>H', 1))       # GadgetType
        f.write(struct.pack('>I', 1))       # GadgetRender (non-zero = image follows)
        f.write(struct.pack('>I', 1))       # SelectRender (non-zero = 2nd image follows)
        f.write(struct.pack('>I', 0))       # GadgetText = NULL
        f.write(struct.pack('>I', 0))       # MutualExclude
        f.write(struct.pack('>I', 0))       # SpecialInfo
        f.write(struct.pack('>H', 0))       # GadgetID
        f.write(struct.pack('>I', 0))       # UserData (revision)

        # DiskObject fields after gadget
        f.write(struct.pack('>B', 1))       # do_Type = WBTOOL
        f.write(struct.pack('>B', 0))       # padding
        f.write(struct.pack('>I', 0))       # do_DefaultTool (0 = none)
        f.write(struct.pack('>I', 0))       # do_ToolTypes (0 = none)
        f.write(struct.pack('>I', 0x80000000))  # do_CurrentX = NO_ICON_POSITION
        f.write(struct.pack('>I', 0x80000000))  # do_CurrentY = NO_ICON_POSITION
        f.write(struct.pack('>I', 0))       # do_DrawerData
        f.write(struct.pack('>I', 0))       # do_ToolWindow
        f.write(struct.pack('>I', stack_size))  # do_StackSize

        # Image 1 header
        f.write(struct.pack('>hh', 0, 0))   # LeftEdge, TopEdge
        f.write(struct.pack('>hh', WIDTH, HEIGHT))  # Width, Height
        f.write(struct.pack('>h', DEPTH))    # Depth
        f.write(struct.pack('>I', 0))        # ImageData (placeholder)
        f.write(struct.pack('>BB', 0x03, 0x00))  # PlanePick, PlaneOnOff
        f.write(struct.pack('>I', 0))        # NextImage

        # Image 1 bitplane data
        f.write(img1_data)

        # Image 2 header
        f.write(struct.pack('>hh', 0, 0))
        f.write(struct.pack('>hh', WIDTH, HEIGHT))
        f.write(struct.pack('>h', DEPTH))
        f.write(struct.pack('>I', 0))
        f.write(struct.pack('>BB', 0x03, 0x00))
        f.write(struct.pack('>I', 0))

        # Image 2 bitplane data
        f.write(img2_data)

    return os.path.getsize(filename)


def print_preview(g):
    """Print ASCII preview of the icon."""
    chars = ['.', '#', 'O', '@']
    for row in g:
        print(''.join(chars[p] for p in row))


if __name__ == '__main__':
    outfile = sys.argv[1] if len(sys.argv) > 1 else 'AmigaAI.info'

    print(f"Generating Amiga icon: {outfile}")
    print(f"  Size: {WIDTH}x{HEIGHT}, {DEPTH} bitplanes (4 colors)")

    grid = make_grid()
    sel_grid = make_selected_grid(grid)

    print("\nNormal state:")
    print_preview(grid)
    print("\nSelected state:")
    print_preview(sel_grid)

    size = write_icon(outfile, grid, sel_grid)
    print(f"\nWrote {size} bytes to {outfile}")
