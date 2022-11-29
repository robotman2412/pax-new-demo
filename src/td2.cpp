
#include "pax_gfx.h"
#include "td2.h"
#include "esp_log.h"

namespace td2 {




/* ===== VARIABLES ====== */
// The background color, alpha is respected.
pax_col_t bg_col;

// Whether to enable the simulation.
bool      sim_enable;
// Simulation speed multiplier.
float     sim_speed;



/* ====== HELPERS ======= */
// Apply the interpolated value to all interps in the list.
void td_apply_lerps(td_ctx_t &ctx, uint64_t now) {
	// ESP_LOGI("lerp", "There are %d lerpers.", ctx.lerp_list.size());
	
	// Apply all time functions.
	for (auto iter = ctx.lerp_list.begin(); iter != ctx.lerp_list.end(); iter++) {
		float coeff;
		
		// Calculate linear coefficient.
		if (now < iter->start) {
			coeff = 0;
		} else if (now > iter->end) {
			coeff = 1;
		} else {
			coeff = (now - iter->start) / (float) (iter->end - iter->start);
		}
		
		const td_lerp_t *subj = iter->subject;
		
		// Calculate coefficient.
		switch (subj->timing) {
			case TD_EASE_OUT:
				// Ease-out:    y=-x²+2x
				coeff = -coeff*coeff + 2*coeff;
				break;
				
			case TD_EASE_IN:
				// Ease-in:     y=x²
				coeff *= coeff;
				break;
				
			case TD_EASE:
				// Ease-in-out: y=-2x³+3x²
				coeff = -2*coeff*coeff*coeff + 3*coeff*coeff;
				break;
		}
		
		// Store resulting value.
		switch (subj->type) {
				uint32_t tmp;
				
			case TD_INTERP_TYPE_INT:
				// Interpolate an integer.
				*subj->int_ptr = subj->int_from + (subj->int_to - subj->int_from) * coeff;
				break;
				
			case TD_INTERP_TYPE_COL:
				// Interpolate a (RGB) color.
				*subj->int_ptr = pax_col_lerp(coeff*255, subj->int_from, subj->int_to);
				break;
				
			case TD_INTERP_TYPE_HSV:
				// Interpolate a (HSV) color.
				tmp = pax_col_lerp(coeff*255, subj->int_from, subj->int_to);
				*subj->int_ptr = pax_col_ahsv(tmp >> 24, tmp >> 16, tmp >> 8, tmp);
				break;
				
			case TD_INTERP_TYPE_FLOAT:
				// Interpolate a float.
				*subj->float_ptr = subj->float_from + (subj->float_to - subj->float_from) * coeff;
				break;
		}
	}
	
	// Delete lerps that ran out.
	ctx.lerp_list.erase(std::remove_if(ctx.lerp_list.begin(), ctx.lerp_list.end(), [now](const td_lerp_list_t &o) { return o.end <= now; }), ctx.lerp_list.end());
}



/* ===== CALLBACKS ====== */
// Adds another interpolated variable to the list.
void td_add_lerp(td_ctx_t &ctx, size_t planned_time, size_t planned_duration, const td_args_t *args) {
	const td_lerp_t *lerp = &args->lerp;
	
	ESP_LOGI("lerp", "Adds an lerper.");
	ctx.lerp_list.push_back((td_lerp_list_t) {
		.subject = lerp,
		.start   = planned_time,
		.end     = planned_time + planned_duration,
	});
}

// Set variable callback.
void td_set_var(td_ctx_t &ctx, size_t planned_time, size_t planned_duration, const td_args_t *args) {
	const td_set_t *set = &args->set;
	
	switch (set->size) {
		case sizeof(uint8_t):  *(uint8_t *)  set->pointer = set->value; break;
		case sizeof(uint16_t): *(uint16_t *) set->pointer = set->value; break;
		case sizeof(uint32_t): *(uint32_t *) set->pointer = set->value; break;
		case sizeof(uint64_t): *(uint64_t *) set->pointer = set->value; break;
	}
}

// Set variable callback for floats.
void td_set_float(td_ctx_t &ctx, size_t planned_time, size_t planned_duration, const td_args_t *args) {
	const td_set_t *set = &args->set;
	
	*(float *) set->pointer = set->f_value;
}



/* ==== CHOREOGRAPHY ==== */
#define TD_DELAY(time) {.duration=time,.callback=NULL,.callback_args={.dummy=0}}
#define TD_INTERP_INT(delay_time, interp_time, timing_func, variable, from, to) {\
			.duration = delay_time,\
			.callback = td_add_lerp,\
			.callback_args = { .lerp = {\
				.duration = interp_time,\
				.int_ptr  = (int *) &(variable),\
				.int_from = (from),\
				.int_to   = (to),\
				.type     =  TD_INTERP_TYPE_INT,\
				.timing   =  timing_func\
			}}\
		}
