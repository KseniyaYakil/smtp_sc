#include "buf.h"
#include "server_types.h"

// for buf ensuring
static uint32_t low_limit = 256;
static int growth = 2;

void buf_init(struct buf *buf, uint32_t prealloc)
{
	prealloc = prealloc == 0 ? prealloc :
		   prealloc > low_limit ? prealloc : low_limit;

	buf->data = prealloc == 0 ? NULL : malloc(prealloc);
	buf->len = 0;
	buf->size = prealloc;
}

void buf_reset(struct buf *buf)
{
	buf->len = 0;
}

void buf_ensure(struct buf *buf, uint32_t len)
{
	if (len == 0)
		len = low_limit;

	if (buf->size == 0) {
		buf->data = (char *)malloc(len);
		if (buf->data == NULL) {
			slog_e("%s", "no mem");
			abort();
		}
		buf->size = len;

		return;
	}

	if ((buf->size - buf->len) < len) {
		uint32_t need_mem = len - buf->size + buf->len;

		if (need_mem < buf->size)
			need_mem = buf->size * growth;

		buf->data = realloc(buf->data, need_mem);
		if (buf->data == NULL) {
			slog_e("%s", "no mem");
			abort();
		}
		buf->size = need_mem;
	}
}

char *buf_get_data(struct buf *buf)
{
	return buf->data;
}

uint32_t buf_get_len(struct buf *buf)
{
	return buf->len;
}

void buf_swap(struct buf *b1, struct buf *b2)
{
	SWAP_VAL(*b1, *b2);
}

void buf_free(struct buf *buf)
{
	if (buf->data != NULL)
		free(buf->data);

	buf->len = buf->size = 0;
}

void buf_append(struct buf *buf, const char *data, uint32_t len)
{
	buf_ensure(buf, len);
	memcpy(buf->data + buf->len, data, len);
	buf->len += len;
}

int buf_copy(struct buf *buf, char **data_p, uint32_t *len)
{
	*data_p = NULL;
	*len = 0;

	if (buf_get_len(buf) == 0)
		return 0;

	char *data = strndup(buf_get_data(buf), buf_get_len(buf));
	if (data == NULL) {
		slog_e("%s", "no mem");
		abort();
	}

	*data_p = data;
	*len = buf_get_len(buf);

	return 0;
}

void buf_move(struct buf *buf, int cnt)
{
	if (buf->len < cnt)
		return;

	if (buf->len == cnt)
		return buf_reset(buf);

	memmove(buf->data, buf->data + cnt, buf->len - cnt);
}


