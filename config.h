#ifndef CONFIG_H
#define CONFIG_H

#include "qfont.h"
#include <QObject>

#define AUTHOR "easai"
#define APPNAME "TreeEdit"
#define GENERAL "General"
#define GEOM "Geometry"
#define FILENAME "File Name"
#define FONT "Font"

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    Config(const Config &);
    Config &operator=(const Config &);

    void load();
    void save();

    QString fileName() const;
    void setFileName(const QString &newFileName);

    QByteArray geom() const;
    void setGeom(const QByteArray &newGeom);

    QFont font() const;
    void setFont(const QFont &newFont);

signals:

private:
    QString m_fileName;
    QByteArray m_geom;
    QFont m_font;
};

#endif // CONFIG_H
