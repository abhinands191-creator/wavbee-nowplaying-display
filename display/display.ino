#include <TFT_eSPI.h>
#include "NotoSansMalayalamRegular20.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

int scrollX = 0;
bool titleNeedsScroll = false;
bool titleUsesMalayalamFont = false;
String currentTitle = "";

unsigned long lastScrollTime = 0;

// Local progress timing
unsigned long displayProgressMs = 0;
unsigned long displayDurationMs = 0;
unsigned long lastProgressTime = 0;
bool displayIsPlaying = false;

int barWidth = 300;


// ------------------------------------------------------------
// Display initialization
// ------------------------------------------------------------

void initDisplay()
{
    tft.init();

    tft.setRotation(1);

    tft.invertDisplay(true);

    drawBootScreen();

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    tft.setTextDatum(
        TL_DATUM
    );
}


// ------------------------------------------------------------
// Fit text
// ------------------------------------------------------------

String fitText(
    String s,
    int maxWidth,
    int font
)
{
    if (tft.textWidth(s, font) <= maxWidth)
    {
        return s;
    }

    while (
        s.length() > 0 &&
        tft.textWidth(
            s + "...",
            font
        ) > maxWidth
    )
    {
        s.remove(
            s.length() - 1
        );
    }

    return s + "...";
}


// ------------------------------------------------------------
// Normalize Latin characters
// Converts accented/special Latin characters to basic ASCII
// Example: JAŸ-Z -> JAY-Z
// ------------------------------------------------------------

