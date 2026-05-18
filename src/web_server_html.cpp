#include "web_server.h"
#include "translations.h"
#include "common_css.h"
#include <Arduino.h>

// Wrapper function to return translations JavaScript
String getTranslationsJs() {
  return String(TRANSLATIONS_JS);
}

// Wrapper function to return common CSS
String getCommonCss() {
  return String(COMMON_CSS);
}

