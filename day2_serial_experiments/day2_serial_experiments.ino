void setup() {
  Serial.begin(115200);
  Serial.println("wavbee now-playing display, boot");
}

void loop() {
  int songNumber = 10;

  if (songNumber == 10) {
    Serial.println("Song number is 10");
  }

  delay(2000);
}