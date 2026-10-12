#ifndef StringCLI_h
#define StringCLI_h

#if defined(ARDUINO)
#include "Arduino.h" // String
#else
#include <cstring> // strlen()
#include <string>  // std::string
typedef std::string String;
inline String F(String s) { return s; };
#endif

#endif // ifndef StringCLI_h