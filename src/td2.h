/*
	MIT License

	Copyright (c) 2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <pax_gfx.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Method that TD2 expects to be externally defined, to get time in milliseconds.
extern int64_t td2_millis_cb();
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
