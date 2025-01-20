function(${SYSBUILD_CURRENT_MODULE_NAME}_post_cmake)
  cmake_parse_arguments(POST_CMAKE "" "" "IMAGES" ${ARGN})

  if(SB_CONFIG_SOC_SERIES_NRF54HX OR SB_CONFIG_SOC_SERIES_NRF92X)
    if(NOT NRF_REGTOOL)
      message(WARNING "nrf-regtool not found - skipping UICR validation")
      return()
    endif()

    foreach(image ${IMAGES})
      unset(has_uicr)
      sysbuild_get(has_uicr IMAGE ${image} VAR CONFIG_NRF_REGTOOL_GENERATE_UICR KCONFIG)
      if(NOT has_uicr)
        continue()
      endif()
      sysbuild_get(image_BINARY_DIR IMAGE ${image} VAR APPLICATION_BINARY_DIR CACHE)
      set(uicr_file ${image_BINARY_DIR}/zephyr/uicr.hex)
      set(debug_file ${uicr_file}.debug.json)
      list(APPEND uicr_target_values "uicr-file=${uicr_file},debug-file=${debug_file}")
    endforeach()

    if(uicr_target_values)
      list(TRANSFORM uicr_target_values PREPEND "--target;" OUTPUT_VARIABLE uicr_targets)
      set(report_file ${APPLICATION_BINARY_DIR}/uicr_report.json)

      execute_process(
        COMMAND
        ${NRF_REGTOOL} uicr-validate
        ${uicr_targets}
        --output-file ${report_file}
        WORKING_DIRECTORY ${BINARY_DIR}
        ERROR_VARIABLE validate_stderr
        RESULT_VARIABLE validate_exit_code
      )
      if(NOT ("${validate_exit_code}" STREQUAL "0"))
        message(FATAL_ERROR
          "UICR validation failed with return code: ${validate_exit_code}. "
          "Error JSON report written to ${report_file}"
        )
      endif()
    endif()
  endif()
endfunction()
