#ifndef _FS_H_
#define _FS_H_

#include <3ds.h>

#define FS_PATH_MAX_LENGTH        (0x106)
#define IO_BUF_SIZE               (0x800000) // 8 MB
#define FS_ERR_DOESNT_EXIST       (0xC8804478)
#define FS_ERR_DOES_ALREADY_EXIST (0xC82044BE)


extern FS_archive sdmcArchive;


typedef struct
{
	char name[FS_PATH_MAX_LENGTH + 1];
	u64 size;
} DirEntry;


inline Result addToPath(char *path, char *dirOrFile);
inline void removeFromPath(char *path);
void sdmcArchiveInit();
void sdmcArchiveExit();
Result getFilesFolders(char *path, FS_archive *archive, u32 *folderCount, u32 *fileCount, DirEntry *filesFolders);

#endif
