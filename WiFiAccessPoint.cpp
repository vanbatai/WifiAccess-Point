#include "WiFiAccessPoint.h"

DNSServer dnsServer;// Create the DNS object
IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);
ESP8266WebServer webServer(80);


char* ssid     = "your_ssid";
char* password = "your_password";


String HTML_WIFI_LIST = "";
String HTML_HEADER = "<html>\
  <head>\
      <title>Phong Lan System Care</title>\
      <script>\
      function c(l){document.getElementById('wifiName').value=l.innerText||l.textContent;}</script>\
  </head>";

String HTML_BODY_START = "<body bgColor = 'white'>";

String HTML_FORM_ENTER = "<form action='/enter'>\
          Security code: <input type = 'text' name='securityCode'><br/>\
          <input type='submit' value='Enter'><br/>\
      </form><br/>";

String HTML_FORM_MENU =
  "       <form action='/wifiScan' method='get'><button>Wifi Configure</button></form>\
          <form action='/noScan' method='get'><button>Wifi Configure (No scan)</button></form>\
          <form action='/reset' method='get'><button>Reset </button></form><br/>";

String HTML_FORM_PASSWORD =
  "       <form action='/login'>\
              Wifi Name: <input type='text' name='wifiName' id='wifiName'><br/>\
              Wifi Password: <input type = 'text' name='wifiPass'><br/>\
          <input type='submit' value='Login'><br/>";

String HTML_BODY_END = "</body></html>";

String responseHTML = HTML_HEADER + HTML_BODY_START + HTML_FORM_ENTER + HTML_BODY_END;


void handleRoot()
{
  webServer.send( 200, "text/html", responseHTML);
}
void handleEnter()
{
  Serial.println("handleEnter");
  if (webServer.arg("securityCode") == "rrrrrrrr")
  {
    Serial.println( webServer.arg("securityCode"));
    responseHTML = HTML_HEADER + HTML_BODY_START + HTML_FORM_MENU + HTML_BODY_END;
  }
  webServer.send( 200, "text/html", responseHTML);
}

void handleWifiScan()
{
  Serial.println("Setup done");
  Serial.println("scan start");

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  HTML_WIFI_LIST = "";
  for (int i = 0; i < n; ++i)
  {
    HTML_WIFI_LIST += "<a href='#p' onclick='c(this)'>" + WiFi.SSID(i) + "</a><br>";
  }
  responseHTML = HTML_HEADER + HTML_BODY_START + HTML_WIFI_LIST + HTML_FORM_PASSWORD + HTML_BODY_END;
  webServer.send( 200, "text/html", responseHTML);
}

void handleLogin()
{
  webServer.arg("wifiName").toCharArray(ssid, 50);
  webServer.arg("wifiPass").toCharArray(password, 50);
  Serial.println( webServer.arg("wifiName"));
  Serial.println( webServer.arg("wifiPass"));
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println(password);

  WiFi.disconnect();
  
  WiFi.begin(ssid, password);


  webServer.send( 200, "text/html", "CONNECTING!!! PLEASE WAIT.....");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //WiFiSave(ssid, password);
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  WiFi.softAPdisconnect();
  webServer.close();
}

void WiFiConfig(char const *apName, char const *apPassword, bool reset)
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPdisconnect();

  if (WiFi.SSID() != "" && reset == false)
  {
    Serial.println("Use the last Wifi connection!!!");
    Serial.println("Connect to WiFI: " + WiFi.SSID());
    WiFi.begin();

    for (int i = 0; i < 15; ++i)
    {
      if (WiFi.status() != WL_CONNECTED) 
      {
        delay(500);
        Serial.print(".");
      }
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    return;
  }

  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft - AP ... ");
  Serial.println(WiFi.softAP(apName, apPassword) ? "Ready" : "Failed!");

  Serial.print("Soft - AP IP address = ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(53, "*", local_IP);

  webServer.on ( "/enter", handleEnter);// event enter security code
  webServer.on ( "/wifiScan", handleWifiScan);// event when sc correct and show menu option
  webServer.on ( "/login", handleLogin);// event when sc correct and show menu option
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();

  Serial.println ( "softAP webServer started" );
  while(WiFi.status() != WL_CONNECTED)
  {
	  dnsServer.processNextRequest();
  	webServer.handleClient();
  }

  Serial.println ( "softAP webServer stoped" );
}