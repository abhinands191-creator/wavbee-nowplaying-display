#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite spr = TFT_eSprite(&tft);

int scrollX = 150;
String currentTitle = "";

unsigned long lastScrollTime = 0;
unsigned long lastProgressTime = 0;

int progress = 0;
int duration = 240;
int barWidth = 300;

void setup() {

  tft.init();

  tft.setRotation(1);

  tft.invertDisplay(true);

  drawBootScreen();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.setTextDatum(TL_DATUM);

  drawNowPlaying(
    "Pavizha Mazhaye",
    "K. S. Harisankar",
    "Athiran",
    120000,
    240000
  );
}

void loop() {

  // Create the sprite
  spr.createSprite(150, 40);

  // Clear the sprite
  spr.fillSprite(TFT_BLACK);

  // Set text
  spr.setTextColor(TFT_WHITE, TFT_BLACK);

  spr.setTextDatum(TL_DATUM);

  // Draw current title inside the sprite
  spr.drawString(currentTitle, scrollX, 5, 2);

  // Push the completed sprite to the TFT
  spr.pushSprite(170, 20);

  // Free the sprite memory
  spr.deleteSprite();

  // Smooth scrolling
  if (millis() - lastScrollTime >= 50) {

    lastScrollTime = millis();

    scrollX--;
  }

  // Restart scrolling
  if (scrollX < -150) {

    scrollX = 150;
  }

  // Update fake progress every second
  if (millis() - lastProgressTime >= 1000) {

    lastProgressTime = millis();

    progress++;

    if (progress >= duration) {

      progress = duration;
    }

    // Redraw the progress bar
    tft.fillRect(10, 210, 300, 10, TFT_BLACK);

    tft.drawRect(10, 210, 300, 10, TFT_WHITE);

    int filledWidth = (progress * barWidth) / duration;

    tft.fillRect(10, 210, filledWidth, 10, TFT_WHITE);
  }
}

void drawBootScreen() {

  tft.fillScreen(TFT_BLACK);
}

void drawNowPlaying(String title, String artist, String album, int progressMs, int durationMs) {

  tft.fillRect(10, 10, 150, 150, TFT_DARKGREY);

  currentTitle = title;

  tft.drawString(artist, 170, 80, 2);

  tft.drawString(album, 170, 120, 2);

  tft.drawRect(10, 210, 300, 10, TFT_WHITE);

  int filledWidth = (progressMs * barWidth) / durationMs;

  tft.fillRect(10, 210, filledWidth, 10, TFT_WHITE);
}

void drawIdleScreen() {

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);

  tft.drawString("Nothing Playing", 160, 120, 2);
}