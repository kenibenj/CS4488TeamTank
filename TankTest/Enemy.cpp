#include "Enemy.h"
#include <QTimer>
#include <QGraphicsScene>
#include <QList>
#include <stdlib.h> // rand() -> really large int

#include <QDebug>
Enemy::Enemy() : QObject(), QGraphicsRectItem() {
    //set random position
    int random_number = rand() % 700;
    setPos(random_number, 0);

    // drew the rect
    setRect(0, 0, 100, 100);

    // connect
    QTimer* enemytimer = new QTimer(this);
    connect(enemytimer, SIGNAL(timeout()), this, SLOT(move()));

    enemytimer->start(7);
}


void Enemy::move() {
    // move enemy down
    setPos(x(), y() + 5);
    if (pos().y() + rect().height() < 0) {
        scene()->removeItem(this);
        delete this;
    }
}
