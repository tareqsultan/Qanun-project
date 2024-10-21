extern float attackTime;  
extern float decayTime;

void Envelope() {  
  display.clear();  
  display.setCursor(0, 0);  
  display.print("Envelope");  
  display.setCursor(0, 1);  
  display.print("Attack: ");  
  display.print(attackTime, 2); // Display with 2 decimal places  
  display.setCursor(0, 2);  
  display.print("Decay: ");  
  display.print(decayTime, 2); // Display with 2 decimal places  
  display.update();  
}

