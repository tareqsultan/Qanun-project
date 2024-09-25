const int signal_pin = 2;
int state;     
         
void setup() {
  Serial.begin(115200);
  pinMode(signal_pin, INPUT);
}
 
void loop() {
  state = digitalRead(signal_pin);
  Serial.println(state);
  if(state == HIGH)
  Serial.println(" TOUCH Detected!");
  delay(500);
}
