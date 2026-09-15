#ifndef MICRODEBUG_H
#define MICRODEBUG_H

// Minimal stand-in for jeremypoulter/MicroDebug so the native unit tests
// don't need to pull in the real dependency. ConfigJson only uses DBUGF.

#ifndef DBUGF
#define DBUGF(...) do { } while (0)
#endif

#endif
