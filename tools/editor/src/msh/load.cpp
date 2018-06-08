#include "load.hpp"
#include "../geom/procedural.hpp"
#include "iterator.hpp"
#include "operations.hpp"
#include "string_utilities.hpp"
#include "ucfb_reader.hpp"

#include <set>
#include <tuple>
#include <utility>

#include <boost/container/flat_map.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gsl/gsl>

#pragma warning(disable : 4063)

namespace fs = std::filesystem;
using namespace std::literals;

namespace sp::editor::msh {

namespace {

Scene read_msh2(ucfb::Reader_strict<"MSH2"_mn> msh_reader);

void read_sinf(ucfb::Reader_strict<"SINF"_mn> scene_info, Scene& scene);

void read_matl(ucfb::Reader_strict<"MATL"_mn> materials, Scene& scene);

Material read_matd(ucfb::Reader_strict<"MATD"_mn> matd_reader);

void read_modl(ucfb::Reader_strict<"MODL"_mn> modl_reader, Scene& scene,
               const boost::container::flat_map<std::int32_t, std::string>& index_map);

auto read_modl_index_name(ucfb::Reader_strict<"MODL"_mn> modl_reader)
   -> std::pair<int, std::string>;

glm::mat4 read_tran(ucfb::Reader_strict<"TRAN"_mn> tran_reader);

void read_geom(ucfb::Reader_strict<"GEOM"_mn> geom_reader, Node& node,
               const boost::container::flat_map<std::int32_t, std::string>& index_map);

Segment read_segm(ucfb::Reader_strict<"SEGM"_mn> segm_reader);

auto read_shdw(ucfb::Reader_strict<"SHDW"_mn> shdw_reader)
   -> std::pair<std::vector<glm::vec3>, std::vector<std::array<std::uint16_t, 3>>>;

auto read_strp(ucfb::Reader_strict<"STRP"_mn> strp_reader)
   -> std::vector<std::array<std::uint16_t, 3>>;

Cloth read_clth(ucfb::Reader_strict<"CLTH"_mn> clth_reader);

auto read_fwgt(ucfb::Reader_strict<"FWGT"_mn> fwgt_reader) -> std::vector<std::string>;

auto read_cloth_coll(ucfb::Reader_strict<"COLL"_mn> coll_reader)
   -> std::vector<Cloth_collision>;

auto read_envl(ucfb::Reader_strict<"ENVL"_mn> envl_reader,
               const boost::container::flat_map<std::int32_t, std::string>& index_map)
   -> std::vector<std::string>;

auto read_swci(ucfb::Reader_strict<"SWCI"_mn> swci_reader) -> Collision_primitive;

template<typename Attribute_type, typename Reader>
auto read_attribute(Reader reader) -> std::vector<Attribute_type>;

auto build_msh_index_map(ucfb::Reader_strict<"MSH2"_mn> msh_reader)
   -> boost::container::flat_map<std::int32_t, std::string>;

Node_type get_node_type(std::string_view name, Model_type model_type) noexcept;

bool is_degenerate_triangle(std::array<std::uint16_t, 3> tri);

}

Scene load_from_file(const std::filesystem::path& path)
{
   Expects(fs::exists(path) && fs::is_regular_file(path));

   boost::iostreams::mapped_file file{path.u8string(),
                                      boost::iostreams::mapped_file::mapmode::readonly};

   return load_from_memory(
      gsl::make_span(reinterpret_cast<std::byte*>(file.data()), file.size()));
}

Scene load_from_memory(gsl::span<const std::byte> bytes)
{

   ucfb::Reader_strict<"HEDR"_mn> reader{bytes};

   auto scene = read_msh2(reader.read_child_strict<"MSH2"_mn>());

   regen_scene_bboxes(scene);

   return scene;
}

namespace {

Scene read_msh2(ucfb::Reader_strict<"MSH2"_mn> msh_reader)
{
   const auto index_map = build_msh_index_map(msh_reader);

   Scene scene;

   while (msh_reader) {
      auto child = msh_reader.read_child();

      switch (child.magic_number()) {
      case "SINF"_mn:
         read_sinf(ucfb::Reader_strict<"SINF"_mn>{child}, scene);
         break;
      case "MATL"_mn:
         read_matl(ucfb::Reader_strict<"MATL"_mn>{child}, scene);
         break;
      case "MODL"_mn:
         read_modl(ucfb::Reader_strict<"MODL"_mn>{child}, scene, index_map);
         break;
      }
   }

   scene.bbox = build_scene_aabb(scene);

   return scene;
}

void read_sinf(ucfb::Reader_strict<"SINF"_mn> scene_info, Scene& scene)
{
   scene.scene_name = scene_info.read_child_strict<"NAME"_mn>().read_string();
}

void read_matl(ucfb::Reader_strict<"MATL"_mn> materials, Scene& scene)
{
   while (materials) {
      scene.materials.emplace_back(read_matd(materials.read_child_strict<"MATD"_mn>()));
   }
}

Material read_matd(ucfb::Reader_strict<"MATD"_mn> matd_reader)
{
   Material material;

   material.name = matd_reader.read_child_strict<"NAME"_mn>().read_string();

   auto data_reader = matd_reader.read_child_strict<"DATA"_mn>();

   material.diffuse_colour = data_reader.read_trivial<glm::vec4>();
   material.specular_colour = data_reader.read_trivial<glm::vec4>();
   material.ambient_colour = data_reader.read_trivial<glm::vec4>();
   material.specular_exponent = data_reader.read_trivial<float>();

   auto atrb_reader = matd_reader.read_child_strict<"ATRB"_mn>();
   material.flags = atrb_reader.read_trivial_unaligned<Render_flags>();
   material.type = atrb_reader.read_trivial_unaligned<Render_type>();
   material.params =
      atrb_reader.read_trivial_unaligned<std::array<std::int8_t, 2>>();

   return material;
}

void read_modl(ucfb::Reader_strict<"MODL"_mn> modl_reader, Scene& scene,
               const boost::container::flat_map<std::int32_t, std::string>& index_map)
{
   Node node{};
   Model_type model_type{};
   std::string_view parent_name{};
   std::int32_t index{-1};

   while (modl_reader) {
      auto child = modl_reader.read_child();

      switch (child.magic_number()) {
      case "MTYP"_mn:
         model_type = child.read_trivial<Model_type>();
         break;
      case "MNDX"_mn:
         index = child.read_trivial<std::int32_t>();
         break;
      case "NAME"_mn:
         node.name = child.read_string();
         break;
      case "PRNT"_mn:
         parent_name = child.read_string();
         break;
      case "FLGS"_mn:
         node.hidden = child.read_trivial<std::uint32_t>() & 1;
         break;
      case "TRAN"_mn:
         node.transform = read_tran(ucfb::Reader_strict<"TRAN"_mn>{child});
         break;
      case "GEOM"_mn:
         read_geom(ucfb::Reader_strict<"GEOM"_mn>{child}, node, index_map);
         break;
      case "SWCI"_mn:
         node.collision = read_swci(ucfb::Reader_strict<"SWCI"_mn>{child});
         break;
      }
   }

   node.type = get_node_type(node.name, model_type);

   if (parent_name.empty() && index == 0) {
      scene.root = std::move(node);
   }
   else if (auto parent = find_node(scene, parent_name); parent) {
      parent->children.emplace_back(std::move(node));
   }
   else {
      throw std::runtime_error{"freestanding scene node found"};
   }
}

auto read_modl_index_name(ucfb::Reader_strict<"MODL"_mn> modl_reader)
   -> std::pair<int, std::string>
{
   std::string_view name;
   int index;

   while (modl_reader) {
      auto child = modl_reader.read_child();

      switch (child.magic_number()) {
      case "MDNX"_mn:
         index = child.read_trivial<std::int32_t>();
         break;
      case "NAME"_mn:
         name = child.read_string();
         break;
      }
   }

   return {index, std::string{name}};
}

glm::mat4 read_tran(ucfb::Reader_strict<"TRAN"_mn> tran_reader)
{
   [[maybe_unused]] auto scale = tran_reader.read_trivial<glm::vec3>();
   auto rotation = tran_reader.read_trivial<glm::quat>();
   auto translation = tran_reader.read_trivial<glm::vec3>();

   glm::mat4 matrix = static_cast<glm::mat4>(rotation);
   matrix *= glm::translate({}, translation);

   return matrix;
}

void read_geom(ucfb::Reader_strict<"GEOM"_mn> geom_reader, Node& node,
               const boost::container::flat_map<std::int32_t, std::string>& index_map)
{
   while (geom_reader) {
      auto child = geom_reader.read_child();

      switch (child.magic_number()) {
      case "SEGM"_mn:
         node.segments.emplace_back(read_segm(ucfb::Reader_strict<"SEGM"_mn>{child}));
         break;
      case "CLTH"_mn:
         node.cloths.emplace_back(read_clth(ucfb::Reader_strict<"CLTH"_mn>{child}));
         break;
      case "ENVL"_mn:
         node.bone_map = read_envl(ucfb::Reader_strict<"ENVL"_mn>{child}, index_map);
         break;
      }
   }
}

Segment read_segm(ucfb::Reader_strict<"SEGM"_mn> segm_reader)
{
   Segment segment{};

   while (segm_reader) {
      auto child = segm_reader.read_child();

      switch (child.magic_number()) {
      case "SHDW"_mn:
         std::tie(segment.shadow_positions, segment.shadow_edges) =
            read_shdw(ucfb::Reader_strict<"SHDW"_mn>{child});
         break;
      case "MATI"_mn:
         segment.material_index = child.read_trivial<std::int32_t>();
         break;
      case "POSL"_mn:
         segment.positions = read_attribute<glm::vec3>(child);
         break;
      case "NRML"_mn:
         segment.normals = read_attribute<glm::vec3>(child);
         break;
      case "UV0L"_mn:
         segment.uv_coords = read_attribute<glm::vec2>(child);
         break;
      case "CLRL"_mn:
         segment.colours = read_attribute<glm::u8vec4>(child);
         break;
      case "WGHT"_mn:
         segment.skin = read_attribute<std::array<Skin_entry, 4>>(child);

         static_assert(sizeof(std::array<Skin_entry, 4>) == 32);
         break;
      case "STRP"_mn:
         segment.indices = read_strp(ucfb::Reader_strict<"STRP"_mn>{child});
         break;
      }
   }

   return segment;
}

auto read_shdw(ucfb::Reader_strict<"SHDW"_mn> shdw_reader)
   -> std::pair<std::vector<glm::vec3>, std::vector<std::array<std::uint16_t, 3>>>
{
   auto pos_count = shdw_reader.read_trivial<std::int32_t>();
   auto positions = shdw_reader.read_array<glm::vec3>(pos_count);

   auto edge_count = shdw_reader.read_trivial<std::int32_t>();
   auto edges_span = shdw_reader.read_array<std::array<std::uint16_t, 4>>(edge_count);

   std::vector<std::array<std::uint16_t, 3>> edges;
   edges.reserve(edge_count);

   for (const auto& edge : edges_span) {
      edges.emplace_back(std::array{edge[0], edge[1], edge[2]});
   }

   return {{std::cbegin(positions), std::cend(positions)}, edges};
}

auto read_strp(ucfb::Reader_strict<"STRP"_mn> strp_reader)
   -> std::vector<std::array<std::uint16_t, 3>>
{
   auto strips_index_count = strp_reader.read_trivial<std::int32_t>();
   auto strips = strp_reader.read_array<std::uint16_t>(strips_index_count);

   std::vector<std::array<std::uint16_t, 3>> indices;

   for (gsl::index i = 0; i < strips.size(); ++i) {
      if (strips[i] & 0x8000u) {
         ++i;
         continue;
      }

      std::array<std::uint16_t, 3> tri{strips[i - 2], strips[i - 1], strips[i]};

      for (auto& v : tri) v &= ~0x8000u;

      if (!is_degenerate_triangle(tri)) indices.emplace_back(tri);
   }

   return indices;
}

Cloth read_clth(ucfb::Reader_strict<"CLTH"_mn> clth_reader)
{
   Cloth cloth{};

   while (clth_reader) {
      auto child = clth_reader.read_child();

      switch (child.magic_number()) {
      case "CTEX"_mn:
         cloth.texture = child.read_string();
         break;
      case "CPOS"_mn:
         cloth.positions = read_attribute<glm::vec3>(child);
         break;
      case "CUV0"_mn:
         cloth.uv_coords = read_attribute<glm::vec2>(child);
         break;
      case "FIDX"_mn:
         cloth.fixed_points = read_attribute<std::uint32_t>(child);
         break;
      case "FWGT"_mn:
         cloth.fixed_weights = read_fwgt(ucfb::Reader_strict<"FWGT"_mn>(child));
         break;
      case "CMSH"_mn:
         cloth.indices = read_attribute<std::array<std::uint32_t, 3>>(child);
         break;
      case "SPRS"_mn:
         cloth.stretch_constraints =
            read_attribute<std::array<std::uint16_t, 2>>(child);
         break;
      case "CPRS"_mn:
         cloth.cross_constraints = read_attribute<std::array<std::uint16_t, 2>>(child);
         break;
      case "BPRS"_mn:
         cloth.bend_constraints = read_attribute<std::array<std::uint16_t, 2>>(child);
         break;
      case "COLL"_mn:
         cloth.collision = read_cloth_coll(ucfb::Reader_strict<"COLL"_mn>{child});
         break;
      }
   }

   return cloth;
}

auto read_fwgt(ucfb::Reader_strict<"FWGT"_mn> fwgt_reader) -> std::vector<std::string>
{
   auto count = fwgt_reader.read_trivial<std::int32_t>();

   std::vector<std::string> weights;
   weights.reserve(count);

   for (auto i = 0; i < count; ++i) {
      weights.emplace_back(fwgt_reader.read_string_unaligned());
   }

   return weights;
}

auto read_cloth_coll(ucfb::Reader_strict<"COLL"_mn> coll_reader)
   -> std::vector<Cloth_collision>
{
   auto count = coll_reader.read_trivial<std::int32_t>();

   std::vector<Cloth_collision> collision_prims;
   collision_prims.reserve(count);

   for (auto i = 0; i < count; ++i) {
      Cloth_collision prim{};

      prim.name = coll_reader.read_string_unaligned();
      prim.parent = coll_reader.read_string_unaligned();
      prim.type = coll_reader.read_trivial_unaligned<Cloth_collision_type>();
      prim.size = coll_reader.read_trivial_unaligned<glm::vec3>();

      collision_prims.emplace_back(std::move(prim));
   }

   return collision_prims;
}

auto read_envl(ucfb::Reader_strict<"ENVL"_mn> envl_reader,
               const boost::container::flat_map<std::int32_t, std::string>& index_map)
   -> std::vector<std::string>
{
   auto count = envl_reader.read_trivial<std::int32_t>();

   std::vector<std::string> bones;
   bones.reserve(count);

   for (auto& index : envl_reader.read_array<std::int32_t>(count)) {
      bones.emplace_back(index_map.at(index));
   }

   return bones;
}

auto read_swci(ucfb::Reader_strict<"SWCI"_mn> swci_reader) -> Collision_primitive
{
   Collision_primitive primitive{};

   primitive.type = swci_reader.read_trivial<Primitive_type>();
   primitive.size = swci_reader.read_trivial<glm::vec3>();

   return primitive;
}

template<typename Attribute_type, typename Reader>
auto read_attribute(Reader reader) -> std::vector<Attribute_type>
{
   auto count = reader.read_trivial<std::int32_t>();
   auto attributes = reader.read_array<Attribute_type>(count);

   return {std::cbegin(attributes), std::cend(attributes)};
}

auto build_msh_index_map(ucfb::Reader_strict<"MSH2"_mn> msh_reader)
   -> boost::container::flat_map<std::int32_t, std::string>
{
   boost::container::flat_map<std::int32_t, std::string> index_map;

   while (msh_reader) {
      auto child = msh_reader.read_child();

      if (child.magic_number() == "MODL"_mn) {
         index_map.emplace(read_modl_index_name(ucfb::Reader_strict<"MODL"_mn>{child}));
      }
   }

   return index_map;
}

Node_type get_node_type(std::string_view name, Model_type model_type) noexcept
{
   if (begins_with(name, "shadowvolume"sv) || begins_with(name, "sv_"sv) ||
       model_type == Model_type::shadow) {
      return Node_type::shadow_mesh;
   }

   if (begins_with(name, "p_"sv)) return Node_type::collision_node;
   if (begins_with(name, "collision"sv)) return Node_type::collision_mesh;

   if (model_type == Model_type::bone) return Node_type::bone;
   if (model_type == Model_type::null) return Node_type::null;
   if (begins_with(name, "hp_")) return Node_type::null;

   return Node_type::mesh;
}

bool is_degenerate_triangle(std::array<std::uint16_t, 3> tri)
{
   return tri[0] == tri[1] || tri[0] == tri[2] || tri[1] == tri[2];
}

}

}
