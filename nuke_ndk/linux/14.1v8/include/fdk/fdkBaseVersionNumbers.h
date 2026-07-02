/** \file fdkBaseVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kFdkVersionInteger which you
  can use in a statement like "#if kFdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _FdkVersionNumber_h_
#define _FdkVersionNumber_h_
#define kFdkVersion "0.2"
#define kFdkVersionInteger (10000 * 0 + 1000 * 2 + 1)
#define kFdkVersionMajorNum 0
#define kFdkVersionMinorNum 2
#define kFdkVersionReleaseNum 1
#define kFdkVersionReleaseStr "v1"
#define kFdkVersionPhaseNum 3
#define kFdkVersion64Bit true
#define kFdk_ABI_version Fdk_ABI_0_2_1
#ifndef kFdkBuildTargetOS
# define kFdkBuildTargetOS "linux"
#endif
#endif
