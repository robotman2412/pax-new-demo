
#include <pax_gfx.h>

#pragma once

#ifdef __cplusplus

#include <vector>
#include <algorithm>

namespace td2 {

#define TD_INTERP_TYPE_INT   0
#define TD_INTERP_TYPE_COL   1
#define TD_INTERP_TYPE_HSV   2
#define TD_INTERP_TYPE_FLOAT 3

#define TD_LINEAR 0
#define TD_EASE 1
#define TD_EASE_IN 2
#define TD_EASE_OUT 3

typedef uint16_t td_delay_t;

struct td_event;
struct td_lerp;
struct td_lerp_list;
struct td_set;
struct td_ctx;
union  td_args;

typedef struct td_event     td_event_t;
typedef struct td_lerp      td_lerp_t;
typedef struct td_lerp_list td_lerp_list_t;
typedef struct td_set       td_set_t;
typedef struct td_ctx       td_ctx_t;
typedef union  td_args      td_args_t;
typedef void (*const td_func_t)(td_ctx_t &ctx, size_t planned_time, size_t planned_duration, const td_args_t *args);

struct td_lerp_list {
	const td_lerp_t *subject;
	uint64_t         start;
	uint64_t         end;
};

struct td_lerp {
	td_delay_t duration;
	union {
		int   *int_ptr;
		float *float_ptr;
	};
	union {
		int    int_from;
		float  float_from;
	};
	union {
		int    int_to;
		float  float_to;
	};
	uint8_t    type;
	uint8_t    timing;
};

struct td_set {
	size_t       size;
	void        *pointer;
	union {
		uint64_t value;
		float    f_value;
	};
};

union td_args {
	uint8_t   dummy;
	td_set_t  set;
	td_lerp_t lerp;
};

struct td_event {
	td_delay_t duration;
	td_func_t  callback;
	td_args_t  callback_args;
};

struct td_ctx {
	std::vector<td_lerp_list_t> lerp_list;
};

// Runs the entire TechDemo 2 system.
// Returns true when complete, false when errored or cancelled.
bool main(pax::Buffer &buf, size_t n_events, const td_event_t *events);

}

extern "C" {
#endif

// Method that TD2 expects to be externally defined, to get time in milliseconds.
extern uint64_t td2_millis_cb();
// Method that TD2 expects to be externally defined, to allow cancellation.
extern bool td2_continue_cb();
// Method that TD2 expects to be externally defined, to flush framebuffer.
extern bool td2_flush_cb();
// Method that TD2 expects to be externally defined, to flush framebuffer (dirty part only).
extern bool td2_sync_cb();

// Runs the entire TechDemo 2 system.
// Returns true when complete, false when errored or cancelled.
bool td2_main(pax_buf_t *buf);

#ifdef __cplusplus
}
#endif
