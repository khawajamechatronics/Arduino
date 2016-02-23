#include "Binh.h"

//////////////////////////////////////
///             SYSTEM	           ///
//////////////////////////////////////

ESPHB::ESPHB(unsigned char _ledpin){
	LEDSTATUS=_ledpin;
	pinMode(LEDSTATUS, OUTPUT);
};
// Start up
void ESPHB::Startup(void){
    EEPROMRead(0,&DV_INF);
    EEPROMRead(50,&WF_INF);
    if(DV_INF.DEBUG==1){
        DEBUG = true;
    }else{
        DEBUG = false;
    }
    DEBUG = true;
    if(DV_INF.FIRST_START!=1)  
        Restore();  // Set defaults on first startup
    if(DV_INF.USE_DHT==1){
        dht.init(DV_INF.DHT_PIN, DV_INF.DHT_TYPE, 11,DV_INF.DEBUG);
    }
}
// Save defaults setup to EEPROM (Restore)
void ESPHB::Restore(void){
    EEPROM.begin(MAX_EEPROM_SIZE);
    for(uint16_t i=MAX_EEPROM_SIZE; i>0; i--){
        EEPROM.write(i-1,0);
    }
    EEPROM.end();
    DV_INF.DEBUG=1;
    DV_INF.FIRST_START=1;    // 1
    StringToArray(&DEFAULT_KEY, DV_INF.DV_ADMIN, MAX_ADMIN_LEN);
    WF_INF.IS_STATICIP = 0;
    WF_INF.WF_CONN_TIMEOUT = 60;
    WF_INF.WF_STATICIP = 0;
    WF_INF.MAX_REQUEST_TIMEOUT = 6;
    WF_INF.MASTER_SERVER_PORT = 80;
    
    StringToArray(&null, WF_INF.WF_SSID, MAX_SSID_LEN);
    StringToArray(&null, WF_INF.WF_PASSWORD, MAX_PASSWORD_LEN);
    StringToArray(&DEFAULT_KEY, WF_INF.WF_KEY, MAX_KEY_LEN);
    StringToArray(&DEFAULT_APPASSWORD, WF_INF.AP_PASSWORD, MAX_PASSWORD_LEN);
    StringToArray(&null, WF_INF.MASTER_SERVER, MAX_SERVER_LEN);
    EEPROMSave(0,&DV_INF);
    EEPROMSave(50,&WF_INF);
    ESP.restart();
}