String normalizeLatin(String s)
{
    String result = "";

    for (int i = 0; i < s.length(); i++)
    {
        uint8_t c = (uint8_t)s[i];

        // ----------------------------------------------------
        // Normal ASCII character
        // ----------------------------------------------------

        if (c < 128)
        {
            result += (char)c;
            continue;
        }


        // ----------------------------------------------------
        // Decode UTF-8
        // ----------------------------------------------------

        uint32_t codepoint = 0;

        if (
            (c & 0xE0) == 0xC0 &&
            i + 1 < s.length()
        )
        {
            codepoint =
                ((uint32_t)(c & 0x1F) << 6) |
                ((uint8_t)s[++i] & 0x3F);
        }

        else if (
            (c & 0xF0) == 0xE0 &&
            i + 2 < s.length()
        )
        {
            codepoint =
                ((uint32_t)(c & 0x0F) << 12) |
                ((uint32_t)((uint8_t)s[++i] & 0x3F) << 6) |
                ((uint8_t)s[++i] & 0x3F);
        }

        else if (
            (c & 0xF8) == 0xF0 &&
            i + 3 < s.length()
        )
        {
            codepoint =
                ((uint32_t)(c & 0x07) << 18) |
                ((uint32_t)((uint8_t)s[++i] & 0x3F) << 12) |
                ((uint32_t)((uint8_t)s[++i] & 0x3F) << 6) |
                ((uint8_t)s[++i] & 0x3F);
        }

        else
        {
            continue;
        }


        // ----------------------------------------------------
        // Latin character conversion
        // ----------------------------------------------------

        switch (codepoint)
        {
            // ------------------------------------------------
            // A
            // ------------------------------------------------

            case 0x00C0:
            case 0x00C1:
            case 0x00C2:
            case 0x00C3:
            case 0x00C4:
            case 0x00C5:
            case 0x0100:
            case 0x0102:
            case 0x0104:
            case 0x01CD:
            case 0x01DE:
            case 0x01E0:
            case 0x01FA:
            case 0x0200:
            case 0x0202:
                result += "A";
                break;


            // ------------------------------------------------
            // a
            // ------------------------------------------------

            case 0x00E0:
            case 0x00E1:
            case 0x00E2:
            case 0x00E3:
            case 0x00E4:
            case 0x00E5:
            case 0x0101:
            case 0x0103:
            case 0x0105:
            case 0x01CE:
            case 0x01DF:
            case 0x01E1:
            case 0x01FB:
            case 0x0201:
            case 0x0203:
                result += "a";
                break;


            // ------------------------------------------------
            // AE
            // ------------------------------------------------

            case 0x00C6:
                result += "AE";
                break;

            case 0x00E6:
                result += "ae";
                break;


            // ------------------------------------------------
            // C
            // ------------------------------------------------

            case 0x00C7:
            case 0x0106:
            case 0x0108:
            case 0x010A:
            case 0x010C:
                result += "C";
                break;

            case 0x00E7:
            case 0x0107:
            case 0x0109:
            case 0x010B:
            case 0x010D:
                result += "c";
                break;


            // ------------------------------------------------
            // D
            // ------------------------------------------------

            case 0x00D0:
            case 0x010E:
            case 0x0110:
                result += "D";
                break;

            case 0x00F0:
            case 0x010F:
            case 0x0111:
                result += "d";
                break;


            // ------------------------------------------------
            // E
            // ------------------------------------------------

            case 0x00C8:
            case 0x00C9:
            case 0x00CA:
            case 0x00CB:
            case 0x0112:
            case 0x0114:
            case 0x0116:
            case 0x0118:
            case 0x011A:
            case 0x0204:
            case 0x0206:
                result += "E";
                break;

            case 0x00E8:
            case 0x00E9:
            case 0x00EA:
            case 0x00EB:
            case 0x0113:
            case 0x0115:
            case 0x0117:
            case 0x0119:
            case 0x011B:
            case 0x0205:
            case 0x0207:
                result += "e";
                break;


            // ------------------------------------------------
            // G
            // ------------------------------------------------

            case 0x011C:
            case 0x011E:
            case 0x0120:
            case 0x0122:
                result += "G";
                break;

            case 0x011D:
            case 0x011F:
            case 0x0121:
            case 0x0123:
                result += "g";
                break;


            // ------------------------------------------------
            // H
            // ------------------------------------------------

            case 0x0124:
            case 0x0126:
                result += "H";
                break;

            case 0x0125:
            case 0x0127:
                result += "h";
                break;


            // ------------------------------------------------
            // I
            // ------------------------------------------------

            case 0x00CC:
            case 0x00CD:
            case 0x00CE:
            case 0x00CF:
            case 0x0128:
            case 0x012A:
            case 0x012C:
            case 0x012E:
            case 0x0130:
            case 0x0208:
            case 0x020A:
                result += "I";
                break;

            case 0x00EC:
            case 0x00ED:
            case 0x00EE:
            case 0x00EF:
            case 0x0129:
            case 0x012B:
            case 0x012D:
            case 0x012F:
            case 0x0131:
            case 0x0209:
            case 0x020B:
                result += "i";
                break;


            // ------------------------------------------------
            // J
            // ------------------------------------------------

            case 0x0134:
                result += "J";
                break;

            case 0x0135:
                result += "j";
                break;


            // ------------------------------------------------
            // K
            // ------------------------------------------------

            case 0x0136:
                result += "K";
                break;

            case 0x0137:
                result += "k";
                break;


            // ------------------------------------------------
            // L
            // ------------------------------------------------

            case 0x0139:
            case 0x013B:
            case 0x013D:
            case 0x013F:
            case 0x0141:
                result += "L";
                break;

            case 0x013A:
            case 0x013C:
            case 0x013E:
            case 0x0140:
            case 0x0142:
                result += "l";
                break;


            // ------------------------------------------------
            // N
            // ------------------------------------------------

            case 0x00D1:
            case 0x0143:
            case 0x0145:
            case 0x0147:
            case 0x014A:
                result += "N";
                break;

            case 0x00F1:
            case 0x0144:
            case 0x0146:
            case 0x0148:
            case 0x014B:
                result += "n";
                break;


            // ------------------------------------------------
            // O
            // ------------------------------------------------

            case 0x00D2:
            case 0x00D3:
            case 0x00D4:
            case 0x00D5:
            case 0x00D6:
            case 0x00D8:
            case 0x014C:
            case 0x014E:
            case 0x0150:
            case 0x01D1:
            case 0x01EA:
            case 0x01EC:
            case 0x020C:
            case 0x020E:
                result += "O";
                break;

            case 0x00F2:
            case 0x00F3:
            case 0x00F4:
            case 0x00F5:
            case 0x00F6:
            case 0x00F8:
            case 0x014D:
            case 0x014F:
            case 0x0151:
            case 0x01D2:
            case 0x01EB:
            case 0x01ED:
            case 0x020D:
            case 0x020F:
                result += "o";
                break;


            // ------------------------------------------------
            // OE
            // ------------------------------------------------

            case 0x0152:
                result += "OE";
                break;

            case 0x0153:
                result += "oe";
                break;


            // ------------------------------------------------
            // R
            // ------------------------------------------------

            case 0x0154:
            case 0x0156:
            case 0x0158:
                result += "R";
                break;

            case 0x0155:
            case 0x0157:
            case 0x0159:
                result += "r";
                break;


            // ------------------------------------------------
            // S
            // ------------------------------------------------

            case 0x015A:
            case 0x015C:
            case 0x015E:
            case 0x0160:
                result += "S";
                break;

            case 0x015B:
            case 0x015D:
            case 0x015F:
            case 0x0161:
                result += "s";
                break;


            // ------------------------------------------------
            // T
            // ------------------------------------------------

            case 0x0162:
            case 0x0164:
            case 0x0166:
                result += "T";
                break;

            case 0x0163:
            case 0x0165:
            case 0x0167:
                result += "t";
                break;


            // ------------------------------------------------
            // U
            // ------------------------------------------------

            case 0x00D9:
            case 0x00DA:
            case 0x00DB:
            case 0x00DC:
            case 0x0168:
            case 0x016A:
            case 0x016C:
            case 0x016E:
            case 0x0170:
            case 0x0172:
            case 0x0214:
            case 0x0216:
                result += "U";
                break;

            case 0x00F9:
            case 0x00FA:
            case 0x00FB:
            case 0x00FC:
            case 0x0169:
            case 0x016B:
            case 0x016D:
            case 0x016F:
            case 0x0171:
            case 0x0173:
            case 0x0215:
            case 0x0217:
                result += "u";
                break;


            // ------------------------------------------------
            // W
            // ------------------------------------------------

            case 0x0174:
                result += "W";
                break;

            case 0x0175:
                result += "w";
                break;


            // ------------------------------------------------
            // Y
            // ------------------------------------------------

            case 0x00DD:
            case 0x0176:
            case 0x0178:
            case 0x0232:
                result += "Y";
                break;

            case 0x00FD:
            case 0x00FF:
            case 0x0177:
            case 0x0233:
                result += "y";
                break;


            // ------------------------------------------------
            // Z
            // ------------------------------------------------

            case 0x0179:
            case 0x017B:
            case 0x017D:
                result += "Z";
                break;

            case 0x017A:
            case 0x017C:
            case 0x017E:
                result += "z";
                break;


            // ------------------------------------------------
            // German sharp S
            // ------------------------------------------------

            case 0x00DF:
                result += "ss";
                break;


            // ------------------------------------------------
            // Thorn
            // ------------------------------------------------

            case 0x00DE:
                result += "TH";
                break;

            case 0x00FE:
                result += "th";
                break;


            // ------------------------------------------------
            // Unknown character
            // ------------------------------------------------

            default:
                // Skip unsupported character
                break;
        }
    }

    return result;
}


