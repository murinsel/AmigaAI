/*
 * png_convert.c - Convert any DataTypes picture to PNG
 *
 * Uses AmigaOS DataTypes to load images (ILBM, BMP, PCX, TIFF, etc.)
 * and writes a PNG file with indexed color (palette).
 *
 * The PNG encoder uses stored (uncompressed) deflate blocks,
 * so no zlib dependency is needed.  The output is larger than
 * compressed PNG but perfectly valid.
 */

#include "png_convert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/datatypes.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

extern struct Library *DataTypesBase;  /* from dt_identify.c */

static struct GfxBase *GfxBase_png = NULL;

/* ===================== CRC-32 ===================== */

static ULONG crc_table[256];
static int   crc_table_ready = 0;

static void crc32_init(void)
{
    ULONG c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (ULONG)n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xEDB88320UL ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_ready = 1;
}

static ULONG crc32_update(ULONG crc, const unsigned char *buf, ULONG len)
{
    ULONG i;
    if (!crc_table_ready) crc32_init();
    crc = crc ^ 0xFFFFFFFFUL;
    for (i = 0; i < len; i++)
        crc = crc_table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFUL;
}

/* ===================== Adler-32 ===================== */

#define ADLER_BASE 65521

static ULONG adler32(const unsigned char *buf, ULONG len)
{
    ULONG a = 1, b = 0, i;
    for (i = 0; i < len; i++) {
        a = (a + buf[i]) % ADLER_BASE;
        b = (b + a) % ADLER_BASE;
    }
    return (b << 16) | a;
}

/* ===================== Big-endian helpers ===================== */

static void put_be32(unsigned char *p, ULONG v)
{
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >>  8) & 0xFF;
    p[3] =  v        & 0xFF;
}

static void put_le16(unsigned char *p, unsigned int v)
{
    p[0] =  v       & 0xFF;
    p[1] = (v >> 8) & 0xFF;
}

/* ===================== PNG chunk writer ===================== */

/* Write a PNG chunk: 4-byte length, 4-byte type, data, 4-byte CRC */
static int write_chunk(FILE *f, const char type[4],
                       const unsigned char *data, ULONG length)
{
    unsigned char hdr[4];
    ULONG crc;

    put_be32(hdr, length);
    if (fwrite(hdr, 1, 4, f) != 4) return -1;
    if (fwrite(type, 1, 4, f) != 4) return -1;

    crc = crc32_update(0, (const unsigned char *)type, 4);

    if (length > 0 && data) {
        if (fwrite(data, 1, length, f) != length) return -1;
        crc = crc32_update(crc, data, length);
    }

    put_be32(hdr, crc);
    if (fwrite(hdr, 1, 4, f) != 4) return -1;

    return 0;
}

/* ===================== zlib stored wrapper ===================== */

/*
 * Wrap raw data in a zlib stream using stored (uncompressed) deflate.
 * Format: 2-byte zlib header + stored blocks + 4-byte Adler-32.
 * Each stored block: 1 byte flags, 2 bytes LEN, 2 bytes ~LEN, data.
 * Max block size 65535 bytes.
 */
static unsigned char *zlib_store(const unsigned char *raw, ULONG raw_len,
                                 ULONG *out_len)
{
    ULONG nblocks, total, pos, src_pos;
    unsigned char *out;

    nblocks = (raw_len + 65534) / 65535;
    if (nblocks == 0) nblocks = 1;  /* at least one (empty) block */

    /* 2 header + nblocks * 5 overhead + raw_len + 4 adler */
    total = 2 + nblocks * 5 + raw_len + 4;

    out = malloc(total);
    if (!out) return NULL;

    /* zlib header: CMF=0x78 (deflate, 32K window), FLG=0x01 */
    out[0] = 0x78;
    out[1] = 0x01;
    pos = 2;

    src_pos = 0;
    while (src_pos < raw_len || src_pos == 0) {
        ULONG remain = raw_len - src_pos;
        unsigned int blen = remain > 65535 ? 65535 : (unsigned int)remain;
        int is_final = (src_pos + blen >= raw_len) ? 1 : 0;

        out[pos++] = is_final ? 0x01 : 0x00;
        put_le16(out + pos, blen); pos += 2;
        put_le16(out + pos, ~blen & 0xFFFF); pos += 2;

        if (blen > 0) {
            memcpy(out + pos, raw + src_pos, blen);
            pos += blen;
        }
        src_pos += blen;

        if (blen == 0) break;  /* empty final block for 0-length input */
    }

    /* Adler-32 of original uncompressed data */
    {
        ULONG a32 = adler32(raw, raw_len);
        put_be32(out + pos, a32);
        pos += 4;
    }

    *out_len = pos;
    return out;
}

