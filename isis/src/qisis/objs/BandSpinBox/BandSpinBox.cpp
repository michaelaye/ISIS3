#include <iostream>

#include "BandSpinBox.h"
#include "iException.h"

namespace Qisis {
  /**
   * BandSpinBox constructor
   *
   *
   * @param parent
   */
  BandSpinBox::BandSpinBox(QWidget *parent) : QSpinBox(parent) {
    QStringList list;
    list.push_back(QString::number(1));
    p_lastKey = "BandNumber";
    p_map[p_lastKey] = list;
    p_bands = 1;

    setValue(1);
    setMinimum(1);
    setMaximum(p_bands);
  }


  /**
   * Sets the band bin
   *
   *
   * @param pvl
   * @param key
   */
  void BandSpinBox::setBandBin(Isis::Pvl &pvl, const QString &key) {
    // Remove all connections and clear the keyword map
    disconnect(this, 0, 0, 0);
    p_map.clear();

    // Get the number of bands and setup the spin box
    Isis::PvlGroup &dim = pvl.FindObject("IsisCube")
                          .FindObject("Core")
                          .FindGroup("Dimensions");
    p_bands = dim["Bands"];

    // Put in the default BandNumber list
    QStringList list;
    for(int i = 1; i <= p_bands; i++) {
      list.push_back(QString::number(i));
    }
    p_map["BandNumber"] = list;

    // Add any other lists
    if(pvl.FindObject("IsisCube").HasGroup("BandBin")) {
      Isis::PvlGroup &bandBin = pvl.FindObject("IsisCube")
                                .FindGroup("BandBin");
      for(int i = 0; i < bandBin.Keywords(); i++) {
        list.clear();
        if(bandBin[i].Size() == p_bands) {
          for(int j = 0; j < bandBin[i].Size(); j++) {
            list.push_back(QString(bandBin[i][j].c_str()));
          }
          std::string bandBinName = bandBin[i].Name();
          p_map[bandBinName.c_str()] = list;
        }
      }
    }

    setKey(key);
    p_keys = p_map.keys();

    setValue(1);
    setMinimum(1);
    setMaximum(p_bands);
    updateGeometry();
  }


  /**
   * returns the list of keys.
   *
   * @return QStringList
   */
  QStringList BandSpinBox::BandBinKeys() {
    return p_keys;
  }


  /**
   * Sets the key to the provided key.
   *
   *
   * @param key
   */
  void BandSpinBox::setKey(QString key) {
    if(p_map.contains(key)) {
      if(key != p_lastKey) {
        p_lastKey = key;
        setSuffix("a");  // This makes the spinbox update because
        setSuffix("");   // the value isn't changing
        repaint();
        updateGeometry();
      }
    }
    else {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Invalid key", _FILEINFO_);
    }
  }


  /**
   * Sets the key to the provided key.
   *
   *
   * @param key
   */
  void BandSpinBox::setKey(int key) {
    if((key < 0) || (key >= (int) p_map.size())) {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Invalid key", _FILEINFO_);
    }
    else {
      setKey(p_keys[key]);
    }
  }


  /**
   * Gets the text using p_map.
   *
   *
   * @param val
   *
   * @return QString
   */
  QString BandSpinBox::textFromValue(int val) const {
    if((val < 1) || (val > p_bands)) {
      std::cout << "BandSpinBox:  Bad index in textFromValue" << std::endl;
      return QString("Error");
    }

    if(p_map.contains(p_lastKey)) {
      return p_map[p_lastKey][val-1];
    }
    else {
      std::cout << "BandSpinBox:  Bad value for p_lastKey in textFromValue" << std::endl;
      return QString("Error");
    }
  }


  /**
   * gets the value (int) using p_map.
   *
   *
   * @param text
   *
   * @return int
   */
  int BandSpinBox::valueFromText(const QString &text) const {
    if(p_map.contains(p_lastKey)) {
      for(int i = 0; i < p_map[p_lastKey].size(); i++) {
        if(text == p_map[p_lastKey][i]) return i + 1;
      }
    }
    std::cout << "BandSpinBox:  Bad text in valueFromText" << std::endl;
    return -1;
  }


  /**
   * returns a size hint for the spin box
   *
   *
   * @return QSize
   */
  QSize BandSpinBox::sizeHint() const {
    QFontMetrics fm(font());

    int w = 0;
    for(int i = minimum(); i <= maximum(); i++) {
      w = qMax(w, fm.width(((BandSpinBox *) this)->textFromValue(i)));
    }

    QSize s = QSpinBox::sizeHint();
    int neww = s.width() + w;

    int minw = fm.width(((BandSpinBox *) this)->textFromValue(minimum()));
    int maxw = fm.width(((BandSpinBox *) this)->textFromValue(maximum()));

    if(minw < maxw) {
      neww -= maxw;
    }
    else {
      neww -= minw;
    }
    s.setWidth(neww + 5);
    return s;
  }


  /**
   * returns how valid the value from the spin box is.
   *
   *
   * @param input
   * @param pos
   *
   * @return QValidator::State
   */
  QValidator::State BandSpinBox::validate(QString &input, int &pos) const {
    int count = 0;
    int exact = false;
    for(int i = 0; i < p_map[p_lastKey].size(); i++) {
      if(p_map[p_lastKey][i].startsWith(input))count++;
      if(p_map[p_lastKey][i] == input) exact = true;
    }

    if(count == 0) return QValidator::Invalid;
    if(count > 0 && exact) return QValidator::Acceptable;
    return QValidator::Intermediate;
  }
}
