#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>


// ------------------------------------------------------------
// Spotify constants
// ------------------------------------------------------------

#define SPOTIFY_MARKET "IN"


// ------------------------------------------------------------
// Currently Playing callback
// ------------------------------------------------------------

void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
{
    g_isPlaying = currentlyPlaying.isPlaying;

    g_trackName =
        String(currentlyPlaying.trackName);

    g_trackUri =
        String(currentlyPlaying.trackUri);

    g_albumName =
        String(currentlyPlaying.albumName);


    // --------------------------------------------------------
    // Get progress and duration
    // --------------------------------------------------------

    g_progressMs =
        currentlyPlaying.progressMs;

    g_durationMs =
        currentlyPlaying.durationMs;

    // Reset local timing whenever Spotify gives us
    // a fresh progress position.

    g_lastProgressUpdate =
        millis();


    // --------------------------------------------------------
    // Get all credited artists
    // --------------------------------------------------------

    g_artistName = "";

    for (int i = 0;
         i < currentlyPlaying.numArtists;
         i++)
    {
        if (i > 0)
        {
            g_artistName += ", ";
        }

        g_artistName +=
            String(
                currentlyPlaying.artists[i].artistName
            );
    }


    // --------------------------------------------------------
    // Album image
    // --------------------------------------------------------

    g_albumImageUrl = "";


    // --------------------------------------------------------
    // Serial output
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "--------- Currently Playing ---------"
    );


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

    Serial.println(
        g_trackName
    );


    Serial.print("Artist: ");

    Serial.println(
        g_artistName
    );


    Serial.print("Album: ");

    Serial.println(
        g_albumName
    );


    Serial.print("Track URI: ");

    Serial.println(
        g_trackUri
    );


    // --------------------------------------------------------
    // Progress information
    // --------------------------------------------------------

    Serial.print("Progress: ");

    Serial.print(
        g_progressMs
    );

    Serial.print(" ms / ");

    Serial.print(
        g_durationMs
    );

    Serial.println(" ms");


    // --------------------------------------------------------
    // Album images
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "Album Images:"
    );


    for (int i = 0;
         i < currentlyPlaying.numImages;
         i++)
    {
        Serial.print("Image ");

        Serial.println(i);


        Serial.print("URL: ");

        Serial.println(
            currentlyPlaying.albumImages[i].url
        );


        Serial.print("Dimensions: ");

        Serial.print(
            currentlyPlaying.albumImages[i].width
        );

        Serial.print(" x ");

        Serial.println(
            currentlyPlaying.albumImages[i].height
        );


        if (currentlyPlaying.albumImages[i].width == 300)
        {
            g_albumImageUrl =
                String(
                    currentlyPlaying.albumImages[i].url
                );
        }


        Serial.println();
    }


    Serial.print(
        "Selected 300x300 image URL: "
    );

    Serial.println(
        g_albumImageUrl
    );


    Serial.println(
        "-------------------------------------"
    );
}


// ------------------------------------------------------------
// Initialize Spotify
// ------------------------------------------------------------

void initSpotify()
{
    client.setInsecure();


    Serial.println();

    Serial.println(
        "HTTPS client configured."
    );


    Serial.println(
        "Testing direct connection to Spotify..."
    );


    if (client.connect(
            "accounts.spotify.com",
            443))
    {
        Serial.println(
            "DIRECT CONNECTION SUCCESS!"
        );

        client.stop();
    }
    else
    {
        Serial.println(
            "DIRECT CONNECTION FAILED!"
        );


        char errorBuffer[100];


        client.lastError(
            errorBuffer,
            sizeof(errorBuffer)
        );


        Serial.print(
            "TLS ERROR: "
        );

        Serial.println(
            errorBuffer
        );
    }


    Serial.println();

    Serial.println(
        "-------------------------------------"
    );

    Serial.println(
        "Refreshing Spotify access token..."
    );

    Serial.println(
        "-------------------------------------"
    );


    bool refreshResult =
        spotify.refreshAccessToken();


    Serial.print(
        "Refresh result: "
    );


    Serial.println(
        refreshResult ? "TRUE" : "FALSE"
    );


    if (refreshResult)
    {
        Serial.println();

        Serial.println(
            "SUCCESS!"
        );

        Serial.println(
            "Spotify access token refreshed."
        );

        Serial.println(
            "Authentication appears to be working."
        );
    }
    else
    {
        Serial.println();

        Serial.println(
            "FAILED!"
        );

        Serial.println(
            "Spotify access token refresh failed."
        );
    }


    lastCheck = 0;
}


// ------------------------------------------------------------
// Update Spotify
// ------------------------------------------------------------

