#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_coord;

layout(location = 0) out vec4 frag_color;

layout(binding = 11) uniform sampler2D cube_texture;

void main()
{
  vec3 dir_light = vec3(0.40, -0.50, 0.10);
  vec4 sampled_color = texture(cube_texture, frag_coord);
  
  vec3 ambient = vec3(0.4118, 0.4118, 0.4118);
  vec3 diffuse = vec3(dot(frag_normal, dir_light));

  vec3 final_color = sampled_color.xyz * (ambient + diffuse);
  frag_color = vec4(final_color.xyz, 1.0f);
} 
