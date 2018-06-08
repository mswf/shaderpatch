#pragma once

#include "file_helpers.hpp"
#include "model.hpp"
#include "odf.hpp"

#include <filesystem>
#include <fstream>

namespace sp::editor {

class Assets_manager;

template<typename Type>
struct Asset_loader {
   Type load(const std::filesystem::path& path, Assets_manager& manager) = delete;
};

template<>
struct Asset_loader<odf::Definition> {
   odf::Definition load(const std::filesystem::path& path, Assets_manager& manager);
};

template<>
struct Asset_loader<Model> {
   Model load(const std::filesystem::path& path, Assets_manager& manager);
};
}
