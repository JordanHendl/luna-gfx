#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_coord;

layout(location = 0) out vec4 frag_color;

void main()
{
  vec3 tmp_normal = frag_normal;
  vec2 tmp_coord = frag_coord;
  vec3 red = vec3(1, 0, 0);
  frag_color = vec4(red, 0.2f);
} 
