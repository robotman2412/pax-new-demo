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

#include "pax_gfx.h"
#include "td2.h"
#include "esp_log.h"
#include "esp_random.h"

/* ==== Helper functions ==== */

// Fast single-byte randomness function.
static uint8_t td2_random_byte() {
	return esp_random();
}

// Fills up the buffer with some noise.
static void td2_gen_noise(pax_buf_t *buf) {
	uint8_t *raw_data = pax_buf_get_pixels_rw(buf);
	size_t   len      = pax_buf_get_size(buf);
	for (size_t i = 0; i < len; i++) {
		raw_data[i] = td2_random_byte();
	}
}

// Generates a color spectrum in a certain hue range.
static void td2_gen_hue_spectrum(pax_col_t *col, size_t len, uint8_t alpha, int16_t hue_min, int16_t hue_max, uint8_t sat, uint8_t bri) {
	for (size_t i = 0; i < len; i++) {
		// Interpolate hue.
		uint8_t hue = hue_min + (hue_max-hue_min) * i / (len - 1);
		// Compute color value.
		col[i] = pax_col_ahsv(alpha, hue, sat, bri);
	}
}

/* ==== Scenes ==== */

// Some sort of psychedelic colors scene.
bool td2_scene_noise(pax_buf_t *buf) {
	bool retval = true;
	
	// Determine buffer dimensions.
	int buf_w = pax_buf_get_width(buf);
	int buf_h = pax_buf_get_height(buf);
	int buf_scale = 8;
	int buf_dims  = (buf_w > buf_h ? buf_w : buf_h) / buf_scale;
	
	// Create a buffer with some noise in it.
	pax_buf_t noise_buf;
	pax_col_t palette[16];
	pax_buf_init(&noise_buf, NULL, buf_dims, buf_dims, PAX_BUF_4_PAL);
	td2_gen_noise(&noise_buf);
	noise_buf.palette = palette;
	noise_buf.palette_size = 16;
	
	// The loop of image generation.
	while (1) {
		int64_t now = td2_millis_cb();
		
		// Generate the color palette.
		float c0 = now % 2000 * (1 / 2000.0 * 2 * M_PI);
		int16_t hue_width  = 92 + 32 * sinf(c0);
		int16_t hue_center = now % 5000 * 256 / 5000;
		int16_t hue_start  = hue_center - hue_width / 2;
		td2_gen_hue_spectrum(palette, 16, 255, hue_start, hue_start+hue_width, 255, 255);
		
		// Generate some angles and positions.
		float a0 = now % 5000 * (1 / 5000.0 * 2 * M_PI);
		float r0 = buf_dims/4 * (1 - 1 / (1 + now / 1000.0));
		
		float a1 = now % 3000 * (1 / 3000.0 * 2 * M_PI);
		float r1 = buf_dims/4 * (1 - 1 / (1 + now / 1500.0));
		
		float a2 = now % 50000 * (1 / 50000.0 * 2 * M_PI);
		float x2 = buf_dims/3;
		float y2 = buf_dims/3;
		
		// Draw the now rainbow-colored buffer.
		pax_push_2d(buf);
		pax_apply_2d(buf, matrix_2d_translate(buf_w/2 + r0*cos(a0), buf_h/2 + r1*sin(a1)));
		pax_apply_2d(buf, matrix_2d_rotate(a2));
		pax_apply_2d(buf, matrix_2d_scale(x2, y2));
		pax_draw_image(buf, &noise_buf, -buf_dims/2.0, -buf_dims/2.0);
		pax_pop_2d(buf);
		
		// Send to screen.
		td2_flush_cb();
	}
	
	// Cleanup.
	pax_buf_destroy(&noise_buf);
	return retval;
}

// Runs the entire TechDemo 2 system.
// Returns true when complete, false when errored or cancelled.
bool td2_main(pax_buf_t *buf) {
	return td2_scene_noise(buf);
}
