#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "fatfs_config.h"

/** @brief virtual during unit testing, #undef in production to reduce code size */

/** @brief FileSystem class wrapping C APIs provided by FatFs */
class FileSystem
{
public:
  FileSystem() {}
  virtual ~FileSystem() {}
  
  /* apparently arduino doesn't support pure virtual, but make these empty 
     for now until we get the real FatFs implementation compiled in.  Then
     these will just delegate to the real C functions */
  virtual FRESULT myf_open(FIL* file, const XCHAR* path, TBYTE modeFlags) const
  {
    //eventually just call f_open(file, path, modeFlags);
    return FR_NOT_READY;
  }
  
  virtual FRESULT myf_read(FIL* file, void* buffer, UINT bytesToRead, UINT*bytesRead) const
  {
    //eventually just call f_read(file, buffer, bytesToRead, bytesRead);
    return FR_NOT_READY;
  }
  
  virtual FRESULT myf_close(FIL* file) const
  {
    //eventually just call f_close(file);
    return FR_NOT_READY;
  }
  
  virtual FRESULT myf_stat(const XCHAR* FileName, FILINFO* FileInfo) const
  {
    //eventually just call f_stat(FileName, FileInfo);
    return FR_NOT_READY;
  }
  
};

#endif