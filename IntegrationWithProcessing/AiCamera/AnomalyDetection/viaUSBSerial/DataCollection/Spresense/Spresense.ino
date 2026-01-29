/*
 *  Spresense.ino - Data Collection Sample on Spresense for Anomaly Detection
 *
 *  Based on Spresense.ino from
 *  https://github.com/Interested-In-Spresense/Spresense-Playground/tree/master/IntegrationWithProcessing/LiveCamera/viaUSBSerial/YUV/Spresense
 *
 *  This sketch captures 64x48 images and sends YUV data via USB Serial.
 *
 *  Author: Interested-In-Spresense
 *  Copyright (c) 2026
 *
 *  This software is released under the MIT License.
 *  http://opensource.org/licenses/mit-license.php
 *
 */
 
/*
 * Spresense.ino - YUV422 (YUYV) Image Downsampling Library for Anomaly Detection
 *
 * Author: Interested-In-Spresense
 * Copyright (c) 2026
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 *
 */


#include <Camera.h>
#include <USBSerial.h>
#include "ResizeYUV422.h"

USBSerial UsbSerial;

// Serial settings
#define SERIAL_BAUDRATE 921600

// Camera capture size (must be supported resolution)
#define CAM_WIDTH  CAM_IMGSIZE_QVGA_H  // 320
#define CAM_HEIGHT CAM_IMGSIZE_QVGA_V  // 240

// Output tiny image size: 64x48
int16_t output_width = 64;
int16_t output_height = 48;

// Global buffer for resized image (avoid malloc/free)
uint16_t resize_buffer[64 * 48];  // 6144 bytes
const int SCALE_FACTOR = 5;  // 320/64 = 5




/****************************************************************************
 * @brief Resizes and transmits YUV422 images for Anomaly Detection.
 * * Process:
 * 1. Format Check: Ensure input is YUV422.
 * 2. Downsampling: Resize to 64x48 using 5x5 area averaging to preserve features.
 * 3. Transmission: Send via USB Serial with custom framing (Header/Size/Data/Footer).
 */
void sendImage(CamImage img) {
  // 1. Ensure YUV422 format
  if (img.getPixFormat() != CAM_IMAGE_PIX_FMT_YUV422) {
    if (img.convertPixFormat(CAM_IMAGE_PIX_FMT_YUV422) != CAM_ERR_SUCCESS) return;
  }

  // 2. Downsample to 64x48 via Area Averaging
  // Preserves essential features while reducing data for processing.
  uint8_t* src_buf = (uint8_t*)img.getImgBuff();
  uint8_t* dst_buf = (uint8_t*)resize_buffer;
  resizeYUV422_average(src_buf, CAM_WIDTH, CAM_HEIGHT, 
                       dst_buf, output_width, output_height, SCALE_FACTOR);
  
  size_t total_size = output_width * output_height * 2;

  // 3. Serial Transmission with Protocol Framing
  UsbSerial.write("SPRS", 4); // Start Marker
  
  // Send 4-byte payload size (Big-endian)
  UsbSerial.write((total_size >> 24) & 0xFF);
  UsbSerial.write((total_size >> 16) & 0xFF);
  UsbSerial.write((total_size >>  8) & 0xFF);
  UsbSerial.write((total_size >>  0) & 0xFF);

  // Send Image Data
  size_t sent = 0;
  while (sent < total_size) {
    size_t s = UsbSerial.write(dst_buf + sent, total_size - sent);
    if ((int)s < 0) return;
    sent += s;
  }

  UsbSerial.write("ENDS", 4); // End Marker
  UsbSerial.flush();
}

/****************************************************************************
 * Setup
 ****************************************************************************/
void setup() {
  CamErr err;
  
  // Initialize debug serial (UART)
  Serial.begin(115200);
  delay(100); // Wait for serial to stabilize
  
  // Initialize USB serial for data transfer
  UsbSerial.begin(SERIAL_BAUDRATE);
  delay(100);
  
  Serial.println("64x48 Snapshot Camera Ready");
  Serial.print("Capture size: ");
  Serial.print(CAM_WIDTH);
  Serial.print("x");
  Serial.println(CAM_HEIGHT);
  Serial.print("Output size: ");
  Serial.print(output_width);
  Serial.print("x");
  Serial.println(output_height);

  // Initialize camera
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("Camera initialization failed");
    while (1);
  }

  // Configure camera for QVGA
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_AUTO);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("Set AWB failed");
  }

  err = theCamera.setStillPictureImageFormat(
    CAM_WIDTH, 
    CAM_HEIGHT,
    CAM_IMAGE_PIX_FMT_YUV422);
  
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("Set still format failed");
    while (1);
  }

  Serial.println("Camera configured successfully");
}

/****************************************************************************
 * Loop
 ****************************************************************************/
void loop() {
  CamImage img = theCamera.takePicture();

  if (img.isAvailable()) {
    sendImage(img);
  }  
  delay(100); // ~10fps
}
