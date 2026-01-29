/*
 *  ResizeYUV422.h - YUV422 (YUYV) Image Downsampling Library for Anomaly Detection
 *
 *  Author: Interested-In-Spresense
 *  Copyright (c) 2026
 *
 *  This software is released under the MIT License.
 *  http://opensource.org/licenses/mit-license.php
 *
 */

#ifndef RESIZE_YUV422_H
#define RESIZE_YUV422_H

#include <stdint.h>

/**
 * @brief Resize YUV422 image with NxN block averaging
 * 
 * @param src Source YUV422 buffer (YUYV format)
 * @param src_w Source image width
 * @param src_h Source image height
 * @param dst Destination YUV422 buffer (YUYV format)
 * @param dst_w Destination image width
 * @param dst_h Destination image height
 * @param scale Downscale factor (e.g., 5 for 1/5 reduction)
 */
void resizeYUV422Average(uint8_t* src, int src_w, int src_h, 
                         uint8_t* dst, int dst_w, int dst_h, int scale);

#endif // RESIZE_YUV422_H
