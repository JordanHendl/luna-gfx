#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out vec2 frag_coord;

layout( binding = 10 ) uniform Transformations { 
float rotation;
} transform;

 
mat4 rotationX( in float angle ) {
	return mat4(	1.0,		0,			0,			0,
			 		0, 	cos(angle),	-sin(angle),		0,
					0, 	sin(angle),	 cos(angle),		0,
					0, 			0,			  0, 		1);
}

mat4 rotationY( in float angle ) {
	return mat4(	cos(angle),		0,		sin(angle),	0,
			 				0,		1.0,			 0,	0,
					-sin(angle),	0,		cos(angle),	0,
							0, 		0,				0,	1);
}

mat4 rotationZ( in float angle ) {
	return mat4(	cos(angle),		-sin(angle),	0,	0,
			 		sin(angle),		cos(angle),		0,	0,
							0,				0,		1,	0,
							0,				0,		0,	1);
}

void main()
{
  frag_normal = in_normal;
  frag_coord = in_tex_coord;

  vec4 out_pos = vec4(in_position, 1) * rotationX(0.25f) * rotationY(transform.rotation) * rotationZ(transform.rotation);
  gl_Position = vec4(out_pos.x, out_pos.y, out_pos.z +10, 1.0);
}
