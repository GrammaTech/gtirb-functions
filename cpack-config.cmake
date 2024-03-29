set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtirb (=${CPACK_GTIRB_VERSION})")

if("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "dev")
  set(CPACK_PACKAGE_NAME "libgtirb-functions-dev")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions-dev")
  set(CPACK_COMPONENTS_ALL headers cmake_config cmake_target)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libgtirb-dev (=${CPACK_GTIRB_VERSION}-${CPACK_UBUNTU_NAME})")

  set(CPACK_RPM_PACKAGE_NAME "libgtirb-functions-devel")
  set(CPACK_RPM_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}.rpm")
  set(CPACK_RPM_PACKAGE_REQUIRES "libgtirb-devel = ${CPACK_GTIRB_VERSION}")

endif()
