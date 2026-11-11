#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <Arduino.h>
#include <FS.h>

File createNewFile(FS *&fs, const String &filepath, const String &filename);

#endif // FILE_UTILS_H