// ------------------------------------------------------------
// Malayalam detection
// ------------------------------------------------------------

bool containsMalayalam(String s)
{
    for (
        int i = 0;
        i < s.length();
        i++
    )
    {
        if ((uint8_t)s[i] >= 128)
        {
            return true;
        }
    }

    return false;
}


// ------------------------------------------------------------
// WAVBEE boot splash
// ------------------------------------------------------------

void drawBootScreen()
{
    tft.fillScreen(
        TFT_BLACK
    );

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    tft.setTextDatum(
        MC_DATUM
    );

    tft.drawString(
        "WAVBEE",
        160,
        110,
        4
    );

    tft.drawString(
        "NOW PLAYING",
        160,
        145,
        2
    );

    delay(2000);

    tft.fillScreen(
        TFT_BLACK
    );

    tft.setTextDatum(
        TL_DATUM
    );
}


// ------------------------------------------------------------
// Format time
// ------------------------------------------------------------

String formatTime(
    unsigned long milliseconds
)
{
    unsigned long totalSeconds =
        milliseconds / 1000;

    unsigned long minutes =
        totalSeconds / 60;

    unsigned long seconds =
        totalSeconds % 60;

    String result =
        String(minutes);

    result += ":";

    if (seconds < 10)
    {
        result += "0";
    }

    result += String(seconds);

    return result;
}


// ------------------------------------------------------------
// Get current NTP time
// ------------------------------------------------------------

String getCurrentTime()
{
    struct tm timeinfo;

    if (!getLocalTime(
            &timeinfo,
            100
        ))
    {
        return "--:--";
    }

    char timeBuffer[10];

    strftime(
        timeBuffer,
        sizeof(timeBuffer),
        "%H:%M",
        &timeinfo
    );

    return String(
        timeBuffer
    );
}


// ------------------------------------------------------------
// Draw live indicator
// ------------------------------------------------------------

void drawLiveDot()
{
    if (liveDotState)
    {
        tft.fillCircle(
            305,
            10,
            4,
            TFT_WHITE
        );
    }
    else
    {
        tft.fillCircle(
            305,
            10,
            4,
            TFT_BLACK
        );
    }
}


// ------------------------------------------------------------
// Draw Now Playing
// ------------------------------------------------------------

