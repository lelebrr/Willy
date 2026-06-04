#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <Arduino.h>
#include <FS.h>

File createNewFile(FS *&fs, String filepath, String filename);

#endif // FILE_UTILS_H
