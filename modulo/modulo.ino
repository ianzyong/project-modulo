#if defined(__IMXRT1062__)
extern "C" uint32_t set_arm_clock(uint32_t frequency);
#endif

unsigned long previousMillis=0;

const int read_pin = A5;
const int haptic_pin = A0;
const int button_pin = 11;
int button_state = 0;
int force_val = 0; // variable to store the read value
int force_threshold = 0.15; // force threshold value
int mode = 0;
float force_total = 0;
float newtons_total = 0;
int num_reads = 0;
int avg_num = 40; // for reporting moving average
float newtons = 0; // to hold the transformed force value
float max_newtons = 20; // linear force value corresponding to maximum vibration
// according to our load specification (60 lbs), this should be 266.893 N
float period = 1000; // milliseconds

// log fit
float a = 79.697;
float b = 753.68;

// A0-A4: haptic motors
// A5-A9: force-sensitive resistors

void setup() {
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
  
  delay(5); // time delay
  // put your main code here, to run repeatedly:

  button_state = digitalRead(button_pin); // read the button state
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
    Serial.print("===== ");
    Serial.println(mode);
    Serial.println(" =====");
  }
  
  force_val = analogRead(read_pin); // read the force
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
      analogWrite(haptic_pin, map(newtons,force_threshold,max_newtons,100,1000));
      
      Serial.print(force_val);
      Serial.print(", ");
      Serial.println(map(newtons,force_threshold,max_newtons,100,1000));
      
    } else if (mode == 1) { // frequency modulation
      Serial.println("MODE 1");
      analogWrite(haptic_pin, 0);

      if (map(newtons,force_threshold,max_newtons,0.1,5) > 0.5){
      
        analogWriteFrequency(haptic_pin, map(newtons,force_threshold,max_newtons,0.1,5));
        
        analogWrite(haptic_pin, 170);

        Serial.print(map(newtons,force_threshold,max_newtons,0.1,5));
        Serial.print(", ");
        Serial.println(map(newtons,force_threshold,max_newtons,100,300));
        
      }
    } else if (mode == 2) { // PWM + frequency modulation
      Serial.println("MODE 2");

      analogWrite(haptic_pin, 0);

      if (map(newtons,force_threshold,max_newtons,0.1,5) > 0.5){
      
        analogWriteFrequency(haptic_pin, map(newtons,force_threshold,max_newtons,0.1,5));
        
        analogWrite(haptic_pin, map(newtons,force_threshold,max_newtons,100,250));

        Serial.print(map(newtons,force_threshold,max_newtons,0.1,5));
        Serial.print(", ");
        Serial.println(map(newtons,force_threshold,max_newtons,100,250));
        
      }
      
    }
  } else {
    digitalWrite(haptic_pin, LOW); // 
  }

}
