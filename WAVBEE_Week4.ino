/*******************************************************************
    WAVBEE - Week 4 Day 4
    Spotify + TFT Display + NTP + Live Indicator
 *******************************************************************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SpotifyArduino.h>
#include <ArduinoJson.h>
#include <time.h>

#include "secrets.h"


// ------------------------------------------------------------
// WiFi
// ------------------------------------------------------------

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;


// ------------------------------------------------------------
// Spotify
// ------------------------------------------------------------

char clientId[] = SPOTIFY_CLIENT_ID;
char clientSecret[] = SPOTIFY_CLIENT_SECRET;

WiFiClientSecure client;

SpotifyArduino spotify(
    client,
    clientId,
    clientSecret,
    SPOTIFY_REFRESH_TOKEN
);


// ------------------------------------------------------------
// Spotify data
// ------------------------------------------------------------

String g_trackName;
String g_artistName;
String g_albumName;
String g_trackUri;
String g_albumImageUrl;

String g_lastTrackUri;

bool g_isPlaying = false;
bool g_lastIsPlaying = false;


// ------------------------------------------------------------
// Display flags
// ------------------------------------------------------------

volatile bool displayNeedsRedraw = false;
volatile bool displayNeedsIdle = false;


// ------------------------------------------------------------
// Live indicator
// ------------------------------------------------------------

volatile bool liveDotState = false;


// ------------------------------------------------------------
// Local progress timing
// ------------------------------------------------------------

unsigned long g_progressMs = 0;
unsigned long g_durationMs = 0;
unsigned long g_lastProgressUpdate = 0;


// ------------------------------------------------------------
// Spotify check interval
// ------------------------------------------------------------

unsigned long lastCheck = 0;

const unsigned long CHECK_INTERVAL = 5000;


// ------------------------------------------------------------
// Spotify FreeRTOS task
// ------------------------------------------------------------

TaskHandle_t spotifyTaskHandle = NULL;


// ------------------------------------------------------------
// WiFi reconnect
// ------------------------------------------------------------

unsigned long lastReconnectAttempt = 0;

const unsigned long RECONNECT_INTERVAL = 5000;


// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------

void setup()
{
    Serial.begin(115200);

    delay(1500);

    Serial.println();
    Serial.println("=====================================");
    Serial.println("     WAVBEE - Week 4 Day 4");
    Serial.println("=====================================");


    // --------------------------------------------------------
    // Connect to WiFi
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        ssid,
        password
    );

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


    // --------------------------------------------------------
    // Initialize NTP time
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Initializing NTP time...");

    // India Standard Time = UTC + 5:30
    configTime(
        19800,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );

    Serial.println("NTP configured.");


    // --------------------------------------------------------
    // Initialize display
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Initializing display...");

    initDisplay();

    Serial.println("Display initialized.");


    // --------------------------------------------------------
    // Initialize Spotify
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Initializing Spotify...");

    initSpotify();

    Serial.println("Spotify initialized.");


    // --------------------------------------------------------
    // Start Spotify task
    // --------------------------------------------------------

    xTaskCreatePinnedToCore(
        spotifyTask,
        "SpotifyTask",
        10000,
        NULL,
        1,
        &spotifyTaskHandle,
        0
    );


    Serial.println();
    Serial.println(
        "WAVBEE Week 4 Day 4 started."
    );

    lastCheck = 0;
}


// ------------------------------------------------------------
// MAIN LOOP
// ------------------------------------------------------------

void loop()
{
    // --------------------------------------------------------
    // WiFi auto-reconnect
    // --------------------------------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastReconnectAttempt >=
            RECONNECT_INTERVAL)
        {
            lastReconnectAttempt = millis();

            Serial.println();
            Serial.println(
                "WiFi disconnected. Reconnecting..."
            );

            WiFi.reconnect();
        }
    }


    // --------------------------------------------------------
    // Free heap monitoring
    // --------------------------------------------------------

    static unsigned long lastHeapPrint = 0;

    if (millis() - lastHeapPrint >= 5000)
    {
        lastHeapPrint = millis();

        Serial.print("Free heap: ");
        Serial.println(
            ESP.getFreeHeap()
        );
    }


    // --------------------------------------------------------
    // Show Nothing Playing screen
    // --------------------------------------------------------

    if (displayNeedsIdle)
    {
        displayNeedsIdle = false;

        drawIdleScreen();
    }

// --------------------------------------------------------
// Live dot update
// --------------------------------------------------------

static bool lastLiveDotState = false;

if (liveDotState != lastLiveDotState)
{
    lastLiveDotState = liveDotState;

    drawLiveDot();
}
    // --------------------------------------------------------
    // Redraw Now Playing screen
    // --------------------------------------------------------

    if (displayNeedsRedraw)
    {
        displayNeedsRedraw = false;

        drawNowPlaying(
            g_trackName,
            g_artistName,
            g_albumName,
            g_progressMs,
            g_durationMs,
            g_isPlaying
        );
    }


    // --------------------------------------------------------
    // Update display
    // --------------------------------------------------------

    updateDisplay();


    // --------------------------------------------------------
    // Small delay
    // --------------------------------------------------------

    delay(10);
}