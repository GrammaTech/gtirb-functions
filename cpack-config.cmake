set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

set(CPACK_DEBEIAN_PACKAGE_DEPENDS "libgtirb-${CPACK_GTIRB_VERSION}")

if("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "lib")
  set(CPACK_PACKAGE_NAME "libgtirb-functions")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions")
  set(CPACK_COMPONENTS_ALL library)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtirb-${CPACK_GTIRB_VERSION}")
  set(CPACK_RPM_PACKAGE_REQUIRES "libgtirb.so.${CPACK_GTIRB_VERSION}")

  # do not use a sepate package for debug info in case of RPM
  set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
  set(CPACK_RPM_DEBUGINFO_FILE_NAME "libgtirb-functions-dbg.rpm")

elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "lib-dbg")
  # this debug info pacakge applies only to Debian generator
  set(CPACK_PACKAGE_NAME "libgtirb-functions-dbg")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions-dbg")
  set(CPACK_COMPONENTS_ALL gtirb-functions-debug-file)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libgtirb-functions (=${CPACK_GTIRB_FUNCTIONS_VERSION})")

elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "dev")
  set(CPACK_PACKAGE_NAME "libgtirb-functions-dev")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions-dev")
  set(CPACK_COMPONENTS_ALL headers cmake_config cmake_target)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libgtirb-dev-${CPACK_GTIRB_VERSION}, libgtirb-functions (=${CPACK_GTIRB_FUNCTIONS_VERSION})"
  )
  message(CPACK_DEBIAN_PACKAGE_DEPENDS=${CPACK_DEBIAN_PACKAGE_DEPENDS})
  set(CPACK_RPM_PACKAGE_REQUIRES
      "libgtirb-dev = ${CPACK_GTIRB_VERSION}, libgtirb-functions = ${CPACK_GTIRB_FUNCTIONS_VERSION}"
  )

endif()
