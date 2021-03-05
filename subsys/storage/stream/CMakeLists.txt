#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

zephyr_sources(stream_flash.c)
zephyr_sources_ifdef(CONFIG_STREAM_FLASH_VOLATILE stream_flash_volatile.c)
zephyr_sources_ifdef(CONFIG_STREAM_FLASH_NV stream_flash_nv.c)
