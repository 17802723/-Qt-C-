#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QObject>
#include <QCamera>

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    explicit VideoHandler(QObject *parent = nullptr);
    ~VideoHandler();

signals:
};

#endif // VIDEOHANDLER_H
