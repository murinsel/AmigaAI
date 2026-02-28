#!/bin/bash
# AmigaAI - Cross-compile with m68k-amigaos-gcc
# Usage: ./build.sh [clean]
#
# Uses native m68k-amigaos-gcc if found in PATH or ~/amiga-gcc-toolchain,
# otherwise falls back to Docker image.

PROJDIR="$(cd "$(dirname "$0")" && pwd)"

if [ "$1" = "clean" ]; then
    rm -rf "$PROJDIR/obj" "$PROJDIR/AmigaAI" "$PROJDIR/FileType"
    echo "Cleaned."
    exit 0
fi

# Locate m68k-amigaos-gcc: PATH first, then known install locations
if command -v m68k-amigaos-gcc >/dev/null 2>&1; then
    CC=m68k-amigaos-gcc
    STRIP=m68k-amigaos-strip
    USE_DOCKER=0
elif [ -x "$HOME/amiga-gcc-toolchain/bin/m68k-amigaos-gcc" ]; then
    export PATH="$HOME/amiga-gcc-toolchain/bin:$PATH"
    CC=m68k-amigaos-gcc
    STRIP=m68k-amigaos-strip
    USE_DOCKER=0
elif [ -x "/opt/amiga/bin/m68k-amigaos-gcc" ]; then
    export PATH="/opt/amiga/bin:$PATH"
    CC=m68k-amigaos-gcc
    STRIP=m68k-amigaos-strip
    USE_DOCKER=0
else
    USE_DOCKER=1
fi

CFLAGS="-m68020 -O2 -Wall -noixemul -fcommon -Isdk/include -Isrc"
LDFLAGS="-noixemul -Lsdk/lib -Wl,--allow-multiple-definition"
LIBS="-lamisslstubs -lsocket -lm"
SOURCES="src/main.c src/http.c src/claude.c src/json_utils.c src/cJSON.c src/gui.c src/arexx_port.c src/config.c src/memory.c src/tools.c src/dt_identify.c"

if [ "$USE_DOCKER" = "1" ]; then
    IMAGE="kareandersen/amiga-gcc"
    echo "=== Using Docker ($IMAGE) ==="
    docker run --rm \
        -v "$PROJDIR:/build" \
        -w /build \
        "$IMAGE" \
        bash -c "
set -e
CC=m68k-amigaos-gcc
STRIP=m68k-amigaos-strip
CFLAGS=\"$CFLAGS\"
LDFLAGS=\"$LDFLAGS\"
LIBS=\"$LIBS\"
SOURCES=\"$SOURCES\"
mkdir -p obj
echo '=== Compiling AmigaAI ==='
for src in \$SOURCES; do
    obj=\"obj/\$(basename \${src%.c}.o)\"
    echo \"  CC  \$src\"
    \$CC \$CFLAGS -c -o \"\$obj\" \"\$src\"
done
echo '=== Linking ==='
OBJS=''
for src in \$SOURCES; do
    OBJS=\"\$OBJS obj/\$(basename \${src%.c}.o)\"
done
\$CC \$LDFLAGS -o AmigaAI \$OBJS \$LIBS
\$STRIP AmigaAI
echo '=== Done: AmigaAI ==='
ls -la AmigaAI
echo ''
echo '=== Compiling FileType ==='
echo '  CC  src/filetype.c'
\$CC \$CFLAGS -c -o obj/filetype.o src/filetype.c
echo '=== Linking FileType ==='
\$CC -noixemul -o FileType obj/filetype.o obj/dt_identify.o
\$STRIP FileType
echo '=== Done: FileType ==='
ls -la FileType
"
else
    echo "=== Using native $CC ($($CC --version | head -1)) ==="
    cd "$PROJDIR"
    set -e
    mkdir -p obj

    echo "=== Compiling AmigaAI ==="
    for src in $SOURCES; do
        obj="obj/$(basename ${src%.c}.o)"
        echo "  CC  $src"
        $CC $CFLAGS -c -o "$obj" "$src"
    done

    echo "=== Linking ==="
    OBJS=""
    for src in $SOURCES; do
        OBJS="$OBJS obj/$(basename ${src%.c}.o)"
    done
    $CC $LDFLAGS -o AmigaAI $OBJS $LIBS
    $STRIP AmigaAI

    echo "=== Done: AmigaAI ==="
    ls -la AmigaAI

    # Build FileType standalone command
    echo ""
    echo "=== Compiling FileType ==="
    echo "  CC  src/filetype.c"
    $CC $CFLAGS -c -o obj/filetype.o src/filetype.c
    echo "=== Linking FileType ==="
    $CC -noixemul -o FileType obj/filetype.o obj/dt_identify.o
    $STRIP FileType
    echo "=== Done: FileType ==="
    ls -la FileType
fi
