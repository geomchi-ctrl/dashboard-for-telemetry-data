#include <Wire.h>
#include <Adafruit_INA219.h>
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// Configurações da rede Wi-Fi
const char* ssid = "A54";
const char* password = "96803316";

const char* serverUrl = "http://127.0.0.1:8080/update"; // Substitua pelo IP do servidor

// Definição dos pinos dos motores
#define motorTractionPWM 12 // PWM para o motor de tração
#define motorTractionPin1 27 // Pino 1 do motor de tração (IN3)
#define motorTractionPin2 14 // Pino 2 do motor de tração (IN4)
#define motorSteeringPWM  33 // PWM para direção
#define motorSteeringPin1 25 // Pino 1 do motor de direção (IN1)
#define motorSteeringPin2 26 // Pino 2 do motor de direção (IN2)

int pwmValue = 110;  // Valor inicial de PWM para o motor de tração

Adafruit_INA219 ina219; // Objeto para o INA219

void setup() {
  Serial.begin(115200); // Taxa de atualização do monitor serial.
  Dabble.begin("POLARIZADOS"); // Nome do Bluetooth.
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");

  // Inicializa o INA219
  if (!ina219.begin()) {
    Serial.println("Erro ao inicializar o INA219! Verifique as conexões.");
    while (1); // Para o código se não inicializar
  }

  // Configura os pinos dos motores como saída
  pinMode(motorTractionPWM, OUTPUT);
  pinMode(motorTractionPin1, OUTPUT);
  pinMode(motorTractionPin2, OUTPUT);
  pinMode(motorSteeringPWM, OUTPUT);
  pinMode(motorSteeringPin1, OUTPUT);
  pinMode(motorSteeringPin2, OUTPUT);

  analogReadResolution(12); // Configura a resolução ADC (0-4095)
}

void loop() {
  Dabble.processInput(); // Atualiza os dados do smartphone

  // Medições do INA219
  float current_mA = ina219.getCurrent_mA();

  // Controle dos motores
  bool motorState1 = false;
  bool motorState2 = false;

  if (GamePad.isUpPressed()) { 
    motorState1 = true; // Motor 1 ligado
    digitalWrite(motorTractionPin1, HIGH); 
    digitalWrite(motorTractionPin2, LOW);
    analogWrite(motorTractionPWM, pwmValue);
  } else if (GamePad.isDownPressed()) {
    motorState1 = true;
    digitalWrite(motorTractionPin1, LOW); 
    digitalWrite(motorTractionPin2, HIGH);
    analogWrite(motorTractionPWM, pwmValue);
  } else { 
    digitalWrite(motorTractionPin1, LOW);
    digitalWrite(motorTractionPin2, LOW);
    analogWrite(motorTractionPWM, 0);
  }

  if (GamePad.isSquarePressed()) { 
    motorState2 = true; // Motor 2 ligado
    digitalWrite(motorSteeringPin1, LOW); 
    digitalWrite(motorSteeringPin2, HIGH);
    analogWrite(motorSteeringPWM, 225);
  } else if (GamePad.isCirclePressed()) {
    motorState2 = true;
    digitalWrite(motorSteeringPin1, HIGH); 
    digitalWrite(motorSteeringPin2, LOW);
    analogWrite(motorSteeringPWM, 225);
  } else {
    digitalWrite(motorSteeringPin1, LOW);
    digitalWrite(motorSteeringPin2, LOW);
    analogWrite(motorSteeringPWM, 0);
  }

  // Envio de telemetria
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.setTimeout(5000);

      if (http.begin(serverUrl)) {
          http.addHeader("Content-Type", "application/json");

          // Criação do JSON com os novos campos
          StaticJsonDocument<200> jsonDoc;
          jsonDoc["current"] = current_mA;
          jsonDoc["speed"] = pwmValue;
          jsonDoc["motor_state1"] = motorState1 ? "Ligado" : "Desligado";
          jsonDoc["motor_state2"] = motorState2 ? "Ligado" : "Desligado";
          jsonDoc["wifi-state"] = (WiFi.status() == WL_CONNECTED) ? "ligado" : "desligado";

          String payload;
          serializeJson(jsonDoc, payload);

          // Envia o POST
          int httpResponseCode = http.POST(payload);

          if (httpResponseCode > 0) {
              Serial.println("Dados enviados com sucesso. Código HTTP: " + String(httpResponseCode));
          } else {
              Serial.println("Erro ao enviar. Código HTTP: " + String(httpResponseCode));
          }

          http.end(); 
      } else {
          Serial.println("Erro ao conectar ao servidor.");
      }
  } else {
      Serial.println("Wi-Fi desconectado!");
  }

  delay(100); // Pausa
}
