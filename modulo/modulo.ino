#if defined(__IMXRT1062__)
extern "C" uint32_t set_arm_clock(uint32_t frequency);
#endif

unsigned long previousMillis=0;

const int read_pin = A5;
const int haptic_pin = A0;
const int button_pin = 11;
int button_state = 0;
int force_val = 0; // variable to store the read value
int force_threshold = 597; // force threshold value
int mode = 0;
float force_total = 0;
float newtons_total = 0;
int num_reads = 0;
int avg_num = 40; // for reporting moving average
float newtons = 0; // to hold the transformed force value
float max_newtons = 20; // linear force value corresponding to maximum vibration
// according to our load specification (60 lbs), this should be 266.893 N
float period = 1000; // milliseconds

// curve-fitting constants
//float a = -295.5;
//float b = -0.4256;
//float c = 787.4*2;

float a = -300.5;
float b = -0.5;
float c = 787.4*2;

// A0-A4: haptic motors
// A5-A9: force-sensitive resistors

void setup() {
  // set_arm_clock(24000000);
  // put your setup code here, to run once:
  pinMode(read_pin, INPUT); // sets the digital pin as input
  pinMode(button_pin, INPUT);
  pinMode(haptic_pin, OUTPUT); // sets the haptic pin as output
  Serial.begin(9600);
  while (!Serial) ;
#if defined(__IMXRT1062__)
    set_arm_clock(16000000);
    //Serial.print("F_CPU_ACTUAL=");
    //Serial.println(F_CPU_ACTUAL);
#endif
}

void loop() {

//int counter = 0;
//while(true){
//  counter = counter + 1;
//  delay(1000);
//  Serial.println(counter);
//}
  
  delay(5); // time delay
  // put your main code here, to run repeatedly:

  button_state = digitalRead(button_pin); // read the button state
  //Serial.print(button_state);
  //Serial.print(",  ");
  if(button_state == 0) { // button is pressed
    while(button_state == 0) { // wait until the button is released
      delay(1);
      button_state = digitalRead(button_pin);
    }
    if (mode < 2) {
      mode = mode + 1;
    } else {
      mode = 0;
    }
    //Serial.print("===== ");
    Serial.println(mode);
    //Serial.println(" =====");
  }
  
  force_val = analogRead(read_pin); // read the force
  force_total = force_total + force_val;
  num_reads = num_reads + 1;

  //newtons = pow((force_val-c)/a,1);
  newtons = pow((force_val-c)/a,1/b); // power transform
  //newtons = force_val;
  newtons_total = newtons_total + newtons;
  //Serial.println(force_val);
  //Serial.print(", ");
  //Serial.println(newtons);

  if (num_reads == avg_num) {
    //Serial.print(force_total/avg_num); // report moving average of force
    //Serial.print(", ");
    //Serial.println(newtons/avg_num);
    num_reads = 0;
    force_total = 0;
    newtons_total = 0;
  }
  
  if (newtons > max_newtons) {
    newtons = max_newtons; // transformed force is set to max if it exceeds the max
  }

  if (force_val > force_threshold) {
    if (mode == 0) { // PWM
      analogWrite(haptic_pin, force_val*0.35);
      //analogWrite(haptic_pin, (newtons/max_newtons)*255);
      //analogWrite(haptic_pin, (force_val/1023)*255);
    } else if (mode == 1) { // frequency modulation
      tone(haptic_pin, force_val/10);
    } else if (mode == 2) { // PWM + frequency modulation
      period = (1000*10)/force_val;
      analogWrite(haptic_pin, force_val*0.35);
      delay(period/2);
      analogWrite(haptic_pin, force_val*0.35);
      delay(period/2);
//      unsigned long currentMillis = millis();
//      if ((unsigned long)(currentMillis - previousMillis) >= interval) {
//        previousMillis = currentMillis;
//      }
    }
  } else {
    analogWrite(haptic_pin, 0); // 
  }

}
