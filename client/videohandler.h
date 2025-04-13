#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QObject>

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    explicit VideoHandler(QObject *parent = nullptr);

signals:
};

#endif // VIDEOHANDLER_H
