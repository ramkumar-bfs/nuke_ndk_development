/** \file ddImageVersionNumbers.h 
  Version numbers. These are compile-time constants that plugin
  source code can use in compiler directives.

  The most useful symbol defined is kDDImageVersionInteger which you
  can use in a statement like "#if kDDImageVersionInteger >= 52100"
  to see if the version contains a given feature or bug fix.
*/
#ifndef _DDImageVersionNumber_h_
#define _DDImageVersionNumber_h_
#define kDDImageVersion "13.2"
#define kDDImageVersionInteger (10000 * 13 + 1000 * 2 + 7)
#define kDDImageVersionMajorNum 13
#define kDDImageVersionMinorNum 2
#define kDDImageVersionReleaseNum 7
#define kDDImageVersionReleaseStr "v7"
#define kDDImageVersionPhase DD::Image::Version::eRelease
#define kDDImageVersionPhaseNum 3
#define kDDImageVersion64Bit true
#define kDDImage_ABI_version DDImage_ABI_13_2_1
#ifndef kDDImageBuildTargetOS
# define kDDImageBuildTargetOS "linux"
#endif
#endif // _DDImageVersionNumber_h_
