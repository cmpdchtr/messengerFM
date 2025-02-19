#include <SPI.h>
#include <RH_ASK.h>

//  Визначення  пінів  для  підключення  RF-модуля.
//  Зазвичай  використовуються  такі  піни:
#define RF_TX_PIN 12    //  Пін  для  передачі  даних (D12  на  ESP32).
#define RF_RX_PIN 13    //  Пін  для  прийому  даних (D13  на  ESP32).
#define RF_PTT_PIN 2   //  Пін, який ініціює передачу.  Деякі модулі не потребують PTT.  Якщо  ваш  не  потребує,  закоментуйте  цей  рядок.

//  Швидкість  передачі  даних  (біт/с).  Рекомендується  використовувати  невелику  швидкість  для  більшої  надійності.
#define RF_SPEED 2000

// Створення об'єкта RH_ASK.
// Параметри: швидкість, пін прийому, пін передачі, пін PTT, чи є PTT інвертованим (false для більшості модулів).
RH_ASK rf_driver(RF_SPEED, RF_RX_PIN, RF_TX_PIN, RF_PTT_PIN, false);

//  Одноразовий  блокнот  (ключ).  ПОВИНЕН  БУТИ  ОДНАКОВИМ  НА  ОБОХ  ПРИСТРОЯХ!
//  Для  реальної  безпеки  ключ  має  бути  згенерований  випадково  і  мати  таку  ж  довжину,  як  і  найдовше  повідомлення.
//  Тут  для  простоти  використовуємо  короткий  ключ.
const byte key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
const size_t keyLength = sizeof(key);


// Функція для шифрування повідомлення за допомогою одноразового блокнота.
void encryptMessage(byte *message, size_t messageLength) {
    for (size_t i = 0; i < messageLength; i++) {
        message[i] = message[i] ^ key[i % keyLength]; // XOR з відповідним байтом ключа (циклічне використання ключа)
    }
}

// Функція для дешифрування повідомлення за допомогою одноразового блокнота.
void decryptMessage(byte *message, size_t messageLength) {
    // Дешифрування ідентичне шифруванню в одноразовому блокноті (XOR).
    encryptMessage(message, messageLength);
}

void setup() {
    Serial.begin(115200);

    if (!rf_driver.init()) {
        Serial.println("RF module initialization failed!");
        while (1);  //  Зависаємо,  якщо  ініціалізація  не  вдалася.
    }
    Serial.println("RF module initialized successfully.");
}

void loop() {
    //  РЕЖИМ  ПЕРЕДАЧІ
    byte message[] = "Hello World!"; //  Повідомлення  для  передачі.
    size_t messageLength = sizeof(message) -1 ; // Get size and do not count end-of-string character

    Serial.print("Sending: ");
    Serial.println((char*)message); //Виводимо оригінальне повідомлення

    encryptMessage(message, messageLength); //  Шифруємо  повідомлення.

    rf_driver.send(message, messageLength);  //  Відправляємо  зашифроване  повідомлення.
    rf_driver.waitPacketSent(); //  Чекаємо  завершення  передачі.

    Serial.print("Sent encrypted: ");
      for (int i = 0; i < messageLength; i++)
      {
        Serial.print(message[i], HEX); // Виводимо зашифроване повідомлення в HEX
        Serial.print(" ");
      }
      Serial.println();


    delay(1000);  //  Затримка  перед  наступною  передачею.


    //  РЕЖИМ  ПРИЙОМУ  (можна  перенести  в  окремий  файл  і  завантажити  на  інший  ESP32)
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];  //  Буфер  для  прийому  повідомлення.
    uint8_t buflen = sizeof(buf);

    if (rf_driver.recv(buf, &buflen)) {  //  Якщо  отримано  повідомлення.
        Serial.print("Received encrypted: ");
        for (int i = 0; i < buflen; i++)
        {
            Serial.print(buf[i], HEX); // Виводимо отримане зашифроване повідомлення
            Serial.print(" ");
        }
        Serial.println();

        decryptMessage(buf, buflen); //  Дешифруємо  повідомлення.
        
        Serial.print("Received decrypted: ");
        Serial.println((char*)buf);
    }
      delay(1000);

}
