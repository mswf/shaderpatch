#pragma once

#include "compose_exception.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include <gsl/gsl>

namespace sp {

inline auto load_string_file(const std::filesystem::path& path)
{
   std::ifstream file{path, std::ios::binary | std::ios::ate};

   if (!file.is_open()) {
      throw compose_exception<std::runtime_error>("Failed to open file: ", path, '.');
   }

   const auto size = file.tellg();

   std::string buf;
   buf.resize(gsl::narrow_cast<std::size_t>(size));

   file.seekg(0);
   file.read(buf.data(), size);

   return buf;
}

inline auto load_vector_file(const std::filesystem::path& path)
{
   std::ifstream file{path, std::ios::binary | std::ios::ate};

   if (!file.is_open()) {
      throw compose_exception<std::runtime_error>("Failed to open file: ", path, '.');
   }

   const auto size = file.tellg();

   std::vector<std::byte> buf;
   buf.resize(gsl::narrow_cast<std::size_t>(size));

   file.seekg(0);
   file.read(reinterpret_cast<char*>(buf.data()), size);

   return buf;
}

inline std::ifstream safe_ifile_open(const std::filesystem::path& path)
{
   std::ifstream file{path};

   if (!file.is_open()) {
      throw compose_exception<std::runtime_error>("Failed to open file: ", path, '.');
   }

   return file;
}

}
