#ifndef I_CACHE_H
#define I_CACHE_H

#include <unordered_map>

class ICache
{
public:
    ICache(){

    }
    struct CacheMissException : std::exception
    {
      public:
        const char *what() const throw()
        {
            return "Cache Miss";
        }
    };
private:
    
};
#endif