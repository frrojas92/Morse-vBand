#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>

// =====================================================
// Morse-vBand
// Seeed Studio XIAO RP2040 + Putikeeg
//
// TRRS:
//   TIP    -> D0  -> Paddle DERECHO  -> DAH
//   RING1  -> D1  -> Paddle IZQUIERDO -> DIT
//   RING2  -> GND
//   SLEEVE -> Sin conectar
//
// USB HID:
//   IZQUIERDO -> LEFT CTRL
//   DERECHO   -> RIGHT CTRL
//
// El WPM, keyer e iambic son controlados por vBand/app.
// =====================================================


// ---------- PADDLES ----------
constexpr uint8_t PADDLE_DIT = D1;
constexpr uint8_t PADDLE_DAH = D0;


// ---------- RGB XIAO RP2040 ----------
constexpr uint8_t RGB_POWER = 11;
constexpr uint8_t RGB_DATA  = 12;

Adafruit_NeoPixel rgb(
  1,
  RGB_DATA,
  NEO_GRB + NEO_KHZ800
);


// ---------- DEBOUNCE ----------
constexpr uint32_t DEBOUNCE_MS = 2;


// ---------- ESTADOS ----------
bool ditState = false;
bool dahState = false;

bool lastRawDit = false;
bool lastRawDah = false;

uint32_t ditChangedAt = 0;
uint32_t dahChangedAt = 0;


// =====================================================
// LED RGB
// =====================================================

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}


void updateRGB() {

  // Reposo
  if (!ditState && !dahState) {
    setRGB(0, 0, 255);        // Azul
  }

  // DIT
  else if (ditState && !dahState) {
    setRGB(0, 255, 0);        // Verde
  }

  // DAH
  else if (!ditState && dahState) {
    setRGB(255, 0, 0);        // Rojo
  }

  // Ambos
  else {
    setRGB(255, 0, 255);      // Violeta
  }
}


// =====================================================
// DEBOUNCE
// =====================================================

bool debounceInput(
  uint8_t pin,
  bool &stableState,
  bool &lastRawState,
  uint32_t &changedAt
) {

  bool raw = (digitalRead(pin) == LOW);
  uint32_t now = millis();

  if (raw != lastRawState) {
    lastRawState = raw;
    changedAt = now;
  }

  if (
    raw != stableState &&
    (now - changedAt) >= DEBOUNCE_MS
  ) {
    stableState = raw;
    return true;
  }

  return false;
}


// =====================================================
// HID
// =====================================================

void updateDIT() {

  if (ditState) {
    Keyboard.press(KEY_LEFT_CTRL);
    Serial.println("DIT DOWN");
  }
  else {
    Keyboard.release(KEY_LEFT_CTRL);
    Serial.println("DIT UP");
  }
}


void updateDAH() {

  if (dahState) {
    Keyboard.press(KEY_RIGHT_CTRL);
    Serial.println("DAH DOWN");
  }
  else {
    Keyboard.release(KEY_RIGHT_CTRL);
    Serial.println("DAH UP");
  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // Paddles
  pinMode(PADDLE_DIT, INPUT_PULLUP);
  pinMode(PADDLE_DAH, INPUT_PULLUP);

  // RGB grande
  pinMode(RGB_POWER, OUTPUT);
  digitalWrite(RGB_POWER, HIGH);

  rgb.begin();
  rgb.setBrightness(255);
  rgb.clear();
  rgb.show();

  // USB HID
  Keyboard.begin();
  Keyboard.releaseAll();

  // Estado inicial
  ditState = (digitalRead(PADDLE_DIT) == LOW);
  dahState = (digitalRead(PADDLE_DAH) == LOW);

  lastRawDit = ditState;
  lastRawDah = dahState;

  updateRGB();

  delay(300);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" Morse-vBand READY");
  Serial.println("==============================");
  Serial.println("DIT -> LEFT CTRL");
  Serial.println("DAH -> RIGHT CTRL");
  Serial.println("WPM -> controlado por vBand");
  Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  bool changed = false;


  // DIT
  if (
    debounceInput(
      PADDLE_DIT,
      ditState,
      lastRawDit,
      ditChangedAt
    )
  ) {
    updateDIT();
    changed = true;
  }


  // DAH
  if (
    debounceInput(
      PADDLE_DAH,
      dahState,
      lastRawDah,
      dahChangedAt
    )
  ) {
    updateDAH();
    changed = true;
  }


  // Solo actualizamos el WS2812 si algo cambió
  if (changed) {
    updateRGB();
  }
}