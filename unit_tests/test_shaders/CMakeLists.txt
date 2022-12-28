include(CompileShader)

set(shader_srcs
  draw.vert
  draw.frag
  simple.vert
  simple.frag
)
compile_shader(TARGETS ${shader_srcs})