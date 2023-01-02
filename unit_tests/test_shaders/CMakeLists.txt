add_custom_target(shader_compilation)
include(CompileShader)

set(shader_srcs
  alpha.vert
  alpha.frag
  draw.vert
  draw.frag
  simple.vert
  simple.frag
  g_buffer.vert
  g_buffer.frag
  deferred.vert
  deferred.frag
)
compile_shader(TARGETS ${shader_srcs})