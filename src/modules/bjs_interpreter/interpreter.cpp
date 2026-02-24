#include "interpreter.h"
#include "core/utils.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
extern "C" {
#include "mquickjs.h"
}

void js_bruce_init(JSContext *ctx);
extern "C" JSSTDLibraryDef bruce_stdlib_def;

char *script = NULL;
char *scriptDirpath = NULL;
char *scriptName = NULL;

TaskHandle_t interpreterTaskHandler = NULL;
JSContext *ctx = NULL;
void *js_mem = NULL;

void closeJS() {
    if (ctx) {
        JS_FreeContext(ctx);
        ctx = NULL;
    }
    if (js_mem) {
        free(js_mem);
        js_mem = NULL;
    }
}

void interpreterHandler(void *pvParameters) {
    if (script == NULL) {
        interpreterTaskHandler = NULL; // Ensure cleared if exiting early
        vTaskDelete(NULL);
        return;
    }

    closeJS();

    size_t mem_size = psramFound() ? 256 * 1024 : 64 * 1024;
    js_mem = psramFound() ? ps_malloc(mem_size) : malloc(mem_size);
    if (!js_mem) {
        log_e("Failed to allocate JS memory");
        if (script) { free(script); script = NULL; }
        interpreterTaskHandler = NULL;
        vTaskDelete(NULL);
        return;
    }


    ctx = JS_NewContext(js_mem, mem_size, &bruce_stdlib_def);
    if (!ctx) {
        log_e("Failed to create JS context");
        free(js_mem);
        js_mem = NULL;
        if (script) { free(script); script = NULL; }
        interpreterTaskHandler = NULL;
        vTaskDelete(NULL);
        return;
    }

    js_bruce_init(ctx); // Register Bruce API

    JSValue val = JS_Eval(ctx, script, strlen(script), scriptName ? scriptName : "index.js", 0);

    if (JS_IsException(val)) {
        JSValue exc = JS_GetException(ctx);
        JSCStringBuf buf;
        const char *error = JS_ToCString(ctx, exc, &buf);
        log_e("JS Error: %s", error ? error : "Unknown");
        displayError("JS Error: " + String(error ? error : "Unknown"), true);
    }

    closeJS();
    if (script) { free(script); script = NULL; }
    if (scriptDirpath) { free(scriptDirpath); scriptDirpath = NULL; }
    if (scriptName) { free(scriptName); scriptName = NULL; }

    interpreterTaskHandler = NULL;
    vTaskDelete(NULL);
}

void startInterpreterTask() {
    if (interpreterTaskHandler != NULL) {
        log_w("Interpreter task already running");
        interpreter_state = 1;
        return;
    }

    xTaskCreateUniversal(
        interpreterHandler,          // Task function
        "interpreterHandler",        // Task Name
        INTERPRETER_TASK_STACK_SIZE, // Stack size
        NULL,                        // Task parameters
        2,                           // Task priority (0 to 3), loopTask has priority 2.
        &interpreterTaskHandler,     // Task handle
        ARDUINO_RUNNING_CORE         // run on core the same core as loop task
    );
}

void run_js_script() {
    String filename;
    FS *fs = &LittleFS;
    setupSdCard();
    if (sdcardMounted) {
        options = {
            {"SD Card",  [&]() { fs = &SD; }      },
            {"LittleFS", [&]() { fs = &LittleFS; }},
        };
        loopOptions(options);
    }
    filename = loopSD(*fs, true, "JS");
    script = readBigFile(fs, filename);
    if (script == NULL) { return; }

    returnToMenu = true;
    interpreter_state = 1;
    startInterpreterTask();
}

bool run_js_script_headless(char *code) {
    script = code;
    if (script == NULL) { return false; }
    scriptDirpath = strdup("/scripts");
    scriptName = strdup("index.js");

    returnToMenu = true;
    interpreter_state = 1;
    startInterpreterTask();
    return true;
}

bool run_js_script_headless(FS fs, String filename) {
    script = readBigFile(&fs, filename);
    if (script == NULL) { return false; }

    int slash = filename.lastIndexOf('/');
    scriptName = strdup(filename.c_str() + slash + 1);
    scriptDirpath = strndup(filename.c_str(), slash);
    returnToMenu = true;
    interpreter_state = 1;
    startInterpreterTask();
    return true;
}

String getScriptsFolder(FS *&fs) {
    String folder;
    String possibleFolders[] = {"/scripts", "/WillyJS", "/BruceScripts"};
    int listSize = sizeof(possibleFolders) / sizeof(possibleFolders[0]);

    for (int i = 0; i < listSize; i++) {
        if (SD.exists(possibleFolders[i])) {
            fs = &SD;
            return possibleFolders[i];
        }
        if (LittleFS.exists(possibleFolders[i])) {
            fs = &LittleFS;
            return possibleFolders[i];
        }
    }
    return "";
}

