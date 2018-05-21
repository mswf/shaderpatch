#pragma once

#include "compose_exception.hpp"
#include "exceptions.hpp"
#include "string_utilities.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace sp::odf {

enum class Type { explosion, game_object, ordnance, weapon };

struct Value {
   std::string value;
   std::string trailing_comment;
   std::vector<std::string> comments;
   int empty_lines_before;
};

class Properties : public std::vector<std::pair<std::string, Value>> {
   using std::vector<std::pair<std::string, Value>>::vector;

   Value& value_at(std::string_view key)
   {
      for (auto& v : *this) {
         if (v.first == key) return v.second;
      }

      throw std::out_of_range{"invalid key"};
   }

   const Value& value_at(std::string_view key) const
   {
      for (auto& v : *this) {
         if (v.first == key) return v.second;
      }

      throw std::out_of_range{"invalid key"};
   }

   Value& operator[](std::string_view key)
   {
      for (auto& v : *this) {
         if (v.first == key) return v.second;
      }

      return emplace_back(std::string{key}, Value{}).second;
   }

   const Value& operator[](std::string_view key) const
   {
      for (auto& v : *this) {
         if (v.first == key) return v.second;
      }

      throw std::out_of_range{"invalid key"};
   }
};

struct Definition {
   Type type;
   std::vector<std::string> header_comments;
   std::string header_trailing_comment;
   Properties header_properties;

   std::vector<std::string> properties_comments;
   std::string properties_trailing_comment;
   Properties properties;

   std::vector<std::string> instance_properties_comments;
   std::string instance_properties_trailing_comment;
   Properties instance_properties;

   std::vector<std::string> freestanding_comments;
};

constexpr std::string_view to_string(Type type)
{
   using namespace std::literals;

   switch (type) {
   case Type::explosion:
      return "ExplosionClass"sv;
   case Type::game_object:
      return "GameObjectClass"sv;
   case Type::ordnance:
      return "OrdnanceClass"sv;
   case Type::weapon:
      return "WeaponClass"sv;
   }
}

inline Type string_to_type(std::string_view string)
{
   using namespace std::literals;

   if (string == "ExplosionClass"sv) return Type::explosion;
   if (string == "GameObjectClass"sv) return Type::game_object;
   if (string == "OrdnanceClass"sv) return Type::ordnance;
   if (string == "WeaponClass"sv) return Type::weapon;

   throw std::invalid_argument{"string is not a valid ODF Type"s};
}

namespace detail {

inline void handle_section(std::istream& stream, Definition& def,
                           std::string_view name, std::string_view trailing_comment,
                           std::vector<std::string>& comments);

inline auto read_section(std::istream& stream)
   -> std::tuple<Properties, std::vector<std::string>>;

inline auto read_property(std::string_view property_comment,
                          std::vector<std::string> comments, int empty_lines_before)
   -> std::pair<std::string, Value>;

}

inline Definition from_istream(std::istream& stream)
{
   using namespace std::literals;

   Definition def;

   std::vector<std::string> comments;

   stream >> std::ws;

   for (std::string line; stream.good(); stream >> std::ws) {
      std::getline(stream, line);

      if (line.empty()) continue;

      if (begins_with(line, "//"sv)) {
         comments.emplace_back(std::cbegin(line) + 2, std::cend(line));
      }
      else if (begins_with(line, "["sv)) {
         auto [section_name, trailing_comment] = split_string_on(line, "//"sv);

         if (auto split_name = sectioned_split_split(section_name, "["sv, "]"sv);
             split_name) {
            section_name = split_name->at(0);
         }
         else {
            throw compose_exception<Parse_error>("Missing closing ']' for section "sv,
                                                 std::quoted(section_name), "."sv);
         }

         detail::handle_section(stream, def, section_name, trailing_comment, comments);
      }
      else {
         throw compose_exception<Parse_error>("Expected [<string>] opening properties block got "sv,
                                              std::quoted(line), "instead."sv);
      }
   }

   def.freestanding_comments = std::move(comments);

   return def;
}

namespace detail {

inline void handle_section(std::istream& stream, Definition& def,
                           std::string_view name, std::string_view trailing_comment,
                           std::vector<std::string>& comments)
{
   using namespace std::literals;

   if (name == "Properties"sv) {
      def.properties_comments = std::move(comments);
      def.properties_trailing_comment = trailing_comment;
      std::tie(def.properties, comments) = read_section(stream);
   }
   else if (name == "InstanceProperties"sv) {
      def.instance_properties_comments = std::move(comments);
      def.instance_properties_trailing_comment = trailing_comment;
      std::tie(def.instance_properties, comments) = read_section(stream);
   }
   else {
      try {
         def.type = string_to_type(name);
      }
      catch (std::exception&) {
         throw compose_exception<Parse_error>("Unknown section "sv,
                                              std::quoted(name), " encountered."sv);
      }

      def.header_comments = std::move(comments);
      def.header_trailing_comment = trailing_comment;
      std::tie(def.header_properties, comments) = read_section(stream);
   }
}

inline auto read_section(std::istream& stream)
   -> std::tuple<Properties, std::vector<std::string>>
{
   using namespace std::literals;

   Properties properties;
   std::vector<std::string> comments;
   int empty_lines_before = 0;

   stream >> std::ws;
   std::streamoff last_line_pos;
   for (std::string line; stream.good(); stream >> std::ws) {
      last_line_pos = stream.tellg();
      std::getline(stream, line);

      if (line.empty()) {
         ++empty_lines_before;
         continue;
      }

      if (begins_with(line, "//"sv)) {
         comments.emplace_back(std::cbegin(line) + 2, std::cend(line));
      }
      else if (begins_with(line, "["sv)) {
         stream.seekg(last_line_pos);

         return std::make_tuple(std::move(properties), std::move(comments));
      }
      else {
         properties.emplace_back(read_property(line, std::move(comments),
                                               std::exchange(empty_lines_before, 0)));
      }
   }

   return std::make_tuple(std::move(properties), std::move(comments));
}

inline auto read_property(std::string_view property_comment,
                          std::vector<std::string> comments,
                          int empty_lines_before) -> std::pair<std::string, Value>
{
   using namespace std::literals;

   auto [property, trailing_comment] = split_string_on(property_comment, "//"sv);

   auto [key, value] = split_string_on(property, "="sv);
   key = trim_whitespace(key);
   value = trim_whitespace(value);

   if (value.empty()) {
      throw compose_exception<Parse_error>("Property "sv, std::quoted(key),
                                           " with empty value."sv);
   }

   if (auto unquoted_value = sectioned_split_split(value, "\""sv, "\""sv);
       unquoted_value) {
      value = unquoted_value->at(0);
   }

   Value prop;
   prop.value = value;
   prop.trailing_comment = trailing_comment;
   prop.comments = std::move(comments);
   prop.empty_lines_before = empty_lines_before;

   return {std::string{key}, prop};
}

}

}
