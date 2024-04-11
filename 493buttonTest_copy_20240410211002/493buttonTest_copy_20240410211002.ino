/*
 * This code uses multiple parts of code from Jon Froehlich, http://makeabilitylab.io, and ChatGPT 3.5
 * Including crossfade, buttonState, photocell segments. 
 * https://makeabilitylab.github.io/physcomp/arduino/buttons.html#internal-pull-up-resistors
 * https://makeabilitylab.github.io/physcomp/sensors/photoresistors.html 
 * https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html
 * 
 * 
 * By Jon Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 * 
 * For a walkthrough and circuit diagram, see:
 * https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade
 * 
 * Based, in part, on:
 * - https://learn.adafruit.com/adafruit-arduino-lesson-3-rgb-leds?view=all
 * - https://gist.github.com/jamesotron/766994 (link no longer available)
 */


// RGB LED. See: https://makeabilitylab.github.io/physcomp/arduino/rgb-led 
// If you are working with a common cathode RGB LED, set this to false.
// Note Adafruit sells a common anode design: https://www.adafruit.com/product/159

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;
const int VOLTAGE_INPUT_PIN = A5;
const int MAX_COLOR_VALUE = 255;

// Change this to based on whether you are using a common anode or common cathode
const boolean COMMON_ANODE = false; 
const int DELAY_MS = 20; // delay in ms between changing colors

const int PHOTOCELL_INPUT_PIN = A0;
const int HAT_INPUT_PIN = A2;
// mess with ranges 
const int MIN_PHOTOCELL_VAL = 200; 
const int MAX_PHOTOCELL_VAL = 1300; 
const boolean PHOTOCELL_IS_R2_IN_VOLTAGE_DIVIDER = true; // set false if photocell is R1

enum RGB{
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

int _rgbLedValues[] = {255, 0, 0}; // Red, Green, Blue
enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;
const int FADE_STEP = 5; 

const int INPUT_BUTTON_PIN = 2;
const int OUTPUT_LED_PIN = LED_BUILTIN;
int currentState = 0; // Variable to track the current state
int clickCount = 0;   // Variable to count button clicks

const int BUTTON_UNPRESSED = 0;
const int BUTTON_PRESSED = 1;

int previousButtonState = BUTTON_UNPRESSED; // Initially assume button is not pressed

void setup() {
  pinMode(INPUT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_LED_PIN, OUTPUT);
  pinMode(VOLTAGE_INPUT_PIN, INPUT);  
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(HAT_INPUT_PIN, INPUT);
  pinMode(PHOTOCELL_INPUT_PIN, INPUT);

  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE]);
  delay(DELAY_MS);

  Serial.begin(9600); 
}

void loop() {
  // * 
  //* By Jon Froehlich
  //* @jonfroehlich
  //* http://makeabilitylab.io
  // Code modified from textbook https://makeabilitylab.github.io/physcomp/arduino/buttons.html#pull-down-resistor-configuration 
  //*/

  
  int buttonState = digitalRead(INPUT_BUTTON_PIN);

  //  Button state code was adpated by ChatGPT 3.5 
  if (buttonState == BUTTON_PRESSED && previousButtonState == BUTTON_UNPRESSED) {
    // Button was just pressed (transition from 0 to 1)
    clickCount++;
    // Change state based on click count
    if (clickCount == 1) {
      currentState = 1;
    } else if (clickCount == 2) {
      currentState = 2;
    } else if (clickCount == 3) {
      currentState = 3;
    } else if (clickCount >= 4) {
      currentState = 0; // Return to first state after fourth click
      clickCount = 0;   // Reset click count
    }
  }

  // Perform actions based on the current state
  switch (currentState) {
    case 0: // First state (LED off)
      digitalWrite(OUTPUT_LED_PIN, LOW);
      break;
    case 1: // Second state (LED on)
      // digitalWrite(OUTPUT_LED_PIN, HIGH);
      inverseCrossFade();
      // int voltage = analogRead(VOLTAGE_INPUT_PIN);  // Read analog voltage from A5
      // int brightness = map(voltage, 0, 1023, 0, 255);  // Map the voltage range to LED brightness (0-255)
      break;
    case 2: // Third state (LED on)
      // digitalWrite(OUTPUT_LED_PIN, HIGH);
      variableResistor();
      break;
    case 3: // Fourth state (LED on)
      creativeFeature();
      break;
    default:
      break;
  }
  Serial.println(currentState);

  previousButtonState = buttonState; // Update previous state
  delay(30);
}