//////////////////////////////////////////
///     Group function for Serial	   ///
//////////////////////////////////////////
void ESPHB::SerialEvent(void) {
	while (Serial.available()){
		char inChar = (char)Serial.read();
		SERIAL_RECEIVER += inChar;
		if (inChar == '\n') {SERIAL_COMPLETE = true;}   
		if(SERIAL_COMPLETE){
			String command=SERIAL_RECEIVER.substring(0,SERIAL_RECEIVER.indexOf("=="));
			String value=SERIAL_RECEIVER.substring(SERIAL_RECEIVER.indexOf("==")+2,SERIAL_RECEIVER.length()-1);
			if((command=="login")&&(!LOGINED)){
				if(value!=String(DV_INF.DV_ADMIN)){
					Serial.println(FPSTR(lb_LOGIN_FAILED));
				}else{
					Serial.println(FPSTR(lb_LOGIN_SUCCESS));
					LOGINED=true;					
				}
			}	
			else if((command=="status")&&LOGINED){
                Serial.print(FPSTR(lb_KEY));    Serial.println(WF_INF.WF_KEY);
				Serial.print(FPSTR(lb_SERIAL)); Serial.println(DV_INF.DV_SERIAL);
				Serial.print(FPSTR(lb_SSID));   Serial.println(WF_INF.WF_SSID);
                Serial.print(FPSTR(lb_PASSWORD));   Serial.println(WF_INF.WF_PASSWORD);
                Serial.print(FPSTR(lb_SERVER));   Serial.println(WF_INF.MASTER_SERVER);
                Serial.print(FPSTR(lb_SERVER));   Serial.println(WF_INF.MASTER_SERVER_PORT);
                Serial.print(FPSTR(lb_CONNECT_STATUS)); if(CONNECTED)   Serial.println(FPSTR(lb_CONNECTED)); else   Serial.println(FPSTR(lb_FAILED_CONNECT));
                Serial.print(FPSTR(lb_STA));    Serial.print(FPSTR(lb_MAC_ADDRESS));    Serial.println(WiFi.macAddress());
                Serial.print(FPSTR(lb_STA));    Serial.print(FPSTR(lb_IP));    Serial.println(WiFi.localIP());	
			}
			else if((command=="debug")&&LOGINED){
                DV_INF.DEBUG=true;
				EEPROMSave(0,&DV_INF);
                Serial.println(FPSTR(lb_DEBUG_ENABLE));
				ESP.restart();
			}
			else if((command=="normal")&&LOGINED){
                DV_INF.DEBUG=false;
				EEPROMSave(0,&DV_INF);
                Serial.println(FPSTR(lb_DEBUG_ENABLE));
				ESP.restart();
			}
			else if((command=="reboot")&&LOGINED){
				Serial.println(FPSTR(lb_REBOOT));
				ESP.restart();
			}
			else if((command=="restore")&&LOGINED){
				Serial.println(FPSTR(lb_RESTORE));
				Restore();
				ESP.restart();
			}
			else if((command=="serial")&&LOGINED){
				if(StringToArray(&value, DV_INF.DV_SERIAL, MAX_SERIAL_LEN)){
                    EEPROMSave(0,&DV_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_SERIAL));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_SERIAL));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="ssid")&&LOGINED){
				if(StringToArray(&value, WF_INF.WF_SSID, MAX_SSID_LEN)){
                    EEPROMSave(50,&WF_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_SSID));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_SSID));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="password")&&LOGINED){
				if(StringToArray(&value, WF_INF.WF_PASSWORD, MAX_PASSWORD_LEN)){
                    EEPROMSave(50,&WF_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_PASSWORD));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_PASSWORD));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="key")&&LOGINED){
				if(StringToArray(&value, WF_INF.WF_KEY, MAX_KEY_LEN)){
                    EEPROMSave(50,&WF_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_KEY));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_KEY));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="admin")&&LOGINED){
				if(StringToArray(&value, DV_INF.DV_ADMIN, MAX_ADMIN_LEN)){
                    EEPROMSave(0,&DV_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_ADMIN_PASSWORD));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_ADMIN_PASSWORD));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="server")&&LOGINED){
				if(StringToArray(&value, WF_INF.MASTER_SERVER, MAX_SERVER_LEN)){
                    EEPROMSave(50,&WF_INF);
					Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                    Serial.print(FPSTR(lb_SERVER));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);	
				}else{
                    Serial.print(FPSTR(lb_CHANGE_FAILED));
                    Serial.print(FPSTR(lb_SERVER));
                    Serial.print(FPSTR(lb_TO));
					Serial.println(value);
                };
			}
			else if((command=="port")&&LOGINED){
                WF_INF.MASTER_SERVER_PORT=value.toInt();
                EEPROMSave(50,&WF_INF);
				Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                Serial.print(FPSTR(lb_SERVER_PORT));
                Serial.print(FPSTR(lb_TO));
			    Serial.println(value);
			}else if((command=="setDHT")&&LOGINED){
                // String setup: setDHT==ON/OFF-type-pin
                // ex: setDHT==ON-22-14
                uint8_t _dhttype,_dhtpin,_sepa,_nextsepa;
                _sepa = value.indexOf("-");
                _nextsepa = value.indexOf("-",_sepa);
                String _dhtset=value.substring(0,sepa);
                _dhttype = (value.substring( _sepa + 1, _nextsepa)).toInt();
                _dhtpin = (value.substring( _nextsepa + 1, value.length())).toInt();
                if(_dhtset=="ON"){
                    DV_INF.USE_DHT = 1;
                    DV_INF.DHT_TYPE = _dhttype;
                    DV_INF.DHT_PIN = _dhtpin;
                }else{
                    DV_INF.USE_DHT = 0;
                }
                
                
                WF_INF.MASTER_SERVER_PORT=value.toInt();
                EEPROMSave(50,&DV_INF);
				Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                Serial.print(FPSTR(lb_SERVER_PORT));
                Serial.print(FPSTR(lb_TO));
			    Serial.println(value);
			}
			else if((command=="ip")&&LOGINED){
				WF_INF.WF_STATICIP = StringToIPAdress(value);
                EEPROMSave(50,&WF_INF);
				Serial.print(FPSTR(lb_CHANGE_SUCCESS));
                Serial.print(FPSTR(lb_STATIC));
                Serial.print(FPSTR(lb_IP));
                Serial.print(FPSTR(lb_TO));
				Serial.println(value);	
			}
			else if((command=="DHCP")&&LOGINED){
                if(WF_INF.IS_STATICIP!=1){
                    WF_INF.IS_STATICIP = 1;
                    Serial.println(FPSTR(lb_DHCP_ENABLE));
                }else{
                    WF_INF.IS_STATICIP = 0;
                    Serial.println(FPSTR(lb_DHCP_DISABLE));	
                }
                EEPROMSave(50,&WF_INF);
			}else{
				Serial.println(FPSTR(lb_COMMAND_ERROR));
			}
			SERIAL_RECEIVER="";
			SERIAL_COMPLETE = false;
		}
	}
}

