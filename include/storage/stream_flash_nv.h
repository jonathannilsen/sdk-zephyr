#ifndef ZEPHYR_INCLUDE_STORAGE_STREAM_FLASH_NV_H_
#define ZEPHYR_INCLUDE_STORAGE_STREAM_FLASH_NV_H_

#include <storage/stream_flash.h>


struct stream_flash_nv_ctx {
	struct stream_flash_ctx sf_ctx;
	const char *progress_key;
};


int stream_flash_nv_init(struct stream_flash_nv_ctx *ctx,
			 const struct device *fdev, uint8_t *buf,
			 size_t buf_len, size_t offset, size_t size,
			 stream_flash_callback_t cb, const char *id);

int stream_flash_nv_buffered_write(struct stream_flash_nv_ctx *ctx,
				   const uint8_t *data,
				   size_t len, bool flush);

static inline size_t stream_flash_nv_bytes_written(
	struct stream_flash_nv_ctx *ctx)
{
	if (!ctx || !ctx->progress_key) {
		return -EFAULT;
	}

	return stream_flash_bytes_written(&ctx->sf_ctx);
}

static inline int stream_flash_nv_erase_page(struct stream_flash_nv_ctx *ctx,
					     off_t off)
{
	if (!ctx || !ctx->progress_key) {
		return -EFAULT;
	}

	return stream_flash_erase_page(&ctx->sf_ctx, off);
}

int stream_flash_nv_finish(struct stream_flash_nv_ctx *ctx, bool flush,
			   bool clear_progress);


#endif /* ZEPHYR_INCLUDE_STORAGE_STREAM_FLASH_NV_H_ */
