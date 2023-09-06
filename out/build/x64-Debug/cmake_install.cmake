# Install script for directory: C:/Users/admin/Desktop/Jaouad Stage/SOEM-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/soem.lib")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake"
         "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/CMakeFiles/Export/share/soem/cmake/soemConfig.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/soem/cmake/soemConfig.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/CMakeFiles/Export/share/soem/cmake/soemConfig.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/soem/cmake" TYPE FILE FILES "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/CMakeFiles/Export/share/soem/cmake/soemConfig-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soem" TYPE FILE FILES
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercat.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatbase.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatcoe.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatconfig.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatconfiglist.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatdc.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercateoe.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatfoe.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatmain.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatprint.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercatsoe.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/soem/ethercattype.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/osal/osal.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/osal/win32/inttypes.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/osal/win32/osal_defs.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/osal/win32/osal_win32.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/osal/win32/stdint.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/oshw/win32/nicdrv.h"
    "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/oshw/win32/oshw.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/test/linux/slaveinfo/cmake_install.cmake")
  include("C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/test/linux/eepromtool/cmake_install.cmake")
  include("C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/test/linux/simple_test/cmake_install.cmake")
  include("C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/master/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/admin/Desktop/Jaouad Stage/SOEM-master/out/build/x64-Debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
