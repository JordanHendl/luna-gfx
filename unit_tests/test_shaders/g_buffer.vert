#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coords;

layout (location = 0) out vec3 frag_position;
layout (location = 1) out vec2 frag_tex_coords;
layout (location = 2) out vec3 frag_normal;

layout(binding = 0) uniform Camera {
  mat4 view;
  vec3 position;
} camera;

layout(binding = 1) uniform Transformations { 
  mat4 transform;
} transform;

void main()
{
  vec4 world_pos = transform.transform * vec4(in_pos, 1.0);
  frag_position = world_pos.xyz; 
  frag_tex_coords = in_tex_coords;
  mat3 normal_matrix = transpose(inverse(mat3(transform.transform)));
  frag_normal = normal_matrix * in_normal;
  frag_normal = in_normal;
  gl_Position = camera.view * world_pos;
}