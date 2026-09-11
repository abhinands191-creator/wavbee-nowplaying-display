
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <HTTPClient.h>

// ------------------------------------------------------------
// Spotify constants
// ------------------------------------------------------------

#define SPOTIFY_MARKET "IN"


// ------------------------------------------------------------
// Week 5 Day 3 - Album artwork RAM buffer
// ------------------------------------------------------------

uint8_t *g_imageBuffer = nullptr;
size_t g_imageSize = 0;

volatile bool g_artworkReady = false;
String g_cachedAlbumImageUrl = "";

bool g_artworkDownloadSucceeded = false;


// ------------------------------------------------------------
// Week 5 Day 3 - Download album artwork over HTTPS
// ------------------------------------------------------------

void downloadAlbumArtwork()
{
    // --------------------------------------------------------
    // Day 4: reset download success status
    // --------------------------------------------------------

    g_artworkDownloadSucceeded = false;


    if (g_albumImageUrl.length() == 0)
    {
        Serial.println(
            "No 300x300 album image URL available."
        );

        return;
    }


    Serial.println();

    Serial.println(
        "========== ARTWORK DOWNLOAD =========="
    );


    // --------------------------------------------------------
    // Check free heap BEFORE allocating
    // --------------------------------------------------------

    size_t freeHeap =
        ESP.getFreeHeap();


    Serial.print(
        "Free heap BEFORE download: "
    );

    Serial.println(
        freeHeap
    );


    // --------------------------------------------------------
    // HTTPS client
    // --------------------------------------------------------

    HTTPClient http;


    if (!http.begin(
            client,
            g_albumImageUrl
        ))
    {
        Serial.println(
            "ERROR: Failed to initialize HTTPS connection."
        );

        return;
    }


    Serial.println(
        "Downloading album artwork..."
    );


    // --------------------------------------------------------
    // Send HTTPS GET request
    // --------------------------------------------------------

    int httpCode =
        http.GET();


    Serial.print(
        "HTTP response code: "
    );

    Serial.println(
        httpCode
    );


    // --------------------------------------------------------
    // Successful download
    // --------------------------------------------------------

    if (httpCode == HTTP_CODE_OK)
    {
        int contentLength =
            http.getSize();


        Serial.print(
            "Expected JPEG size: "
        );

        Serial.print(
            contentLength
        );

        Serial.println(
            " bytes"
        );


        // ----------------------------------------------------
        // Check whether Content-Length is available
        // ----------------------------------------------------

        if (contentLength <= 0)
        {
            Serial.println(
                "ERROR: Invalid or unknown JPEG size."
            );

            http.end();

            return;
        }


        // ----------------------------------------------------
        // Make sure image fits into available RAM
        // ----------------------------------------------------

        if (
            (size_t)contentLength >=
            freeHeap
        )
        {
            Serial.println(
                "ERROR: Image is too large for available RAM."
            );

            http.end();

            return;
        }


        // ----------------------------------------------------
        // If an old artwork buffer still exists,
        // free it before allocating a new one.
        // ----------------------------------------------------

        if (g_imageBuffer != nullptr)
        {
            free(
                g_imageBuffer
            );

            g_imageBuffer =
                nullptr;

            g_imageSize =
                0;

            g_artworkReady =
                false;
        }


        // ----------------------------------------------------
        // Allocate RAM buffer
        // ----------------------------------------------------

        g_imageBuffer =
            (uint8_t *)malloc(
                contentLength
            );


        if (g_imageBuffer == nullptr)
        {
            Serial.println(
                "ERROR: RAM allocation failed."
            );

            http.end();

            return;
        }


        Serial.println(
            "RAM buffer allocated."
        );


        // ----------------------------------------------------
        // Download JPEG into RAM
        // ----------------------------------------------------

        WiFiClient *stream =
            http.getStreamPtr();


        int totalBytes =
            0;


        while (
            http.connected() &&
            totalBytes < contentLength
        )
        {
            size_t available =
                stream->available();


            if (available > 0)
            {
                int bytesToRead =
                    available;


                if (
                    totalBytes +
                    bytesToRead >
                    contentLength
                )
                {
                    bytesToRead =
                        contentLength -
                        totalBytes;
                }


                int bytesRead =
                    stream->readBytes(
                        g_imageBuffer +
                        totalBytes,
                        bytesToRead
                    );


                if (bytesRead > 0)
                {
                    totalBytes +=
                        bytesRead;
                }
            }


            delay(1);
        }


        // ----------------------------------------------------
        // Download result
        // ----------------------------------------------------

        Serial.print(
            "Downloaded bytes: "
        );

        Serial.println(
            totalBytes
        );


        Serial.print(
            "Free heap AFTER download: "
        );

        Serial.println(
            ESP.getFreeHeap()
        );


        if (
            totalBytes ==
            contentLength
        )
        {
            g_imageSize =
                totalBytes;


            // ------------------------------------------------
            // JPEG HEADER CHECK
            // ------------------------------------------------

            Serial.println();

            Serial.println(
                "---------- JPEG DIAGNOSTIC ----------"
            );


            Serial.print(
                "JPEG first bytes: "
            );


            Serial.print(
                g_imageBuffer[0],
                HEX
            );


            Serial.print(
                " "
            );


            Serial.print(
                g_imageBuffer[1],
                HEX
            );


            Serial.print(
                " "
            );


            Serial.println(
                g_imageBuffer[2],
                HEX
            );


            // ------------------------------------------------
            // JPEG END MARKER CHECK
            // ------------------------------------------------

            Serial.print(
                "JPEG last bytes: "
            );


            Serial.print(
                g_imageBuffer[
                    g_imageSize - 2
                ],
                HEX
            );


            Serial.print(
                " "
            );


            Serial.println(
                g_imageBuffer[
                    g_imageSize - 1
                ],
                HEX
            );


            Serial.println(
                "-------------------------------------"
            );


            // ------------------------------------------------
            // Tell MAIN LOOP that image is ready.
            // Main loop will decode and draw it.
            // ------------------------------------------------

            g_artworkReady =
                true;


            // ------------------------------------------------
            // Day 4: download completed successfully
            // ------------------------------------------------

            g_artworkDownloadSucceeded =
                true;


            Serial.println();

            Serial.println(
                "SUCCESS: JPEG downloaded completely into RAM."
            );

            Serial.println(
                "Artwork buffer is ready for TJpg_Decoder."
            );
        }
        else
        {
            Serial.println();

            Serial.println(
                "WARNING: Download incomplete."
            );


            // ------------------------------------------------
            // Free incomplete image
            // ------------------------------------------------

            free(
                g_imageBuffer
            );

            g_imageBuffer =
                nullptr;

            g_imageSize =
                0;

            g_artworkReady =
                false;
        }


        Serial.print(
            "Free heap AFTER download handling: "
        );

        Serial.println(
            ESP.getFreeHeap()
        );
    }


    // --------------------------------------------------------
    // Download failed
    // --------------------------------------------------------

    else
    {
        Serial.println();

        Serial.println(
            "ERROR: Failed to download album artwork."
        );
    }


    // --------------------------------------------------------
    // Close HTTP connection
    // --------------------------------------------------------

    http.end();


    Serial.println(
        "====================================="
    );
}


