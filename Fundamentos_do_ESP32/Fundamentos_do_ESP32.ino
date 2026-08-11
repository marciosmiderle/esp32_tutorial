const int buttonPin = 4;
const int ledPin = 12; // Pino conectado ao LED
int buttonState = LOW;
int ledState = LOW;
bool viewValid = false;

void viewInvalidate()
{
  viewValid = false;
}

void viewSetValid()
{
  viewValid = true;
}

void logLedState(int state)
{
  Serial.print("LED state is ");
  if (state == LOW) {
    Serial.println("LOW");
  }
  else {
    Serial.println("HIGH");
  }
}

void buttonPressed()
{

}

void buttonReleased()
{
  if (ledState == HIGH) {
    ledState = LOW;
  } else {
    ledState = HIGH;
  }
  viewInvalidate();
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
  if (!viewValid) {
    digitalWrite(ledPin, ledState);
    logLedState(ledState);
    viewSetValid();
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  processInput();
  renderState();
}
