/*
 * FileType - Identify file types using the AmigaOS DataType system
 *
 * Standalone CLI command.
 * Usage: FileType <path> [FILTER <group>] [ALL] [NOPATH]
 *
 * Examples:
 *   FileType SYS:Utilities/MultiView
 *   FileType WORK:Images FILTER picture
 *   FileType DH0: FILTER picture ALL
 *   FileType WORK: FILTER picture ALL NOPATH
 */

#include "dt_identify.h"

#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>

/* Version string for AmigaOS VERSION command */
static const char *verstag = "\0$VER: FileType 1.0 (28.02.2026)";

/* ReadArgs template */
#define TEMPLATE "PATH/A,FILTER/K,ALL/S,NOPATH/S,MAXFILES/K/N"
enum { ARG_PATH, ARG_FILTER, ARG_ALL, ARG_NOPATH, ARG_MAXFILES, ARG_COUNT };

/* Callback: print one file entry */
static void print_entry(const char *path, const char *name,
                         const char *dt_name, const char *group,
                         void *userdata)
{
    int no_path = (int)(LONG)userdata;
    const char *display = no_path ? name : path;
    int has_space = (strchr(display, ' ') != NULL);

    if (has_space)
        printf("\"%-28s\" %-16s %s\n", display, dt_name, group);
    else
        printf("%-30s %-16s %s\n", display, dt_name, group);
}

int main(int argc, char *argv[])
{
    LONG args[ARG_COUNT] = { 0, 0, 0, 0, 0 };
    struct RDArgs *rda;
    const char *path;
    const char *filter;
    int recursive;
    int no_path;
    int maxfiles;
    int rc = 0;

    (void)argc;
    (void)argv;
    (void)verstag;

    rda = ReadArgs((CONST_STRPTR)TEMPLATE, args, NULL);
    if (!rda) {
        PrintFault(IoErr(), (CONST_STRPTR)"FileType");
        return 20;
    }

    path      = (const char *)args[ARG_PATH];
    filter    = args[ARG_FILTER] ? (const char *)args[ARG_FILTER] : NULL;
    recursive = args[ARG_ALL] ? 1 : 0;
    no_path   = args[ARG_NOPATH] ? 1 : 0;
    maxfiles  = args[ARG_MAXFILES] ? *(LONG *)args[ARG_MAXFILES] : 0;

    /* Initialize datatypes.library */
    if (dt_init() != 0) {
        printf("ERROR: Cannot open datatypes.library v39\n");
        FreeArgs(rda);
        return 20;
    }

    /* Check if path is a file or directory */
    {
        BPTR lock = Lock((CONST_STRPTR)path, ACCESS_READ);
        if (!lock) {
            printf("Cannot find: %s\n", path);
            rc = 10;
        } else {
            struct FileInfoBlock *fib;
            fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
            if (fib) {
                if (Examine(lock, fib)) {
                    if (fib->fib_DirEntryType > 0) {
                        /* Directory — scan it */
                        int count;
                        UnLock(lock);
                        lock = 0;

                        count = dt_scan_dir(path, filter, recursive,
                                            maxfiles,
                                            print_entry, (void *)(LONG)no_path);
                        if (count == -2) {
                            printf("***Break\n");
                            rc = 5;
                        } else if (count == 0)
                            printf("No matching files found.\n");
                        else if (count < 0) {
                            printf("Error scanning directory.\n");
                            rc = 10;
                        }
                    } else {
                        /* Single file */
                        char dt_name[64], dt_group[32];
                        UnLock(lock);
                        lock = 0;

                        if (dt_identify_file(path, dt_name, sizeof(dt_name),
                                             dt_group, sizeof(dt_group)) == 0)
                        {
                            /* Apply filter for single file too */
                            if (!filter || strcasecmp(filter, dt_group) == 0
                                       || strcasecmp(filter, dt_name) == 0) {
                                if (strchr(path, ' '))
                                    printf("\"%-28s\" %-16s %s\n",
                                           path, dt_name, dt_group);
                                else
                                    printf("%-30s %-16s %s\n",
                                           path, dt_name, dt_group);
                            }
                            else
                                printf("No match (type: %s %s)\n",
                                       dt_name, dt_group);
                        } else {
                            printf("Unknown file type: %s\n", path);
                            rc = 5;
                        }
                    }
                }
                FreeDosObject(DOS_FIB, fib);
            }
            if (lock) UnLock(lock);
        }
    }

    dt_cleanup();
    FreeArgs(rda);
    return rc;
}
