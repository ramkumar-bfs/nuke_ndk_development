// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __Build_fnWindows_h__
#define __Build_fnWindows_h__

#include <windows.h>

#ifndef NTDDI_VERSION
  #define NTDDI_VERSION NTDDI_WIN7
  #define _WIN32_WINNT 0x0502
#endif

#ifdef _MSC_VER
  #define FN_COMPILER_VC

  #if defined(_MSC_VER) && (_MSC_VER < 1300)
    #error VC6 NOT SUPPORTED
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 1300) && (_MSC_VER < 1400)
    #define FN_COMPILER_VC_7
  #endif

  #if defined(_MSC_VER) && (_MSC_VER == 1300)
    #define FN_COMPILER_VC_7_0
    #define FN_COMPILER_VERSION 700
  #endif

  #if defined(_MSC_VER) && (_MSC_VER == 1310)
    #define FN_COMPILER_VC_7_1
    #define FN_COMPILER_VERSION 701
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #define FN_COMPILER_VC_8
  #endif

  #if defined(_MSC_VER) && (_MSC_VER == 1400)
    #define FN_COMPILER_VC_8_0
    #define FN_COMPILER_VERSION 800
  #endif

#endif

#ifdef FN_COMPILER_VC_7
  #include "Compiler/fnVC7.h"
#endif

#ifdef FN_COMPILER_VC_8
  #include "Build/Compiler/fnVC8.h"
#endif

#ifdef __GNUC__
  #include "Build/Compiler/fnGPlusPlus.h"
  #define FN_COMPILER_GNU
#endif

#ifndef  FN_PROCESSOR_INTEL
  #define FN_PROCESSOR_INTEL
#endif

#ifndef mFnThrowSpec
  #define mFnThrowSpec(...)
#endif

#ifndef FN_STATIC
  #if defined(FN_SHARED)
    #define FoundryAPI
  #else
    #define FoundryAPI
  #endif
#else
  #define FoundryAPI
#endif