void updateSpotify()
{
    if (millis() - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = millis();


        Serial.println();

        Serial.println(
            "====================================="
        );

        Serial.println(
            "Getting currently playing song..."
        );

        Serial.println(
            "====================================="
        );


        // ----------------------------------------------------
        // Heap BEFORE Spotify request
        // ----------------------------------------------------

        Serial.print(
            "Heap BEFORE Spotify: "
        );

        Serial.println(
            ESP.getFreeHeap()
        );


        // ----------------------------------------------------
        // Spotify request
        // ----------------------------------------------------

        unsigned long spotifyStart =
            millis();


        int status =
            spotify.getCurrentlyPlaying(
                printCurrentlyPlayingToSerial,
                SPOTIFY_MARKET
            );


        unsigned long spotifyTime =
            millis() - spotifyStart;


        Serial.print(
            "Spotify request took: "
        );

        Serial.print(
            spotifyTime
        );

        Serial.println(
            " ms"
        );


        // ----------------------------------------------------
        // Heap AFTER Spotify request
        // ----------------------------------------------------

        Serial.print(
            "Heap AFTER Spotify: "
        );

        Serial.println(
            ESP.getFreeHeap()
        );


        Serial.println();

        Serial.print(
            "Spotify response status: "
        );

        Serial.println(
            status
        );


        // ----------------------------------------------------
        // Successful response
        // ----------------------------------------------------

        if (status == 200)
        {
            Serial.println(
                "200 OK - Spotify data received."
            );


            // ------------------------------------------------
            // Day 4: toggle live indicator
            //
            // Only change the shared state here.
            // The MAIN LOOP draws the TFT indicator.
            // ------------------------------------------------

            liveDotState =
                !liveDotState;


            // ------------------------------------------------
            // Synchronize local display timing
            // ------------------------------------------------

            displayProgressMs =
                g_progressMs;

            displayDurationMs =
                g_durationMs;

            displayIsPlaying =
                g_isPlaying;

            lastProgressTime =
                millis();


            // ------------------------------------------------
            // Redraw when track OR play/pause changes
            // ------------------------------------------------

            if (g_trackUri != g_lastTrackUri ||
                g_isPlaying != g_lastIsPlaying)
            {
                Serial.println();


                if (g_trackUri != g_lastTrackUri)
                {
                    Serial.println(
                        "******** NEW TRACK! ********"
                    );
                }
                else
                {
                    Serial.println(
                        "******** PLAY/PAUSE CHANGED! ********"
                    );
                }


                Serial.print(
                    "Track: "
                );

                Serial.println(
                    g_trackName
                );


                Serial.print(
                    "Artist: "
                );

                Serial.println(
                    g_artistName
                );


                Serial.print(
                    "Playing: "
                );

                Serial.println(
                    g_isPlaying ? "Yes" : "No"
                );


                Serial.println(
                    "***************************************"
                );


                g_lastTrackUri =
                    g_trackUri;

                g_lastIsPlaying =
                    g_isPlaying;


                // Tell main loop to redraw
                displayNeedsRedraw = true;

                // Make sure idle screen is not pending
                displayNeedsIdle = false;
            }
            else
            {
                Serial.println(
                    "Same track and same play state - progress synchronized."
                );
            }
        }


        // ----------------------------------------------------
        // Nothing playing
        // ----------------------------------------------------

        else if (status == 204)
        {
            Serial.println(
                "204 - Nothing is currently playing."
            );


            // ------------------------------------------------
            // Reset local progress
            // ------------------------------------------------

            displayProgressMs = 0;

            displayDurationMs = 0;

            displayIsPlaying = false;


            // ------------------------------------------------
            // Tell main loop to show idle screen
            // ------------------------------------------------

            displayNeedsIdle = true;

            displayNeedsRedraw = false;


            // ------------------------------------------------
            // Clear current track information
            // ------------------------------------------------

            g_trackName = "";

            g_artistName = "";

            g_albumName = "";

            g_trackUri = "";


            g_lastTrackUri = "";

            g_lastIsPlaying = false;
        }


        // ----------------------------------------------------
        // Unauthorized
        // ----------------------------------------------------

        else if (status == 401)
        {
            Serial.println();

            Serial.println(
                "******** 401 UNAUTHORIZED ********"
            );

            Serial.println(
                "Spotify rejected the access token."
            );

            Serial.println(
                "****************************************"
            );
        }


        // ----------------------------------------------------
        // Other error
        // ----------------------------------------------------

        else
        {
            Serial.print(
                "Spotify API error: "
            );

            Serial.println(
                status
            );
        }
    }
}


// ------------------------------------------------------------
// Spotify FreeRTOS task
// ------------------------------------------------------------

void spotifyTask(void *parameter)
{
    while (true)
    {
        updateSpotify();

        vTaskDelay(
            10 / portTICK_PERIOD_MS
        );
    }
}