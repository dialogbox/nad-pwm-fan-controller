#include "fan.h"

bool isKickstarting = false;
unsigned long kickstartEndTime = 0;
const unsigned long kickstartDuration = 800; // 800ms kickstart

unsigned long lastRampTime = 0;
const unsigned long rampInterval = 15; // smooth ramping update rate

void triggerKickstart() {
  isKickstarting = true;
  kickstartEndTime = millis() + kickstartDuration;
  currentSpeed = 255; 
  analogWrite(fanPin, 255);
  Serial.println("[Fan] Kick-starting motor (100% PWM)");
}

void updateFan() {
  if (powerMode == 0) {
    targetSpeed = 0;
    isKickstarting = false;
  } else if (powerMode == 1) {
    if (isKickstarting) {
      if (millis() < kickstartEndTime) {
        targetSpeed = 255;
      } else {
        isKickstarting = false;
        targetSpeed = sliderSpeed;
      }
    } else {
      targetSpeed = sliderSpeed;
    }
  } else if (powerMode == 2) {
    if (bluosPowerOn) {
      if (isKickstarting) {
        if (millis() < kickstartEndTime) {
          targetSpeed = 255;
        } else {
          isKickstarting = false;
          targetSpeed = sliderSpeed;
        }
      } else {
        targetSpeed = sliderSpeed;
      }
    } else {
      targetSpeed = 0;
      isKickstarting = false;
    }
  }
  
  // Non-blocking smooth ramping update
  unsigned long now = millis();
  if (now - lastRampTime >= rampInterval) {
    lastRampTime = now;
    if (currentSpeed < targetSpeed) {
      currentSpeed += 2;
      if (currentSpeed > targetSpeed) currentSpeed = targetSpeed;
      analogWrite(fanPin, currentSpeed);
    } else if (currentSpeed > targetSpeed) {
      currentSpeed -= 2;
      if (currentSpeed < targetSpeed) currentSpeed = targetSpeed;
      analogWrite(fanPin, currentSpeed);
    }
  }
}
