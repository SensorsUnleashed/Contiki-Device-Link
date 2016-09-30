#ifndef GUIGLUE_H
#define GUIGLUE_H

#include <QObject>

class guiglue : public QObject
{
    Q_OBJECT
public:
    explicit guiglue(QObject *parent = 0);

signals:

public slots:
};

#endif // GUIGLUE_H