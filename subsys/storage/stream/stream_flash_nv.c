#include <storage/stream_flash_nv.h>
#include <settings/settings.h>

#define LOG_MODULE_NAME STREAM_FLASH
#define LOG_LEVEL CONFIG_STREAM_FLASH_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_DECLARE(LOG_MODULE_NAME, LOG_LEVEL);

static inline bool progress_enabled(struct stream_flash_nv_ctx *ctx)
{
	return ctx->progress_key != NULL;
}

static int settings_direct_loader(const char *key, size_t len,
				  settings_read_cb read_cb, void *cb_arg,
				  void *param)
{
	struct stream_flash_nv_ctx *ctx = (struct stream_flash_ctx *) param;
	struct stream_flash_ctx *sf_ctx = &ctx->sf_ctx;

	/* Handle the subtree if it is an exact key match. */
	if (progress_enabled(ctx) && settings_name_next(key, NULL) == 0) {
		ssize_t len = read_cb(cb_arg, &sf_ctx->bytes_written,
				      sizeof(sf_ctx->bytes_written));

		if (len != sizeof(sf_ctx->bytes_written)) {
			LOG_ERR("Unable to read bytes_written from storage");
			return len;
		}

#ifdef CONFIG_STREAM_FLASH_ERASE
		int rc;
		struct flash_pages_info page;
		int abs_offset = sf_ctx->offset + sf_ctx->bytes_written;
		
		rc = flash_get_page_info_by_offs(sf_ctx->fdev, abs_offset,
						 &page);
		if (rc != 0) {
			LOG_ERR("Error %d while getting page info", rc);
			return rc;
		}

		/* Update the last erased page to avoid deleting already
		 * written data.
		 */
		sf_ctx->last_erased_page_start_offset = page.start_offset;

#endif /* CONFIG_STREAM_FLASH_ERASE */
	}

	return 0;
}

static int progress_save(struct stream_flash_nv_ctx *ctx)
{
	int rc = settings_save_one(ctx->progress_key,
		&ctx->sf_ctx.bytes_written, sizeof(ctx->sf_ctx.bytes_written));
	if (rc != 0) {
		LOG_ERR("Error %d while storing progress", rc);
	}

	return rc;
}

static int progress_load(struct stream_flash_nv_ctx *ctx)
{
	int rc = settings_load_subtree_direct(ctx->progress_key,
		settings_direct_loader, (void *) ctx);
	if (rc != 0) {
		LOG_ERR("Error %d while loading progress", rc);
	}

	return rc;
}

static int progress_clear(struct stream_flash_nv_ctx *ctx)
{
	int rc = settings_delete(ctx->progress_key);
	if (rc != 0) {
		LOG_ERR("Error %d while deleting progress", rc);
	}

	return rc;
}

int stream_flash_nv_init(struct stream_flash_nv_ctx *ctx,
			 const struct device *fdev, uint8_t *buf,
			 size_t buf_len, size_t offset, size_t size,
			 stream_flash_callback_t cb, const char *id)
{
	if (!ctx) {
		return -EFAULT;
	}

	int rc = settings_subsys_init();
	if (rc != 0) {
		LOG_ERR("settings_subsys_init failed: %d", rc);
		return rc;
	}
	
	rc = stream_flash_init(&ctx->sf_ctx, fdev, buf, buf_len, offset, cb);
	if (rc == 0) {
		ctx->progress_key = id;
		if (progress_enabled(ctx)) {
			rc = progress_load(ctx);
		}
	}

	return rc;
}

int stream_flash_nv_buffered_write(struct stream_flash_nv_ctx *ctx,
				   const uint8_t *data,
				   size_t len, bool flush)
{
	if (!ctx) {
		return -EFAULT;
	}

	int rc = stream_flash_buffered_write(&ctx->sf_ctx, data, len, flush);
	
	if (rc == 0 && progress_enabled(ctx)) {
		rc = progress_save(ctx);
	}

	return rc;
}

int stream_flash_nv_finish(struct stream_flash_nv_ctx *ctx, bool clear_progress)
{
	int rc;

	if (!ctx) {
		return -EFAULT;
	}

	if (clear_progress && progress_enabled(ctx)) {
		rc = progress_clear(ctx);
	} else {
		rc = progress_save(ctx);
	}
	ctx->progress_key = NULL;

	return rc;
}