/* Function to retrieve the c:\Program Files directory
*/
#ifdef __cplusplus
  #include <regstr.h>
  #include <string>
  #include <shlobj.h>
  #include <VersionHelpers.h>


  #ifndef FN_BUILD_WINDOWS_VERSION_FUNCS_DECLARED
  #define FN_BUILD_WINDOWS_VERSION_FUNCS_DECLARED
  namespace Foundry {

  namespace Windows {

  //Append new versions of Windows at end
  //so we can do checks such as eFnWinVer >= eWin2K
    enum eFnWinVer {
      eFnWinUnkown = 0,
      eFnWin95,
      eFnWinNT,
      eFnWinSE,
      eFnWinME,
      eFnWin2K,
      eFnWinSrv2K8,
      eFnWinXP,
      eFnWinVista,
      eFnWin7,
      eFnWinSrv2K8R2,
      eFnWin8,
      eFnWinSrv2K12,
      eFnWin81,
      eFnWinSrv2K12R2,
      eFnWin10,
      eFnWinSrv2K16
    };


    inline eFnWinVer FnGetWinVersion(bool ignoreServerEditions = false)
    {
      if(IsWindowsVersionOrGreater(10, 0, 0)) {return eFnWin10; }
      OSVERSIONINFO osvi;
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      GetVersionEx (&osvi);
      //RtlGetVersion(&osvi);

      auto checkWinVer = [&osvi](int major, int minor) {
        return osvi.dwMajorVersion == major && osvi.dwMinorVersion == minor;
      };

      if(IsWindowsServer() && !ignoreServerEditions)
      {
        if(checkWinVer(10, 0)) return eFnWinSrv2K16;
        if(checkWinVer(6, 3)) return eFnWinSrv2K12R2;
        if(checkWinVer(6, 2)) return eFnWinSrv2K12;
        if(checkWinVer(6, 1)) return eFnWinSrv2K8R2;
        if(checkWinVer(6, 0)) return eFnWinSrv2K8;
      }
      else
      {
        if(checkWinVer(10, 0)) return eFnWin10;
        if(checkWinVer(6, 3)) return eFnWin81;
        if(checkWinVer(6, 2)) return eFnWin8;
        if(checkWinVer(6, 1)) return eFnWin7;
        if(checkWinVer(6, 0)) return eFnWinVista;
        if((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)) return eFnWinXP;
        if(checkWinVer(5, 0)){ return eFnWin2K; }
      }

      return eFnWinUnkown;
    }

    inline std::string winVersionToString(eFnWinVer ver)
    {
      switch(ver)
      {
        case eFnWinUnkown: return "Windows unknown";
        case eFnWin95: return "Windows 95";
        case eFnWinNT: return "Windows NT";
        case eFnWinSE: return "Windows SE";
        case eFnWinME: return "Windows ME";
        case eFnWin2K: return "Windows 2000";
        case eFnWinSrv2K8: return "Windows Server 2008";
        case eFnWinXP: return "Windows XP";
        case eFnWinVista: return "Windows Vista";
        case eFnWin7: return "Windows 7";
        case eFnWinSrv2K8R2: return "Windows Server 2008 R2";
        case eFnWin8: return "Windows 8";
        case eFnWinSrv2K12: return "Windows Server 2012";
        case eFnWin81: return "Windows 8.1";
        case eFnWinSrv2K12R2: return "Windows Server 2012 R2";
        case eFnWin10: return "Windows 10";
        case eFnWinSrv2K16: return "Windows Server 2016";
      }
    }

    inline bool IsWindowsVista(void)
    {
      return (FnGetWinVersion() == eFnWinVista);
    }

    inline bool IsWindowsXP(void)
    {
      return (FnGetWinVersion() == eFnWinXP);
    }

    inline bool Is64Bit(void)
    {
      //GetNativeSystemInfo does not exist on Win2K so we have to do this nasty little hack
      //Also, GetSystemInfo returns the incorrect value for a 32 bit app on a 64 bit OS
      SYSTEM_INFO siSysInfo;
      ZeroMemory(&siSysInfo, sizeof(SYSTEM_INFO));
      typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
      PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
      if(NULL != pGNSI)
        pGNSI(&siSysInfo);
      else
        GetSystemInfo(&siSysInfo);
      if(siSysInfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64) { return true; }
      return false;
    }
    
  #ifndef UNICODE
    typedef std::string FnBuildString;
    #define FnLiteralBuildString(_arg) _arg
  #else
    typedef std::wstring FnBuildString;
    #define FnLiteralBuildString(_arg) L##_arg
  #endif

    //WARNING! If run on a 32 bit OS this returns and empty string
    inline FnBuildString GetProgramFiles64(void)
    {
      static TCHAR programFilesPath[MAX_PATH];
      static bool programFilesPathValid = false;
        
      if(programFilesPathValid)
      {
        return FnBuildString(programFilesPath);
      }
        
      //If we are on a 32 bit OS return an empty string
      if(!Is64Bit())
      {
        return FnBuildString(FnLiteralBuildString(""));
      }
    
      #ifdef WIN64 //And we are 64 bit too
      //If we are 64 bit and OS is 64 bit, use std mechanism
      HRESULT st = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
      SHGFP_TYPE_CURRENT, programFilesPath);
      if(st == S_OK)
      {
        programFilesPathValid = true;
        return FnBuildString(programFilesPath);
      }
      else { return FnBuildString(FnLiteralBuildString("C:\\Program Files\\")); }
      #else //We are 32 bit
      //If we are 32 bit and OS is 64 bit, we have to hack it to get the correct path
      //the only msdn mechanisms to get it are for Vista, thanks Micorsoft!
      HRESULT st = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
      SHGFP_TYPE_CURRENT, programFilesPath);
      if(st == S_OK)
      {
        FnBuildString x86Part(" (x86)");
        FnBuildString pfp(programFilesPath);
            
        size_t start = pfp.rfind(x86Part);
        if(start!=FnBuildString::npos){
            pfp.erase(start,x86Part.size());
        }
        programFilesPathValid = true;
        sprintf_s(programFilesPath, MAX_PATH, "%s",pfp.c_str());
        return FnBuildString(programFilesPath);
      }
      else { return FnBuildString(FnLiteralBuildString("C:\\Program Files\\")); }
      #endif
    }
    
    inline FnBuildString GetProgramFiles32(void)
    {
      static TCHAR programFilesPath[MAX_PATH];
      static bool programFilesPathValid = false;
        
      if(programFilesPathValid)
      {
        return FnBuildString(programFilesPath);
      }
        
      //If we are 64 bit and OS is 64 bit, use std mechanism
      HRESULT st = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL,
      SHGFP_TYPE_CURRENT, programFilesPath);
      if(st == S_OK)
      {
        programFilesPathValid = true;
        return FnBuildString(programFilesPath);
      }
      else { return FnBuildString(FnLiteralBuildString("C:\\Program Files (x86)\\")); }
    }
    
    inline FnBuildString GetProgramFiles(void)
    {
      static TCHAR programFilesPath[MAX_PATH];
      static bool programFilesPathValid = false;
        
      if(!programFilesPathValid)
      {
        HRESULT st = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, programFilesPath);
        if(st == S_OK)
        {
          programFilesPathValid = true;
          return FnBuildString(programFilesPath);
        }
        else
        {
          if(IsWindowsVista())
          {
            return FnBuildString(FnLiteralBuildString("C:\\Program Files (x86)\\"));
          }
          else{
            return FnBuildString(FnLiteralBuildString("C:\\Program Files\\"));
          }
        }
      }
      return FnBuildString(programFilesPath);
      /*******
      char pfDir[PATH_MAX];
      HKEY hKey;
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
      {
      DWORD m = _MAX_PATH;
      if (RegQueryValueEx(hKey, TEXT("ProgramFilesDir"), NULL, NULL, (LPBYTE)pfDir, &m) != ERROR_SUCCESS) {
      *pfDir = '\0';
      }
      RegCloseKey(hKey);
      return std::string(pfDir);
      }
      return "";
      *****/
    }
    
    inline FnBuildString GetProgramData(void)
    {
      static TCHAR programDataPath[MAX_PATH];
      static bool programDataPathValid = false;
        
      if(!programDataPathValid)
      {
        HRESULT st = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, programDataPath);
        if(st == S_OK)
        {
          programDataPathValid = true;
          return FnBuildString(programDataPath);
        }
        else
        {
          return FnBuildString(FnLiteralBuildString("C:\\ProgramData\\"));
        }
      }
      return FnBuildString(programDataPath);
    }
    
    
  } // Windows

  } // Foundry 
  #endif

#endif // __cplusplus

#endif
