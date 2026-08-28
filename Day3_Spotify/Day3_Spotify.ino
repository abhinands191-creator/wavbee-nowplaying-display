
/*******************************************************************
    WAVBEE - Week 3 Day 5
    Spotify Currently Playing + WiFi Resilience
 *******************************************************************/

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <ArduinoJson.h>

#include "secrets.h"

#define SPOTIFY_MARKET "IN"

char ssid[] = WIFI_SSID;
char password[] = WIFI_PASSWORD;

char clientId[] = SPOTIFY_CLIENT_ID;
char clientSecret[] = SPOTIFY_CLIENT_SECRET;

WiFiClientSecure client;

SpotifyArduino spotify(
    client,
    clientId,
    clientSecret,
    SPOTIFY_REFRESH_TOKEN
);

// Safe copies of Spotify data
String g_trackName;
String g_artistName;
String g_albumName;
String g_trackUri;
String g_albumImageUrl;
String g_lastTrackUri;

unsigned long lastCheck = 0;
const unsigned long CHECK_INTERVAL = 5000;


// ------------------------------------------------------------
// Currently Playing callback
// ------------------------------------------------------------

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
    // Copy Spotify data immediately
    g_trackName = String(currentlyPlaying.trackName);
    g_trackUri = String(currentlyPlaying.trackUri);
    g_albumName = String(currentlyPlaying.albumName);

    g_artistName = "";

    if (currentlyPlaying.numArtists > 0)
    {
        g_artistName = String(currentlyPlaying.artists[0].artistName);
    }

    g_albumImageUrl = "";

    Serial.println();
    Serial.println("--------- Currently Playing ---------");

    Serial.print("Is Playing: ");

    if (currentlyPlaying.isPlaying)
    {
        Serial.println("Yes");
    }
    else
    {
        Serial.println("No");
    }

    Serial.print("Track: ");
    Serial.println(g_trackName);

    Serial.print("Artist: ");
    Serial.println(g_artistName);

    Serial.print("Album: ");
    Serial.println(g_albumName);

    Serial.print("Track URI: ");
    Serial.println(g_trackUri);

    Serial.println();
    Serial.println("Album Images:");

    for (int i = 0; i < currentlyPlaying.numImages; i++)
    {
        Serial.print("Image ");
        Serial.println(i);

        Serial.print("URL: ");
        Serial.println(currentlyPlaying.albumImages[i].url);

        Serial.print("Dimensions: ");
        Serial.print(currentlyPlaying.albumImages[i].width);
        Serial.print(" x ");
        Serial.println(currentlyPlaying.albumImages[i].height);

        if (currentlyPlaying.albumImages[i].width == 300)
        {
            g_albumImageUrl =
                String(currentlyPlaying.albumImages[i].url);
        }

        Serial.println();
    }

    Serial.print("Selected 300x300 image URL: ");
    Serial.println(g_albumImageUrl);

    Serial.println("-------------------------------------");
}


// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("WAVBEE_TEST_123");

    delay(1500);

    Serial.println();
    Serial.println("=====================================");
    Serial.println("     WAVBEE Spotify Week 3 Day 5");
    Serial.println("=====================================");

    // --------------------------------------------------------
    // WiFi
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

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
    // HTTPS
    // --------------------------------------------------------

#if defined(ESP8266)

    client.setFingerprint(SPOTIFY_FINGERPRINT);

#elif defined(ESP32)

    // Keep this because it is currently working
    client.setInsecure();

#endif

    Serial.println();
    Serial.println("HTTPS client configured.");

    // --------------------------------------------------------
    // Direct Spotify connection test
    // --------------------------------------------------------

    Serial.println("Testing direct connection to Spotify...");

    if (client.connect("accounts.spotify.com", 443))
    {
        Serial.println("DIRECT CONNECTION SUCCESS!");
        client.stop();
    }
    else
    {
        Serial.println("DIRECT CONNECTION FAILED!");

        char errorBuffer[100];
        client.lastError(errorBuffer, sizeof(errorBuffer));

        Serial.print("TLS ERROR: ");
        Serial.println(errorBuffer);
    }

    // --------------------------------------------------------
    // Refresh Spotify access token
    // --------------------------------------------------------

    Serial.println();
    Serial.println("-------------------------------------");
    Serial.println("Refreshing Spotify access token...");
    Serial.println("-------------------------------------");

    bool refreshResult = spotify.refreshAccessToken();

    Serial.print("Refresh result: ");
    Serial.println(refreshResult ? "TRUE" : "FALSE");

    if (refreshResult)
    {
        Serial.println();
        Serial.println("SUCCESS!");
        Serial.println("Spotify access token refreshed.");
        Serial.println("Authentication appears to be working.");
    }
    else
    {
        Serial.println();
        Serial.println("FAILED!");
        Serial.println("Spotify access token refresh failed.");
        Serial.println("The 401 may be related to authentication.");
    }

    Serial.println();
    Serial.println("Starting Spotify currently-playing checks...");

    lastCheck = 0;
}


// ------------------------------------------------------------
// Main loop
// ------------------------------------------------------------

void loop()
{
    // --------------------------------------------------------
    // WiFi auto-reconnect
    // --------------------------------------------------------

    static unsigned long lastReconnectAttempt = 0;
    const unsigned long RECONNECT_INTERVAL = 5000;

    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastReconnectAttempt >= RECONNECT_INTERVAL)
        {
            lastReconnectAttempt = millis();

            Serial.println();
            Serial.println("WiFi disconnected. Reconnecting...");

            WiFi.reconnect();
        }
    }
    else
    {
        Serial.println("WiFi connected.");
    }

    // --------------------------------------------------------
    // Free heap monitoring
    // --------------------------------------------------------

    Serial.print("Free heap: ");
    Serial.println(ESP.getFreeHeap());

    // --------------------------------------------------------
    // Spotify currently-playing check
    // --------------------------------------------------------

    if (millis() - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = millis();

        Serial.println();
        Serial.println("=====================================");
        Serial.println("Getting currently playing song...");
        Serial.println("=====================================");

        int status = spotify.getCurrentlyPlaying(
            printCurrentlyPlayingToSerial,
            SPOTIFY_MARKET
        );

        Serial.println();
        Serial.print("Spotify response status: ");
        Serial.println(status);

        // ----------------------------------------------------
        // Successful response
        // ----------------------------------------------------

        if (status == 200)
        {
            Serial.println("200 OK - Spotify data received.");

            if (g_trackUri != g_lastTrackUri)
            {
                Serial.println();
                Serial.println("******** NEW TRACK! ********");

                Serial.print("Track: ");
                Serial.println(g_trackName);

                Serial.print("Artist: ");
                Serial.println(g_artistName);

                Serial.println("*****************************");

                g_lastTrackUri = g_trackUri;
            }
            else
            {
                Serial.println("Same track - no action needed.");
            }
        }

        // ----------------------------------------------------
        // Nothing playing
        // ----------------------------------------------------

        else if (status == 204)
        {
            Serial.println("204 - Nothing is currently playing.");
        }

        // ----------------------------------------------------
        // Unauthorized
        // ----------------------------------------------------

        else if (status == 401)
        {
            Serial.println();
            Serial.println("******** 401 UNAUTHORIZED ********");
            Serial.println("Spotify rejected the access token.");
            Serial.println("****************************************");
        }

        // ----------------------------------------------------
        // Other error
        // ----------------------------------------------------

        else
        {
            Serial.print("Spotify API error: ");
            Serial.println(status);
        }
    }

    delay(10);
}

