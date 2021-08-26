set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

if("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "lib")
  set(CPACK_PACKAGE_NAME "libgtirb-functions")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions")
  set(CPACK_COMPONENTS_ALL library)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtirb (>=1.10.0)")
  set(CPACK_RPM_PACKAGE_REQUIRES "libgtirb.so")

  # do not use a sepate package for debug info in case of RPM
  set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
  set(CPACK_RPM_DEBUGINFO_FILE_NAME "libgtirb-functions-dbg.rpm")
elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "lib-dbg")
  # this debug info pacakge applies only to Debian generator
  set(CPACK_PACKAGE_NAME "libgtirb-functions-dbg")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions-dbg")
  set(CPACK_COMPONENTS_ALL GTIRB_FUNCTIONS-debug-file)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libgtirb-functions (=${CPACK_GTIRB_FUNCTIONS_VERSION})")

elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "dev")
  set(CPACK_PACKAGE_NAME "libgtirb-functions-dev")
  set(CPACK_PACKAGE_FILE_NAME "libgtirb-functions-dev")
  set(CPACK_COMPONENTS_ALL headers cmake_config cmake_target)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libgtirb-dev, libgtirb-functions (=${CPACK_GTIRB_FUNCTIONS_VERSION})")
  set(CPACK_RPM_PACKAGE_REQUIRES
      "libgtirb-functions = ${CPACK_GTIRB_FUNCTIONS_VERSION}")

elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "driver")
  set(CPACK_PACKAGE_NAME "gtirb-functions-driver")
  set(CPACK_PACKAGE_FILE_NAME "gtirb-functions-driver")
  set(CPACK_COMPONENTS_ALL driver)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libstdc++6, libc6, libgcc1, libgtirb (>=1.10.0), libgtirb-functions (=${CPACK_GTIRB_FUNCTIONS_VERSION})"
  )
  set(CPACK_RPM_PACKAGE_REQUIRES
      "libstdc++.so.6, libc.so.6, libgcc_s.so.1, libgtirb-functions = ${CPACK_GTIRB_FUNCTIONS_VERSION}"
  )

  # do not use a sepate package for debug info in case of RPM
  set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
  set(CPACK_RPM_DEBUGINFO_FILE_NAME "gtirb-functions-driver-dbg.rpm")
elseif("${CPACK_GTIRB_FUNCTIONS_PACKAGE}" STREQUAL "driver-dbg")
  # this debug info pacakge applies only to Debian generator
  set(CPACK_PACKAGE_NAME "gtirb-functions-driver-dbg")
  set(CPACK_PACKAGE_FILE_NAME "gtirb-functions-driver-dbg")
  set(CPACK_COMPONENTS_ALL gtirb-functions-driver-debug-file)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "gtirb-functions-driver (=${CPACK_GTIRB_FUNCTIONS_VERSION})")
endif()
