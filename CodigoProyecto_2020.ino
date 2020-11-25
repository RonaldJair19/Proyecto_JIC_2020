#include <DHT_U.h>
#include <Ticker.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

void sensores_humedad();
void sensores_temperatura();
void transmicion_ubidots(int indice, double valor_sensor);
void sensor_luz();
void sensor_mq135();
double AnalogTemperature(int voltaje);
void sensor_mq2();
void sensor_mq135();
void relays(int pin, int estado);
//void pausa_ticker();


/****************************************
 * Definicion de constantes
 ****************************************/

 //Se utiliza WIFI RX --> ARDUINO TX y WIFI TX --> ARDUINO RX
namespace {
  const char * USER_AGENT = "UbidotsESP8266"; // Assgin the user agent
  const char * VERSION =  "1.0"; // Assign the version
  const char * METHOD = "POST"; // Set the method
  const char * TOKEN = "BBFF-WKtYtJ8EmI1J4pMM11V5uwn1rcVM5E"; // Assign your Ubidots TOKEN
  const char * DEVICE_LABEL = "ESP8266"; // Assign the device label
  const char * VARIABLE_LABEL = "Temperatura"; // Assign the variable label
  const char * VARIABLE_LABEL1 = "Humedad"; // Assign the variable label
  const char * VARIABLE_LABEL2 = "Gas(Humo-Propano)"; // Assign the variable label
  const char * VARIABLE_LABEL3 = "Gas(Amoniaco)"; // Assign the variable label
  const char * VARIABLE_LABEL4 = "Luz"; // Assign the variable label
  const char * VARIABLE_LABEL5 = "Abanico"; // Assign the variable label
  const char * VARIABLE_LABEL6 = "Bombilla"; // Assign the variable label
}

/* Space to store values to send */
char str_sensor_temp[10];
char str_sensor_humedad[10];
char str_sensor_amoniaco[10];
char str_sensor_humo[10];
char str_sensor_luz[2];
char str_relay_abanico[4];
char str_relay_bombilla[4];

//Definición de pines
#define pin_dht11 9
#define pin_dht22 10
#define pin_relay_bombillo 11
#define pin_relay_abanico 12
#define pin_ldr A0
#define pin_mq135 A1
#define pin_mq2 A2
#define mostrarenpantalla(cadena, variable){ lcd.clear(); lcd.setCursor(0,0); lcd.print(variable); lcd.setCursor(3,1); lcd.print(cadena);}

/****************************************
 * Cuerpo del programa princial
 ****************************************/
double temperatura_promedio, humedad_promedio;
float dht11_humedad, dht11_temperatura, dht22_temperatura, dht22_humedad,valor_luz, valor_mq135, valor_mq2;

Ticker Tic_Temperatura(sensores_temperatura,10000);
Ticker Tic_Luz(sensor_luz,12000);
Ticker Tic_Humedad(sensores_humedad,13000);
Ticker Tic_Humo(sensor_mq2,14000);
Ticker Tic_Amoniaco(sensor_mq135,15000);

DHT dht11 (pin_dht11, DHT11);
DHT dht22 (pin_dht22, DHT22);

void setup() {
  Serial.begin(115200);
  dht11.begin();
  dht22.begin();
  Tic_Temperatura.start();
  Tic_Luz.start();
  Tic_Humedad.start();
  Tic_Humo.start();
  Tic_Amoniaco.start();
  pinMode(pin_relay_abanico, OUTPUT);
  pinMode(pin_relay_bombillo, OUTPUT);
  //pinMode(pin_dht11, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.begin(16,2);
  lcd.clear();
}

void loop() {
  Tic_Temperatura.update();
  Tic_Luz.update();
  Tic_Humedad.update();
  Tic_Humo.update();
  Tic_Amoniaco.update();
}



void transmicion_ubidots(int indice, double valor_sensor){
  char* command = (char *) malloc(sizeof(char) * 128);
  sprintf(command, "init#");
  sprintf(command, "%s%s/%s|%s|%s|", command, USER_AGENT, VERSION, METHOD, TOKEN);
  sprintf(command, "%s%s=>", command, DEVICE_LABEL);

  switch (indice) {
      case 0:
      dtostrf(valor_sensor, 4, 2, str_sensor_temp);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL, str_sensor_temp);
        break;
      case 1:
      dtostrf(valor_sensor, 4, 2, str_sensor_humedad);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL1, str_sensor_humedad);
        break;
      case 2:
      dtostrf(valor_sensor, 4, 2, str_sensor_humo);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL2, str_sensor_humo);
        break;
      case 3:
      dtostrf(valor_sensor, 4, 2, str_sensor_amoniaco);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL3, str_sensor_amoniaco);
        break;
      case 4:
      dtostrf(valor_sensor, 4, 2, str_sensor_luz);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL4, str_sensor_luz);
        break;
      case 5:
      dtostrf(valor_sensor, 4, 2, str_relay_abanico);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL5, str_relay_abanico);
        break;
      case 6:
      dtostrf(valor_sensor, 4, 2, str_relay_bombilla);
      sprintf(command, "%s%s:%s", command, VARIABLE_LABEL6, str_relay_bombilla);
        break;
      default:
        break;
  }

  sprintf(command, "%s|end", command);

  Serial.println(command);
  
  free(command);
}


