/** \file usgUsdEngineVersionNumbers.h 
  Library version numbers.
  These are compile-time constants that source code can use in compiler directives.

  The most useful symbol defined is kFnUsdEngineVersionInteger which you
  can use in a statement like "#if kFnUsdEngineVersionInteger >= 12000"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _usgUsdEngineVersionNumber_h_
#define _usgUsdEngineVersionNumber_h_

#define kFnUsdEngineVersion "14.0."
#define kFnUsdEngineVersionInteger (10000 * 14 + 1000 * 0 + 6)
#define kFnUsdEngineVersionMajorNum 14
#define kFnUsdEngineVersionMinorNum 0
#define kFnUsdEngineVersionPatchNum 6

/* #undef kUsdAbstractionPythonEnabled */

#endif
