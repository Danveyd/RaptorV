#include "AndroidFileSystem.h"
#include <fstream>

std::vector<uint8_t> raptor::AndroidFileSystem::readFile(const std::string& path) {
    std::vector<uint8_t> data;

    if (!path.empty() && path[0] == '/') {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            data.resize(size);
            if (file.read(reinterpret_cast<char*>(data.data()), size)) {
                return data;
            }
        }
        return data;
    }

    if (!m_Mgr) return data;
    AAsset* asset = AAssetManager_open(m_Mgr, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) return data;

    size_t length = AAsset_getLength(asset);
    data.resize(length);

    AAsset_read(asset, data.data(), length);
    AAsset_close(asset);

    return data;
}

bool raptor::AndroidFileSystem::exists(const std::string& path) {
    if (!path.empty() && path[0] == '/') {
        std::ifstream file(path);
        return file.good();
    }

    if (!m_Mgr) return false;
    AAsset* asset = AAssetManager_open(m_Mgr, path.c_str(), AASSET_MODE_UNKNOWN);
    if (asset) {
        AAsset_close(asset);
        return true;
    }
    return false;
}
