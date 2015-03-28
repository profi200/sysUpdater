#include <string.h>
#include <misc.h>
#include <fs.h>



// Simple qsort() compar function for file names
int fileNameCmp(const void* nameA, const void* nameB)
{
	return strcmp(((DirEntry*)nameA)->name, ((DirEntry*)nameB)->name);
}
