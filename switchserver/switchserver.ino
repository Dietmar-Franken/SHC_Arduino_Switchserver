/**
 * Schaltserver für Arduino 
 * 
 * Benötigte Hardware:
 * - Arduino Uno, Mega, Pro Mini oder Nano (für Pro Mini wird zum flashen noch ein FTDI/USB Adapter benötigt)
 * - 433MHz Sender
 * - ENC28J60 Netzwerk Chip
 *
 * @author Oliver Kleditzsch
 * @copyright Copyright (c) 2015, Oliver Kleditzsch
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @since 1.0.0
 * @version 1.0.0
 */
 
//Konfiguration
#define SERIAL_DEBUG 1          //ausgeführte Befehle auf die Serielle Schnittstelle ausgeben
#define SEND_LED 2              //Pin für die Sende LED (-1 -> deaktiviert)
#define RCSWITCH_SEND_PIN 3     //Pin an dem der DATA Eingang des Senders Angeschlossen ist
#define SWITCHSERVER_PORT 9274  //Port des Schaltservers
 
//Imports
#include <RCSwitch.h>    // https://code.google.com/p/rc-switch/
#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip
#include <Base64.h>      // https://github.com/adamvr/arduino-base64
//#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

//Funktionsdeklarationen
String readRequest(EthernetClient* client);

//Initalisieren
RCSwitch rcSwitch = RCSwitch();
EthernetServer server = EthernetServer(SWITCHSERVER_PORT);
EthernetClient client;

void setup() {
	
	//Serielle Verbindung initalisieren
	if(SERIAL_DEBUG) {
		Serial.begin(9600);
	}
	
	//RCSwitch Initialisieren
	rcSwitch.enableTransmit(RCSWITCH_SEND_PIN);
	
	//Netzwerk Initialisieren
	uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};  //MAC Adresse
	IPAddress ip(192, 168, 115, 155);                  //IP Adresse
	Ethernet.begin(mac, ip);
	server.begin();	
}

void loop() {
	
	size_t size;
	if (EthernetClient client = server.available()) {
		
		//Anfrage empfangen
		String request = readRequest(&client);
		int len = request.length();
		char buffer[len];
		request.toCharArray(buffer, len);
		
		//Base64 decode
		char out[len];
		base64_decode(out, buffer, sizeof(buffer));
                Serial.println(out);
		
		//JSON decode
		//StaticJsonBuffer<200> jsonBuffer;
		//JsonObject& root = jsonBuffer.parseObject(out);
                //const char* type = root[0]["type"];
		//Serial.println(type);
		
                //Empfangene Daten verarbeiten
                //rcSwitch.switchOn("11111", 1);
	        //delay(5000);
	        //rcSwitch.switchOff("11111", 1);
	        //delay(5000);
		
		client.stop();
	}
}

String readRequest(EthernetClient* client) {
	
	String request = "";
	// Loop while the client is connected.
	while (client->connected()) {
		// Read available bytes.
		while (client->available()) {
			// Read a byte.
			char c = client->read();

			// Exit loop if end of line.
			if ('\n' == c) {
				return request;
			}
			// Add byte to request line.
			request += c;
		}
	}
	return request;
}
