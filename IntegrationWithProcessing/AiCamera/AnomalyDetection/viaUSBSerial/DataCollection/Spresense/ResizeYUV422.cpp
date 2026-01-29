/*
 *  ResizeYUV422.cpp - YUV422 (YUYV) Image Downsampling Library for Anomaly Detection
 *
 *  Author: Interested-In-Spresense
 *  Copyright (c) 2026
 *
 *  This software is released under the MIT License.
 *  http://opensource.org/licenses/mit-license.php
 *
 */

#include "ResizeYUV422.h"

/**
 * @brief Resize YUV422 image with NxN block averaging
 * 
 * YUV422 format: YUYV (4 bytes for 2 pixels)
 * Each 2 pixels share U and V values
 * Layout: Y0 U Y1 V Y2 U Y3 V ...
 * 
 * This function averages NxN blocks to create downsampled image.
 * For each output pixel, it averages Y values from the corresponding block.
 * U and V are averaged from the block covering 2 output pixels.
 */
void resizeYUV422Average(uint8_t* src, int src_w, int src_h, 
                         uint8_t* dst, int dst_w, int dst_h, int scale)
{
  // Process output image pixel by pixel
  for (int dst_y = 0; dst_y < dst_h; dst_y++) {
    for (int dst_x = 0; dst_x < dst_w; dst_x++) {

      // Calculate source block position (top-left corner)
      int src_x = dst_x * scale;
      int src_y = dst_y * scale;

      // Accumulate Y, U, V values from the scale x scale block
      int sum_y = 0;
      int sum_u = 0;
      int sum_v = 0;
      int count_y = 0;
      int count_uv = 0;

      // Scan the scale x scale block
      for (int by = 0; by < scale && (src_y + by) < src_h; by++) {
        for (int bx = 0; bx < scale && (src_x + bx) < src_w; bx++) {
          int pixel_x = src_x + bx;
          int pixel_y = src_y + by;

          // Calculate byte offset in YUV422 buffer
          // YUV422: every 2 pixels = 4 bytes (Y0 U Y1 V)
          int is_odd = pixel_x & 1;  // Check if odd pixel
          int pair_idx = (pixel_y * src_w + (pixel_x & ~1)) * 2; // Start of YUYV pair

          // Get Y value (always available for each pixel)
          int y_offset = pair_idx + (is_odd ? 2 : 0);
          sum_y += src[y_offset];
          count_y++;

          // Get U and V values (shared by pixel pair, accumulate once per pair)
          if (bx % 2 == 0) {  // Only accumulate U/V once per pair
            sum_u += src[pair_idx + 1];  // U
            sum_v += src[pair_idx + 3];  // V
            count_uv++;
          }
        }
      }

      // Calculate average values
      uint8_t avg_y = (count_y > 0) ? (sum_y / count_y) : 0;
      uint8_t avg_u = (count_uv > 0) ? (sum_u / count_uv) : 128;
      uint8_t avg_v = (count_uv > 0) ? (sum_v / count_uv) : 128;

      // Write to destination buffer
      // YUV422: process in pairs
      int is_odd_dst = dst_x & 1;
      int dst_pair_idx = (dst_y * dst_w + (dst_x & ~1)) * 2;

      // Write Y value
      dst[dst_pair_idx + (is_odd_dst ? 2 : 0)] = avg_y;

      // Write U and V values (only for even pixels to avoid overwriting)
      if (!is_odd_dst) {
        dst[dst_pair_idx + 1] = avg_u;
        dst[dst_pair_idx + 3] = avg_v;
      }
    }
  }
}
