#ifndef __SD_FUNCTIONS_H__
#define __SD_FUNCTIONS_H__

#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>

struct FileList {
  String filename;
  bool folder;
  bool operation;
};

// extern SPIClass sdcardSPI;

bool setupSdCard();
String getHierarchicalPath(String baseDir);

void closeSdCard();

bool ToggleSDCard();

bool deleteFromSd(fs::FS &fs, String path);

bool renameFile(fs::FS &fs, String path, String filename);

bool copyFile(fs::FS &fs, String path);

bool copyToFs(fs::FS &from, fs::FS &to, String path, bool draw = true);

bool pasteFile(fs::FS &fs, String path);

bool createFolder(fs::FS &fs, String path);

String readLineFromFile(File myFile);

String readSmallFile(fs::FS &fs, String filepath);

char *readBigFile(fs::FS *fs, String filepath, bool binary = false,
                  size_t *fileSize = NULL);

String md5File(fs::FS &fs, String filepath);

String crc32File(fs::FS &fs, String filepath);

void readFs(fs::FS &fs, String folder, String allowed_ext = "*");

bool sortList(const FileList &a, const FileList &b);

String loopSD(fs::FS &fs, bool filePicker = false, String allowed_ext = "*",
              String rootPath = "/");

void viewFile(fs::FS &fs, String filepath);

bool checkLittleFsSize();

bool checkLittleFsSizeNM(); // Don't display msg

bool getFsStorage(fs::FS *&fs);

void fileInfo(fs::FS &fs, String filepath);

File createNewFile(fs::FS *&fs, String filepath, String filename);

#endif