std::vector<Option> getScriptsOptionsList(String currentPath, bool saveStartupScript, int rememberedIndex) {
    std::vector<Option> opt = {};
#if !defined(LITE_VERSION) && !defined(DISABLE_INTERPRETER)
    FS *fs;
    String folder;

    if (currentPath == "") {
        folder = getScriptsFolder(fs);
        if (folder == "") return opt; // did not find
    } else {
        folder = currentPath;
        // Determine filesystem based on path
        if (currentPath.startsWith("/")) {
            fs = &LittleFS;
            if (SD.exists(currentPath)) { fs = &SD; }
        }
    }

    File root = fs->open(folder);
    if (!root || !root.isDirectory()) return opt; // not a dir

    while (true) {
        bool isDir;
        String fullPath = root.getNextFileName(&isDir);
        String nameOnly = fullPath.substring(fullPath.lastIndexOf("/") + 1);
        if (fullPath == "") { break; }
        // Serial.printf("Path: %s (isDir: %d)\n", fullPath.c_str(), isDir);

        if (isDir) {
            // Skip hidden folders (starting with .)
            if (nameOnly.startsWith(".")) continue;

            // Add folder option
            String folderTitle = "[ " + nameOnly + " ]";
            opt.push_back({folderTitle.c_str(), [=]() {
                               auto subOptions = getScriptsOptionsList(fullPath, saveStartupScript);
                               if (subOptions.size() > 0) {
                                   String displayPath = fullPath;
                                   int secondSlash = displayPath.indexOf('/', 1);
                                   if (secondSlash >= 0) {
                                       displayPath = displayPath.substring(secondSlash + 1);
                                   }
                                   loopOptions(subOptions, MENU_TYPE_SUBMENU, displayPath.c_str());
                               }
                           }});
        } else {
            // Handle files
            int dotIndex = nameOnly.lastIndexOf(".");
            String ext = dotIndex >= 0 ? nameOnly.substring(dotIndex + 1) : "";
            ext.toUpperCase();
            if (ext != "JS") continue;

            String entry_title = nameOnly.substring(0, nameOnly.lastIndexOf(".")); // remove the extension
            opt.push_back({entry_title.c_str(), [=]() {
                               if (saveStartupScript) {
                                   bruceConfig.startupAppLuaScript = fullPath; // keep config name or rename in config.h later
                                   bruceConfig.saveFile();
                               } else {
                                   Serial.printf("Running script: %s\n", fullPath.c_str());
                                   run_js_script_headless(*fs, fullPath);
                               }
                           }});
        }
    }

    root.close();

    // Sort options
    auto sortStart = opt.begin();
    std::sort(sortStart, opt.end(), [](const Option &a, const Option &b) {
        // Check if items start with '[' (folders)
        bool aIsFolder = a.label[0] == '[';
        bool bIsFolder = b.label[0] == '[';

        // If one is a folder and the other isn't, folder comes first
        if (aIsFolder != bIsFolder) {
            return aIsFolder; // true if a is folder, false if b is folder
        }

        // If both are the same type, sort alphabetically
        return strcasecmp(a.label.c_str(), b.label.c_str()) < 0;
    });

    // Add back navigation if we're in a subdirectory
    if (currentPath != "" && currentPath != getScriptsFolder(fs)) {
        opt.push_back(
            {"< Back", [=]() {
                 // Calculate parent directory
                 String parentPath = currentPath;
                 int lastSlash = parentPath.lastIndexOf('/');
                 if (lastSlash > 0) {
                     parentPath = parentPath.substring(0, lastSlash);
                 } else {
                     // If we can't go up, go to scripts root
                     FS *parentFs;
                     parentPath = getScriptsFolder(parentFs);
                 }

                 auto parentOptions = getScriptsOptionsList(parentPath, saveStartupScript);
                 if (parentOptions.size() > 0) {
                     String displayPath = parentPath;
                     int secondSlash = displayPath.indexOf('/', 1);
                     if (secondSlash >= 0) { displayPath = displayPath.substring(secondSlash + 1); }

                     // Find the folder we just came from to restore selection
                     int restoreIndex = 0;
                     String currentFolderName = currentPath.substring(currentPath.lastIndexOf('/') + 1);
                     String searchTitle = "[ " + currentFolderName + " ]";

                     for (int i = 0; i < parentOptions.size(); i++) {
                         if (parentOptions[i].label == searchTitle) {
                             restoreIndex = i;
                             break;
                         }
                     }

                     loopOptions(parentOptions, MENU_TYPE_SUBMENU, displayPath.c_str(), restoreIndex);
                 }
             }}
        );
    }

#endif
    return opt;
}

#endif