#define TD_INTERP_COL(delay_time, interp_time, timing_func, variable, from, to) {\
			.duration = delay_time,\
			.callback = td_add_lerp,\
			.callback_args = { .lerp = {\
				.duration = interp_time,\
				.int_ptr  = (int *) &(variable),\
				.int_from = (int) (from),\
				.int_to   = (int) (to),\
				.type     =  TD_INTERP_TYPE_COL,\
				.timing   =  timing_func\
			}}\
		}
#define TD_INTERP_AHSV(delay_time, interp_time, timing_func, variable, from, to) {\
			.duration = delay_time,\
			.callback = td_add_lerp,\
			.callback_args = { .lerp = {\
				.duration = interp_time,\
				.int_ptr  = (int *) &(variable),\
				.int_from = (int) (from),\
				.int_to   = (int) (to),\
				.type     =  TD_INTERP_TYPE_HSV,\
				.timing   =  timing_func\
			}}\
		}
#define TD_INTERP_FLOAT(delay_time, interp_time, timing_func, variable, from, to) {\
			.duration = delay_time,\
			.callback = td_add_lerp,\
			.callback_args = { .lerp = {\
				.duration   = interp_time,\
				.float_ptr  = (float *) &(variable),\
				.float_from = (from),\
				.float_to   = (to),\
				.type       =  TD_INTERP_TYPE_FLOAT,\
				.timing     =  timing_func\
			}}\
		}
#define TD_SET_0(type_size, variable, new_value) {\
			.duration = 0,\
			.callback = td_set_var,\
			.callback_args = { .set = {\
				.size     = (type_size),\
				.pointer  = (void *) &(variable),\
				.value    = (new_value)\
			}}\
		}
#define TD_SET_BOOL(variable, value) TD_SET_0(sizeof(bool), variable, value)
#define TD_SET_INT(variable, value) TD_SET_0(sizeof(int), variable, value)
#define TD_SET_LONG(variable, value) TD_SET_0(sizeof(long), variable, value)
#define TD_SET_FLOAT(variable, new_value) {\
			.duration = 0,\
			.callback = td_set_float,\
			.callback_args = &(const td_set_t){\
				.size     = sizeof(float),\
				.pointer  = (void *) &(variable),\
				.f_value  = (new_value)\
			}\
		}

const td_event_t entries[] = {
	TD_SET_INT(bg_col, 0xffff0000),
	TD_INTERP_COL(1000, 1000, TD_EASE, bg_col, 0xffff0000, 0xff00ff00),
	TD_DELAY(1000),
	TD_DELAY(0),
};
size_t numEntries = sizeof(entries) / sizeof(td_event_t);



// Runs the entire TechDemo 2 system.
// Returns true when complete, false when errored or cancelled.
bool main(pax::Buffer &buf, size_t numEntries, const td_event_t *entries) {
	const char *TAG = "td2";
	ESP_LOGI(TAG, "TD2 main");
	
	// Context for function callbacks.
	td_ctx_t cb_ctx;
	
	// Next event up to run.
	size_t   index      = 0;
	// Time at which the demo was started.
	uint64_t start_time = td2_millis_cb();
	// Next time at which to run an event.
	uint64_t event_time = start_time;
	
	while (index < numEntries) {
		if (!td2_continue_cb()) {
			ESP_LOGI(TAG, "TD2 cancelled");
			return false;
		}
		
		// Current time in milliseconds.
		uint64_t now = td2_millis_cb();
		
		// Are there events to run?
		while (event_time <= now && index < numEntries) {
			// Run callback.
			if (entries[index].callback) {
				entries[index].callback(cb_ctx, event_time, entries[index].duration, &entries[index].callback_args);
				ESP_LOGI(TAG, "Running event %u", index);
			} else {
				ESP_LOGI(TAG, "Skipping event %u", index);
			}
			
			// Add duration.
			event_time += entries[index].duration;
			index ++;
		}
		
		// Apply interpolations.
		td_apply_lerps(cb_ctx, now);
		
		// Draw background.
		if ((bg_col & 0xff000000) == 0xff000000) {
			buf.background(bg_col);
		} else if (bg_col & 0xff000000) {
			buf.drawRect(bg_col, 0, 0, buf.width(), buf.height());
		}
		
		// FLUSH it.
		td2_flush_cb();
	}
	
	ESP_LOGI(TAG, "TD2 fin");
	return true;
}

} // namespace td2




/* ======= C API ======== */
// Runs the entire TechDemo 2 system.
// Returns true when complete, false when errored or cancelled.
extern "C" bool td2_main(pax_buf_t *buf) {
	pax::Buffer cbuf(buf);
	return td2::main(cbuf, td2::numEntries, td2::entries);
}
