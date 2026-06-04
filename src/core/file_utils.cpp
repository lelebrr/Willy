#include "file_utils.h"

/*********************************************************************
**  Function: createNewFile
**  Function will save a file into FS. If file already exists it will
**  append a version number to the file name.
**********************************************************************/
File createNewFile(FS *&fs, String filepath, String filename) {
    int extIndex = filename.lastIndexOf('.');
    String name = filename.substring(0, extIndex);
    String ext = filename.substring(extIndex);

    if (filepath.endsWith("/")) filepath = filepath.substring(0, filepath.length() - 1);
    if (!(*fs).exists(filepath)) (*fs).mkdir(filepath);

    name = filepath + "/" + name;

    if ((*fs).exists(name + ext)) {
        int i = 1;
        name += "_";
        while ((*fs).exists(name + String(i) + ext)) i++;
        name += String(i);
    }

    Serial.println("Creating file: " + name + ext);
    File file = (*fs).open(name + ext, FILE_WRITE);
    return file;
}
