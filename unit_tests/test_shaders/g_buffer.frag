#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 frag_position;
layout (location = 1) in vec2 frag_tex_coords;
layout (location = 2) in vec3 frag_normal;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_spec;

layout(binding = 10) uniform sampler2D texture_diffuse1;
layout(binding = 11) uniform sampler2D texture_specular1;

void main()
{    
  // store the fragment position vector in the first gbuffer texture
  g_position = frag_position;
  // also store the per-fragment normals into the gbuffer
  g_normal = normalize(frag_normal);
  // and the diffuse per-fragment color
  g_albedo_spec.rgb = texture(texture_diffuse1, frag_tex_coords).rgb;
  // store specular intensity in g_albedo_spec's alpha component
  g_albedo_spec.a = texture(texture_specular1, frag_tex_coords).r;
}