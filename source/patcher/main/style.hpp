#pragma once
#include <QString>

namespace Style {
    QString getStyleSheet();
    
    QString getStyleSheet(bool darkMode);
    
    QString getLightStyleSheet();
    
    QString getDarkStyleSheet();
}
