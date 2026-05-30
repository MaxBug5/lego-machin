#include <SPI.h>
#include <PowerFunctions.h>

const int IR_PIN = 4; 
PowerFunctions pf(IR_PIN, 0); 

// Масив команд швидкості LEGO Power Functions
const int legoSpeeds[] = {
  PWM_REV7, PWM_REV6, PWM_REV5, PWM_REV4, PWM_REV3, PWM_REV2, PWM_REV1, 
  PWM_BRK,                                                             
  PWM_FWD1, PWM_FWD2, PWM_FWD3, PWM_FWD4, PWM_FWD5, PWM_FWD6, PWM_FWD7  
};

volatile int currentSpeedIndex = 11; 
volatile char receivedChar = 0;
volatile bool dataReady = false;

void setup() {
  Serial.begin(115200);
  
  // ПІН D12 (MISO) робимо входом, щоб він НЕ конфліктував з периферією!
  pinMode(MISO, INPUT); 
  
  // Налаштовуємо Nano як Slave приймач SPI
  SPCR |= _BV(SPE);     // Вмикаємо SPI
  SPCR &= ~_BV(MSTR);   // Точно пересвідчуємось, що це режим Slave
  
  SPI.attachInterrupt(); // Вмикаємо переривання порту
  
  Serial.println("Ніжки SPI: D10(SS), D11(MOSI), D13(SCK). MISO(D12) вимкнено.");
  Serial.println("Nano WASD + IR готова до тесту...");
}

// Переривання SPI: ловимо 1 байт від ESP32
ISR(SPI_STC_vect) {
  receivedChar = SPDR; 
  dataReady = true;
}

int getInvertedCommand(int speedIndex) {
  return legoSpeeds[14 - speedIndex];
}

void loop() {
  if (dataReady) {
    char cmd = receivedChar;
    dataReady = false; 

    Serial.print("SPI Отримано: ");
    Serial.println(cmd);

    // Виконуємо ІК команду залежно від літери
    switch (cmd) {
      case 'w': // ВПЕРЕД
        pf.single_pwm(RED, legoSpeeds[currentSpeedIndex]);
        pf.single_pwm(BLUE, getInvertedCommand(currentSpeedIndex));
        Serial.println("ІК Надіслано: Вперед");
        break;

      case 's': // НАЗАД
        int backIndex;
        if (currentSpeedIndex > 7) backIndex = 7 - (currentSpeedIndex - 7);
        else if (currentSpeedIndex < 7) backIndex = 7 + (7 - currentSpeedIndex);
        else backIndex = 7;
        
        pf.single_pwm(RED, legoSpeeds[backIndex]);
        pf.single_pwm(BLUE, getInvertedCommand(backIndex));
        Serial.println("ІК Надіслано: Назад");
        break;

      case 'a': // ЛІВОРУЧ
        pf.single_pwm(RED, legoSpeeds[4]); 
        pf.single_pwm(BLUE, getInvertedCommand(10)); 
        Serial.println("ІК Надіслано: Ліворуч");
        break;

      case 'd': // ПРАВОРУЧ
        pf.single_pwm(RED, legoSpeeds[10]); 
        pf.single_pwm(BLUE, getInvertedCommand(4)); 
        Serial.println("ІК Надіслано: Праворуч");
        break;

      case 'x': // СТОП
        pf.single_pwm(RED, PWM_BRK);
        pf.single_pwm(BLUE, PWM_BRK);
        Serial.println("🛑 ІК Надіслано: СТОП");
        break;

      case 'e': // Швидкість +
        if (currentSpeedIndex < 14) {
          currentSpeedIndex++;
          Serial.print("Індекс швидкості: "); Serial.println(currentSpeedIndex - 7);
        }
        break;

      case 'q': // Швидкість -
        if (currentSpeedIndex > 8) { 
          currentSpeedIndex--;
          Serial.print("Індекс швидкості: "); Serial.println(currentSpeedIndex - 7);
        }
        break;
    }
  }
}