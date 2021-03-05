#include <storage/stream_flash.h>

#ifdef CONFIG_STREAM_FLASH_SAVE_PROGRESS
#include <settings/settings.h>


static inline bool progress_enabled(struct stream_flash_ctx *ctx)
{
	return ctx->progress_key != NULL;
}

static int settings_direct_loader(const char *key, size_t len,
				  settings_read_cb read_cb, void *cb_arg,
				  void *param)
{
	struct stream_flash_ctx *ctx = (struct stream_flash_ctx *) param;

	/* Handle the subtree if it is an exact key match. */
	if (progress_enabled(ctx) && settings_name_next(key, NULL) == 0) {
		ssize_t len = read_cb(cb_arg, &ctx->bytes_written,
				      sizeof(ctx->bytes_written));

		if (len != sizeof(ctx->bytes_written)) {
			LOG_ERR("Unable to read bytes_written from storage");
			return len;
		}

#ifdef CONFIG_STREAM_FLASH_ERASE
		int rc;
		struct flash_pages_info page;
		int abs_offset = ctx->offset + ctx->bytes_written;
		
		rc = flash_get_page_info_by_offs(ctx->fdev, abs_offset, &page);
		if (rc != 0) {
			LOG_ERR("Error %d while getting page info", rc);
			return rc;
		}

		/* Update the last erased page to avoid deleting already
		 * written data.
		 */
		ctx->last_erased_page_start_offset = page.start_offset;

#endif /* CONFIG_STREAM_FLASH_ERASE */
	}

	return 0;
}

static int progress_save(struct stream_flash_ctx *ctx)
{
	int rc;

	if (!progress_enabled(ctx)) {
		return -EFAULT;
	}

	rc = settings_save_one(ctx->progress_key, &ctx->bytes_written,
		sizeof(ctx->bytes_written));
	if (rc != 0) {
		LOG_ERR("Error %d while storing progress", rc);
	}

	return rc;
}

static int progress_load(struct stream_flash_ctx *ctx)
{
	int rc;

	if (!progress_enabled(ctx)) {
		return -EFAULT;
	}
	rc = settings_load_subtree_direct(ctx->progress_key,
		settings_direct_loader, (void *) ctx);
	if (rc != 0) {
		LOG_ERR("Error %d while loading progress", rc);
	}

	return rc;
}

#endif /* CONFIG_STREAM_FLASH_SAVE_PROGRESS */

#if 0
/* stream_flash_buffered_write()... */


#ifdef CONFIG_STREAM_FLASH_SAVE_PROGRESS
	if (rc == 0) {
		/* FIXME: note to self: error handling */
		rc = progress_save(ctx);
	}
#endif

#endif

#if 0

/* stream_flash_init */


#ifdef CONFIG_STREAM_FLASH_SAVE_PROGRESS
	int rc = settings_subsys_init();
	if (rc != 0) {
		LOG_ERR("settings_subsys_init failed: %d", rc);
		return rc;
	}
#endif

#endif


#if 0

#ifdef CONFIG_STREAM_FLASH_SAVE_PROGRESS
	ctx->progress_key = id;
	/* FIXME: Should we have automatic progress load? */
	if (id) {
		return progress_load(ctx);
	}
#endif


#endif

#if 0


#endif


#if 0

/* FIXME: is this API necessary? */
#ifdef CONFIG_STREAM_FLASH_SAVE_PROGRESS

int stream_flash_progress_load(struct stream_flash_ctx *ctx)
{
	if (!ctx) {
		return -EFAULT;
	}
	return progress_load(ctx);
}

int stream_flash_progress_save(struct stream_flash_ctx *ctx)
{
	if (!ctx) {
		return -EFAULT;
	}
	return progress_save(ctx);
}

#endif  /* CONFIG_STREAM_FLASH_SAVE_PROGRESS */
#endif
