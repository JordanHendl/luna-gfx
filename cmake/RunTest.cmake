macro(RunTestsAfterBuild)
  add_custom_target(LunaGfxTests)
  add_custom_command(TARGET LunaGfxTests
                     POST_BUILD
                     COMMAND ctest -C $<CONFIGURATION> --output-on-failure -j12)
endmacro()