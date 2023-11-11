#include "config.h"
#include <QSettings>

Config::Config(QObject *parent) : QObject{parent} {}

Config::Config(const Config &o) : m_fileName(o.m_fileName), m_geom(o.m_geom) {}

Config &Config::operator=(const Config &o) {
  if (this != &o) {
    m_fileName = o.m_fileName;
    m_geom = o.m_geom;
  }
  return *this;
}

void Config::load() {
  QSettings settings(AUTHOR, APPNAME);
  settings.beginGroup(GENERAL);
  m_fileName = settings.value(FILENAME).toString();
  m_geom = settings.value(GEOM).toByteArray();
  settings.endGroup();
}

void Config::save() {
  QSettings settings(AUTHOR, APPNAME);
  settings.beginGroup(GENERAL);
  settings.setValue(FILENAME, m_fileName);
  settings.setValue(GEOM, m_geom);
  settings.endGroup();
}

QString Config::fileName() const { return m_fileName; }

void Config::setFileName(const QString &newFileName) {
  m_fileName = newFileName;
}

QByteArray Config::geom() const { return m_geom; }

void Config::setGeom(const QByteArray &newGeom) { m_geom = newGeom; }
