#include "AndroidFileSystem.h"

namespace raptor {

    std::vector<uint8_t> AndroidFileSystem::readFile(const std::string& path) {
        std::vector<uint8_t> data;
        if (!m_Mgr) return data;

        AAsset* asset = AAssetManager_open(m_Mgr, path.c_str(), AASSET_MODE_BUFFER);
        if (!asset) return data;

        size_t length = AAsset_getLength(asset);
        data.resize(length);

        AAsset_read(asset, data.data(), length);
        AAsset_close(asset);

        return data;
    }

    bool AndroidFileSystem::exists(const std::string& path) {
        if (!m_Mgr) return false;

        AAsset* asset = AAssetManager_open(m_Mgr, path.c_str(), AASSET_MODE_UNKNOWN);
        if (asset) {
            AAsset_close(asset);
            return true;
        }
        return false;
    }
}
