right/left is according to the front of the robot (ultrasound is the front)

wheel sensors
- gnd, vcc -> 5V 
- (Irish flag; right one) out -> B13
- (Romanian flag; left one) out -> B12

infrared sensors
- gnd, vcc -> 3v3
- (French flag; left one) A0 -> A1
- (right one) A0 -> A0

ultrasound sensors
- gnd, vcc -> 5v
- (Front) trig -> A10
- (Side) trig -> A9
- (Front) Echo -> B9
- (Side) Echo -> B8

H bridge
- gnd, vcc -> 5V
- B-IA (right backward) -> B15
- B-IB (right forward) -> B14
- A-IA (left forward) -> B3
- A-IB (left backward) -> A8

LEDs
- short -> GND
- red long -> B1
- yellow long -> B2

OLED screen
- vcc, gnd -> 3v3
- SCL -> B6
- SDA -> B7