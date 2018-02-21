
/*
 * Make sure that the LED does not sink more than 12 mA from each pin.
*/
const int pin = 2; //2 is the onboard blue LED.

void setup() {
  pinMode(pin, OUTPUT); //Declare GPIO13 as output
}

void loop() {

  digitalWrite(pin, HIGH);      // turn the LED on
  delay(1000);                         // Wait 1 second
  digitalWrite(pin, LOW);       // turn the LED off
  delay(1000);                        // wait 1 second

}


