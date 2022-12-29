#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 frag_tex_coords;

void main()
{
  frag_tex_coords = in_tex_coords;
  vec4 out_pos = vec4(in_position, 1.0f);
  gl_Position = vec4(out_pos.xyz, 1.0);
}