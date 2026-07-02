/** \file ndkVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kNdkVersionInteger which you
  can use in a statement like "#if kNdkVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/

#ifndef _NdkVersionNumber_h_
#define _NdkVersionNumber_h_
#define kNdkVersion "15.2"
#define kNdkVersionInteger (10000 * 15 + 1000 * 2 + 9)
#define kNdkVersionMajorNum 15
#define kNdkVersionMinorNum 2
#define kNdkVersionReleaseNum 9
#define kNdkVersionReleaseStr "v9"
#define kNdkVersionPhaseNum 3
#define kNdkVersion64Bit true
#define kNdk_ABI_version Ndk_ABI_15_2_1
#ifndef kNdkBuildTargetOS
# define kNdkBuildTargetOS "linux"
#endif
#endif
