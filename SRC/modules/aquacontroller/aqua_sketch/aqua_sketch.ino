// библиотека для работы с протоколом 1-Wire
#include <OneWire.h>
// библиотека для работы с датчиком DS18B20
#include <DallasTemperature.h>

// напряжение питания
#define OPERATING_VOLTAGE   5.0

// параметры датчика кислотности
#define pH1                 0.0  // нижняя измеряемая граница кислотности
#define pH2                 14.0 // верхняя измеряемая граница кислотности
#define UpH1                0.0  // напряжение, соответствующее pH1, В
#define UpH2                5.0  // напряжение, соответствующее pH2, В

const int PH_SENSOR_PIN    = A0; // сигнальный пин датчика кислотности
const int TDS_SENSOR_PIN   = A5; // сигнальный пин датчика солёности
const int ONE_WIRE_BUS     = 10; // сигнальный пин датчика температуры
const int LEVEL_LED_PIN    = 11; // светодиод для сигнализации о низком уровне воды
const int LEVEL_SENSOR_PIN = 12;
const int SERIAL_LED_PIN   = 13;
 
// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
 
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);

int filterState = 0;
int debugPrint = 1;  // включение/выключение отладочной печати

// Преобразовать значение напряжения, считанное с датчика,
// в значение кислотности.
float convertVoltageToPH(float voltage) 
{
  float UpHR = (pH2 - pH1) / (UpH2 - UpH1);
  float pH = (voltage - UpH1) * UpHR + pH1;
  return pH;
}

// Преобразовать значение напряжения, считанное с TDS-датчика,
// в значение концентрации растворённых солей (в PPM, parts per million).
float convertVoltageToTDS(float voltage)
{
  float tds = (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) * 0.5;
  return tds;
}
 
void setup() 
{
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
 
void loop() 
{
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

  // измеряем кислотность
  int pHAdcValue = analogRead(PH_SENSOR_PIN);
  float pHVoltage = pHAdcValue * OPERATING_VOLTAGE / 1024.0;
  float pH = convertVoltageToPH(pHVoltage);

  // измеряем солёность
  int tdsAdcValue = analogRead(TDS_SENSOR_PIN);
  float tdsVoltage = tdsAdcValue * OPERATING_VOLTAGE / 1024.0;
  float tds = convertVoltageToTDS(tdsVoltage);

  // отладочная информация
  if (debugPrint) {
    Serial.print("DBG01");
    Serial.println(pHAdcValue);
    delay(100);
  }
 
  // выводим значение датчика уровня
  Serial.print("LEVL1");
  Serial.println(level);
  delay(100);

  // выводим температуру в Serial-порт
  Serial.print("TMPR1");
  Serial.println(temperature);
  delay(100);

  // выводим кислотность в Serial-порт
  Serial.print("PHVL1");
  Serial.println(pH);
  delay(100);

  // выводим солёность в Serial-порт
  Serial.print("TDS01");
  Serial.println(tds);
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
