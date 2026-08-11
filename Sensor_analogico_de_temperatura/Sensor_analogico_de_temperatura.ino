
const int buttonPin = 4;
const int ntcPin    = 14;

int buttonState = LOW;
int ntcState    = 0;

bool viewNtcValid = true;

void viewNtcInvalidate()
{
  viewNtcValid = false;
}

void viewNtcSetValid()
{
  viewNtcValid = true;
}

void logNtcState()
{
  const float C = temperatureInCensius(ntcState);
  const float V = tension(ntcState);
  Serial.printf("NTC raw value = %u / Tension = %.1fV / Temperature = %.1f°C", ntcState, V, C);
  Serial.println("");
}

float tension(int analogValue)
{
  return (analogValue / 4095.0) * 3.3; // linear
}

float temperatureInCensius(int analogValue)
{
  const float BETA = 3950;
  const float maxAnalogValue = 4096 - 1;
  return 1 / (log(1 / (maxAnalogValue / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;
}

void buttonPressed()
{

}

void buttonReleased()
{
  ntcReadValue();
  viewNtcInvalidate();
}

void ntcReadValue()
{
  ntcState = analogRead(ntcPin);
}

void processInput()
{
  int buttonCurrentState = digitalRead(buttonPin);
  if (buttonCurrentState == HIGH && buttonState == LOW) {
    buttonPressed();
    buttonState = HIGH;
  }
  if (buttonCurrentState == LOW && buttonState == HIGH) {
    buttonReleased();
    buttonState = LOW;
  }
}

void renderState() 
{
  renderNTCView();
}

void renderNTCView()
{
  if (!viewNtcValid) {
    logNtcState();
    viewNtcSetValid();
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ntcPin, INPUT);
}

void loop()
{
  processInput();
  renderState();
}