/* ===================== PNG conversion ===================== */

/* Recompute CRC used above — keep separate from crc32_update for clarity */

int png_convert_file(const char *input_path, const char *output_path)
{
    Object *dto = NULL;
    struct BitMapHeader *bmhd = NULL;
    struct BitMap *bm = NULL;
    struct ColorRegister *cregs = NULL;
    ULONG ncols = 0;
    UWORD width, height;
    UBYTE depth;
    UBYTE *pen_data = NULL;
    ULONG row_bytes, raw_len;
    unsigned char *raw_data = NULL;
    unsigned char *zdata = NULL;
    ULONG zlen;
    FILE *f = NULL;
    int result = -1;
    struct RastPort src_rp, tmp_rp;
    struct BitMap tmp_bm;
    UWORD alloc_width;
    int p;

    static const unsigned char png_sig[8] = {
        0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A
    };

    if (!DataTypesBase) return -1;

    /* Open graphics.library if needed */
    if (!GfxBase_png) {
        GfxBase_png = (struct GfxBase *)OpenLibrary(
            (CONST_STRPTR)"graphics.library", 39);
        if (!GfxBase_png) return -1;
    }
    /* Assign to the linker symbol GCC expects */
    #define GfxBase GfxBase_png

    /* Load picture via DataTypes */
    {
        struct TagItem attrs[] = {
            {DTA_SourceType,  DTST_FILE},
            {DTA_GroupID,     GID_PICTURE},
            {PDTA_Remap,     FALSE},
            {TAG_DONE,       0}
        };
        dto = NewDTObjectA((APTR)input_path, attrs);
        if (!dto) goto cleanup;
    }

    /* Trigger layout (decode image) */
    {
        struct gpLayout gpl;
        memset(&gpl, 0, sizeof(gpl));
        gpl.MethodID = DTM_PROCLAYOUT;
        gpl.gpl_GInfo = NULL;
        gpl.gpl_Initial = 1;
        DoMethodA(dto, (Msg)&gpl);
    }

    /* Get image attributes */
    GetDTAttrs(dto,
        PDTA_BitMapHeader,   (ULONG)&bmhd,
        PDTA_BitMap,         (ULONG)&bm,
        PDTA_ColorRegisters, (ULONG)&cregs,
        PDTA_NumColors,      (ULONG)&ncols,
        TAG_DONE);

    if (!bmhd || !bm) goto cleanup;

    width  = bmhd->bmh_Width;
    height = bmhd->bmh_Height;
    depth  = bmhd->bmh_Depth;

    if (width == 0 || height == 0) goto cleanup;
    if (depth > 8) goto cleanup;  /* HAM/24-bit not supported */

    /* Cap palette to 2^depth entries */
    {
        ULONG max_cols = 1UL << depth;
        if (ncols > max_cols) ncols = max_cols;
        if (ncols == 0) ncols = max_cols;
    }

    /* Allocate pen data buffer */
    row_bytes = (ULONG)width;
    pen_data = malloc(row_bytes * height);
    if (!pen_data) goto cleanup;

    /* Set up temp RastPort for ReadPixelArray8 */
    alloc_width = (width + 15) & ~15;
    InitBitMap(&tmp_bm, depth, alloc_width, 1);
    memset(tmp_bm.Planes, 0, sizeof(tmp_bm.Planes));

    for (p = 0; p < depth; p++) {
        tmp_bm.Planes[p] = AllocRaster(alloc_width, 1);
        if (!tmp_bm.Planes[p]) goto cleanup;
    }

    InitRastPort(&src_rp);
    src_rp.BitMap = bm;
    InitRastPort(&tmp_rp);
    tmp_rp.BitMap = &tmp_bm;

    ReadPixelArray8(&src_rp, 0, 0, width - 1, height - 1,
                    pen_data, &tmp_rp);

    /* Build raw PNG image data: filter byte (0) + row pixels per row */
    raw_len = (ULONG)height * (1 + (ULONG)width);
    raw_data = malloc(raw_len);
    if (!raw_data) goto cleanup;

    {
        ULONG y;
        unsigned char *dst = raw_data;
        UBYTE *src = pen_data;
        for (y = 0; y < height; y++) {
            *dst++ = 0x00;  /* filter: None */
            memcpy(dst, src, width);
            dst += width;
            src += width;
        }
    }

    /* Compress with stored deflate (no actual compression) */
    zdata = zlib_store(raw_data, raw_len, &zlen);
    if (!zdata) goto cleanup;

    /* Write PNG file */
    f = fopen(output_path, "wb");
    if (!f) goto cleanup;

    /* PNG signature */
    if (fwrite(png_sig, 1, 8, f) != 8) goto cleanup;

    /* IHDR chunk */
    {
        unsigned char ihdr[13];
        put_be32(ihdr + 0, (ULONG)width);
        put_be32(ihdr + 4, (ULONG)height);
        ihdr[8]  = 8;  /* bit depth */
        ihdr[9]  = 3;  /* color type: indexed */
        ihdr[10] = 0;  /* compression method */
        ihdr[11] = 0;  /* filter method */
        ihdr[12] = 0;  /* interlace method */
        if (write_chunk(f, "IHDR", ihdr, 13) != 0) goto cleanup;
    }

    /* PLTE chunk */
    {
        unsigned char *plte;
        ULONG plte_len = ncols * 3;

        plte = malloc(plte_len);
        if (!plte) goto cleanup;

        if (cregs) {
            ULONG i;
            for (i = 0; i < ncols; i++) {
                plte[i * 3 + 0] = cregs[i].red;
                plte[i * 3 + 1] = cregs[i].green;
                plte[i * 3 + 2] = cregs[i].blue;
            }
        } else {
            /* Fallback: greyscale palette */
            ULONG i;
            for (i = 0; i < ncols; i++) {
                UBYTE v = (UBYTE)((i * 255) / (ncols - 1));
                plte[i * 3 + 0] = v;
                plte[i * 3 + 1] = v;
                plte[i * 3 + 2] = v;
            }
        }

        if (write_chunk(f, "PLTE", plte, plte_len) != 0) {
            free(plte);
            goto cleanup;
        }
        free(plte);
    }

    /* IDAT chunk */
    if (write_chunk(f, "IDAT", zdata, zlen) != 0) goto cleanup;

    /* IEND chunk */
    if (write_chunk(f, "IEND", NULL, 0) != 0) goto cleanup;

    result = 0;  /* success */

cleanup:
    if (f) fclose(f);
    if (result != 0 && output_path)
        DeleteFile((CONST_STRPTR)output_path);

    free(zdata);
    free(raw_data);
    free(pen_data);

    /* Free temp bitmap planes */
    for (p = 0; p < 8; p++) {
        if (tmp_bm.Planes[p])
            FreeRaster(tmp_bm.Planes[p], alloc_width, 1);
    }

    if (dto) DisposeDTObject(dto);

    #undef GfxBase
    return result;
}

void png_convert_cleanup(void)
{
    if (GfxBase_png) {
        CloseLibrary((struct Library *)GfxBase_png);
        GfxBase_png = NULL;
    }
}
