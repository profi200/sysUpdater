#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <error.h>
#include <fs.h>
#include <title.h>



Result installCia(u8 mediatype, char *path)
{
	if(!path) return ERR_NULL_PTR;


	Handle fileHandle, ciaHandle;
	u32 i, bytesRead, bytesWritten, blockSize;
	u64 ciaSize, offset = 0;
	u8 *buffer = NULL;
	Result res;

	FS_path filePath = (FS_path){PATH_CHAR, strlen(path) + 1, (u8*)path};



	if((res = FSUSER_OpenFile(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE))) return res;
	if((res = FSFILE_GetSize(fileHandle, &ciaSize))) goto cleanup;
	if((res = AM_StartCiaInstall(mediatype, &ciaHandle))) goto cleanup;



	buffer = (u8*)malloc(IO_BUF_SIZE);
	if(!buffer)
	{
		res = ERR_NOT_ENOUGH_MEM;
		goto cleanup;
	}


	for(i=0; i<=ciaSize / IO_BUF_SIZE; i++)
	{
		blockSize = ((ciaSize - offset<IO_BUF_SIZE) ? ciaSize - offset : IO_BUF_SIZE);

		if(blockSize>0)
		{
			if((res = FSFILE_Read(fileHandle, &bytesRead, offset, buffer, blockSize))) break;
			if((res = FSFILE_Write(ciaHandle, &bytesWritten, offset, buffer, blockSize, FS_WRITE_FLUSH))) break;

			offset += blockSize;
		}
	}



	cleanup:

	if(buffer) free(buffer);
	FSFILE_Close(fileHandle);
	if(res) AM_CancelCIAInstall(&ciaHandle);
	else res = AM_FinishCiaInstall(mediatype, &ciaHandle);


	return res;
}
