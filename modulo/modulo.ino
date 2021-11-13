const int read_pin = A5;
const int haptic_pin = A0;
int force_val = 0; // variable to store the read value
int force_threshold = 0; // force threshold value
float force_total = 0;
float newtons_total = 0;
int num_reads = 0;
int avg_num = 40; // for reporting moving average
float newtons = 0; // to hold the transformed force value
float max_newtons = 20; // linear force value corresponding to maximum vibration
// according to our load specification (60 lbs), this should be 266.893 N

// curve-fitting constants
//float a = -295.5;
//float b = -0.4256;
//float c = 787.4*2;

float a = -300.5;
float b = -0.5;
float c = 787.4*2;

// A0-A4: haptic motors
// A5-A9: force-sensitive resistors

// extern "C" uint32_t set_arm_clock(uint32_t frequency);

void setup() {
  // set_arm_clock(24000000);
  // put your setup code here, to run once:
  pinMode(read_pin, INPUT); // sets the digital pin as input
  Serial.begin(9600);
  pinMode(haptic_pin, OUTPUT); // sets the haptic pin as output
}

void loop() {
  delay(5); // time delay
  // put your main code here, to run repeatedly:
  force_val = analogRead(read_pin); // read the force
  force_total = force_total + force_val;
  num_reads = num_reads + 1;

  //newtons = pow((force_val-c)/a,1);
  newtons = pow((force_val-c)/a,1/b); // power transform
  //newtons = force_val;
  newtons_total = newtons_total + newtons;
  Serial.println(force_val);
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

  if (newtons > force_threshold) {
    //analogWrite(haptic_pin, (newtons/max_newtons)*255);
    //analogWrite(haptic_pin, (force_val/1023)*255);
    analogWrite(haptic_pin, force_val*0.35);
  } else {
    analogWrite(haptic_pin, 0); // 
  }

}
