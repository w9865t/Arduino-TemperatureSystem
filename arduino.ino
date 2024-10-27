#include <Wire.h>
#include <ESP8266.h>
#include <SoftwareSerial.h>

// define LCD
#define LCD 0x3E

// define LEDs
#define LED_G 9
#define LED_B 10
#define LED_R 11

// define WIFI
#define SSID0 "ssid"
#define PASSWORD "password"
#define HOST_NAME "192.168.0.4"
#define HOST_PORT 9999

// define temp sensor
#define LM75B 0x48
#define temp_reg 0x00   // Tempereture register
#define conf_reg 0x01   //Configuration register

// define INF
#define INF 9999

// wifi
SoftwareSerial Wserial(2,4);
ESP8266 wifi(Wserial);

void setup(void){
  // begin serial
  Serial.begin(9600);

  // setup wifi
  Wserial.begin(115200);
  Wserial.println("AT+UART_CUR=9600,8,1,0,0");
  delay(10);
  Wserial.begin(9600);
  delay(10);

  while(1) {
    if(wifi.setOprToStation()) {
      Serial.println("Setup:OK");
      break;
    } else {
      Serial.println("Setup:Error");
      delay(3000);
    }
  }
  
  while(1) {
    if(wifi.joinAP(SSID0,PASSWORD)){
      Serial.println("joinAP:OK");
      break;
    } else {
      Serial.println("JoinAP:Failed");
      delay(3000);
    }
  }
  
  while(1) {
    if(wifi.disableMUX()) {
      Serial.println("disableMUX:OK");
      break;
    } else {
      Serial.println("disableMUX:Error");
      delay(3000);
    }
  }


  // set temp sensor
  Wire.begin();
  Wire.beginTransmission(LM75B);
  Wire.write(conf_reg);           // 温度設定
  Wire.write(0x00);               // thystの温度設定
  Wire.endTransmission();         //

  Wire.beginTransmission(LM75B);  //
  Wire.write(temp_reg);           // 温度読み出しモードに設定
  Wire.endTransmission();         //


  // set LCD
  Wire.begin();
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x38);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x39);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x14);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x70);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x56);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x6c);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x38);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x0c);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x01);
  Wire.endTransmission();
  delay(10);

  // setup LEDs
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);  
}


// ----------------------------- //
// --------- variables --------- //
// ----------------------------- //

float temp_online = INF;    // weather data obtained from server
float temp_room = INF;      // temperature data obtained from sensor
char send_msg[128] = {0};   // array
char recv_temp[128] = {0};  // array




// --------------------------------- //
// --------- main function --------- //
// --------------------------------- //

void loop(void) {
  
  // get weather from server
  comm_server();

  // get temp data from sensor
  get_temp();

  // print data to LCD
  ctrl_LCD();

  // turn on LEDs
  ctrl_LED();
}



// ----------------------------- //
// --------- functions --------- //
// ----------------------------- //

void comm_server() {
  // send current room temp to server
  if(wifi.createTCP(HOST_NAME,HOST_PORT)){
      Serial.println("OK:TCP Connect");
      String sendval = String(temp_room);
      sendval.toCharArray(send_msg, sendval.length()+1);
      wifi.send(send_msg, strlen(send_msg));
  } else {
      Serial.println("Faild: TCP Connect");
  }
  
  // receive online temp
  wifi.recv(recv_temp, sizeof(recv_temp), 10000);
  temp_online = atof(recv_temp);
  Serial.println(temp_online);
  delay(1000);
}

void get_temp() {
  // get temperature data from sensor
  int raw_data = 0;
  Wire.requestFrom(LM75B,2);           
  while(Wire.available()){
    raw_data = (Wire.read() << 8); //温度レジスタの上位8bit取得
    raw_data |= Wire.read();       //温度レジスタの下位8bit取得(有効3bit)
  } 
  temp_room = (raw_data >> 5) * 0.125;  //レジスタの値を温度に変換
  Serial.println(temp_room);
  delay(100);
}

void ctrl_LCD() {
  // print datas to LCD

  // upper side
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0x02);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(LCD);
  Wire.write(0x40);
  // ナカ
  Wire.write(0xc5);
  Wire.write(0xb6);
  Wire.write(0x3a);

  // inside temp
  char t_i[10] = {0};
  dtostrf(temp_room, 4, 2, t_i);
  Wire.write(t_i);
  Wire.write(0xdf);  
  Wire.write(0x43);  
  Wire.endTransmission();
  delay(10);  

  // down side
  Wire.beginTransmission(LCD);
  Wire.write(0x00);
  Wire.write(0xc0);
  Wire.endTransmission();
  delay(10);    
  Wire.beginTransmission(LCD);
  Wire.write(0x40);
  // ソト
  Wire.write(0xbf);    
  Wire.write(0xc4);  
  Wire.write(0x3a);  

  // outside temp
  char t_o[10] = {0};
  dtostrf(temp_online, 4, 2, t_o);
  Wire.write(t_o);
  Wire.write(0xdf);  
  Wire.write(0x43);    
  Wire.endTransmission();
  delay(10);
}

void ctrl_LED() {
  float temp_diff = fabs(temp_online - temp_room);

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  
  if (temp_diff >= 10) {
    // if temp diff is larger than 10, turn on red
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
    digitalWrite(LED_R, HIGH);
  } else if (temp_diff >= 5) {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_B, LOW);
    digitalWrite(LED_G, HIGH);
  } else {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
  }
}