void drawNowPlaying(
    String title,
    String artist,
    String album,
    int progressMs,
    int durationMs,
    bool isPlaying
)
{
    tft.fillScreen(
        TFT_BLACK
    );


    // --------------------------------------------------------
    // Sync local progress
    // --------------------------------------------------------

    displayProgressMs =
        progressMs;

    displayDurationMs =
        durationMs;

    displayIsPlaying =
        isPlaying;

    lastProgressTime =
        millis();


    // --------------------------------------------------------
    // Artwork placeholder
    // --------------------------------------------------------

    tft.fillRect(
        10,
        10,
        150,
        150,
        TFT_DARKGREY
    );


    // --------------------------------------------------------
    // Live dot
    // --------------------------------------------------------

    drawLiveDot();


    // --------------------------------------------------------
    // TITLE
    // --------------------------------------------------------

    currentTitle =
        title;

    scrollX = 0;

    lastScrollTime =
        millis();

    titleUsesMalayalamFont =
        containsMalayalam(
            currentTitle
        );


    // --------------------------------------------------------
    // Malayalam title
    // --------------------------------------------------------

    if (titleUsesMalayalamFont)
    {
        tft.loadFont(
            NotoSansMalayalamRegular20
        );

        tft.setTextColor(
            TFT_WHITE,
            TFT_BLACK
        );

        tft.setTextDatum(
            TL_DATUM
        );

        int titleWidth =
            tft.textWidth(
                currentTitle
            );

        titleNeedsScroll =
            (titleWidth > 150);

        if (!titleNeedsScroll)
        {
            tft.drawString(
                currentTitle,
                170,
                20
            );
        }

        tft.unloadFont();
    }


    // --------------------------------------------------------
    // English title
    // --------------------------------------------------------

    else
    {
        tft.setTextColor(
            TFT_WHITE,
            TFT_BLACK
        );

        tft.setTextDatum(
            TL_DATUM
        );

        int titleWidth =
            tft.textWidth(
                currentTitle,
                2
            );

        titleNeedsScroll =
            (titleWidth > 150);

        if (!titleNeedsScroll)
        {
            tft.drawString(
                currentTitle,
                170,
                20,
                2
            );
        }
    }


    // --------------------------------------------------------
    // Artist
    // --------------------------------------------------------

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    tft.setTextDatum(
        TL_DATUM
    );

    String displayArtist =
        fitText(
            normalizeLatin(artist),
            150,
            2
        );

    tft.drawString(
        displayArtist,
        170,
        80,
        2
    );


    // --------------------------------------------------------
    // Album
    // --------------------------------------------------------

    String displayAlbum =
        fitText(
            normalizeLatin(album),
            150,
            2
        );

    tft.drawString(
        displayAlbum,
        170,
        120,
        2
    );


    // --------------------------------------------------------
    // Paused
    // --------------------------------------------------------

    if (!isPlaying)
    {
        tft.drawString(
            "PAUSED",
            170,
            150,
            2
        );
    }


    // --------------------------------------------------------
    // Progress bar
    // --------------------------------------------------------

    tft.drawRect(
        10,
        210,
        300,
        10,
        TFT_WHITE
    );


    if (durationMs > 0)
    {
        int filledWidth =
            (int)(
                ((uint64_t)progressMs * 298)
                / durationMs
            );

        if (filledWidth > 298)
            filledWidth = 298;

        if (filledWidth < 0)
            filledWidth = 0;

        if (filledWidth > 0)
        {
            tft.fillRect(
                11,
                211,
                filledWidth,
                8,
                TFT_WHITE
            );
        }
    }


    // --------------------------------------------------------
    // Initial time
    // --------------------------------------------------------

    tft.setTextDatum(
        MC_DATUM
    );

    String timeText =
        formatTime(
            progressMs
        );

    timeText += " / ";

    timeText +=
        formatTime(
            durationMs
        );

    tft.drawString(
        timeText,
        160,
        235,
        2
    );

    tft.setTextDatum(
        TL_DATUM
    );
}


// ------------------------------------------------------------
// Idle screen
// ------------------------------------------------------------

void drawIdleScreen()
{
    tft.fillScreen(
        TFT_BLACK
    );


    currentTitle = "";

    titleNeedsScroll = false;

    titleUsesMalayalamFont = false;

    scrollX = 0;


    tft.unloadFont();


    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );


    // --------------------------------------------------------
    // WAVBEE wordmark
    // --------------------------------------------------------

    tft.setTextDatum(
        MC_DATUM
    );

    tft.drawString(
        "WAVBEE",
        160,
        80,
        4
    );


    // --------------------------------------------------------
    // Nothing Playing
    // --------------------------------------------------------

    tft.drawString(
        "Nothing Playing",
        160,
        125,
        2
    );


    // --------------------------------------------------------
    // Current time
    // --------------------------------------------------------

    tft.drawString(
        getCurrentTime(),
        160,
        165,
        4
    );


    // --------------------------------------------------------
    // Live dot
    // --------------------------------------------------------

    drawLiveDot();


    tft.setTextDatum(
        TL_DATUM
    );
}


