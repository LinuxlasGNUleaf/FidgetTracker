#include <U8g2lib.h>
#include <Wire.h>

class DisplayHandler {
private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C *display;

public:
  DisplayHandler();
  void init();
  void drawStr(int x, int y, const char *str);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawGauge(int x1, int y1, int x2, int y2, double percent);
};
