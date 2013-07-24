WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwtwocolorindicatoriface.cpp
  HEADERS += $$WD/plugin/qwwtwocolorindicatoriface.h
} else {
  SOURCES += $$WD/widgets/qwwtwocolorindicator/qwwtwocolorindicator.cpp
  HEADERS += $$WD/widgets/qwwtwocolorindicator/qwwtwocolorindicator.h
}
INCLUDEPATH += $$WD/widgets/qwwtwocolorindicator
INSTALL_HEADERS += $$WD/widgets/qwwtwocolorindicator/qwwtwocolorindicator.h $$WD/widgets/qwwtwocolorindicator/QwwTwoColorIndicator