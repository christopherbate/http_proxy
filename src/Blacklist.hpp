#ifndef _BL_H
#define _BL_H

#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

class Blacklist
{
public:
    Blacklist( std::string filename ){
        m_infile.open(filename.c_str(),std::ifstream::in);
        if(!m_infile.is_open()){
            throw std::runtime_error("Could not open blacklist file.");
        }        
    }
    ~Blacklist(){}

    void ParseList()
    {

    }

private:
    std::ifstream m_infile;
};

#endif