// ------------------------------------------------------------
// Currently Playing callback
// ------------------------------------------------------------

void printCurrentlyPlayingToSerial(
    CurrentlyPlaying currentlyPlaying
)
{
    g_isPlaying =
        currentlyPlaying.isPlaying;


    g_trackName =
        String(
            currentlyPlaying.trackName
        );


    g_trackUri =
        String(
            currentlyPlaying.trackUri
        );


    g_albumName =
        String(
            currentlyPlaying.albumName
        );


    // --------------------------------------------------------
    // Get progress and duration
    // --------------------------------------------------------

    g_progressMs =
        currentlyPlaying.progressMs;


    g_durationMs =
        currentlyPlaying.durationMs;


    g_lastProgressUpdate =
        millis();


    // --------------------------------------------------------
    // Get all credited artists
    // --------------------------------------------------------

    g_artistName =
        "";


    for (
        int i = 0;
        i < currentlyPlaying.numArtists;
        i++
    )
    {
        if (i > 0)
        {
            g_artistName +=
                ", ";
        }


        g_artistName +=
            String(
                currentlyPlaying.artists[i].artistName
            );
    }


    // --------------------------------------------------------
    // Album image
    // --------------------------------------------------------

    g_albumImageUrl =
        "";


    // --------------------------------------------------------
    // Serial output
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "--------- Currently Playing ---------"
    );


    Serial.print(
        "Is Playing: "
    );


    if (currentlyPlaying.isPlaying)
    {
        Serial.println(
            "Yes"
        );
    }
    else
    {
        Serial.println(
            "No"
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
        "Album: "
    );


    Serial.println(
        g_albumName
    );


    Serial.print(
        "Track URI: "
    );


    Serial.println(
        g_trackUri
    );


    // --------------------------------------------------------
    // Progress information
    // --------------------------------------------------------

    Serial.print(
        "Progress: "
    );


    Serial.print(
        g_progressMs
    );


    Serial.print(
        " ms / "
    );


    Serial.print(
        g_durationMs
    );


    Serial.println(
        " ms"
    );


    // --------------------------------------------------------
    // Album images
    // --------------------------------------------------------

    Serial.println();

    Serial.println(
        "Album Images:"
    );


    for (
        int i = 0;
        i < currentlyPlaying.numImages;
        i++
    )
    {
        Serial.print(
            "Image "
        );


        Serial.println(
            i
        );


        Serial.print(
            "URL: "
        );


        Serial.println(
            currentlyPlaying.albumImages[i].url
        );


        Serial.print(
            "Dimensions: "
        );


        Serial.print(
            currentlyPlaying.albumImages[i].width
        );


        Serial.print(
            " x "
        );


        Serial.println(
            currentlyPlaying.albumImages[i].height
        );


        if (
            currentlyPlaying.albumImages[i].width == 300
        )
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
        "NOTE: setInsecure() is being used as a deliberate"
    );


    Serial.println(
        "prototype shortcut. Production firmware should"
    );


    Serial.println(
        "verify or pin the server certificate."
    );


    Serial.println(
        "Testing direct connection to Spotify..."
    );


    if (
        client.connect(
            "accounts.spotify.com",
            443
        )
    )
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
        refreshResult
            ? "TRUE"
            : "FALSE"
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


    lastCheck =
        0;
}


