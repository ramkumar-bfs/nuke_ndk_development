/** \file ndkVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kNdkVersionInteger which you
  can use in a statement like "#if kNdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _NdkVersionNumber_h_
#define _NdkVersionNumber_h_
#define kNdkVersion "14.0"
#define kNdkVersionInteger (10000 * 14 + 1000 * 0 + 6)
#define kNdkVersionMajorNum 14
#define kNdkVersionMinorNum 0
#define kNdkVersionReleaseNum 6
#define kNdkVersionReleaseStr "v6"
#define kNdkVersionPhaseNum 3
#define kNdkVersion64Bit true
#define kNdk_ABI_version Ndk_ABI_14_0_1
#ifndef kNdkBuildTargetOS
# define kNdkBuildTargetOS "linux"
#endif
#endif
