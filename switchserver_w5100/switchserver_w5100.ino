/**
 * Schaltserver für Arduino 
 * 
 * Benötigte Hardware:
 * - Arduino Uno, Mega, Due
 * - 433MHz Sender
 * - Arduino Ethernet Shield
 *
 * Anschluss:
 * - 433MHz Sender
 *   DATA -> Arduino Pin 3 
 *   VCC -> 3 - 12V
 *   GND -> 0V/GND
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
#define STATE_LED -1            //Pin für die Status LED (-1 -> deaktiviert)
#define RCSWITCH_SEND_PIN 3     //Pin an dem der DATA Eingang des Senders Angeschlossen ist
#define SWITCHSERVER_PORT 9274  //Port des Schaltservers
 
//Imports
#include <RCSwitch.h>    // https://github.com/agent4788/rc-switch exportiert von https://code.google.com/p/rc-switch/
#include <SPI.h>
#include <Ethernet.h>

//Funktionsdeklarationen
String readRequest(EthernetClient* client);

//Initalisieren
RCSwitch rcSwitch = RCSwitch();
EthernetServer server = EthernetServer(SWITCHSERVER_PORT);
EthernetClient client;

//Status LED
bool StateLedValue = false;
unsigned long previousMillis = 0;
unsigned long interval = 1000;

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

        //Sende LED initalisieren
        if(SEND_LED != -1) {
                                 
                 pinMode(SEND_LED, OUTPUT); 
        }
        
        //Status LED initalisieren
        if(STATE_LED != -1) {
                                 
                 pinMode(STATE_LED, OUTPUT); 
        }
}

void loop() {
	
	size_t size;
	if (EthernetClient client = server.available()) {
		
                //Sende LED schalten
                if(SEND_LED != -1) {
                                 
                        digitalWrite(SEND_LED, HIGH); 
                }
  
		//Anfrage empfangen
		String request = readRequest(&client);
                
                //Debug Ausgabe
                if(SERIAL_DEBUG) {
                                  
		        Serial.println("request " + request);
	        }
                
                // Variablen Aufteilen
                // Data is now available in pieces array
                // pieces[0] is first item
                // pieces[1] is second item, and so on
                // You can call toInt() on the data to convert it to an int
                // ex. int value = pieces[0].toInt();
	        const int numberOfPieces = 6;
	        String pieces[numberOfPieces];
	        int counter = 0;
	        int lastIndex = 0;
	 
	        for (int i = 0; i < request.length(); i++) {
		
		        if (request.substring(i, i + 1) == ":") {
			        pieces[counter] = request.substring(lastIndex, i);
			        lastIndex = i + 1;
			        counter++;
		        }
	        }
                pieces[counter] = request.substring(lastIndex, request.length() + 1);

                //Anfrage bearbeiten
                int type = pieces[0].toInt();
                if(type == 1) {
                  
                        //433MHz Befehl senden Typ 1
                        //(Typ[0]:Hauscode[1]:Geraetecode[2]:Befehl[3]) + Leerzeichen am Ende
                        int command = pieces[3].toInt();
                        char homeCode[6];
                        pieces[1].toCharArray(homeCode, 6);
                        int deviceCode = pieces[2].toInt();
                        int continues = pieces[4].toInt();
                        if(command == 1) {
                          
                                //Einschaltbefehl senden
                                for(int i = 0; i < continues; i++) {
                                  
                                  rcSwitch.switchOn(homeCode, deviceCode);
                                }                                
                        } else {
                          
                                //Asuschaltbefehl senden
                                for(int i = 0; i < continues; i++) {
                                  
                                  rcSwitch.switchOff(homeCode, deviceCode);
                                }
                        }
                        
                        //Debug Ausgabe
                        if(SERIAL_DEBUG) {
                                  
		                Serial.print("send ");
                                Serial.print(homeCode);
                                Serial.print(" ");
                                Serial.print(deviceCode);
                                Serial.print(" ");
                                Serial.print(command);
                                Serial.print(" -> ");
                                Serial.print(continues);
                                Serial.println("x gesendet");
	                }
                } else if(type == 2) {
                  
                        //GPIO Ausgang schalten Typ 2
                        //(Typ[0]:Pin[1]:Befehl[2]) + Leerzeichen am Ende
                        int pin = pieces[1].toInt();
                        int command = pieces[2].toInt();
                        
                        pinMode(pin, OUTPUT);
                        if(command == 1) {
                              
                                digitalWrite(pin, HIGH);
                        } else {
                                
                                digitalWrite(pin, LOW);
                        }
                        
                        //Debug Ausgabe
                        if(SERIAL_DEBUG) {
                                  
		                Serial.print("write ");
                                Serial.print(pin);
                                Serial.print(" ");
                                Serial.println(command);
	                }
                } else if(type == 3) {
                  
                        //GPIO Eingang schalten Typ 3
                        //(Typ[0]:Pin[1]) + Leerzeichen am Ende
                        int pin = pieces[1].toInt();
                        
                        pinMode(pin, INPUT);
                        int state = digitalRead(pin);
                        client.println(state);
                        client.flush();
                        
                        //Debug Ausgabe
                        if(SERIAL_DEBUG) {
                                  
		                Serial.print("read ");
                                Serial.print(pin);
                                Serial.print(" state ");
                                Serial.println(state);
	                }
                }
		
                //verbindung beenden
		client.stop();

                //Sende LED schalten
                if(SEND_LED != -1) {
                                 
                        digitalWrite(SEND_LED, LOW); 
                }
	}

        //Status LED schalten
        if(STATE_LED != -1 && (millis() - previousMillis > interval)) {
                
                previousMillis = millis();
                stateLedValue = !stateLedValue;
                digitalWrite(STATE_LED, stateLedValue);
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
