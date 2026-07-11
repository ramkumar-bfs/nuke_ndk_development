/** \file usgUsdAbstractionVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFnUsdAbstractionVersionInteger which you
  can use in a statement like "#if kFnUsdAbstractionVersionInteger >= 14000"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _usgUsdAbstractionVersionNumber_h_
#define _usgUsdAbstractionVersionNumber_h_

#define kFnUsdAbstractionVersion "14.0."
#define kFnUsdAbstractionVersionInteger (10000 * 14 + 1000 * 0 + 5)
#define kFnUsdAbstractionVersionMajorNum 14
#define kFnUsdAbstractionVersionMinorNum 0
#define kFnUsdAbstractionVersionPatchNum 5

#endif
