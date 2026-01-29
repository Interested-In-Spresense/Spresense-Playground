/*
 * Receiver.pde - Data Collection Client for Anomaly Detection
 *
 * This sketch receives downsampled YUV422 (64x48) images from Spresense 
 * via USB Serial, provides real-time visualization, and saves images for 
 * training data.
 *
 * Based on Processing.pde from
 * https://github.com/TomonobuHayakawa/Spresense-Playground/blob/master/IntegrationWithProcessing/LiveCamera/viaUSBSerial/YUV/Processing/Processing.pde
 *
 * Author: Interested-In-Spresense
 * Copyright (c) 2026
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 *
 */

import java.util.Date;
import java.text.SimpleDateFormat;
import processing.serial.*;

Serial serial;
final String SERIAL_PORTNAME = "COM64";
final int SERIAL_BAUDRATE = 921600;
final int IMG_WIDTH = 64;
final int IMG_HEIGHT = 48;

int count = 0;
int base_time = 0;
PImage displayImg;
byte[] yuv_data;
boolean grayscaleMode = true; // デフォルトはグレースケール

/* setup */
void setup()
{
  size(640, 480);
  displayImg = createImage(IMG_WIDTH, IMG_HEIGHT, RGB);
  yuv_data = new byte[IMG_WIDTH * IMG_HEIGHT * 2];

  println("64x48 Snapshot Camera");
  println("Press 'g' for grayscale, 'c' for color (default: grayscale)");
  println("Press 's' to save PNG");

  serial = new Serial(this, SERIAL_PORTNAME, SERIAL_BAUDRATE);
  serial.buffer(8192);

  base_time = millis();
  println("Connected! Mode: Grayscale");
}

/* Main loop: synchronization with SPRS header, data readout, and display update. */
void draw()
{
  if (!find_start()) return;

  int size = 0;
  for (int i = 0; i < 4; i++) {
    if (serial.available() > 0) {
      size = (size << 8) | (serial.read() & 0xFF);
    } else {
      println("recover1");
      recover();
      return;
    }
  }

  if (size <= 0) {
    serial.clear();
    return;
  }

  int now = millis();
  println(count++, ": size=", size, "time=", now - base_time, "[ms]");
  base_time = now;

  int timeout = millis() + 5000;
  for (int i = 0; i < size; ) {
    if (serial.available() > 0) {
      yuv_data[i] = (byte)serial.read();
      i++;
    } else {
      if (millis() > timeout) {
        println("recover2");
        recover();
        return;
      }
      delay(50);
    }
  }

  if (!find_end()) return;
  displayYUV422(yuv_data);
}

/* Converts raw YUV422 bytes to RGB or Grayscale and updates the PImage pixels. */
void displayYUV422(byte[] yuv)
{
  displayImg.loadPixels();
  
  if (grayscaleMode) {
    // グレースケール表示（Y成分のみ）
    for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
      int idx = i * 2;
      int y = yuv[idx + 1] & 0xFF;
      displayImg.pixels[i] = color(y, y, y);
    }
  } else {
    // カラー表示（YUV to RGB変換）
    for (int y = 0; y < IMG_HEIGHT; y++) {
      for (int x = 0; x < IMG_WIDTH; x += 2) {
        int idx = (y * IMG_WIDTH + x) * 2;
        int u  = yuv[idx + 0] & 0xFF;
        int y1 = yuv[idx + 1] & 0xFF;
        int v  = yuv[idx + 2] & 0xFF;
        int y2 = yuv[idx + 3] & 0xFF;
        displayImg.pixels[y * IMG_WIDTH + x] = yuvToRgb(y1, u, v);
        displayImg.pixels[y * IMG_WIDTH + x + 1] = yuvToRgb(y2, u, v);
      }
    }
  }
  
  displayImg.updatePixels();
  image(displayImg, 0, 0, width, height);
}

/* Calculates RGB values from YUV components using standard CCIR 601 conversion formula. */
color yuvToRgb(int y, int u, int v)
{
  float yf = y - 16.0;
  float uf = u - 128.0;
  float vf = v - 128.0;
  int r = constrain(int(1.164 * yf + 1.596 * vf), 0, 255);
  int g = constrain(int(1.164 * yf - 0.392 * uf - 0.813 * vf), 0, 255);
  int b = constrain(int(1.164 * yf + 2.017 * uf), 0, 255);
  return color(r, g, b);
}

/* Scans the serial buffer for the 'SPRS' start marker to synchronize the frame. */
boolean find_start()
{
  while (serial.available() > 0) {
    if (serial.read() != 'S') continue;
    if (serial.available() == 0) return false;
    if (serial.read() != 'P') continue;
    if (serial.available() == 0) return false;
    if (serial.read() != 'R') continue;
    if (serial.available() == 0) return false;
    if (serial.read() != 'S') continue;
    return true;
  }
  return false;
}

/* Verifies the 'ENDS' footer at the end of the packet to ensure data integrity. */
boolean find_end()
{
  if (serial.available() == 0) return false;
  if (serial.read() != 'E') {
    println("recover3");
    recover();
    return false;
  }
  if (serial.available() == 0) return false;
  if (serial.read() != 'N') {
    println("recover4");
    recover();
    return false;
  }
  if (serial.available() == 0) return false;
  if (serial.read() != 'D') {
    println("recover5");
    recover();
    return false;
  }
  if (serial.available() == 0) return false;
  if (serial.read() != 'S') {
    println("recover6");
    recover();
    return false;
  }
  return true;
}

void recover()
{
  serial.clear();
}

/* Handles user input for toggling view modes (g/c) and capturing training data (s). */
void keyPressed()
{
  if (key == 'g' || key == 'G') {
    grayscaleMode = true;
    println("Mode: Grayscale");
  }
  else if (key == 'c' || key == 'C') {
    grayscaleMode = false;
    println("Mode: Color");
  }
  else if (key == 's' || key == 'S') {
    // data/フォルダを作成
    java.io.File dataDir = new java.io.File(sketchPath("data"));
    if (!dataDir.exists()) {
      dataDir.mkdir();
    }

    // 次の番号を探す
    int num = 0;
    String filename;
    java.io.File file;
    do {
      filename = String.format("data/%04d.png", num);
      file = new java.io.File(sketchPath(filename));
      num++;
    } while (file.exists());

    // 現在のモードで保存
    displayImg.save(filename);
    println("Saved: " + filename + (grayscaleMode ? " (grayscale)" : " (color)"));
  }
}
