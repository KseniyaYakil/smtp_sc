#ifndef _BUF_H_
#define _BUF_H_

#include <stdint.h>
#include <inttypes.h>

#define SWAP_VAL(v1_, v2_) ({		\
	typeof(v1_) v_tmp_ = v1_;	\
	v1_ = v2_;			\
	v2_ = v_tmp_;			\
})

struct buf {
	char *data;
	uint32_t len; // used
	uint32_t size; //alloced size
};

#define BUF_STATIC_INITIALIZER() (struct buf) {	\
	.data = NULL,				\
	.len = 0,				\
	.size = 0				\
}

void buf_init(struct buf *buf, uint32_t prealloc);
void buf_reset(struct buf *buf);
void buf_ensure(struct buf *buf, uint32_t len);
void buf_append(struct buf *buf, const char *data, uint32_t len);
void buf_swap(struct buf *b1, struct buf *b2);
char *buf_get_data(struct buf *buf);
uint32_t buf_get_len(struct buf *buf);
int buf_copy(struct buf *buf, char **data_p, uint32_t *len);
int buf_copy_tail(struct buf *buf, char *str, int len);
void buf_move(struct buf *buf, int cnt);
void buf_free(struct buf *buf);

#endif
