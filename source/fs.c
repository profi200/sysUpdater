#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <error.h>
#include <fs.h>
#include <misc.h>



FS_archive sdmcArchive;


inline Result addToPath(char *path, char *dirOrFile)
{
	u32 pathSize = strlen(path);


	if(pathSize + strlen(dirOrFile)>1024) return ERR_PATH_TOO_LONG;
	else
	{
		if(pathSize == 1) sprintf(&path[pathSize], "%s", dirOrFile);
		else sprintf(&path[pathSize], "/%s", dirOrFile);
	}

	return 0;
}


inline void removeFromPath(char *path)
{
	char *lastSlash = strrchr(path, (int)*"/");


	if(lastSlash>path) *lastSlash = 0;
	else *(lastSlash + 1) = 0;
}


void sdmcArchiveInit()
{
	sdmcArchive = (FS_archive){0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
}

void sdmcArchiveExit()
{
	FSUSER_CloseArchive(NULL, &sdmcArchive);
}


// Expects an array of DirEntry structs with 512 entries as the last param
Result getFilesFolders(char *path, FS_archive *archive, u32 *folderCount, u32 *fileCount, DirEntry *filesFolders)
{
	if(!path) return ERR_NULL_PTR;
	if(!archive) return ERR_NULL_PTR;
	if(!folderCount) return ERR_NULL_PTR;
	if(!fileCount) return ERR_NULL_PTR;
	if(!filesFolders) return ERR_NULL_PTR;


	Handle dirHandle;
	Result res;
	u32 entriesRead, i;

	*folderCount = 0;
	*fileCount = 0;
	FS_path dirPath = (FS_path){PATH_CHAR, strlen(path) + 1, (u8*)path};
	memset(filesFolders, 0, sizeof(DirEntry) * 512);



	if((res = FSUSER_OpenDirectory(NULL, &dirHandle, *archive, dirPath))) return res;


	FS_dirent *entries = (FS_dirent*)malloc(sizeof(FS_dirent) * 16);
	if(!entries)
	{
		res = ERR_NOT_ENOUGH_MEM;
		goto cleanup;
	}


	do
	{
		entriesRead = 0;
		memset(entries, 0, sizeof(FS_dirent) * 16);
		if((res = FSDIR_Read(dirHandle, &entriesRead, 16, entries))) break;

		for(i=0; i<entriesRead; i++)
		{
			if(entries[i].isDirectory)
			{
				utf16_to_utf8((u8*)filesFolders[(*folderCount)].name, entries[i].name, FS_PATH_MAX_LENGTH);
				(*folderCount)++;
			}
			else
			{
				utf16_to_utf8((u8*)filesFolders[(*fileCount) + 256].name, entries[i].name, FS_PATH_MAX_LENGTH);
				filesFolders[(*fileCount) + 256].size = entries[i].fileSize;
				//filesFolders[(*fileCount) + 256].rOnly = entries[i].isReadOnly;
				(*fileCount)++;
			}
		}
	} while((entriesRead == 16) && ((*folderCount)<256) && ((*fileCount)<256));


	// Not very nice but works
	if((*folderCount)<256) memcpy(&filesFolders[(*folderCount)], &filesFolders[256], (*fileCount) * sizeof(DirEntry));


	// Sort folders and files
	qsort(filesFolders, (*folderCount), sizeof(DirEntry), fileNameCmp);
	qsort(&filesFolders[(*folderCount)], (*fileCount), sizeof(DirEntry), fileNameCmp);



	// Most efficient way to do a cleanup to prevent handle leaks.
	cleanup:

	if(entries) free(entries);
	FSDIR_Close(dirHandle);

	return res;
}
