#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 frag_position;
layout (location = 1) in vec2 frag_tex_coords;
layout (location = 2) in vec3 frag_normal;

layout (location = 0) out vec4 g_position;
layout (location = 1) out vec4 g_normal;
layout (location = 2) out vec4 g_albedo_spec;

layout(binding = 10) uniform sampler2D texture_diffuse1;
layout(binding = 11) uniform sampler2D texture_specular1;

void main()
{    
  vec4 diffuse = texture(texture_diffuse1, frag_tex_coords);
  float spec = texture(texture_specular1, frag_tex_coords).r;

  g_position = vec4(frag_position, 1.0f);
  g_normal = vec4(normalize(frag_normal), 1.0f);
  g_albedo_spec = vec4(diffuse.rgb, spec);
}