void sensores_humedad(){
  dht11_humedad = dht11.readHumidity();
  dht22_humedad = dht22.readHumidity();
  humedad_promedio = (dht11_humedad + dht22_humedad)/2;
  if(dht11_humedad < 60){
    relays(2,0);
    relays(1,1);
  }
  if(dht11_humedad > 72){
    relays(1,0);
    relays(2,1);
  }
  transmicion_ubidots(1,humedad_promedio);
  mostrarenpantalla(humedad_promedio, "Humedad relativa");
  //Serial.println(dht11_humedad);
}

void sensores_temperatura(){
  dht11_temperatura = dht11.readTemperature();
  dht22_temperatura = dht22.readTemperature();
  //temperatura_digital = sensor_digital_temperatura();
  temperatura_promedio = (dht11_temperatura + dht22_temperatura)/2;
 //temperatura_promedio = 35;
  if (temperatura_promedio < 24 ){
    relays(2,1);
    relays(1,0);
  }
  if (temperatura_promedio > 30){
    relays(1,1);
    relays(2,0);
  }
  //Serial.println(temperatura_digital);
  transmicion_ubidots(0,temperatura_promedio);
  mostrarenpantalla(temperatura_promedio, "Temperatura prom");
}

void sensor_luz(){
  valor_luz = analogRead(pin_ldr);
  //valor_luz = 300;
  //Si detecta oscuridad
  if(valor_luz < 200){
    transmicion_ubidots(4,0);
    //pausa_ticker();
    relays(2,1);
    mostrarenpantalla("Noche", "Iluminacion");
    //reanudar_ticker();
  }
  //Si es de día
  else{
    transmicion_ubidots(4,1);
    mostrarenpantalla("Día", "Iluminacion");
    //pausa_ticker();
    relays(2,0);
    //reanudar_ticker();
  }

}

void sensor_mq135(){
  valor_mq135 = analogRead(pin_mq135);
  //valor_mq135 = 300;
  if(valor_mq135 > 200){
    //encender relay del abanico
    relays(1,1);
    mostrarenpantalla("Nivel Alto","Amoniaco:");
  }
  else{
    relays(1,0);
    mostrarenpantalla("Nivel Bajo","Amoniaco:");
  }
  transmicion_ubidots(3,valor_mq135);
  
}

void sensor_mq2(){
  valor_mq2 = analogRead(pin_mq2);
  //valor_mq2 = 150;
  if(valor_mq135 > 200){
    //encender relay del abanico
    mostrarenpantalla("Nivel Alto","Humo:");
     relays(1,1);
  }
  else{
    mostrarenpantalla("Nivel Bajo","Humo:");
    relays(1,0);
  }
  transmicion_ubidots(2,valor_mq2);
  //mostrarenpantalla( "Humo",valor_mq2);
}


//Actuadores
void relays(int pin, int estado){
  switch (pin) {
      case 1://abanico
        switch (estado) {
            case 0: //estado encender
              digitalWrite(pin_relay_abanico,LOW);
              mostrarenpantalla("Apagado","Abanico:");
              break;
            case 1://estado apagar
              digitalWrite(pin_relay_abanico,HIGH);
              mostrarenpantalla("Encendido","Abanico:");
              break;
            default:
            break;
        }
        //transmicion_ubidots(5,estado);
        break;
      case 2://bombilla
        switch (estado) {
            case 0: //estado encender
              digitalWrite(pin_relay_bombillo,LOW);
              mostrarenpantalla("Encendido","Bombillo:");
              break;
            case 1://estado apagar
              digitalWrite(pin_relay_bombillo,HIGH);
              mostrarenpantalla("Apagado","Bombillo:");
              break;
            default:
            break;
        }
        //transmicion_ubidots(6,estado);
        break;
      default:
      break;
  }
}
/*
void pausa_ticker(){
  Tic_Temperatura.pause();
  Tic_Luz.pause();
  Tic_Humedad.pause();
  Tic_Humo.pause();
  Tic_Amoniaco.pause();
}

void reanudar_ticker(){
  Tic_Temperatura.resume();
  Tic_Luz.resume();
  Tic_Humedad.resume();
  Tic_Humo.resume();
  Tic_Amoniaco.resume();
}*/