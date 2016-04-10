
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include "hackcommand.h"

#define TOKEN "ESP8266_1"

WebSocketsClient webSocket;


long timeSinceLastConnection;

struct hackcommand inCmd = {"","","",TOKEN};
struct hackcommand outCmd = {"","","",TOKEN};


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



void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
  Serial.println("Event detected");
  String payloadString;//= String('"');
  String message;
  for(int i = 0; i < lenght; i++){
    payloadString += (char)payload[i];
  }
  //payloadString += '"';

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
      //char buf[payloadString.length()];
      //payloadString.toCharArray(buf,payloadString.length());
      if(parseJson(payloadString)){
      //if(parseJson("{\"cmd\":\"digitalWriteOn\",\"msg\":\"14\",\"client\":\"\"}")){
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
      hexdump(payload, lenght);
      
      // send data to server
      // webSocket.sendBIN(payload, lenght);
      break;
    default:
      Serial.println("Event: No known event");
  }

}


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


