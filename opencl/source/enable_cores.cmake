#
# Copyright (C) 2018-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(RUNTIME_SRCS_COREX_CPP_BASE
    buffer
    cl_hw_helper
    command_queue
    gpgpu_walker
    hardware_commands_helper
    image
    sampler
)

macro(macro_for_each_core_type)
  foreach(BRANCH_DIR ${BRANCH_DIR_LIST})
    set(COREX_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}${BRANCH_DIR}${CORE_TYPE_LOWER})
    # Add default CORE files

    if(EXISTS "${COREX_PREFIX}/additional_files_${CORE_TYPE_LOWER}.cmake")
      include("${COREX_PREFIX}/additional_files_${CORE_TYPE_LOWER}.cmake")
    endif()

    if(${SUPPORT_DEVICE_ENQUEUE_${CORE_TYPE}})
      if(EXISTS ${COREX_PREFIX}/device_enqueue.h)
        list(APPEND RUNTIME_SRCS_${CORE_TYPE}_H_BASE ${COREX_PREFIX}/device_enqueue.h)
      endif()
      if(EXISTS ${COREX_PREFIX}/scheduler_definitions.h)
        list(APPEND RUNTIME_SRCS_${CORE_TYPE}_H_BASE ${COREX_PREFIX}/scheduler_definitions.h)
      endif()
      if(EXISTS ${COREX_PREFIX}/scheduler_builtin_kernel.inl)
        list(APPEND RUNTIME_SRCS_${CORE_TYPE}_H_BASE ${COREX_PREFIX}/scheduler_builtin_kernel.inl)
      endif()
      if(EXISTS ${COREX_PREFIX}/device_queue_${CORE_TYPE_LOWER}.cpp)
        list(APPEND RUNTIME_SRCS_${CORE_TYPE}_CPP_BASE ${COREX_PREFIX}/device_queue_${CORE_TYPE_LOWER}.cpp)
      endif()
    endif()

    foreach(SRC_IT ${RUNTIME_SRCS_COREX_CPP_BASE})
      if(EXISTS ${COREX_PREFIX}/${SRC_IT}_${CORE_TYPE_LOWER}.cpp)
        list(APPEND RUNTIME_SRCS_${CORE_TYPE}_CPP_BASE ${COREX_PREFIX}/${SRC_IT}_${CORE_TYPE_LOWER}.cpp)
      endif()
    endforeach()

    if(EXISTS ${COREX_PREFIX}/enable_family_full_ocl_${CORE_TYPE_LOWER}.cpp)
      list(APPEND ${CORE_TYPE}_SRC_LINK_BASE ${COREX_PREFIX}/enable_family_full_ocl_${CORE_TYPE_LOWER}.cpp)
    endif()
    if(NOT DISABLED_GTPIN_SUPPORT)
      if(EXISTS ${COREX_PREFIX}/gtpin_setup_${CORE_TYPE_LOWER}.cpp)
        list(APPEND ${CORE_TYPE}_SRC_LINK_BASE ${COREX_PREFIX}/gtpin_setup_${CORE_TYPE_LOWER}.cpp)
      endif()
    endif()

    list(APPEND RUNTIME_SRCS_COREX_ALL_BASE ${RUNTIME_SRCS_${CORE_TYPE}_H_BASE})
    list(APPEND RUNTIME_SRCS_COREX_ALL_BASE ${RUNTIME_SRCS_${CORE_TYPE}_CPP_BASE})
    list(APPEND HW_SRC_LINK ${${CORE_TYPE}_SRC_LINK_BASE})
    list(APPEND RUNTIME_SRCS_COREX_ALL_WINDOWS ${RUNTIME_SRCS_${CORE_TYPE}_CPP_WINDOWS})
    list(APPEND RUNTIME_SRCS_COREX_ALL_LINUX ${RUNTIME_SRCS_${CORE_TYPE}_CPP_LINUX})
    if(UNIX)
      list(APPEND HW_SRC_LINK ${${CORE_TYPE}_SRC_LINK_LINUX})
    endif()
  endforeach()
endmacro()

get_property(RUNTIME_SRCS_COREX_ALL_BASE GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_BASE)
get_property(RUNTIME_SRCS_COREX_ALL_LINUX GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_LINUX)
get_property(RUNTIME_SRCS_COREX_ALL_WINDOWS GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_WINDOWS)

apply_macro_for_each_core_type("SUPPORTED")

target_sources(${NEO_STATIC_LIB_NAME} PRIVATE ${RUNTIME_SRCS_COREX_ALL_BASE})
if(WIN32)
  target_sources(${NEO_STATIC_LIB_NAME} PRIVATE ${RUNTIME_SRCS_COREX_ALL_WINDOWS})
else()
  target_sources(${NEO_STATIC_LIB_NAME} PRIVATE ${RUNTIME_SRCS_COREX_ALL_LINUX})
endif()

set_property(GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_BASE ${RUNTIME_SRCS_COREX_ALL_BASE})
set_property(GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_LINUX ${RUNTIME_SRCS_COREX_ALL_LINUX})
set_property(GLOBAL PROPERTY RUNTIME_SRCS_COREX_ALL_WINDOWS ${RUNTIME_SRCS_COREX_ALL_WINDOWS})
