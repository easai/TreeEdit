#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

#define AUTHOR "easai"
#define APPNAME "TreeEdit"
#define GENERAL "General"
#define GEOM "Geometry"
#define FILENAME "File Name"

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

signals:

private:
    QString m_fileName;
    QByteArray m_geom;

};

#endif // CONFIG_H