// ------------------------------------------------------------
// Update Spotify
// ------------------------------------------------------------

void updateSpotify()
{
    if (
        millis() -
        lastCheck >=
        CHECK_INTERVAL
    )
    {
        lastCheck =
            millis();


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

        Serial.println(
            ">>> BEFORE Spotify request"
        );


        unsigned long spotifyStart =
            millis();


        int status =
            spotify.getCurrentlyPlaying(
                printCurrentlyPlayingToSerial,
                SPOTIFY_MARKET
            );


        Serial.println(
            ">>> AFTER Spotify request"
        );


        unsigned long spotifyTime =
            millis() -
            spotifyStart;


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


            if (
                g_albumImageUrl !=
                g_cachedAlbumImageUrl
            )
            {
                Serial.println(
                    "New album artwork detected - downloading."
                );


                downloadAlbumArtwork();


                // ------------------------------------------------
                // Day 4: only cache URL if download succeeded
                // ------------------------------------------------

                if (
                    g_artworkDownloadSucceeded
                )
                {
                    g_cachedAlbumImageUrl =
                        g_albumImageUrl;
                }
            }
            else
            {
                Serial.println(
                    "Same album artwork - using cached image."
                );
            }


            // ------------------------------------------------
            // Day 4: toggle live indicator
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

            if (
                g_trackUri != g_lastTrackUri ||
                g_isPlaying != g_lastIsPlaying
            )
            {
                Serial.println();


                if (
                    g_trackUri !=
                    g_lastTrackUri
                )
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
                    g_isPlaying
                        ? "Yes"
                        : "No"
                );


                Serial.println(
                    "***************************************"
                );


                g_lastTrackUri =
                    g_trackUri;


                g_lastIsPlaying =
                    g_isPlaying;


                // Tell main loop to redraw
                displayNeedsRedraw =
                    true;


                displayNeedsIdle =
                    false;
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
            // Free any artwork waiting in RAM
            // ------------------------------------------------

            if (g_imageBuffer != nullptr)
            {
                free(
                    g_imageBuffer
                );

                g_imageBuffer =
                    nullptr;

                g_imageSize =
                    0;
            }


            g_artworkReady =
                false;


            // ------------------------------------------------
            // Reset local progress
            // ------------------------------------------------

            displayProgressMs =
                0;


            displayDurationMs =
                0;


            displayIsPlaying =
                false;


            // ------------------------------------------------
            // Tell main loop to show idle screen
            // ------------------------------------------------

            displayNeedsIdle =
                true;


            displayNeedsRedraw =
                false;


            // ------------------------------------------------
            // Clear current track information
            // ------------------------------------------------

            g_trackName =
                "";


            g_artistName =
                "";


            g_albumName =
                "";


            g_trackUri =
                "";


            g_albumImageUrl =
                "";


            // ------------------------------------------------
            // Day 4: clear cached artwork URL
            // ------------------------------------------------

            g_cachedAlbumImageUrl =
                "";


            g_lastTrackUri =
                "";


            g_lastIsPlaying =
                false;
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

void spotifyTask(
    void *parameter
)
{
    while (true)
    {
        updateSpotify();


        vTaskDelay(
            10 /
            portTICK_PERIOD_MS
        );
    }
}
