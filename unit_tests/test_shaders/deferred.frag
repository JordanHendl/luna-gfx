#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

layout(location = 0) in vec2 frag_tex_coords;
layout(location = 0) out vec4 out_color;

layout(binding = 10) uniform sampler2D g_position;
layout(binding = 11) uniform sampler2D g_normal;
layout(binding = 12) uniform sampler2D g_albedo_spec;

layout(binding = 13) uniform Camera {
  mat4 view;
  vec3 pos;
} camera;

layout(binding = 14) uniform Switch {
  int which_layer;
} layer;

struct Light {
    vec3 position;
    vec3 color;
    
    float linear;
    float quadratic;
    float radius;
} base_light;

vec3 calculate_lighting() {
    // retrieve data from gbuffer
    vec3 frag_pos = texture(g_position, frag_tex_coords).rgb;
    vec3 frag_normal = texture(g_normal, frag_tex_coords).rgb;
    vec3 t_diffuse = texture(g_albedo_spec, frag_tex_coords).rgb;
    float t_specular = texture(g_albedo_spec, frag_tex_coords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = t_diffuse * 0.1; // hard-coded ambient component
    vec3 view_dir  = normalize(camera.pos - frag_pos);

  // calculate distance between light source and current fragment
  float distance = length(base_light.position - frag_pos);
  if(distance < base_light.radius) {
      vec3 light_dir = normalize(base_light.position - frag_pos);
      vec3 diffuse = max(dot(frag_normal, light_dir), 0.0) * t_diffuse * base_light.color;
      vec3 halfway_dir = normalize(light_dir + view_dir);  
      float spec = pow(max(dot(frag_normal, halfway_dir), 0.0), 16.0);
      vec3 specular = base_light.color * spec * t_specular;
      float attenuation = 1.0 / (1.0 + base_light.linear * distance + base_light.quadratic * distance * distance);
      diffuse *= attenuation;
      specular *= attenuation;
      lighting += diffuse + specular;
  }

  return lighting;
}

void main()
{    
  vec4 t_diffuse = texture(g_albedo_spec, frag_tex_coords);
  vec3 lighting = calculate_lighting();
  vec3 color = vec3(0, 0, 0);
  switch(layer.which_layer) {
    case 0: color = texture(g_position, frag_tex_coords).rgb; break;
    case 1: color = texture(g_normal, frag_tex_coords).rgb; break;
    case 2: color = texture(g_albedo_spec, frag_tex_coords).rgb; break;
    case 3: color = vec3(texture(g_albedo_spec, frag_tex_coords).a); break;
    default: color = vec3(1.0f, 1.0f, 1.0f);
  }
  out_color = vec4(color.xyz, 1.0);
}