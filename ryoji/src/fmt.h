#ifndef __RYOJI_FMT_H__
#define __RYOJI_FMT_H__

#include <string>
#include <assert.h>

namespace ryoji::fmt {
    template<size_t BufferSize, typename ... Args>
    std::string fixedFormat(const std::string& format, Args ... args)
    {
        char buffer[BufferSize];
        int count = snprintf(buffer, BufferSize, format.c_str(), args ...);
        return std::string(buffer); // We don't want the '\0' inside
    }
}


#endif