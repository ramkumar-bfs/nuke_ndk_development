/** \file fdkBaseVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFdkVersionInteger which you
  can use in a statement like "#if kFdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _FdkVersionNumber_h_
#define _FdkVersionNumber_h_
#define kFdkVersion "14.0"
#define kFdkVersionInteger (10000 * 14 + 1000 * 0 + 6)
#define kFdkVersionMajorNum 14
#define kFdkVersionMinorNum 0
#define kFdkVersionReleaseNum 6
#define kFdkVersionReleaseStr "v6"
#define kFdkVersionPhaseNum 3
#define kFdkVersion64Bit true
#define kFdk_ABI_version Fdk_ABI_14_0_1
#ifndef kFdkBuildTargetOS
# define kFdkBuildTargetOS "linux"
#endif
#endif