// ------------------------------------------------------------
// Update display
// ------------------------------------------------------------

void updateDisplay()
{
    // ==========================================
    // SMOOTH LOCAL PROGRESS
    // ==========================================

    static int lastFilledWidth = -1;

    static unsigned long lastDisplayedSecond =
        999999;


    if (displayDurationMs == 0)
    {
        return;
    }


    unsigned long currentProgress =
        displayProgressMs;


    if (displayIsPlaying)
    {
        unsigned long elapsed =
            millis() - lastProgressTime;

        currentProgress +=
            elapsed;
    }


    if (currentProgress >
        displayDurationMs)
    {
        currentProgress =
            displayDurationMs;
    }


    // ==========================================
    // PROGRESS BAR
    // ==========================================

    int filledWidth = 0;


    if (displayDurationMs > 0)
    {
        filledWidth =
            (int)(
                ((uint64_t)currentProgress * 298)
                / displayDurationMs
            );

        if (filledWidth > 298)
        {
            filledWidth = 298;
        }

        if (filledWidth < 0)
        {
            filledWidth = 0;
        }
    }


    if (filledWidth !=
        lastFilledWidth)
    {
        tft.fillRect(
            11,
            211,
            298,
            8,
            TFT_BLACK
        );


        if (filledWidth > 0)
        {
            tft.fillRect(
                11,
                211,
                filledWidth,
                8,
                TFT_WHITE
            );
        }


        lastFilledWidth =
            filledWidth;
    }


    // ==========================================
    // ELAPSED / TOTAL TIME
    // ==========================================

    unsigned long currentSecond =
        currentProgress / 1000;


    if (currentSecond !=
        lastDisplayedSecond)
    {
        lastDisplayedSecond =
            currentSecond;


        tft.fillRect(
            80,
            222,
            160,
            25,
            TFT_BLACK
        );


        tft.setTextDatum(
            MC_DATUM
        );


        String timeText =
            formatTime(
                currentProgress
            );

        timeText += " / ";

        timeText +=
            formatTime(
                displayDurationMs
            );


        tft.drawString(
            timeText,
            160,
            235,
            2
        );


        tft.setTextDatum(
            TL_DATUM
        );
    }


    // ==========================================
    // TITLE SCROLLING
    // ==========================================

    if (!titleNeedsScroll)
    {
        return;
    }


    // ==========================================
    // MALAYALAM SCROLLING
    // ==========================================

    if (titleUsesMalayalamFont)
    {
        spr.createSprite(
            150,
            50
        );

        spr.fillSprite(
            TFT_BLACK
        );

        spr.loadFont(
            NotoSansMalayalamRegular20
        );

        spr.setTextColor(
            TFT_WHITE,
            TFT_BLACK
        );

        spr.setTextDatum(
            TL_DATUM
        );

        spr.drawString(
            currentTitle,
            scrollX,
            8
        );

        spr.pushSprite(
            170,
            20
        );


        int titleWidth =
            spr.textWidth(
                currentTitle
            );


        spr.unloadFont();

        spr.deleteSprite();


        if (
            millis() -
            lastScrollTime >= 50
        )
        {
            lastScrollTime =
                millis();

            scrollX--;
        }


        if (
            scrollX <
            -titleWidth
        )
        {
            scrollX = 150;
        }
    }


    // ==========================================
    // ENGLISH SCROLLING
    // ==========================================

    else
    {
        spr.createSprite(
            150,
            40
        );

        spr.fillSprite(
            TFT_BLACK
        );

        spr.setTextColor(
            TFT_WHITE,
            TFT_BLACK
        );

        spr.setTextDatum(
            TL_DATUM
        );

        spr.drawString(
            currentTitle,
            scrollX,
            5,
            2
        );

        spr.pushSprite(
            170,
            20
        );


        spr.deleteSprite();


        int titleWidth =
            tft.textWidth(
                currentTitle,
                2
            );


        if (
            millis() -
            lastScrollTime >= 50
        )
        {
            lastScrollTime =
                millis();

            scrollX--;
        }


        if (
            scrollX <
            -titleWidth
        )
        {
            scrollX = 150;
        }
    }
}