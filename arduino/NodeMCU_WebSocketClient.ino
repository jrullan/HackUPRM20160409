/*
 * This sketch is part of MakerspacePR team participation
 * in the HackUPRM hackathon @ 2016-04-09 12hrs hackathon
 * 
 * The project consisted of developing an IoT (Internet of Things) service
 * similar to Blynk.cc (sans the mobile app) via a Websocket protocol server.
 * The main components of the project are:
 * 
 * 1. A Websocket server - We implemented this in PHP on a remote server
 * 2. An Arduino sketch running on an ESP8266 board (NodeMCU v1.0)
 * 3. A web application running on a local web server
 * 4. A communications protocol
 * 
 * Both the Arduino sketch and the web application are subscribed to the
 * Websocket server in order to achieve full-duplex (quasi real-time)
 * communications. A communications protocol was implemented to provide
 * web-to-device (W2D) and device-to-web (D2W) communications with acknowledges.
 * 
 * The includes in this sketch are required for an ESP8266 board.
 * In particular, we are using the WebSockets library from 
 * https://github.com/Links2004/arduinoWebSockets to implement the websocket
 * client in the ESP8266. 
 * (Note: We had to modify this library's Hash.h file with an <Arduino.h> include
 * because the Arduino compiler complained about invalid declarations.)
 * 
 * The original implementation of the communications protocol was done in json,
 * so we used a JSON parser library from blanchon:
 * https://github.com/bblanchon/ArduinoJson. We only used it to parse the json
 * sent from the Web application. The json sent to the websocket server
 * is created in a custom function createJson. One optimization is to use the
 * library's createObject();
 * 
 * Finally we defined a data structure for the messages in the 
 * file hackcommand.h. It was meant to be a class, but at the end 
 * to simplify we simply used a struct.
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include "hackcommand.h"

#define TOKEN "ESP8266_1" //Unique identifier for this device messages

WebSocketsClient webSocket;

long timeSinceLastConnection;

struct hackcommand inCmd = {"","","",TOKEN};
struct hackcommand outCmd = {"","","",TOKEN}; //not used

void setup() {
  Serial.begin(115200);
      for(uint8_t t = 4; t > 0; t--) {
          delay(1000);
      }
    WiFi.begin("HACKUPRM","hackathonuprm");
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("Connecting socket");
    webSocket.begin("74.213.70.221", 443);
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}


//=================================================
// Functions
//=================================================

// This is the function called when the webSocket receives a message
// from the server
void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
  Serial.println("Event detected");
  String payloadString;
  for(int i = 0; i < lenght; i++){
    payloadString += (char)payload[i];
  }

  String message;
 
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.print("Event: Disconnected! ");
      Serial.print((millis()-timeSinceLastConnection)/1000);
      Serial.println("s");
      timeSinceLastConnection = millis();
      break;
    case WStype_CONNECTED:
      Serial.println("Event: Connected!");
      // send message to server when Connected
      message = createJson("ID","","",TOKEN);
      webSocket.sendTXT(message);
      break;
    case WStype_TEXT:
      Serial.print("Event: TEXT! ");
      Serial.print(payloadString);
      Serial.println();
      if(parseJson(payloadString)){
        Serial.print("Cmd: ");
        Serial.println(inCmd.cmd);
        Serial.print("Msg: ");
        Serial.println(inCmd.msg);
        Serial.print("Client: ");
        Serial.println(inCmd.cli);
        Serial.print("Token: ");
        Serial.println(inCmd.tok);
        parseCommand(&inCmd);
      }
      break;
    case WStype_BIN:
      Serial.print("Event: BIN! ");
      Serial.print(payloadString);
      Serial.println();
      break;
    default:
      Serial.println("Event: No known event");
  }

}

// This is a pin mapping function to correctly
// address the pins in the NodeMCU board. The pin
// numbers in the board labels do not match the
// GPIO pin numbers which are the ones used by 
// Arduino.
int pinMap(int pin){
  switch(pin){
    case 0:
      return 16;
    case 1:
      return 5;
    case 2:
      return 4;
    case 3:
      return 0;
    case 4:
      return 2;
    case 5:
      return 14;
    case 6:
      return 12;
    case 7:
      return 13;
    case 8:
      return 15;
    case 9:
      return 3;
    case 10:
      return 1;
    default:
      return 16;
  }
}

// This function parses the command received in the websocket
// message to execute the requested action. Every action generates
// an ACK response even if there is an error.
void parseCommand(struct hackcommand *cmd){
  
  if(cmd->cmd == "digitalWriteOn"){
    int pin = cmd->msg.toInt();
    if(pin > 10 && pin < 0){
      String message = createJson("ACK","Wrong Pin","",TOKEN);
      webSocket.sendTXT(message);
      return;
    }
    pin = pinMap(pin);
    Serial.print("Turn ON pin ");
    Serial.println(pin);
    pinMode(pin,OUTPUT);
    digitalWrite(pin,HIGH);
    String message = createJson("ACK",cmd->cmd,"",TOKEN);
    webSocket.sendTXT(message);
  }
  
  if(cmd->cmd == "digitalWriteOff"){
    int pin = cmd->msg.toInt();
    if(pin > 10 && pin < 0){
      String message = createJson("ACK","Wrong Pin","",TOKEN);
      webSocket.sendTXT(message);
      return;
    }
    pin = pinMap(pin);    
    Serial.print("Turn OFF pin ");
    Serial.println(pin);
    pinMode(pin,OUTPUT);
    digitalWrite(pin,LOW);
    String message = createJson("ACK",cmd->cmd,"",TOKEN);
    webSocket.sendTXT(message);
  }
}


// This function uses the library to parse the json message
// and decompose it into our command data structure.
bool parseJson(String payloadString){
  StaticJsonBuffer<400> jsonBuffer;  
  JsonObject& root = jsonBuffer.parseObject(payloadString);
  if(!root.success()){
    Serial.print("Could not parse JSON object: ");
    Serial.println(payloadString);
    return false;
  }else{
    inCmd.cmd = root["cmd"].as<String>();
    inCmd.msg = root["msg"].as<String>();
    inCmd.cli = root["cli"].as<String>();
    inCmd.tok = root["tok"].as<String>();
  }
  return true;
}

// This function creates the json response or message to be sent
// to the websocket server.
String createJson(String cmd, String msg, String cli, String tok){
  String json = "";
  json += "{";

  json += "\"cmd\":";
  json += "\"";
  json += cmd;
  json += "\"";
  json += ",";
  
  json += "\"msg\":";
  json += "\"";
  json += msg;
  json += "\"";
  json += ",";  

  json += "\"client\":";
  json += "\"";
  json += cli;
  json += "\"";
  json += ",";

  json += "\"token\":";
  json += "\"";
  json += tok;
  json += "\"";
  //json += ",";

  json += "}";  
  return json;
}


