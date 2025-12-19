/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>

// #define CYCLES_TO_TEST 1000000
#define CYCLES_TO_TEST  2
#define CYCLE_STAT_FREQ 1

int main(void)
{
	const struct device *mram = DEVICE_DT_GET(DT_NODELABEL(mram1x));

	const uint32_t checker_pattern[4] = {0x55555555UL, 0xAAAAAAAAUL, 0x55555555UL,
					     0xAAAAAAAAUL};
	const uint32_t inverse_checker_pattern[4] = {0xAAAAAAAAUL, 0x55555555UL, 0xAAAAAAAAUL,
						     0x55555555UL};

	uint32_t readback_pattern[4];

	__ASSERT_NO_MSG(device_is_ready(mram));

	/* First address of the application MRAM region */
	const off_t start_offset = DT_REG_ADDR(DT_NODELABEL(cpuapp_boot_partition));
	uint64_t mram_size = 0;

	flash_get_size(mram, &mram_size);

	printk("Using MRAM offsets: [0x%lx, 0x%llx)\n", start_offset, mram_size);

	__ASSERT((mram_size - start_offset) % sizeof(checker_pattern) == 0,
		 "expected write area to be some multiple of the pattern size");

	int err;
	uint64_t cycle_num = 0;
	uint32_t start_time_us = 0;

	for (cycle_num = 0; cycle_num < CYCLES_TO_TEST; cycle_num++) {
		if (cycle_num % CYCLE_STAT_FREQ == 0) {
			printk("Cycle %llu\n", cycle_num);
			start_time_us = k_cyc_to_us_near32(sys_clock_cycle_get_32());
		}

		/* 4B writes */
		for (off_t offset = start_offset; offset < mram_size;
		     offset += sizeof(checker_pattern)) {
			for (size_t i = 0; i < sizeof(checker_pattern) / sizeof(uint32_t); i++) {
				sys_write32(checker_pattern[i],
					    0x0E000000 + offset + i * sizeof(uint32_t));
			}

			for (size_t i = 0; i < sizeof(checker_pattern) / sizeof(uint32_t); i++) {
				readback_pattern[i] =
					sys_read32(0x0E000000 + offset + i * sizeof(uint32_t));
			}

			/* TODO: probably need to check this in a custom bus fault handler instead
			 */
			if (memcmp(checker_pattern, readback_pattern, sizeof(checker_pattern)) !=
			    0) {
			}
		}

		/* flash api */
		for (off_t offset = start_offset; offset < mram_size;
		     offset += sizeof(inverse_checker_pattern)) {
			err = flash_write(mram, offset, inverse_checker_pattern,
					  sizeof(inverse_checker_pattern));
			__ASSERT_NO_MSG(err == 0);

			err = flash_read(mram, offset, readback_pattern, sizeof(readback_pattern));
			__ASSERT_NO_MSG(err == 0);

			/* TODO: probably need to check this in a custom bus fault handler instead
			 */
			if (memcmp(inverse_checker_pattern, readback_pattern,
				   sizeof(inverse_checker_pattern)) != 0) {
			}
		}

		if ((cycle_num + 1) % CYCLE_STAT_FREQ == 0) {
			const uint32_t end_time_us = k_cyc_to_us_near32(sys_clock_cycle_get_32());
			const uint32_t time_diff_us = end_time_us - start_time_us;
			const uint32_t time_diff_per_cycle_us = time_diff_us / CYCLE_STAT_FREQ;

			printk("Time delta for %d cycles: %u\n", CYCLE_STAT_FREQ, time_diff_us);
			printk("Time delta per cycle: %u\n", time_diff_per_cycle_us);

			start_time_us = k_cyc_to_us_near32(sys_clock_cycle_get_32());
		}
	}

	printk("Finished %llu cycles\n", cycle_num);

	return 0;
}
