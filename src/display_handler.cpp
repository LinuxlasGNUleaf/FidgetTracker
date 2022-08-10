#include <display_handler.h>

DisplayHandler::DisplayHandler() {
  this->display =
      new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
}

void DisplayHandler::drawStr(int x, int y, const char *str) {
  this->display->drawStr(x, y, str);
}

void DisplayHandler::drawLine(int x1, int y1, int x2, int y2) {
  this->display->drawLine(x1, y1, x2, y2);
}

void DisplayHandler::drawGauge(int x1, int y1, int x2, int y2, double percent) {
  this->display->clearBuffer();
  int mid_x = (x2 + x1) / 2;
  int mid_y = y2;
  int height_y = (y2 - y1);
  int height_x = mid_x - x1;

  this->display->drawEllipse(mid_x, mid_y - 1, height_x, height_y - 2,
                             U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);

  percent = constrain(percent, 0.04f, 0.96f);
  float angle = radians(percent * 180);
  int x_end = mid_x - round(height_x * 0.8 * cos(angle));
  int y_end = mid_y - round(height_y * 0.8 * sin(angle));

  this->display->drawLine(mid_x, mid_y, x_end, y_end);
  this->display->sendBuffer();
}

void DisplayHandler::init() { this->display->begin(); }