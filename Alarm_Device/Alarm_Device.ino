#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>


ESP8266WebServer webServer(80);


// NTP SETUP
WiFiUDP u;
NTPClient n(u, "2.asia.pool.ntp.org", 7 * 3600); // khai bao truy cap NTP gio asia


/****/
int mins, hours;
int set_mins_on = 10, set_mins_off = 15;
short int state;

//==========AP info=======================//
char* ssid_ap = "IOT_ESP8266_2";
char* pass_ap = "10100101";
IPAddress ip_ap(192, 168, 1, 1);
IPAddress gateway_ap(192, 168, 1, 1);
IPAddress subnet_ap(255, 255, 255, 0);
String ssid;
String pass;
boolean statusD4;
//=========Biến chứa mã HTLM Website==//
const char MainPage[] PROGMEM = R"=====(
  <!DOCTYPE html> 
  <html>
   <head> 
       <title>HOME PAGE</title> 
       <style> 
          body {text-align:center;}
          h1 {color:#003399;}
          a {text-decoration: none;color:#FFFFFF;}
          .bt_on {height:50px; width:100px; margin:10px 0;background-color:#FF6600;border-radius:5px;}
          .bt_off {height:50px; width:100px; margin:10px 0;background-color:#00FF00;border-radius:5px;}
          .bt_write {height:30px; width:70px; margin:10px 0;background-color:#00FF00;border-radius:5px;}
          .bt_restart {height:30px; width:70px; margin:10px 0;background-color:#FF6600;border-radius:5px;}
          .bt_clear {height:30px; width:70px; margin:10px 0;background-color:#00FF00;border-radius:5px;}
       </style>
       <meta name="viewport" content="width=device-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <h1>ESP8266 Web Server</h1> 
      <div>Trạng thái trên chân D4: <b><pan id="trangthaiD4"><pan></b></div>
       <div>
        <button class="bt_on" onclick="getdata('onD4')">ON</button>
        <button class="bt_off" onclick="getdata('offD4')">OFF</button>
      </div>
      <div>Thiết lập thông tin Wifi kết nối</div>
      <div>Nhập tên wifi: </div>
      <div><input id="ssid"/></div>
      
      <div>Password wifi: </div>
      <div><input id="pass"/></div>
      <div>
        <button class="bt_write" onclick="writeEEPROM()">WRITE</button>
        <button class="bt_restart" onclick="restartESP()">RESTART</button>
        <button class="bt_clear" onclick="clearEEPROM()">CLEAR</button>
      </div>
      <div id>IP connected: <pan id="ipconnected"></pan></div>
      <div id="reponsetext"></div>
      <script>
        //-----------Tạo đối tượng request----------------
        function create_obj(){
          td = navigator.appName;
          if(td == "Microsoft Internet Explorer"){
            obj = new ActiveXObject("Microsoft.XMLHTTP");
          }else{
            obj = new XMLHttpRequest();
          }
          return obj;
        }
        var xhttp = create_obj();
        //------------------------------------------------
        window.onload = function(){
          xhttp.open("GET","/getIP",true);
          xhttp.onreadystatechange = process_ip;//nhận reponse 
          xhttp.send();
        }
        //-----------Kiểm tra response IP------------------
        function process_ip(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            document.getElementById("ipconnected").innerHTML=ketqua;       
          }
        }
        //==============================================================
        //-----------Thiết lập dữ liệu và gửi request ON/OFF D4---
        function getdata(url){
          xhttp.open("GET","/"+url,true);
          xhttp.onreadystatechange = process_onofd4;//nhận reponse 
          xhttp.send();
        }
        //-----------Kiểm tra response ON/OFF D4------------------
        function process_onofd4(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            document.getElementById("trangthaiD4").innerHTML=ketqua;       
          }
        }
         //-----------Thiết lập dữ liệu và gửi request Status D4----
        //---Ham update duu lieu tu dong---
        var xhttp2 = create_obj();
        setInterval(function() {
          getstatusD4();
        }, 1000);
        function getstatusD4(){
          xhttp2.open("GET","/getstatusD4",true);
          xhttp2.onreadystatechange = process_statusd4;//nhận reponse 
          xhttp2.send();
        }
        //-----------Kiểm tra response ON/OFF D4------------------
        function process_statusd4(){
          if(xhttp2.readyState == 4 && xhttp2.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp2.responseText; 
            document.getElementById("trangthaiD4").innerHTML=ketqua;       
          }
        }
        //===============================================================
        //-----------Thiết lập dữ liệu và gửi request ssid và password---
        function writeEEPROM(){
          var ssid = document.getElementById("ssid").value;
          var pass = document.getElementById("pass").value;
          xhttp.open("GET","/writeEEPROM?ssid="+ssid+"&pass="+pass,true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();
        }
        function clearEEPROM(){
          xhttp.open("GET","/clearEEPROM",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();
        }
        function restartESP(){
          xhttp.open("GET","/restartESP",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();
        }
        //-----------Kiểm tra response ------------------
        function process(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            document.getElementById("reponsetext").innerHTML=ketqua;       
          }
        }
      </script>
   </body> 
  </html>
)=====";
//=========================================//
void setup() {
  //Serial.begin(115200);
  pinMode(D4,OUTPUT);
  digitalWrite(D4,LOW);
  mins = hours = 0;
  state = 1;
  
  EEPROM.begin(512);       //Khởi tạo bộ nhớ EEPROM
  delay(10);
  
  if(read_EEPROM()){
    checkConnection();
  }else{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
    WiFi.softAP(ssid_ap,pass_ap);
    /*Serial.println("Soft Access Point mode!");
    Serial.print("Please connect to ");
    Serial.println(ssid_ap);
    Serial.print("Web Server IP Address: ");
    Serial.println(ip_ap);*/
  }
  /******************************/
  // tuy cap NTP
  n.begin();
  //*************************************//
  webServer.on("/",mainpage);
  webServer.on("/onD4",on_D4);
  webServer.on("/offD4",off_D4);
  webServer.on("/getstatusD4",get_statusD4);
  webServer.on("/getIP",get_IP);
  webServer.on("/writeEEPROM",write_EEPROM);
  webServer.on("/restartESP",restart_ESP);
  webServer.on("/clearEEPROM",clear_EEPROM);
  webServer.begin();
}
void loop() 
{
  webServer.handleClient();
  // lay thoi gian
  n.update();
   mins = n.getMinutes();
   hours = n.getHours(); 
  /*Serial.print( "bay gio la: " );
  Serial.print(hours);
  Serial.print(":");
  Serial.print(mins);
  Serial.print("\n");*/
  
  State_led();
  delay(100);
  
}
//==========Chương trình con=================//
void mainpage(){
  String s = FPSTR(MainPage);
  webServer.send(200,"text/html",s);
}

void State_led()
{
  if( (mins == 5) || (mins == 17)|| (mins == 25)|| (mins == 35)|| (mins == 45)) {state =0;} // high 10
  if((mins == 7 )|| (mins == 20)|| (mins == 27)|| (mins == 37) || (mins == 50)){state = 1;} /// low 15
  if(state == 0)  digitalWrite(D4,  HIGH);
  if(state ==1)   digitalWrite(D4, LOW);
  ///*/////
  statusD4 = digitalRead(D4);
  EEPROM.write(96,statusD4);
  EEPROM.commit(); 
}

void on_D4()
{
  state = 1;
  webServer.send(200,"text/html","ON");
}
void off_D4()       /********************** off function **********************/
{
  state = 0;
  webServer.send(200,"text/html","OFF");
}
void get_statusD4(){
  String s;
  if(digitalRead(D4)==1){
    s = "OFF";
  }else{
    s = "ON";
  }
  webServer.send(200,"text/html", s);
}
void get_IP(){
  String s = WiFi.localIP().toString();
  webServer.send(200,"text/html", s);
}
boolean read_EEPROM(){
  Serial.println("Reading EEPROM...");
  statusD4 = char(EEPROM.read(96));
  Serial.print("Trạng thái chân D4: ");
  if(statusD4 == 0){
    digitalWrite(D4,LOW);
    Serial.println("ON");
  }else{
    digitalWrite(D4,HIGH);
    Serial.println("OFF");
  }
  if(EEPROM.read(0)!=0)
  {
    ssid = "";
    pass = "";
    for (int i=0; i<32; ++i){
      ssid += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(ssid);
    for (int i=32; i<96; ++i){
      pass += char(EEPROM.read(i));
    }
    Serial.print("PASSWORD: ");
    Serial.println(pass);
    ssid = ssid.c_str();
    pass = pass.c_str();
    return true;
  }
  else
  {
    Serial.println("Data wifi not found!");
    return false;
  }
}
//---------------SETUP WIFI------------------------------
void checkConnection() {
  Serial.println();
  Serial.print("Check connecting to ");
  Serial.println(ssid);
  WiFi.disconnect();
  WiFi.begin(ssid,pass);
  int count=0;
  while(count < 50){
    if(WiFi.status() == WL_CONNECTED){
      WiFi.mode(WIFI_STA);
      Serial.println();
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("Web Server IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(200);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
  WiFi.softAP(ssid_ap,pass_ap);
  Serial.println("Soft Access Point mode!");
  Serial.print("Please connect to ");
  Serial.println(ssid_ap);
  Serial.print("Web Server IP Address: ");
  Serial.println(ip_ap);
}
void write_EEPROM(){
  ssid = webServer.arg("ssid");
  pass = webServer.arg("pass");
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);           
    delay(10);
  }
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  for (int i = 0; i < pass.length(); ++i) {
    EEPROM.write(32 + i, pass[i]);
  }
  EEPROM.commit();
  Serial.println("Writed to EEPROM!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PASS: ");
  Serial.println(pass);
  String s = "Đã lưu thông tin wifi";
  webServer.send(200, "text/html", s);
}
void restart_ESP(){
  ESP.restart();
  String s = "Đã khởi động lại ESP8266";
  webServer.send(200, "text/html", s);
}
void clear_EEPROM(){
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);           
    delay(10);
  }
  EEPROM.commit();
  String s = "Đã xóa bộ nhớ EEPROM";
  webServer.send(200,"text/html", s);
}
