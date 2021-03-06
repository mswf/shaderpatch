
#include "patch_material.hpp"
#include "material_constant_buffers.hpp"

#include <iomanip>

namespace sp::core {

namespace {

auto init_resources(const std::vector<std::string>& resource_names,
                    const Shader_resource_database& resource_database) noexcept
   -> std::vector<Com_ptr<ID3D11ShaderResourceView>>
{
   std::vector<Com_ptr<ID3D11ShaderResourceView>> resources;

   for (const auto& name : resource_names) {
      if (name.empty()) {
         resources.emplace_back(nullptr);
         continue;
      }

      resources.emplace_back(resource_database.at_if(name));

      if (!resources.back()) {
         log(Log_level::warning, "Shader resource "sv, std::quoted(name),
             " does not exist."sv);
      }
   }

   return resources;
}

auto init_fail_safe_texture(const std::vector<Com_ptr<ID3D11ShaderResourceView>>& shader_resources,
                            const std::int32_t fail_safe_texture_index) noexcept -> Game_texture
{
   Expects(fail_safe_texture_index == -1 ||
           fail_safe_texture_index < shader_resources.size());

   if (fail_safe_texture_index == -1) return {};

   return {shader_resources[fail_safe_texture_index],
           shader_resources[fail_safe_texture_index]};
}

}

Patch_material::Patch_material(Material_config material_config,
                               Material_shader_factory& shader_factory,
                               const Shader_resource_database& resource_database,
                               ID3D11Device5& device)
   : overridden_rendertype{material_config.overridden_rendertype},
     shader{shader_factory.create(material_config.rendertype)},
     cb_shader_stages{material_config.cb_shader_stages},
     constant_buffer{create_material_constant_buffer(device, material_config.cb_name,
                                                     material_config.properties)},
     vs_shader_resources{init_resources(material_config.vs_resources, resource_database)},
     hs_shader_resources{init_resources(material_config.hs_resources, resource_database)},
     ds_shader_resources{init_resources(material_config.ds_resources, resource_database)},
     gs_shader_resources{init_resources(material_config.gs_resources, resource_database)},
     ps_shader_resources{init_resources(material_config.ps_resources, resource_database)},
     fail_safe_game_texture{
        init_fail_safe_texture(ps_shader_resources,
                               material_config.fail_safe_texture_index)},
     tessellation{material_config.tessellation},
     tessellation_primitive_topology{material_config.tessellation_primitive_topology},
     name{std::move(material_config.name)},
     rendertype{material_config.rendertype},
     cb_name{material_config.cb_name},
     properties{material_config.properties},
     vs_shader_resources_names{material_config.vs_resources},
     hs_shader_resources_names{material_config.hs_resources},
     ds_shader_resources_names{material_config.ds_resources},
     gs_shader_resources_names{material_config.gs_resources},
     ps_shader_resources_names{material_config.ps_resources}
{
}

void Patch_material::update_resources(const Shader_resource_database& resource_database) noexcept
{
   vs_shader_resources = init_resources(vs_shader_resources_names, resource_database);
   hs_shader_resources = init_resources(hs_shader_resources_names, resource_database);
   ds_shader_resources = init_resources(ds_shader_resources_names, resource_database);
   gs_shader_resources = init_resources(gs_shader_resources_names, resource_database);
   ps_shader_resources = init_resources(ps_shader_resources_names, resource_database);
}

void Patch_material::bind_constant_buffers(ID3D11DeviceContext1& dc) noexcept
{
   auto* const cb = constant_buffer.get();

   if ((cb_shader_stages & Material_cb_shader_stages::vs) ==
       Material_cb_shader_stages::vs)
      dc.VSSetConstantBuffers(vs_cb_offset, 1, &cb);

   if ((cb_shader_stages & Material_cb_shader_stages::hs) ==
       Material_cb_shader_stages::hs)
      dc.HSSetConstantBuffers(hs_cb_offset, 1, &cb);

   if ((cb_shader_stages & Material_cb_shader_stages::ds) ==
       Material_cb_shader_stages::ds)
      dc.DSSetConstantBuffers(ds_cb_offset, 1, &cb);

   if ((cb_shader_stages & Material_cb_shader_stages::gs) ==
       Material_cb_shader_stages::gs)
      dc.GSSetConstantBuffers(gs_cb_offset, 1, &cb);

   if ((cb_shader_stages & Material_cb_shader_stages::ps) ==
       Material_cb_shader_stages::ps)
      dc.PSSetConstantBuffers(ps_cb_offset, 1, &cb);
}

void Patch_material::bind_shader_resources(ID3D11DeviceContext1& dc) noexcept
{
   static_assert(sizeof(Com_ptr<ID3D11ShaderResourceView>) ==
                 sizeof(ID3D11ShaderResourceView*));

   if (!vs_shader_resources.empty())
      dc.VSSetShaderResources(vs_resources_offset, vs_shader_resources.size(),
                              reinterpret_cast<ID3D11ShaderResourceView**>(
                                 vs_shader_resources.data()));

   if (!hs_shader_resources.empty())
      dc.HSSetShaderResources(hs_resources_offset, hs_shader_resources.size(),
                              reinterpret_cast<ID3D11ShaderResourceView**>(
                                 hs_shader_resources.data()));

   if (!ds_shader_resources.empty())
      dc.DSSetShaderResources(ds_resources_offset, ds_shader_resources.size(),
                              reinterpret_cast<ID3D11ShaderResourceView**>(
                                 ds_shader_resources.data()));

   if (!gs_shader_resources.empty())
      dc.GSSetShaderResources(gs_resources_offset, gs_shader_resources.size(),
                              reinterpret_cast<ID3D11ShaderResourceView**>(
                                 gs_shader_resources.data()));

   if (!ps_shader_resources.empty())
      dc.PSSetShaderResources(ps_resources_offset, ps_shader_resources.size(),
                              reinterpret_cast<ID3D11ShaderResourceView**>(
                                 ps_shader_resources.data()));
}

}
