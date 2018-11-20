#ifndef FILE_MGR_H
#define FILE_MGR_H

#include <string>
#include <fstream>

class FileManager
{
public:
  FileManager(const char *root);
  ~FileManager();

  /**
     * Tries to find file at the specified url under root.
     * If no file is found, exception is thrown
     */
  std::string GetFilename(std::string &url, uint64_t &size);
  std::string GetFilename(const char *filename, uint64_t &size)
  {
    std::string temp = filename;
    return GetFilename(temp, size);
  }

  std::string GetContentType( std::string filename );  

  inline static uint64_t GetFileSize(std::ifstream *infile);

  std::string &GetRoot()
  {
    return m_root;
  }

private:  
  std::string m_root;
};

#endif