//////////////////////////////////////
///     Group function for WiFi	   ///
//////////////////////////////////////

// Connect to wifi
void ESPHB::wifi_connect(void){
    if(String(WF_INF.WF_SSID)!=""){
		WiFi.mode(WIFI_STA);
		CONNECTED=false;
		unsigned char timeout=0;	// khởi tạo biến timeout

		if(DEBUG) Serial.println(FPSTR(lb_CONNECTING));
		if(DEBUG) {   Serial.print(FPSTR(lb_SSID));   Serial.println(WF_INF.WF_SSID);}
		if(DEBUG) {   Serial.print(FPSTR(lb_PASSWORD));Serial.println(WF_INF.WF_PASSWORD);}	
		WiFi.begin(WF_INF.WF_SSID, WF_INF.WF_PASSWORD);	// kết nối tới mạng wifi
		while (WiFi.status() != WL_CONNECTED) {	// nếu chưa kết nối được
			delay(500); if(DEBUG)    Serial.print(FPSTR(lb_DOT));	// xuất ký tự . mỗi 0.5s
			timeout++;	// tăng timeout
			APMODE=false;
			if(timeout>WF_INF.WF_CONN_TIMEOUT){	// nếu timeout > thời gian cho phép timeout_STA
				APMODE=true;	// bật cờ apmode
				CONNECTED=false;
                if(DEBUG)   Serial.println(FPSTR(lb_FAILED_CONNECT));
                if(DEBUG)   Serial.println(FPSTR(lb_APMODE_START));
                wifi_apmode();
                if(DEBUG)   Serial.println(FPSTR(lb_APMODE_STARTED));
				break;
			}
            CONNECTED=true;
		}
		if(DEBUG&&CONNECTED)  Serial.print(FPSTR(lb_CONNECTED));
        if(DEBUG&&CONNECTED){
            Serial.print(FPSTR(lb_STA));
            Serial.print(FPSTR(lb_MAC_ADDRESS));
            Serial.println(WiFi.macAddress());
        }  
        if(DEBUG&&CONNECTED){
            Serial.print(FPSTR(lb_STA));
            Serial.print(FPSTR(lb_IP));
            Serial.println(WiFi.localIP());
        }         
        
    }else{
        CONNECTED=false;
        APMODE=true;
        wifi_apmode();
        if(DEBUG)   Serial.println(FPSTR(lb_APMODE_STARTED));       
    }
 	
};
void ESPHB::wifi_reconnect(void){
	if(!APMODE&&(WiFi.status() != WL_CONNECTED)){
		wifi_connect();
	}
}
void ESPHB::wifi_apmode(void){
	if(APMODE){
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP(DV_INF.DV_SERIAL, WF_INF.AP_PASSWORD);
        if(DEBUG&&CONNECTED){
            Serial.print(FPSTR(lb_AP));
            Serial.print(FPSTR(lb_MAC_ADDRESS));
            Serial.println(WiFi.softAPmacAddress());
        }
        if(DEBUG&&CONNECTED){
            Serial.print(FPSTR(lb_AP));
            Serial.print(FPSTR(lb_IP));
            Serial.println(WiFi.softAPIP());
        }
	}
}
//////////////////////////////////////////
///     Group function for Utility	   ///
//////////////////////////////////////////


