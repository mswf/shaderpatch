#pragma once

#include "scene.hpp"

#include <cstddef>
#include <filesystem>

#include <gsl/gsl>

namespace sp::editor::msh {

Scene load_from_file(const std::filesystem::path& path);

Scene load_from_memory(gsl::span<const std::byte> bytes);

}
