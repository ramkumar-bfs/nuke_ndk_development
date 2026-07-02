/** \file fdkBaseVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFdkVersionInteger which you
  can use in a statement like "#if kFdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _FdkVersionNumber_h_
#define _FdkVersionNumber_h_
#define kFdkVersion "0.5"
#define kFdkVersionInteger (10000 * 0 + 1000 * 5 + 3)
#define kFdkVersionMajorNum 0
#define kFdkVersionMinorNum 5
#define kFdkVersionReleaseNum 3
#define kFdkVersionReleaseStr "v3"
#define kFdkVersionPhaseNum 3
#define kFdkVersion64Bit true
#define kFdk_ABI_version Fdk_ABI_0_5_1
#ifndef kFdkBuildTargetOS
# define kFdkBuildTargetOS "linux"
#endif
#endif
