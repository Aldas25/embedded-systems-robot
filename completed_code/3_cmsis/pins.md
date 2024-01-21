right/left is according to the front of the robot (ultrasound is the front)

wheel sensors
- gnd, vcc -> 5V 
- (Irish flag; right one) out -> B13
- (Romanian flag; left one) out -> B12

infrared sensors
- gnd, vcc -> 3v3
- (French flag; left one) A0 -> A1 (ADC1)
- (right one) A0 -> A0 (ADC0)

ultrasound sensors
- gnd, vcc -> 5v
- (Front) trig -> A15
- (Side) trig -> ?
- (Front) Echo -> B9
- (Side) Echo -> ?

H bridge
- gnd, vcc -> 5V
- B-IA (left backward) -> A11 (T1_CH4)
- B-IB (left forward) -> A10 (T1_CH3)
- A-IA (right forward) -> A9 (T1_CH2)
- A-IB (right backward) -> A8 (T1_CH1) 

LEDs
- short -> GND
- red long -> B1
- yellow long -> B2

OLED screen
- vcc, gnd -> 3v3
- SCL -> B6
- SDA -> B7