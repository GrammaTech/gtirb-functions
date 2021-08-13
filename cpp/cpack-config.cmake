set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

if("${CPACK_HELLO_PACKAGE}" STREQUAL "lib")
  set(CPACK_PACKAGE_NAME "libhello")
  set(CPACK_PACKAGE_FILE_NAME "libhello")
  set(CPACK_COMPONENTS_ALL library)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libc6, libgcc1")
  set(CPACK_RPM_PACKAGE_REQUIRES "libstdc++.so.6, libc.so.6, libgcc_s.so.1")

  # do not use a sepate package for debug info in case of RPM
  set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
  set(CPACK_RPM_DEBUGINFO_FILE_NAME "libhello-dbg.rpm")
elseif("${CPACK_HELLO_PACKAGE}" STREQUAL "lib-dbg")
  # this debug info pacakge applies only to Debian generator
  set(CPACK_PACKAGE_NAME "libhello-dbg")
  set(CPACK_PACKAGE_FILE_NAME "libhello-dbg")
  set(CPACK_COMPONENTS_ALL hello-debug-file)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libhello (=${CPACK_HELLO_VERSION})")
elseif("${CPACK_HELLO_PACKAGE}" STREQUAL "dev")
  set(CPACK_PACKAGE_NAME "libhello-dev")
  set(CPACK_PACKAGE_FILE_NAME "libhello-dev")
  set(CPACK_COMPONENTS_ALL headers cmake_config cmake_target)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libstdc++6, libc6, libgcc1, libhello (=${CPACK_HELLO_VERSION})")
  set(CPACK_RPM_PACKAGE_REQUIRES
      "libstdc++.so.6, libc.so.6, libgcc_s.so.1, libhello = ${CPACK_HELLO_VERSION}"
  )
elseif("${CPACK_HELLO_PACKAGE}" STREQUAL "driver")
  set(CPACK_PACKAGE_NAME "hello-driver")
  set(CPACK_PACKAGE_FILE_NAME "hello-driver")
  set(CPACK_COMPONENTS_ALL driver)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libstdc++6, libc6, libgcc1, libhello (=${CPACK_HELLO_VERSION})")
  set(CPACK_RPM_PACKAGE_REQUIRES
      "libstdc++.so.6, libc.so.6, libgcc_s.so.1, libhello = ${CPACK_HELLO_VERSION}"
  )

  # do not use a sepate package for debug info in case of RPM
  set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
  set(CPACK_RPM_DEBUGINFO_FILE_NAME "hello-driver-dbg.rpm")
elseif("${CPACK_HELLO_PACKAGE}" STREQUAL "driver-dbg")
  # this debug info pacakge applies only to Debian generator
  set(CPACK_PACKAGE_NAME "hello-driver-dbg")
  set(CPACK_PACKAGE_FILE_NAME "hello-driver-dbg")
  set(CPACK_COMPONENTS_ALL hello-driver-debug-file)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "hello-driver (=${CPACK_HELLO_VERSION})")
endif()
