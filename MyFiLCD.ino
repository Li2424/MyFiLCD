#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"

Waveshare_LCD1602_RGB lcd(16, 2);

//Put your network data in file arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiServer server(80);

void setup() {
  lcd.init();
  display("MyFi LCD", "");
  delay(3600);

  int status = WL_IDLE_STATUS;
  if (WiFi.status() == WL_NO_MODULE) {
    display("MyFi LCD", "WiFi error");
    lcd.setRGB(255, 0, 0);
    while (true)
      ;
  }

  display("MyFi LCD", "WiFi Connecting");
  lcd.blink();
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(2000);
  }
  lcd.stopBlink();
  display("MyFi LCD", "WiFi connected!");

  lcd.setRGB(0, 255, 0);
  server.begin();
  delay(4000);

  //Get IP adress in network and show it on the screen
  String ip = IpToStr(WiFi.localIP());
  display("Ready, IP is:", ip);

  lcd.setRGB(255, 255, 255);
}

//Server logic

String lines[90];
void loop() {
  WiFiClient client = server.available();

  if (client) {
    int line = 0;
    String REQ_fl = "";
    String REQ_METHOD = "";
    String REQ_PATH = "";
    while (client.connected()) {
      if (client.available()) {
        String c = client.readStringUntil('\n');
        if (line == 0) {
          REQ_fl = c;
          REQ_METHOD = c;
          REQ_METHOD = REQ_METHOD.substring(0, REQ_METHOD.indexOf(" "));

          REQ_PATH = c;
          REQ_PATH = REQ_PATH.substring(REQ_METHOD.length(), REQ_PATH.length());
          REQ_PATH = REQ_PATH.substring(1, REQ_PATH.indexOf(" HTTP"));
          lines[line] = c;
        }

        if (c == "\r") {
          if (REQ_METHOD != "POST") {
            http404Response(client);
            break;
          }

          //Paths
          if (REQ_PATH == "/") {
            httpResponseHeaders(client, "text/html");
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><title>MyFi - Text Display</title><meta charset=\"utf-8\"></head><body><h1>MyFi Text display</h1><p>Created by <a href=\"https://github.com/Li2424\">Li2424</a>.</p></body>");
            client.println("</html>");
          } else if (REQ_PATH.startsWith("/api/set/txt/")) {
            httpResponseHeaders(client, "application/json");
            set_display(REQ_PATH, 0);
            client.println("{\"ok\": true}");
          } else if (REQ_PATH.startsWith("/api/set/txt2/")) {
            httpResponseHeaders(client, "application/json");
            set_display(REQ_PATH, 1);
            client.println("{\"ok\": true}");
          } else if (REQ_PATH.startsWith("/api/set/color/")) {
            httpResponseHeaders(client, "application/json");
            set_displayColor(REQ_PATH);
            client.println("{\"ok\": true}");
          } else {
            http404Response(client);
          }
          break;
        }
        line++;
      }
    }
    delay(1);
    client.stop();
  }
}

//Helper functions

void http404Response(WiFiClient client) {
  http404ResponseHeaders(client, "text/html");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<meta charset=\"utf-8\"><h1>404 NOT FOUND</h1><p>Cannot find page.</p>");
  client.println("</html>");
}

void httpResponseHeaders(WiFiClient client, String contentType) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + contentType);
  client.println("Connection: close");
  client.println();
}

void http404ResponseHeaders(WiFiClient client, String contentType) {
  client.println("HTTP/1.1 404 NOT FOUND");
  client.println("Content-Type: " + contentType);
  client.println("Connection: close");
  client.println();
}

void display(String line1, String line2) {
  //First clear the screen
  lcd.setCursor(0, 0);
  lcd.send_string("                ");
  lcd.setCursor(0, 1);
  lcd.send_string("                ");
  //Then show the text
  lcd.setCursor(0, 0);
  lcd.send_string(line1.c_str());
  lcd.setCursor(0, 1);
  lcd.send_string(line2.c_str());
}


void set_display(String pth, int line) {
  pth = pth.substring(13, pth.length());
  String x = pth;
  //Space converting
  x.replace("%20", " ");
  //Clear the display
  lcd.setCursor(0, line);
  lcd.send_string("                +");
  //Then show the text
  lcd.setCursor(0, line);
  lcd.send_string(x.c_str());
}
void set_displayColor(String pth) {
  pth = pth.substring(15, pth.length());
  //HEX String to RGB converting
  String x = pth;
  x = "#" + x;
  x.toUpperCase();
  byte r, b, g = 0;
  calculateRGB(x.c_str(), r, g, b);

  //Set the display color
  lcd.setRGB(r, g, b);
}

void calculateRGB(const char *hex, byte &r, byte &g, byte &b) {
  long l = strtol(hex + 1, NULL, 16);
  r = l >> 16;
  g = l >> 8;
  b = l;
}

String IpToStr(const IPAddress &ip) {
  return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
}