//////////////////////////////////////
///     Group function for HTML	   ///
//////////////////////////////////////
// Event when ESP8266 server receive request 
boolean ESPHB::CheckArlert(String *request){
    if((request->indexOf(">ON"))>0){
        if(DEBUG)   Serial.print(F("alert"));
        return true;       
    }else{
        return false;
    }
}

// Send GET Request and receive respone from server
// Ex html GET: "GET /hostlink/local/file.php?key1=value1&&key2=value2 HTTP/1.1\r\nHost: 192.168.0.3\r\nConnection: close\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: wifi-switch\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: vi-VN,vi;q=0.8,fr-FR;q=0.6,fr;q=0.4,en-US;q=0.2,en;q=0.2\r\n\r\n"
String ESPHB::sendRequest(void){
    String _jsonToSend="",respone="";
	WiFiClient client3;
	unsigned char _times_out=0, _timeout_get=0;
	if (!client3.connect(WF_INF.MASTER_SERVER, WF_INF.MASTER_SERVER_PORT)) {
		if(DEBUG){
            Serial.println(FPSTR(lb_FAILED_CONNECT));
            Serial.println(FPSTR(lb_TRY_RECONNECT));
        };
		delay(500);
		_times_out++;
		if(_times_out>WF_INF.MAX_REQUEST_TIMEOUT){
			if(DEBUG){
                Serial.println(FPSTR(lb_ERROR_CONNECT));
            };
			return FPSTR(lb_TIMEOUT_CONNECT);
		}	
	};
    jsonEncode(FIRSTJSON,&_jsonToSend,"serial",String(DV_INF.DV_SERIAL));
    jsonEncode(NEXTJSON,&_jsonToSend,"ssid",String(WF_INF.WF_SSID));
    jsonEncode(NEXTJSON,&_jsonToSend,"password",String(WF_INF.WF_PASSWORD));
    jsonEncode(NEXTJSON,&_jsonToSend,"keycontrol",String(WF_INF.WF_KEY));
    if(DV_INF.USE_DHT==1){
        jsonEncode(NEXTJSON,&_jsonToSend,"dht_humi", String(dht.readHumidity(),2));
        jsonEncode(NEXTJSON,&_jsonToSend,"dht_temp", String(dht.readTemperature(),2));        
    }

    jsonEncode(NEXTJSON,&_jsonToSend,"localip",String(WiFi.localIP()));
    jsonEncode(LASTJSON,&_jsonToSend,"MAC",String(WiFi.macAddress()));
    
    
    client3.print(_jsonToSend);
	if(DEBUG){
        Serial.print("\n");
        Serial.println(_jsonToSend);
    };
    delay(200);
	while (client3.available()) { 
		respone = client3.readString();
    }
	if(DEBUG){
        Serial.println(respone);
    };
	client3.stop();                                                                                                                                
	return respone;
}
boolean ESPHB::sendGETRequest(String *_link,String *respone){
	*respone="";
	WiFiClient client3;
	unsigned char _times_out=0, _timeout_get=0;
	if (!client3.connect(WF_INF.MASTER_SERVER, WF_INF.MASTER_SERVER_PORT)) {
		if(DEBUG){
            Serial.println(FPSTR(lb_FAILED_CONNECT));
            Serial.println(FPSTR(lb_TRY_RECONNECT));
        };
		delay(500);
		_times_out++;
		if(_times_out>WF_INF.MAX_REQUEST_TIMEOUT){
			if(DEBUG){
                Serial.println(FPSTR(lb_ERROR_CONNECT));
            };
			return 0;
		}	
	};
    String tosend="";
    tosend+= FPSTR(lb_HTTP_GET_PREFIX);
    tosend+= *_link;
    tosend+= FPSTR(lb_SERIAL_KEY_GET);
    tosend+= String(DV_INF.DV_SERIAL);
    tosend+= FPSTR(lb_HTTP_HEADER_VERSION);
    tosend+= FPSTR(lb_HTTP_HEADER_HOST);
    tosend+= String(WF_INF.MASTER_SERVER);
    tosend+= FPSTR(lb_NEWLINE);
    tosend+= FPSTR(lb_HTTP_HEADER_CONNECTION);
    client3.print(tosend);	// prefix "GET "
	if(DEBUG){
        Serial.print("\n");
        Serial.print(tosend);
    };
    delay(200);
	while (client3.available()) { 
		*respone = client3.readString();
    }
	if(DEBUG){
        Serial.println(*respone);
    };
	client3.stop();                                                                                                                                
	return 1;
}
// Add key, value to GETRequest String
void ESPHB::AddGetRequest(String *_s, String _key,String _val) {
    *_s += "&&" + _key + "=" + _val;
}


