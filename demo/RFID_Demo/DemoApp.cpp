// Do not remove the include below
#include "DemoApp.h"
#include "RFIDModule.h"

#include "GPS/GPS.h"

//---------------------------
// Modules
//---------------------------
RFIDModule rfidModule;
bool rfid_activated;

//---------------------------
// App Menu/LCD/Keypad setup
//---------------------------
// LCD setup (pins)
#ifndef BOARD
	#warning Board type not defined, using -DBOARD=xxx to specify one. Supported board types: arduion_mega_2560, arduino_promini_328
	#define BOARD arduion_mega_2560
#endif
#if BOARD == arduino_mega_2560
		#define LCD_RST 6
		#define LCD_CE 5
		#define LCD_DC 4
		#define LCD_DIN 3
		#define LCD_CLK 2
		#define LCD_BL 7
	#elif BOARD == arduino_promini_328
		#define LCD_RST A3
		#define LCD_CE 2
		#define LCD_DC 3
		#define LCD_DIN 4
		#define LCD_CLK 5
		#define LCD_BL 6
#endif

LCD5110 lcd(LCD_RST, LCD_CE, LCD_DC, LCD_DIN, LCD_CLK, LCD_BL);

// Keypad setup
#define KEYPAD_INPUT A0
#define KEY_UP 5
#define KEY_DOWN 3
#define KEY_LEFT 1
#define KEY_RIGHT 2
#define KEY_ENTER 4

AnalogKeypad keypad(KEYPAD_INPUT);
bool default_keypad_callback(Keycode keycode);

// GPS module
SoftwareSerial gpsPort(10,11);

GPS gps(&gpsPort,&lcd,0);
bool gps_keypad_callback(Keycode keycode);
bool gps_menu_callback(void* item);
bool gps_activated;

// Menus
bool mHello(void* item);
bool mSettings_backlight(void* item);
bool mAbout(void* item);
bool mBack(void* item);


//MenuItem menu_a1[] = {
//		{"A-1-1","",mHello,0},
//		{"A-1-2","",mHello,0},
//		{"A-1-3","",mHello,0},
//		{"A-1-4","",mHello,0},
//		{"A-1-5","",mHello,0},
//		{NULL,NULL,NULL,NULL}
//};
//MenuItem menu_a2[] = {
//		{"A-2-1","",mHello,0},
//		{"A-2-2","",mHello,0},
//		{"A-2-3","",mHello,0},
//		{"A-2-4","",mHello,0},
//		{"A-2-5","",mHello,0},
//		{NULL,NULL,NULL,NULL}
//};
//
//MenuItem menu_a[] = {
//		{"A-1","",mHello,menu_a1},
//		{"A-2","",mHello,menu_a2},
//		{"A-3","",mHello,0},
//		{"A-4","",mHello,0},
//		{"A-5","",mHello,0},
//		{NULL,NULL,NULL,NULL}
//};
MenuItem menu_settings[] = {
		{"..","",mBack,0},
		{"Back light","",mSettings_backlight,0},
		{"Overclock","",0,0},
		{"Boot to windows","",0,0},
		{"...->","",0,0},
		{NULL,NULL,NULL,NULL}
} ;
MenuItem menu_root[] = {
		{"GPS Monitor","",gps_menu_callback,0},
		{"RPi Monitor","",mHello,0},
		{"RFID Reader","",mHello,0},
		{"Settings","",0,menu_settings},
		{"About","",mAbout,0},
		{NULL,NULL,NULL,NULL}
};
Menu menu(menu_root,&lcd);

//-------------------
// Arduino Entrance
//-------------------
void setup() {
	Serial.begin(9600); // RFID reader SOUT pin connected to Serial RX pin at 2400bps

	// LCD init
	lcd.init();
	lcd.setBacklight(ON);
	lcd.clear();
	lcd.setBacklight(OFF);

	// Keypad init
	keypad.init(default_keypad_callback,0,0,0);
	menu.updateLCD();

	// RFID module setup
	rfidModule.setup();

	// GPS module setup
	gps.setup();
}

void loop(){
	keypad.runloop();

	if(rfid_activated)
		rfidModule.loop();

	if(gps_activated)
		gps.loop();
}

//-----------------------------------------
// Keypad & menu callback implementations
//-----------------------------------------
bool default_keypad_callback(Keycode keycode){
	char buf[64];
	snprintf(buf,64,"Key pressed: %d", keycode);
	Serial.println(buf);
	lcd.drawString(0,5,buf);

	//FIXME call delegated method
	if(gps_activated){
		return gps_keypad_callback(keycode);
	}

	switch(keycode){
	case KEY_UP: // up
		menu.prev();
		return false; // allows key repeat
		break;
	case KEY_DOWN: // down
		menu.next();
		return false; // allow key repeat
		break;
	case KEY_ENTER: // center
		menu.enter();
		break;
	case KEY_LEFT: // left
		menu.leave();
		break;
	case KEY_RIGHT: // right
		menu.enter();
		break;
	default:
		break;
	}
	return true; // done, do not repeat.
}

bool mBack(void* item){
	menu.leave();
	return true;
}

bool mHello(void* item){
	lcd.clear();
	lcd.drawString(25,0,"DEMO",true);
	char buf[128];
	snprintf(buf,128,"%s",((MenuItem*)item)->title);
	Serial.println(buf);
	lcd.drawString(0,2,buf);
	return true;
}

bool mSettings_backlight(void* item){
	lcd.setBacklight(!lcd.backlight());
	return true;
}

bool mAbout(void* item){
	lcd.clear();
	lcd.drawString(25,0,"About",true);
	lcd.drawString(0,2,"MagicBox 1.0");
	lcd.drawString(0,5,"S.C. 2013");
//	char buf[128];
//	snprintf(buf,128,"Greetings, this is %s",((MenuItem*)item)->title);
//	Serial.println(buf);
//	lcd.drawString(2,0,buf);
	return true;
}

// GPS Module
bool gps_menu_callback(void* item){
	if(!gps_activated){
		// activate the gps modules
		gps_activated = true;
	}

	// draw gps info on screen
	lcd.clear();
	lcd.setBacklight(1);
//	lcd.drawString(0,5,"GPS Info");
//	lcd.drawString(25,0,"GPS Info",true);
//	char buf[128];
//	snprintf(buf,128,"%s",((MenuItem*)item)->title);
//	Serial.println(buf);
//	lcd.drawString(0,2,buf);
	return true;
}

bool gps_keypad_callback(Keycode keycode){
	switch (keycode) {
	case KEY_LEFT: // leave gps module
		if (gps_activated) {
			//TODO - confirm for the GPS activation
			gps_activated = false;
			// move back
			menu.leave();
		}
		menu.leave();
		break;
	case KEY_ENTER:
		// TODO - press enter to pause update
	default:
		break;
	}
	return true; // done, do not repeat.
}

void gps_lcd_update(void){

}