void variableResistor() {
  int voltage = analogRead(VOLTAGE_INPUT_PIN);  // Read analog voltage from A5
  int brightness = map(voltage, 0, 1023, 0, 255);  // Map the voltage range to LED brightness (0-255)

      // Serial.println(voltage);  // Print the voltage value for debugging
  Serial.println(MAX_COLOR_VALUE - brightness);
      // Control LED brightness based on the analog voltage
  analogWrite(RGB_BLUE_PIN, MAX_COLOR_VALUE - brightness);
  analogWrite(RGB_GREEN_PIN, MAX_COLOR_VALUE - brightness);
  analogWrite(RGB_RED_PIN, MAX_COLOR_VALUE - brightness);
}


void inverseCrossFade() { 
  // Code taken from * https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html

  _rgbLedValues[_curFadingUpColor] += FADE_STEP;
  _rgbLedValues[_curFadingDownColor] -= FADE_STEP;

  // Check to see if we've reached our maximum color value for fading up
  // If so, go to the next fade up color (we go from RED to GREEN to BLUE
  // as specified by the RGB enum)
  // This fade code partially based on: https://gist.github.com/jamesotron/766994
  if(_rgbLedValues[_curFadingUpColor] > MAX_COLOR_VALUE){
    _rgbLedValues[_curFadingUpColor] = MAX_COLOR_VALUE;
    _curFadingUpColor = (RGB)((int)_curFadingUpColor + 1);

    if(_curFadingUpColor > (int)BLUE){
      _curFadingUpColor = RED;
    }
  }

  // Check to see if the current LED we are fading down has gotten to zero
  // If so, select the next LED to start fading down (again, we go from RED to 
  // GREEN to BLUE as specified by the RGB enum)
  if(_rgbLedValues[_curFadingDownColor] < 0){
    _rgbLedValues[_curFadingDownColor] = 0;
    _curFadingDownColor = (RGB)((int)_curFadingDownColor + 1);

    if(_curFadingDownColor > (int)BLUE){
      _curFadingDownColor = RED;
    }
  }

  // Set the color and then delay
  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE]);
  delay(DELAY_MS);
}


void setColor(int red, int green, int blue)
{
  // https://makeabilitylab.github.io/physcomp/sensors/photoresistors
  // The map function does not constrain output outside of the provided range
  // so, we need to make sure that things are within range for the led

  // Read the photo-sensitive resistor value. If you have the photocell resistor hooked 
  // up as Rtop in the voltage divider (that is, one leg of the photocell is connected to 5V), 
  // then higher values correspond to brightness. If you have the photocell hooked up as Rbottom 
  // in the voltage divider (that is, one leg of the photocell is connected to GND), then
  // higher values correspond to darkness.
  int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);
  // Remap the value for output. 
  int newVal = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 0, 255);
  newVal = constrain(newVal, 0, 255);

  if(photocellVal > 320){
    // check if each of these are negative and change to 0 
    red = red - newVal / 2; 
    blue = blue - newVal / 2;
    green = green - newVal / 2;
  } else { 
    red = 0;
    green = 0;
    blue = 0;
  } 
}

void creativeFeature() {


  int voltage = analogRead(HAT_INPUT_PIN);

  // Code created thanks to ChatGPT 3.5
  // Set RGB LED to purple (red + blue) when voltage is HIGH
  if (voltage < 500) {
    analogWrite(RGB_RED_PIN, 255);
    analogWrite(RGB_GREEN_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 255);
    delay(500);  // Blink for 500 milliseconds
    analogWrite(RGB_RED_PIN, 0);
    analogWrite(RGB_BLUE_PIN, 0);
    delay(500);  // Wait for 500 milliseconds
  }
}