// Handler Event when ESP8266 server receive request
String ESPHB::httpHandlerEvent(String *request){
	String cssid,cpassword,ckey,cserver,respone="";
	GETValue(request,"key",&ckey);
	GETValue(request,"ssid",&cssid);
	GETValue(request,"password",&cpassword);
	GETValue(request,"server",&cserver);
	if((ckey==String(WF_INF.WF_KEY))&&(cssid.length()<=MAX_SSID_LEN)&&(cpassword.length()<=MAX_PASSWORD_LEN)&&(cserver.length()<=MAX_SERVER_LEN)){
		StringToArray(&cssid, WF_INF.WF_SSID, MAX_SSID_LEN);
        StringToArray(&cpassword, WF_INF.WF_PASSWORD, MAX_PASSWORD_LEN);
        StringToArray(&cserver, WF_INF.MASTER_SERVER, MAX_SERVER_LEN);
        EEPROMSave(50,&WF_INF);        
		jsonEncode(ONEJSON,&respone,"result",F("Success"));
		if(DEBUG){
            Serial.println(FPSTR(lb_CHANGE_SUCCESS));
            Serial.print(FPSTR(lb_SSID));
            Serial.print(FPSTR(lb_TO));
            Serial.println(cssid);
            Serial.print(FPSTR(lb_PASSWORD));
            Serial.print(FPSTR(lb_TO));
            Serial.println(cpassword);
            Serial.print(FPSTR(lb_SERVER));
            Serial.print(FPSTR(lb_TO));
            Serial.println(cserver);            
        };
	}else{
        jsonEncode(ONEJSON,&respone,"result",F("Error"));
    }
    return respone;
}
//////////////////////////////////////
///   Group function for Hardware  ///
//////////////////////////////////////

// Toggle PIN OUT
// Ex: PIN2=ON/OFF
boolean ESPHB::togglePin(String _pinname, String _value){	// toggle pinout
    uint8_t _pinControl = getPin(_pinname);
    digitalWrite(_pinControl, pinValue(_value));
    return digitalRead(_pinControl);
}
// Convert String PINxx to (int)xx (xx: 0-16)
uint8_t ESPHB::getPin(String _pin){	// Get pin name from string to int
	return _pin.substring(3,_pin.length()).toInt();
}
// Convert String ON/OFF to (boolean)HIGH/LOW
boolean ESPHB::pinValue(String _value){	// Get pin value from string to boolean
	if((_value=="ON")||(_value=="on")){
        return HIGH;}
    else{
        if((_value=="OFF")||(_value=="off")){return LOW;};
    } 
}
// Blink led
void ESPHB::LedBlink(unsigned long _interval){
	if(Timer(&last_blink, _interval)){
		if (LedState == LOW)
			LedState = HIGH;  // Note that this switches the LED *off*
		else if(LedState == HIGH)
			LedState = LOW;   // Note that this switches the LED *on*
		digitalWrite(LEDSTATUS, LedState);         
    }
}
void ESPHB::LedOn(void){
	digitalWrite(LEDSTATUS, HIGH);         
}
void ESPHB::LedOff(void){
	digitalWrite(LEDSTATUS, LOW);         
}
boolean ESPHB::Timer(unsigned long *_last_time, unsigned long _interval){
	unsigned long time_now=millis();
	if((time_now-*_last_time)>=_interval){
		*_last_time=time_now;
		return 1;        
    }else{
		return 0;
	}
}