// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include "message.h"
#include <AFMotor.h>
#include <SoftwareSerial.h>

AF_DCMotor motor_FWD_R(1);
AF_DCMotor motor_FWD_L(2);
AF_DCMotor motor_RWD_R(3);
AF_DCMotor motor_RWD_L(4);

AF_DCMotor motors[4] = {motor_FWD_R, motor_FWD_L, motor_RWD_R, motor_RWD_L};

SoftwareSerial interboard(2, 10);
byte recv = 0;
uint32_t control = 0;
uint32_t last_control = 0;
Message ctrl_msg = {0};

void bytes_to_msg(uint32_t bytes) {
  ctrl_msg.header = (bytes >> (8 * 3)) & 0xFF;
  ctrl_msg.cmd_ident = (bytes >> (8 * 2)) & 0xFF;
  ctrl_msg.payload = (bytes >> (8 * 1)) & 0xFF;
  ctrl_msg.end = bytes & 0xFF;
}

void setup() {
  Serial.begin(9600);       /* set up debugging serial communication */
  interboard.begin(115200); /* set up interboard uart communication */
  Serial.println("Motor test!");

  // turn on motor
  motor_FWD_R.setSpeed(200);
  motor_FWD_L.setSpeed(200);
  motor_RWD_R.setSpeed(200);
  motor_RWD_L.setSpeed(200);

  motor_FWD_R.run(RELEASE);
  motor_FWD_L.run(RELEASE);
  motor_RWD_R.run(RELEASE);
  motor_RWD_L.run(RELEASE);
}

void loop() {
  int i = 0;

  // Serial.print("tick");
  while (interboard.available() > 0) {
    if (i == 4) {
      i = 0;
      return;
    }
    recv = interboard.read();
    Serial.print("Byte");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(recv);
    Serial.print("\n");
    control |= (uint32_t)recv << (8 * i);
    i++;
  }
  if (last_control != control) {
    bytes_to_msg(control);
    Serial.println(control);
    Serial.println(ctrl_msg.payload);

    if (ctrl_msg.cmd_ident == MOTOR) {
      // Some safety checks...
      if (((ctrl_msg.cmd_ident & FWD_R) && (ctrl_msg.cmd_ident & FWD_R_BACK)) ||
          ((ctrl_msg.cmd_ident & FWD_L) && (ctrl_msg.cmd_ident & FWD_L_BACK)) ||
          ((ctrl_msg.cmd_ident & RWD_R) && (ctrl_msg.cmd_ident & RWD_R_BACK)) ||
          ((ctrl_msg.cmd_ident & RWD_L) && (ctrl_msg.cmd_ident & RWD_L_BACK))) {
        Serial.println(
            "Some wheel has been instructed to move backwards and forwards \
        at the same time");
        return;
      }
      // Forward movement
      if (ctrl_msg.payload & FWD_R)
        motor_FWD_R.run(FORWARD);
      if (ctrl_msg.payload & FWD_L)
        motor_FWD_L.run(FORWARD);
      if (ctrl_msg.payload & RWD_R)
        motor_RWD_R.run(FORWARD);
      if (ctrl_msg.payload & RWD_L)
        motor_RWD_L.run(FORWARD);
      // Backward movement
      if (ctrl_msg.payload & FWD_R_BACK)
        motor_FWD_R.run(BACKWARD);
      if (ctrl_msg.payload & FWD_L_BACK)
        motor_FWD_L.run(BACKWARD);
      if (ctrl_msg.payload & RWD_R_BACK)
        motor_RWD_R.run(BACKWARD);
      if (ctrl_msg.payload & RWD_L_BACK)
        motor_RWD_L.run(BACKWARD);
    }
    last_control = control;

  } else { /* Stop all wheels if no command has been received, to prevent */
           /* them from moving permanently */
    for (int i = 0; i < 4; i++)
      motors[i].run(RELEASE);
  }
  delay(1000);
}