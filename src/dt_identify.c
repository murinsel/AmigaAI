/*
 * dt_identify.c - DataType identification for AmigaAI
 *
 * Shared logic for identifying file types via the AmigaOS
 * DataType system. Used by both the identify_file tool
 * and the standalone FileType command.
 */

#include "dt_identify.h"

#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>
#include <datatypes/datatypes.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/datatypes.h>

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG)(a)<<24|(ULONG)(b)<<16|(ULONG)(c)<<8|(ULONG)(d))
#endif

/* datatypes.library base — shared between tool and standalone */
struct Library *DataTypesBase = NULL;

/* Convert a GroupID to a human-readable name */
static const char *gid_to_name(ULONG gid)
{
    switch (gid) {
    case GID_SYSTEM:     return "system";
    case GID_TEXT:       return "text";
    case GID_DOCUMENT:   return "document";
    case GID_SOUND:      return "sound";
    case GID_INSTRUMENT: return "instrument";
    case GID_MUSIC:      return "music";
    case GID_PICTURE:    return "picture";
    case GID_ANIMATION:  return "animation";
    case GID_MOVIE:      return "movie";
    default:             return "unknown";
    }
}

int dt_init(void)
{
    if (DataTypesBase) return 0;  /* already open */

    DataTypesBase = OpenLibrary((CONST_STRPTR)"datatypes.library", 39);
    if (!DataTypesBase) {
        return -1;
    }
    return 0;
}

void dt_cleanup(void)
{
    if (DataTypesBase) {
        CloseLibrary(DataTypesBase);
        DataTypesBase = NULL;
    }
}

int dt_identify_file(const char *path,
                     char *name_out, int name_size,
                     char *group_out, int group_size)
{
    BPTR lock;
    struct DataType *dt;

    if (!DataTypesBase) return -1;

    lock = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (!lock) return -1;

    dt = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL);
    if (!dt) {
        UnLock(lock);
        return -1;
    }

    if (name_out && name_size > 0) {
        strncpy(name_out, (const char *)dt->dtn_Header->dth_Name,
                name_size - 1);
        name_out[name_size - 1] = '\0';
    }

    if (group_out && group_size > 0) {
        const char *gname = gid_to_name(dt->dtn_Header->dth_GroupID);
        strncpy(group_out, gname, group_size - 1);
        group_out[group_size - 1] = '\0';
    }

    ReleaseDataType(dt);
    UnLock(lock);
    return 0;
}

/* Case-insensitive string compare (AmigaDOS style) */
static int stricmp_local(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return *a - *b;
}

/* Internal recursive scanner */
static int scan_dir_internal(const char *dir, const char *filter,
                             int recursive, int maxfiles,
                             DtScanCallback cb,
                             void *userdata, int depth)
{
    BPTR lock;
    struct FileInfoBlock *fib;
    int count = 0;
    int dirlen;

    /* Prevent infinite recursion */
    if (depth > 20) return 0;

    lock = Lock((CONST_STRPTR)dir, ACCESS_READ);
    if (!lock) return -1;

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        UnLock(lock);
        return -1;
    }

    if (!Examine(lock, fib)) {
        FreeDosObject(DOS_FIB, fib);
        UnLock(lock);
        return -1;
    }

    /* Check that it's actually a directory */
    if (fib->fib_DirEntryType < 0) {
        FreeDosObject(DOS_FIB, fib);
        UnLock(lock);
        return -1;  /* Not a directory */
    }

    dirlen = strlen(dir);

    while (ExNext(lock, fib)) {
        char fullpath[512];
        const char *sep = "";

        /* Check for CTRL-C break */
        if (CheckSignal(SIGBREAKF_CTRL_C)) {
            count = -2;  /* Signal abort */
            break;
        }

        /* Build full path */
        if (dirlen > 0 && dir[dirlen - 1] != ':' && dir[dirlen - 1] != '/')
            sep = "/";
        snprintf(fullpath, sizeof(fullpath), "%s%s%s",
                 dir, sep, (const char *)fib->fib_FileName);

        if (fib->fib_DirEntryType > 0) {
            /* Directory entry */
            if (recursive) {
                int remaining = (maxfiles > 0) ? (maxfiles - count) : 0;
                int sub = scan_dir_internal(fullpath, filter, recursive,
                                            remaining, cb, userdata,
                                            depth + 1);
                if (sub == -2) { count = -2; break; }
                if (sub > 0) count += sub;
                if (maxfiles > 0 && count >= maxfiles) break;
            }
        } else {
            /* File entry — identify its DataType */
            char dt_name[64], dt_group[32];

            if (dt_identify_file(fullpath, dt_name, sizeof(dt_name),
                                 dt_group, sizeof(dt_group)) == 0)
            {
                /* Apply filter: match group name or DataType name */
                if (!filter || stricmp_local(filter, dt_group) == 0
                            || stricmp_local(filter, dt_name) == 0) {
                    if (cb)
                        cb(fullpath, (const char *)fib->fib_FileName,
                           dt_name, dt_group, userdata);
                    count++;
                    if (maxfiles > 0 && count >= maxfiles) break;
                }
            }
        }
    }

    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);
    return count;
}

int dt_scan_dir(const char *dir, const char *filter, int recursive,
                int maxfiles, DtScanCallback cb, void *userdata)
{
    return scan_dir_internal(dir, filter, recursive, maxfiles,
                             cb, userdata, 0);
}
