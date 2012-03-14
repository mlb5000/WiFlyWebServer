#ifndef FILE_SYSTEM_MOCK_H
#define FILE_SYSTEM_MOCK_H

#include "WiFlyWebServer.h"

class FileSystemMock : public FileSystem
{
public:
  FileSystemMock() : m_path(0), m_contents(0) {}
  virtual ~FileSystemMock() {}
  
  FRESULT myf_open(FIL* file, const XCHAR* path, TBYTE modeFlags) const;
  FRESULT myf_read(FIL* file, void* buffer, UINT bytesToRead, UINT*bytesRead) const;
  FRESULT myf_close (FIL* file) const;
  FRESULT myf_stat(const XCHAR* FileName, FILINFO* FileInfo) const;
  
  void addFakeFile(const XCHAR *path, const XCHAR *contents);
  
private:
  XCHAR *m_path;
  mutable XCHAR *m_contents;
  mutable FIL m_file;
};

void
FileSystemMock::addFakeFile(const XCHAR *path, const XCHAR *contents)
{
  m_path = (XCHAR*)path;
  m_contents = (XCHAR*)contents;
  m_file.fsize = strlen(contents);
  m_file.fs = (FATFS*)0x12345; //make this a "valid" FS pointer
}

FRESULT
FileSystemMock::myf_open(FIL* file, const XCHAR* path, TBYTE modeFlags) const
{
  if (0 != strcmp(path, m_path)) {
    return FR_DISK_ERR;
  }
  
  memcpy(file, &m_file, sizeof(FIL));
  return FR_OK;
}

FRESULT
FileSystemMock::myf_read(FIL* file, void* buffer, UINT bytesToRead, UINT*bytesRead) const
{
  int size = bytesToRead >= strlen((char*)m_contents) ? strlen((char*)m_contents) : bytesToRead;
  *bytesRead = size;
  
  if (size > 0) {
    strncpy((char*)buffer, (char*)m_contents, size);
    m_contents += size; 
  }
  
  return FR_OK;
}

FRESULT
FileSystemMock::myf_close(FIL* file) const
{
  return FR_OK;
}

FRESULT
FileSystemMock::myf_stat(const XCHAR* FileName, FILINFO* FileInfo) const
{
  if (0 != strcmp(FileName, m_path)) {
    return FR_DISK_ERR;
  }
  
  FileInfo->fsize = m_file.fsize;
  
  return FR_OK;
}

#endif