#pragma once
#include "raptor/Platform.h"
#include <android/asset_manager.h>

namespace raptor {

    class AndroidFileSystem : public IFileSystem {
    public:
        explicit AndroidFileSystem(AAssetManager* mgr) : m_Mgr(mgr) {}
        std::vector<uint8_t> readFile(const std::string& path) override;
        bool exists(const std::string& path) override;
    private:
        AAssetManager* m_Mgr;
    };
}
