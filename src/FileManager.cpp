/**
FileManager.cpp
*/
#include "FileManager.h"
#include <sstream>
#include <fstream>

FileManager::FileManager(const char *root)
{
    m_root = std::string(root);
}

FileManager::~FileManager()
{
}

std::string FileManager::GetFilename(std::string &url, uint64_t &size)
{
    std::stringstream ss;
    ss << m_root << "." << url;

    std::string filename = ss.str();
    if (filename[filename.length() - 1] == '/')
    {
        // Look for index file.
        std::ifstream in(filename + "index.html", std::ifstream::in);
        if (in.is_open())
        {
            size = FileManager::GetFileSize(&in);
            in.close();
            return filename + "index.html";
        }
        else
        {
            in.open(filename + "index.htm", std::ifstream::in);
            if (in.is_open())
            {
                size = FileManager::GetFileSize(&in);
                in.close();                
                return filename + "index.htm";
            }
        }
    }
    std::ifstream in(filename, std::ifstream::in);
    if (!in.is_open())
    {
        throw std::runtime_error("File does not exist.");
    }
    size = FileManager::GetFileSize(&in);
    in.close();
    return filename;
}

inline uint64_t FileManager::GetFileSize(std::ifstream *infile)
{
    if(!infile->is_open()){
        throw std::runtime_error("Tried to get file size for file that is not open.");
    }
    
    infile->seekg(0,std::ios::end);
    uint64_t size = infile->tellg();
    infile->seekg(0,std::ios::beg);
    return size;
}

std::string FileManager::GetContentType( std::string filename )
{
    // Work backwards and find the first . for the extension
    auto lastDot = filename.find_last_of('.');    
    std::string ext = filename.substr(lastDot+1);
    
    // Now return the correct content length
    if( ext == "html" || ext == "htm"){
        return "text/html";
    }

    if(ext == "txt"){
        return "text/plain";        
    }

    if(ext =="png"){
        return "image/png";
    }

    if(ext=="gif"){
        return "image/gif";
    }

    if(ext=="jpg"){
        return "image/jpg";
    }

    if(ext =="js"){
        return "application/javascript";
    }
    if(ext =="css"){
        return "text/css";
    }

    return "text/html";
    
}