#include "ease.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <unistd.h>

enum {
  kGetSensorReadingID = 0, // getSensorReading(int *, int *)
  kGetLEDBrightnessID = 1, // getLEDBrightness(int, int *)
  kSetLEDBrightnessID = 2, // setLEDBrightness(int, int, int *)
  kSetLEDFadeID = 3,       // setLEDFade(int, int, int, int *)
};

static io_connect_t dataPort = 0;

io_connect_t getDataPort(void) {
  kern_return_t kr;
  io_service_t serviceObject;

  if (dataPort)
    return dataPort;

  // Look up a registered IOService object whose class is AppleLMUController
  serviceObject = IOServiceGetMatchingService(
      kIOMasterPortDefault, IOServiceMatching("AppleLMUController"));

  if (!serviceObject) {
    printf("Failed to connect to AppleLMUController\n");
    return 0;
  }

  // Create a connection to the IOService object
  kr = IOServiceOpen(serviceObject, mach_task_self(), 0, &dataPort);
  IOObjectRelease(serviceObject);

  if (kr != KERN_SUCCESS) {
    printf("Failed to open IoService object\n");
    return 0;
  }
  return dataPort;
}

float getKeyboardBrightness(void) {
  float f;
  kern_return_t kr;

  uint64_t inputCount = 1;
  uint64_t inputValues[1] = {0};

  uint32_t outputCount = 1;
  uint64_t outputValues[1];

  uint32_t out_brightness;

  kr =
      IOConnectCallScalarMethod(getDataPort(), kGetLEDBrightnessID, inputValues,
                                inputCount, outputValues, &outputCount);

  out_brightness = outputValues[0];

  if (kr != KERN_SUCCESS) {
    printf("getKeyboardBrightness() error\n");
    return 0;
  }

  f = out_brightness;
  f /= 0xfff;
  return (float)f;
}

void setKeyboardBrightness(float in) {
  kern_return_t kr;

  uint64_t inputCount = 2;
  uint64_t inputValues[2];
  uint64_t in_unknown = 0;
  uint64_t in_brightness = in * 0xfff;

  inputValues[0] = in_unknown;
  inputValues[1] = in_brightness;

  uint32_t outputCount = 1;
  uint64_t outputValues[1];

  uint32_t out_brightness;

  kr =
      IOConnectCallScalarMethod(getDataPort(), kSetLEDBrightnessID, inputValues,
                                inputCount, outputValues, &outputCount);

  out_brightness = outputValues[0];

  if (kr != KERN_SUCCESS) {
    printf("setKeyboardBrightness() error\n");
    return;
  }
}

#define easer LinearInterpolation
const float speed = 0.0020;

int main(int argc, char **argv) {
  while (true) {
    for (float i = 0; i < 1; i += speed) {
      usleep(easer(i) * 1000);
      setKeyboardBrightness(i);
    }
    for (float i = 1; i > 0; i -= speed) {
      usleep(easer(i) * 1000);
      setKeyboardBrightness(i);
    }
  }
  exit(0);
}
