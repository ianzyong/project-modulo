#if defined(__IMXRT1062__)
extern "C" uint32_t set_arm_clock(uint32_t frequency);
#endif

unsigned long previousMillis=0;

const int read_pin = A5;
const int haptic_pin = A0;
const int button_pin = 11;
const int led1_pin = 2;
const int led2_pin = 3;
const int red_led_pin = 4;
const int battery_pin = 15;
const int led_brightness = 1;
int button_state = 0;
int force_val = 0; // variable to store the read value
int force_threshold = 0.02; // force threshold value
int del_threshold = 3; // delta threshold value
int del_max = 10;
int mode = 0;
int countdown = 0; // ticks
int countdown_max = 100; // ticks
float force_total = 0;
float newtons_total = 0;
int num_reads = 0;
int avg_num = 40; // for reporting moving average
float newtons = 0; // to hold the transformed force value
float del_newtons = 0;
float max_newtons = 20; // linear force value corresponding to maximum vibration
// according to our load specification (60 lbs), this should be 266.893 N
float period = 1000; // 
float del_force_val = 0;
float low_batt_per = 10;

// log fit
float a = 79.697;
float b = 753.68;

// A0-A4: haptic motors
// A5-A9: force-sensitive resistors

void setup() {
  // put your setup code here, to run once:
  pinMode(read_pin, INPUT); // sets the digital pin as input
  pinMode(button_pin, INPUT);
  pinMode(battery_pin, INPUT);
  pinMode(haptic_pin, OUTPUT); // sets the haptic pin as output
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  Serial.begin(9600);
  //while (!Serial) ;
//#if defined(__IMXRT1062__)
    //set_arm_clock(16000000);
    //Serial.print("F_CPU_ACTUAL=");
    //Serial.println(F_CPU_ACTUAL);
//#endif
}

void loop() {
  // if battery is low
  if (100*analogRead(battery_pin)/573.5 < low_batt_per) {
    //digitalWrite(red_led_pin, HIGH);
    analogWrite(red_led_pin, led_brightness);
  } else {
    //digitalWrite(red_led_pin, LOW);
    analogWrite(red_led_pin, 0);
  }
  
  delay(5); // time delay
  // put your main code here, to run repeatedly:

  button_state = digitalRead(button_pin); // read the button state
  if(button_state == 0) { // button is pressed
    // turn on LEDs to indicate mode
    if (mode == 0) {
      analogWrite(led2_pin, led_brightness);
      //digitalWrite(led1_pin, HIGH);
    } else if (mode == 1) {
      analogWrite(led1_pin, led_brightness);
      analogWrite(led2_pin, led_brightness);
      //digitalWrite(led2_pin, HIGH);
    } else if (mode == 2) {
      analogWrite(led1_pin, led_brightness);
      //digitalWrite(led1_pin, HIGH);
      //digitalWrite(led2_pin, HIGH);
    }
    
    while(button_state == 0) { // wait until the button is released
      delay(1);
      button_state = digitalRead(button_pin);
    }

    // turn off LEDs

    analogWrite(led1_pin, 0);
    analogWrite(led2_pin, 0);

    //digitalWrite(led1_pin, LOW);
    //digitalWrite(led2_pin, LOW);
    
    if (mode < 2) {
      mode = mode + 1;
    } else {
      mode = 0;
    }
    Serial.print("===== ");
    Serial.println(mode);
    Serial.println(" =====");
  }

  del_force_val = abs(analogRead(read_pin) - force_val);
  force_val = analogRead(read_pin); // read the force
  del_newtons = exp((del_force_val-b)/a);
  newtons = exp((force_val-b)/a);
  

  force_total = force_total + force_val;
  num_reads = num_reads + 1;

  newtons_total = newtons_total + newtons;

  if (num_reads == avg_num) {
    num_reads = 0;
    force_total = 0;
    newtons_total = 0;
  }
  
  if (newtons > max_newtons) {
    newtons = max_newtons; // transformed force is set to max if it exceeds the max
  }

  if (newtons > force_threshold) {
    if (mode == 0) { // PWM
      Serial.println("MODE 0");

      analogWriteFrequency(haptic_pin, 25000);
      analogWrite(haptic_pin, map(newtons,force_threshold,max_newtons,120,4023));
      
      Serial.print(force_val);
      Serial.print(", ");
      Serial.println(map(newtons,force_threshold,max_newtons,100,1023));
      
    } else if (mode == 1) { // delta modulation
      Serial.println("MODE 1");

      analogWrite(haptic_pin, 0);
      analogWriteFrequency(haptic_pin, 25000);

      if (del_force_val > del_threshold || countdown > 0) {

        if (countdown == 0) {
          countdown = countdown_max;
        }
        analogWrite(haptic_pin, 255);
        //analogWrite(haptic_pin, map(del_force_val,0,del_max,500,1000));
        //Serial.print(del_force_val);
        //Serial.print(", ");
        //Serial.print("del_input:");
        Serial.print(del_force_val);
        Serial.print(", ");
        //Serial.print("output:");
        Serial.println(255);

        countdown = countdown - 1;

      } else {
        Serial.print(del_force_val);
        Serial.print(", ");
        Serial.println(0);
      }
      
    } else if (mode == 2) { // PWM + frequency modulation
      Serial.println("MODE 2");

      analogWriteFrequency(haptic_pin, map(newtons,force_threshold,max_newtons,0.1,5));
        
      analogWrite(haptic_pin, map(newtons,force_threshold,max_newtons,100,4050));

      Serial.print(map(newtons,force_threshold,max_newtons,0.1,5));
      Serial.print(", ");
      Serial.println(map(newtons,force_threshold,max_newtons,100,250));
      
    }
  } else {
    digitalWrite(haptic_pin, LOW); // 
  }

}
