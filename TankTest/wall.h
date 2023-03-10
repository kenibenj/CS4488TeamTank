#pragma once


#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QBrush>
#include <QImage>
#include <QFile>
#include <QPixmap>
#include <QLabel>
#include <QMessageBox>


class Wall : public QObject, public QGraphicsItem
{
public:
    explicit Wall(QObject* parent = 0);
    ~Wall();
    //bool is_box = 0;

protected:
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
};

