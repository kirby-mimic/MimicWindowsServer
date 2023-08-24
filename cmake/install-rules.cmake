install(
    TARGETS MimicWindowsServer_exe
    RUNTIME COMPONENT MimicWindowsServer_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
