#include <ESP8266WiFi.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <time.h>
#define ONE_WIRE_BUS D2
#define TEMPERATURE_PRECISION 9
const char* ssid = "MaracaNet";//"dlink-8A5C";
const char* password = "carlapersonal";//"vzsng15127";
const float PWM_DT=10;//Fatia do PWM em ms
const float TRange=70;//Amplitude de valores p temperatura
const float kp=1;
const float ki=1;
const float kd=1;
const short unsigned int PWMslices=10;//Resolução de um periodo de PWM
double dt0pwm=0, dt1=0;//Referência de tempo
byte led1 = D7;
//byte led2 = D4;

;

String setpoint1, setpoint2;

char temperatureString1[6];
char temperatureString2[6];

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer, outsideThermometer;
WiFiServer server(80);

void  softPWM(const int duty,byte PIN1)
  {
    const short unsigned int PWMslices=10;
    static int count;
    
    ++count;
    count=count%PWMslices;
  
    if((count<duty)) {
      
      
      digitalWrite(PIN1,HIGH);//high
      
    }
    else
    {
      digitalWrite(PIN1,LOW);
      
    }
  }

void setup() {
  Serial.begin(9600);
  delay(10);
 pinMode(D7,OUTPUT);
 sensors.begin();
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
 
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");

  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(outsideThermometer), DEC);
  Serial.println();
}


/*float getTemperature() {
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}
 */

void loop() {
  float teste= 10;
  float e0,e1,e2,eTemp;
  float iae,m;
  int m1;
  WiFiClient client = server.available();
  dt1= millis();
  
 float temp1 = sensors.getTempC(insideThermometer);
     float temp2 = sensors.getTempC(outsideThermometer);
     String tempstring1 = String(temp1);
      //Serial.print("Requesting temperatures...");
    // Serial.print("temperatura:");
    // Serial.println(tempstring1);
      sensors.requestTemperatures();
 //CONTROLE 
 
 //calculo do erro
      eTemp=(temp1-(setpoint1.toInt()))/TRange; //erro normalizado
      if(eTemp==eTemp) 
      {
        e0=e1;
        e1=e2;
        e2=eTemp;
        iae+=e2*e2;
      }
      else
      {
        //È um NaN
        
      }
      //if(e2<0) e2=-e2;
//FOrça DE CONTROLE
      m = m + kp*(e2-e1)+ ki*e2+(kd*(e2-2*e1 + e0));//equação "velocidade" do PID
      if(m>1) 
        m=1;
      else
      if(m<-1)
        m=-1;
      
      m1 = PWMslices*m;//Adequação da força à resolução do PWM
      
 
 
 //chamar o PWM
  if((dt1-dt0pwm)>=PWM_DT) 
  {
    
    //softPWM(0,24,25);  
    //softPWM(0,23,27);
   softPWM(m1,led1);  
  //  softPWM( pwm2.toInt(),led2);  
  
    dt0pwm=dt1;
  }
  
  
  if (!client) {
    return;
  }
  
  Serial.println("new client");
  while(!client.available()){
   delay(1);}
  
  
     
      /*Serial.println("DONE");
    
      Serial.print("Temp 1: ");
      Serial.print(temp1);*/
  //dtostrf(temp1, 2, 2, temperatureString1);
  //dtostrf(temp2, 2, 2, temperatureString2);
  //float temp = getTemperature();
  //dtostrf(temp, 2, 2, temperatureString);
    
 String req = client.readStringUntil('\r');// until r?
 Serial.println(req);
 client.flush();
 String buf = "";
 if(req.indexOf("temperatura")!=-1){
  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+tempstring1+"\r\n";
  
 }else{
    buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  
    }
  
 
    
 
  client.print(buf);
   if(req.indexOf("setpoint1=")!=-1){
       setpoint1= req.charAt(req.indexOf("setpoint1=")+10);
       setpoint1+= req.charAt(req.indexOf("setpoint1=")+11);
       //setpoint1 += req.charAt(req.indexOf("setpoint1=")+12);
       //setpoint1 += req.charAt(req.indexOf("setpoint1=")+13);
       //pwm1=pwm1.toInt();
       Serial.print("setpoint1:");
       Serial.println(setpoint1);

       setpoint2 = req.charAt(req.indexOf("setpoint2=")+5);
      // pwm2 += req.charAt(req.indexOf("pwm2=")+6);
      // pwm2=pwm2.toInt();
       Serial.print("setpoint2:");
       Serial.println(setpoint2);
 }else{
        Serial.print("setpoint1:");
       Serial.println(setpoint1.toInt());
        Serial.print("setpoint2:");
       Serial.println(setpoint2.toInt());
    } 
  
 
   
client.flush();
  client.stop();
 
  Serial.println("Client disonnected");

}
