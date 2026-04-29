#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace raptor {

    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;
        virtual std::vector<uint8_t> readFile(const std::string& path) = 0;
        virtual bool exists(const std::string& path) = 0;
    };

    class IWindow {
    public:
        virtual ~IWindow() = default;
        virtual void* getNativeHandle() = 0;
        virtual int   getWidth()  const = 0;
        virtual int   getHeight() const = 0;
    };
}
