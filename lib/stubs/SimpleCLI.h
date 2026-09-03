#ifndef _SIMPLECLI_STUB_H_
#define _SIMPLECLI_STUB_H_

// Redirect to the real SimpleCLI library header.
// Path antigo hardcoded quebrava fresh builds e outros envs;
// usa o path baixado quando existir, senao a lib do LDF.
#if __has_include("../../.pio/libdeps/willy_release/SimpleCLI/src/SimpleCLI.h")
#include "../../.pio/libdeps/willy_release/SimpleCLI/src/SimpleCLI.h"
#else
#include <SimpleCLI.h>
#endif

#endif // _SIMPLECLI_STUB_H_
