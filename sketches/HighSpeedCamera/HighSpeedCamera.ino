/*
 *  HighSpeedCamera.ino - High Speed Camera example sketch
 *  Copyright 2025 T.Hayakawa
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */
// #define USE_EMMC 

#ifdef USE_EMMC
#include <eMMC.h>
#define EMMC_POWER_PIN 26
#else /* Use SD */
#include <SDHCI.h>
SDClass  theSD;
#endif

#include <stdio.h>  /* for sprintf */

#include <Camera.h>

#define BAUDRATE                (115200)
#define MAX_QUALITY             (80)
#define DEFAULT_QUALITY         MAX_QUALITY
#define MIN_QUALITY             (20)
#define QUALITY_STEP            (10)
#define SUCCESS_COUNT_LIMIT     (20)
#define MAX_EXPOSURE            (30)

#define BUTTON_PIN 2

int jpeg_quality = DEFAULT_QUALITY;
int success_count = 0;
int take_picture_count = 0;

/**
 * Print error message
 */

void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

/**
 * @brief Initialize camera
 */
void setup()
{
  CamErr err;

  /* Open serial communications and wait for port to open */

  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }

#ifdef USE_EMMC
  /* Initialize eMMC */
  eMMC.begin(EMMC_POWER_PIN);
#else /* Use SD */
  /* Initialize SD */
  while (!theSD.begin()) 
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }
#endif

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* set HDR Mode on */

  err = theCamera.setHDR(CAM_HDR_MODE_ON);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
 
  /* Auto white balance configuration */

  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_AUTO);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Auto Exposure */
  err = theCamera.setAutoExposure(true);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Auto Exposure */
  err = theCamera.setAutoExposure(true);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  err = theCamera.setAutoISOSensitivity(true);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  if (!setMaxExposure(MAX_EXPOSURE))
    {
      puts("setMaxExposure error!");
    }

  /* Set parameters about still picture.
   * In the following case, QUADVGA and JPEG.
   */

  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_HD_H,
     CAM_IMGSIZE_HD_V,
//     CAM_IMGSIZE_QUADVGA_H,
//     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG,
     4 /* jpgbufsize_divisor = 3 */
     );
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  Serial.println("Set default JPEG quality.");
  err = theCamera.setJPEGQuality(DEFAULT_QUALITY);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

}

void adjustQuality(bool success)
{
  if (success)
  {
    success_count++;
    if (success_count >= SUCCESS_COUNT_LIMIT && jpeg_quality < MAX_QUALITY)
    {
      jpeg_quality += QUALITY_STEP;
      if (jpeg_quality > MAX_QUALITY) jpeg_quality = MAX_QUALITY;
      theCamera.setJPEGQuality(jpeg_quality);
      Serial.print("Increased JPEG quality to ");
      Serial.println(jpeg_quality);
      success_count = 0;
    }
  }
  else
  {
    success_count = 0;
    jpeg_quality -= QUALITY_STEP;
    if (jpeg_quality < MIN_QUALITY) jpeg_quality = MIN_QUALITY;
    theCamera.setJPEGQuality(jpeg_quality);
    Serial.print("Decreased JPEG quality to ");
    Serial.println(jpeg_quality);
  }
}

bool sense_trigger()
{
  return true;
}

void enterMSCMode()
{
  Serial.println("Switching to USB MSC mode...");
  theCamera.end();

#ifdef USE_EMMC
  eMMC.end();
  /* Start USB MSC */
  if (eMMC.beginUsbMsc()) {

#else /* Use SD */

  /* Start USB MSC */
  if (theSD.beginUsbMsc()) {

#endif

    Serial.println("USB MSC Failure!");
  } else {
    Serial.println("*** USB MSC Prepared! ***");
    Serial.println("Connect Extension Board USB to PC.");
  }

  Serial.println("SD card mounted as USB storage.");
  Serial.println("To exit, disconnect USB or reset board.");

}

void exitMSCMode()
{
#ifdef USE_EMMC

  // Finish USB Mass Storage
  if (eMMC.endUsbMsc()) {
    Serial.println("UsbMsc disconnect error");
  }
  /* Initialize eMMC */
  eMMC.begin(EMMC_POWER_PIN);

#else /* Use SD */

  // Finish USB Mass Storage
  if (theSD.endUsbMsc()) {
    Serial.println("UsbMsc disconnect error");
  }
  /* Initialize SD */
  while (!theSD.begin()) 
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

#endif

}

/**
 * @brief Take picture with format JPEG per second
 */

void loop()
{
  sleep(1); /* wait for one second to take still picture. */

  if (sense_trigger())
    {

      /* Take still picture.
      * Unlike video stream(startStreaming) , this API wait to receive image data
      *  from camera device.
      */
  
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();

      Serial.println("call takePicture() end");

      /* Check availability of the img instance. */
      /* If any errors occur, the img is not available. */

      if (img.isAvailable())
        {
          /* Create file name */
    
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
    
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");

          /* Remove the old file with the same file name as new created file,
           * and create new file.
           */

#ifdef USE_EMMC
          eMMC.remove(filename);
          File myFile = eMMC.open(filename, FILE_WRITE);
#else /* Use SD */
          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
#endif
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();

          adjustQuality(true);
          take_picture_count++;
        }
      else
        {
          /* The size of a picture may exceed the allocated memory size.
           * Then, allocate the larger memory size and/or decrease the size of a picture.
           * [How to allocate the larger memory]
           * - Decrease jpgbufsize_divisor specified by setStillPictureImageFormat()
           * - Increase the Memory size from Arduino IDE tools Menu
           * [How to decrease the size of a picture]
           * - Decrease the JPEG quality by setJPEGQuality()
           */

          Serial.println("Failed to take picture");
          adjustQuality(false);
        }
    }

  if(digitalRead(BUTTON_PIN) == LOW){
    enterMSCMode();
    while (digitalRead(BUTTON_PIN) == LOW) {
      sleep(1);
    }
    exitMSCMode();
  }

}
