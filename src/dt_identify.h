#ifndef AMIGAAI_DT_IDENTIFY_H
#define AMIGAAI_DT_IDENTIFY_H

/* Callback for directory scanning.
 * path:    full AmigaDOS path to the file
 * name:    filename only
 * dt_name: DataType name (e.g. "ILBM", "ASCII")
 * group:   group name (e.g. "picture", "text")
 * userdata: user-provided context */
typedef void (*DtScanCallback)(const char *path, const char *name,
                                const char *dt_name, const char *group,
                                void *userdata);

/* Initialize datatypes.library. Returns 0 on success. */
int dt_init(void);

/* Close datatypes.library. */
void dt_cleanup(void);

/* Identify a single file's DataType.
 * name_out:  receives the DataType name (e.g. "ILBM")
 * group_out: receives the group name (e.g. "picture")
 * Returns 0 on success, -1 on error. */
int dt_identify_file(const char *path,
                     char *name_out, int name_size,
                     char *group_out, int group_size);

/* Scan a directory and call cb for each file.
 * filter:    NULL for all files, or a group name (e.g. "picture")
 * recursive: 1 to scan subdirectories (ALL)
 * maxfiles:  stop after this many matches (0 = unlimited)
 * Returns number of matching files, -1 on error, -2 on CTRL-C. */
int dt_scan_dir(const char *dir, const char *filter, int recursive,
                int maxfiles, DtScanCallback cb, void *userdata);

#endif /* AMIGAAI_DT_IDENTIFY_H */
