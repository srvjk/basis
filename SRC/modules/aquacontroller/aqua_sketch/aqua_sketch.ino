// библиотека для работы с протоколом 1-Wire
#include <OneWire.h>
// библиотека для работы с датчиком DS18B20
#include <DallasTemperature.h>

const int ONE_WIRE_BUS     = 10; // сигнальный пин датчика температуры
const int LEVEL_LED_PIN    = 11; // светодиод для сигнализации о низком уровне воды
const int LEVEL_SENSOR_PIN = 12;
const int SERIAL_LED_PIN   = 13;
 
// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
 
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);

int filterState = 0;
 
void setup(){
  // инициализируем работу Serial-порта
  Serial.begin(9600);
  // начинаем работу с датчиком
  sensor.begin();
  // устанавливаем разрешение датчика от 9 до 12 бит
  sensor.setResolution(12);

  pinMode(LEVEL_SENSOR_PIN, INPUT);
  pinMode(SERIAL_LED_PIN, OUTPUT);
  pinMode(LEVEL_LED_PIN, OUTPUT);
}
 
void loop(){
  if (filterState == 1) {
    digitalWrite(SERIAL_LED_PIN, HIGH);
  }
  else {
    digitalWrite(SERIAL_LED_PIN, LOW);
  }

  int level = digitalRead(LEVEL_SENSOR_PIN);

  // сигнализируем светодиодом о низком уровне воды
  if (level == 0) {
    digitalWrite(LEVEL_LED_PIN, HIGH);
  }
  else {
    digitalWrite(LEVEL_LED_PIN, LOW);
  }
  
  // переменная для хранения температуры
  float temperature;
  // отправляем запрос на измерение температуры
  sensor.requestTemperatures();
  // считываем данные из регистра датчика
  temperature = sensor.getTempCByIndex(0);

  // выводим значение датчика уровня
  Serial.print("LEVL1");
  Serial.println(level);
  delay(100);

  // выводим температуру в Serial-порт
  Serial.print("TMPR1");
  Serial.println(temperature);
  delay(100);

  // выводим состояние фильтра в Serial-порт
  Serial.print("FLTR1");
  Serial.println(filterState);
  delay(100);

  // считываем команды из Serial-порта
  if (Serial.available() > 0) {
    String str = Serial.readString();

    if (str.length() >= 5) {
      String cmdName = str.substring(0, 5);
      String cmdBody = str.substring(5);

      if (cmdName == "FLTR1") {
        int val = cmdBody.toInt();
        if (filterState != val) {
          filterState = val;
        }
      }
    }
  }
  // ждём
  delay(100);
}
