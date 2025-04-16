# Copyright (c) 2025 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_NRF_PERIPHCONF_SECTION)
  zephyr_linker_sources(SECTIONS uicr.ld)
endif()

if(CONFIG_NRF_HALTIUM_GENERATE_UICR)
  set(uicr_hex_file ${PROJECT_BINARY_DIR}/uicr.hex)
  set(periphconf_hex_file ${PROJECT_BINARY_DIR}/periphconf.hex)

  set(uicrgen_command
    ${CMAKE_COMMAND} -E env PYTHONPATH=${ZEPHYR_BASE}/scripts/dts/python-devicetree/src
    ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/uicrgen.py
    --in-config ${DOTCONFIG}
    --in-edt-pickle ${EDT_PICKLE}
    --in-periphconf-elf $<TARGET_FILE:${ZEPHYR_LINK_STAGE_EXECUTABLE}>
    --out-uicr-hex ${uicr_hex_file}
    --out-periphconf-hex ${periphconf_hex_file}
  )

  if(CONFIG_NRF_HALTIUM_UICR_INCLUDE_IN_IMAGE)
    set(uicr_source_file ${PROJECT_BINARY_DIR}/uicr_generated.c)
    add_custom_command(
      OUTPUT ${uicr_hex_file} ${periphconf_hex_file} ${uicr_source_file}
      COMMAND ${uicrgen_command} --out-uicr-source ${uicr_source_file}
      DEPENDS ${ZEPHYR_LINK_STAGE_EXECUTABLE}
    )
    add_library(generate_uicr_lib OBJECT ${uicr_source_file})
    target_link_libraries(generate_uicr_lib PRIVATE zephyr_interface)

    set_property(
      GLOBAL APPEND
      PROPERTY GENERATED_KERNEL_OBJECT_FILES $<TARGET_OBJECTS:generate_uicr_lib>
    )
  else()
    set_property(
      GLOBAL APPEND
      PROPERTY extra_post_build_commands COMMAND ${uicrgen_command}
    )
  endif()
endif()
