#include <TimeKeeping.h>

TimeKeeper tk;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  tk.begin();
}

void loop() {
  Serial.println(tk.getMillis());

  delay(1000);
}
