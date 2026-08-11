
const int buttonPin = 4;
const int ntcPin    = 14;

int buttonState = LOW;
float ntcProcessedValue    = 0;

float temperaturaFiltrada = 25.0;
bool coletaCompleta = false;
bool samplingEnabled = false;
int indiceAmostra = 0;
const int NUM_SAMPLES = 32;

int amostras[NUM_SAMPLES];
unsigned long ultimoTempoAmostra = 0;

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
  const float C = temperatureInCensius(ntcProcessedValue);
  const float CwithEMA = temperaturaFiltrada;
  const float V = tension(ntcProcessedValue);
  Serial.printf("NTC raw value = %.0f / Tension = %.1fV / Temperature = %.1f°C %.1f°C (EMA)", ntcProcessedValue, V, C, CwithEMA);
  Serial.println("");
}

float tension(float analogValue)
{
  return (analogValue / 4095.0) * 3.3; // linear
}

float temperatureInCensius(float analogValue)
{
  const float BETA = 3950;
  const float maxAnalogValue = 4096 - 1;
  return 1.0 / (log(1.0 / (maxAnalogValue / analogValue - 1.0)) / BETA + 1.0 / 298.15) - 273.15;
}

void buttonPressed()
{

}

void buttonReleased()
{
  ntcEnableSampling();
}

void ntcEnableSampling()
{
  samplingEnabled = true;
}

void ntcColetaDeAmostrasCompletaEvent()
{
  ntcProcessedValue = ntcProcessedValueRemovingOutliersFromSamples();
  temperaturaFiltrada = ntcProcessedTemperatureWithEMAFilter(ntcProcessedValue);

  viewNtcInvalidate();
}

float ntcProcessedTemperatureWithEMAFilter(float ntcProcessedVal)
{
  float tempAtual = constrain(temperatureInCensius(ntcProcessedVal), -273, 1000);
  // Filtro EMA (Exponential Moving Average)
  const float EMA_ALPHA = 0.15;
  return EMA_ALPHA * tempAtual + (1.0 - EMA_ALPHA) *
    constrain(temperaturaFiltrada, -273, 1000);
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

void updateModel()
{
  coletarEProcessarAmostras();
}

void coletarEProcessarAmostras()
{
  if (!samplingEnabled) return;
  // --- Coleta não-bloqueante das amostras ---
  coletarAmostras();

  if (coletaCompleta) {
    samplingEnabled = false;
    ntcColetaDeAmostrasCompletaEvent();

    // prepara para o próximo clique no botão
    coletaCompleta = false;
    indiceAmostra = 0;
  }
}

// Coleta uma amostra por vez, sem travar
void coletarAmostras()
{
  if (coletaCompleta) return;

  const unsigned long INTERVALO_AMOSTRAS = 300; // µs
  unsigned long agora = micros();
  if (agora - ultimoTempoAmostra >= INTERVALO_AMOSTRAS) {
    ultimoTempoAmostra = agora;
    
    amostras[indiceAmostra] = analogRead(ntcPin);
    indiceAmostra++;

    if (indiceAmostra >= NUM_SAMPLES) {
      coletaCompleta = true;
    }
  }
}

// Processa as amostras já coletadas (média aparada)
float ntcProcessedValueRemovingOutliersFromSamples()
{
  const int len = sizeof(amostras) / sizeof(amostras[0]);
  std::sort(amostras, amostras + len);

  // Descarta 25% de cada lado
  int inicio = len / 4;
  int fim = len - inicio;
  long soma = 0;
  int contagem = 0;

  for (int i = inicio; i < fim; i++) {
    soma += amostras[i];
    contagem++;
  }

  float mediaADC = (float)soma / contagem;
  return (mediaADC <= 0 || mediaADC >= 4095) ? -999 : mediaADC;
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
  updateModel();
  renderState();
}
