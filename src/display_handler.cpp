#include <display_handler.h>

#define GAUGE_MARGIN 2
#define NEEDLE_LENGTH 0.8f

#define BOLT_MARGIN 5
#define BOLT_RADIUS 3

#define BIG_FONT u8g2_font_10x20_tf
#define MED_FONT u8g2_font_t0_17_tf
#define SMALL_FONT u8g2_font_t0_11_tf

DisplayHandler::DisplayHandler() {
  this->display =
      new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
}

void DisplayHandler::drawBigStr(int x, int y, const char *str) {
  this->display->setFont(BIG_FONT);
  this->display->drawStr(x, y, str);
}

void DisplayHandler::drawMedStr(int x, int y, const char *str) {
  this->display->setFont(MED_FONT);
  this->display->drawStr(x, y, str);
}

void DisplayHandler::drawSmallStr(int x, int y, const char *str) {
  this->display->setFont(SMALL_FONT);
  this->display->drawStr(x, y, str);
}

void DisplayHandler::drawLine(int x1, int y1, int x2, int y2) {
  this->display->drawLine(x1, y1, x2, y2);
}

void DisplayHandler::drawGauge(int x1, int y1, int x2, int y2, double percent) {
  int pivot_x = (x2 + x1) / 2;
  int pivot_y = y2 - GAUGE_MARGIN;

  int rad_x = pivot_x - x1 - GAUGE_MARGIN;
  int rad_y = (y2 - y1) - GAUGE_MARGIN;

  this->display->drawEllipse(pivot_x, pivot_y, rad_x, rad_y,
                             U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  this->display->drawEllipse(pivot_x, pivot_y, rad_x + GAUGE_MARGIN,
                             rad_y + GAUGE_MARGIN,
                             U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);

  // connecting lines from ellipses to bottom line
  this->display->drawLine(pivot_x - rad_x - GAUGE_MARGIN, pivot_y,
                          pivot_x - rad_x - GAUGE_MARGIN,
                          pivot_y + GAUGE_MARGIN);
  this->display->drawLine(pivot_x + rad_x + GAUGE_MARGIN, pivot_y,
                          pivot_x + rad_x + GAUGE_MARGIN,
                          pivot_y + GAUGE_MARGIN);
  this->display->drawLine(pivot_x - rad_x, pivot_y, pivot_x - rad_x,
                          pivot_y + GAUGE_MARGIN);
  this->display->drawLine(pivot_x + rad_x, pivot_y, pivot_x + rad_x,
                          pivot_y + GAUGE_MARGIN);
  // bottom line
  this->display->drawLine(
      pivot_x - rad_x - GAUGE_MARGIN, pivot_y + GAUGE_MARGIN,
      pivot_x + rad_x + GAUGE_MARGIN, pivot_y + GAUGE_MARGIN);

  // drawing the needle
  percent = constrain(percent, 0.04f, 0.96f);
  float angle = radians(percent * 180);
  int x_end = pivot_x - round(rad_x * cos(angle) * NEEDLE_LENGTH);
  int y_end = pivot_y - round(rad_y * sin(angle) * NEEDLE_LENGTH);

  this->display->drawLine(pivot_x, pivot_y, x_end, y_end);
  this->display->drawDisc(pivot_x, pivot_y, 2);
}

void DisplayHandler::drawPlate(int x1, int y1, int x2, int y2, int r,
                               bool bolts) {
  this->display->drawRFrame(x1, y1, x2 - x1 + 1, y2 - y1 + 1, r);

  this->display->drawDisc(x1 + BOLT_MARGIN, y1 + BOLT_MARGIN, BOLT_RADIUS);
  this->display->drawDisc(x1 + BOLT_MARGIN, y2 - BOLT_MARGIN, BOLT_RADIUS);
  this->display->drawDisc(x2 - BOLT_MARGIN, y1 + BOLT_MARGIN, BOLT_RADIUS);
  this->display->drawDisc(x2 - BOLT_MARGIN, y2 - BOLT_MARGIN, BOLT_RADIUS);
}

void DisplayHandler::init() { this->display->begin(); }

void DisplayHandler::clearBuffer() { this->display->clearBuffer(); }

void DisplayHandler::sendBuffer() { this->display->sendBuffer(); }