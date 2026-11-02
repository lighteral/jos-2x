/*  [RASPBERRY PI PICO IMPLEMENTATION]
	JOS REWRITTEN v2.0.0
	src/JOS_onfig.h
  Central User configuration file.
	Lighteral (2026)
*/
#ifndef JOS_CONFIG
#define JOS_CONFIG
#include <Arduino.h>
/* Section A: Pin Mapping */
#define UP_BUTTON        6
#define DOWN_BUTTON      7
#define SELECT_BUTTON    8
#define BACK_BUTTON      9
#define BUZZER           20
#define DHTPIN           21
/* Section B: System settings */
#define SPLASHSCREEN     true   // Display the Splash screen.
#define SPLASH_LENGTH_MS 2000   // Length of the Splash screen. Time in milliseconds
#define SCREEN_WIDTH     128    // Width of OLED display
#define SCREEN_HEIGHT    64     // Height of OLED display
#define SCREEN_ADDRESS   0x3C   // Address of OLED display
#define RTC_ADDRESS      0x68   // Address of RTC module
#define DHTTYPE          DHT11  // Type of DHT module
#define USE_RTC          false  // Read a DS3231/DS1307 RTC; false uses a software clock.
// PRANKS WERE REMOVED.
/* Section C: Useful stuff */
#define GAME             true  // Show the game in the menu.
#define DEVICE_NAME      "Mainstream" // Name of the device, shown on the About page.
/* Section D: Application-defined */
constexpr unsigned long BALL_RATE          = 16;
constexpr unsigned long PADDLE_RATE        = 64;
constexpr uint8_t       PADDLE_HEIGHT      = 12;
constexpr uint8_t       SCORE_LIMIT        = 9;
constexpr uint8_t       PLAYER_X           = 115;
constexpr uint8_t       MCU_X              = 12;
constexpr uint8_t       MENU_ITEMS         = 6;
constexpr uint8_t       HEADER_HEIGHT      = 10; // Inverted title/clock bar.
constexpr unsigned long DHT_READ_INTERVAL  = 1000;
constexpr bool          BOOT_TUNE_ENABLED  = true; // Play a jingle during the splash.
/* Section E: Build metadata */
#define JOS_VERSION            "2.0.0"
#define JOS_EEPROM_FS_VERSION  "0.0.5"
#define JOS_BOARD_NAME         "Raspi Pico 1"
#define JOS_PROCESSOR          "RP2040"
#define JOS_OLED_RESET         -1
#endif
