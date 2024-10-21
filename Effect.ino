extern ReverbSc rev;

float feedbackValue = 0.05f; // Global variable to store feedback value  
  
void Effect() {  
  float lpFreq = 18000.0f; // Initialize with default value  
  int effectValue = 0; // Initialize effect value  
  bool effectSelected = false; // Flag variable to track whether an effect has been selected  
  
  while (!effectSelected) {  
   display.clear();  
   display.setCursor(0, 0);  
   display.print("Effect");  
   display.setCursor(0, 1);  
   display.print("1. LP Freq: ");  
   display.print(lpFreq);  
   display.setCursor(0, 2);  
   display.print("2. Feedback: ");  
   display.print(feedbackValue); // Use global variable to display feedback value  
   display.setCursor(0, 3);  
   display.print("3. Back");  
   display.update();  
  
   // Highlight selected row  
   display.setCursor(0, 1);  
   display.invertText(true);  
   display.print("1. LP Freq: ");  
   display.print(lpFreq);  
   display.invertText(false);  
   display.update();  
  
   // Wait for user to select effect  
   while (true) {  
    synth_loop();  
    hw.DebounceControls();  
  
    if (hw.encoder.Increment() != 0) {  
      if (hw.encoder.Increment() > 0) {  
       effectValue++;  
      } else {  
       effectValue--;  
      }  
      // Update highlighted row  
      display.clear();  
      display.setCursor(0, 0);  
      display.print("Effect");  
      display.setCursor(0, 1);  
      display.print("1. LP Freq: ");  
      display.print(lpFreq);  
      display.setCursor(0, 2);  
      display.print("2. Feedback: ");  
      display.print(feedbackValue); // Use global variable to display feedback value  
      display.setCursor(0, 3);  
      display.print("3. Back");  
      display.update();  
  
      // Highlight selected row  
      display.setCursor(0, effectValue + 1);  
      display.invertText(true);  
      if (effectValue == 0) {  
       display.print("1. LP Freq: ");  
       display.print(lpFreq);  
      } else if (effectValue == 1) {  
       display.print("2. Feedback: ");  
       display.print(feedbackValue); // Use global variable to display feedback value  
      } else if (effectValue == 2) {  
       display.print("3. Back");  
      }  
      display.invertText(false);  
      display.update();  
    }  
  
    if (hw.encoder.RisingEdge()) {  
      // Select effect  
      if (effectValue == 0) {  
       setLpFreq(lpFreq, effectValue); // Call the setLpFreq function  
      } else if (effectValue == 1) {  
       setFeedback(feedbackValue, effectValue); // Call the setFeedback function  
      } else if (effectValue == 2) {  
       // Go back to main menu  
       display.clear();  
       display.setCursor(0, 0);  
       display.print("MENU");  
       display.setCursor(0, 1);  
       display.print("Waveform");  
       display.setCursor(0, 2);  
       display.print("Envelope");  
       display.update();  
       effectSelected = true; // Set flag to true to exit the while loop  
       break;  
      }  
    }  
   }  
  }  
}  
  
void setFeedback(float feedback, int effectValue) {  
  display.setCursor(0, 2);  
  display.print("2. Feedback: ");  
  display.print(feedback);  
  display.update();  
  
  while (true) {  
   synth_loop();  
   hw.DebounceControls();  
  
   if (hw.encoder.Increment() != 0) {  
    if (hw.encoder.Increment() > 0) {  
      feedback += 0.01f; // Increment by 1  
    } else {  
      feedback -= 0.01f; // Decrement by 1  
    }  
    feedback = constrain(feedback, 0.0f, 1.0f);  
    rev.SetFeedback(feedback);  
    display.setCursor(0, 2);  
    display.print("2. Feedback: ");  
    display.print(feedback);  
    display.update();  
   }  
  
   if (hw.encoder.RisingEdge()) {  
    // Go back to Effect menu  
    feedbackValue = feedback; // Update global variable with new feedback value  
    display.setCursor(0, 0);  
    display.invertText(true);  
    display.print("Effect");  
    display.invertText(false);  
    display.update();  
    return;  
   }  
  }  
}  
  
void setLpFreq(float lpFreq, int effectValue) {  
  display.setCursor(0, 1);  
  display.print("1. LP Freq: ");  
  display.print(map(lpFreq, 100.0f, 20000.0f, 0, 100)); // Print LP frequency value as a percentage  
  display.print("%");  
  display.update();  
  
  while (true) {  
   synth_loop();  
   hw.DebounceControls();  
  
   if (hw.encoder.Increment() != 0) {  
    if (hw.encoder.Increment() > 0) {  
      lpFreq += 100.0f; // Increment by 100  
    } else {  
      lpFreq -= 100.0f; // Decrement by 100  
    }  
    lpFreq = constrain(lpFreq, 100.0f, 20000.0f);  
    rev.SetLpFreq(lpFreq);  
    display.setCursor(0, 1);  
    display.print("1. LP Freq: ");  
    display.print(map(lpFreq, 100.0f, 20000.0f, 0, 100)); // Print LP frequency value as a percentage  
    display.print("%");  
    display.update();  
   }  
  
   if (hw.encoder.RisingEdge()) {  
    // Go back to Effect menu  
    display.setCursor(0, 0);  
    display.invertText(true);  
    display.print("Effect");  
    display.invertText(false);  
    display.update();  
    return;  
   }  
  }  
}