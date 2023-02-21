#include "Tank.h"
#include "MyRect.h"
#include <QKeyEvent>
#include "Bullet.h"
#include <QGraphicsScene>
#include <Enemy.h>

Tank::Tank(QGraphicsView* view, QGraphicsItem* parent) : QGraphicsPixmapItem(parent)
{
    v = view;

    turret = new QGraphicsPixmapItem();
    fireFlash = new QGraphicsPixmapItem();

    //Sets up Key Map
    keys.insert(Qt::Key_W, false);
    keys.insert(Qt::Key_A, false);
    keys.insert(Qt::Key_S, false);
    keys.insert(Qt::Key_D, false);
    keys.insert(Qt::Key_Space, false);

    setPixmap(QPixmap(":/images/greenChasis.png"));
    setTransformOriginPoint(boundingRect().width() / 2, boundingRect().height() / 2);

    distance = 2;
    direction = 'w';
    counter = 0;
    changeTreads = false;

    bulletHandler = new QMediaPlayer();
    movingHandler = new QMediaPlayer();
    idleHandler = new QMediaPlayer();
    bulletAudioPlayer = new QAudioOutput();
    movingAudioPlayer = new QAudioOutput();
    idleAudioPlayer = new QAudioOutput();

    fireRateTimer = new QTimer();
    fireRateTimer->setInterval(2000);
    fireRateTimer->setSingleShot(true);

    keyTimer = new QTimer();

    bulletAudioPlayer->setVolume(.3);
    movingAudioPlayer->setVolume(.1);
    idleAudioPlayer->setVolume(.1);

    bulletHandler->setAudioOutput(bulletAudioPlayer);
    bulletHandler->setSource(QUrl("qrc:/sounds/bulletFireThree.wav"));

    movingHandler->setAudioOutput(movingAudioPlayer);
    movingHandler->setSource(QUrl("qrc:/sounds/engineMovingThree.wav"));

    idleHandler->setAudioOutput(idleAudioPlayer);
    idleHandler->setSource(QUrl("qrc:/sounds/engineIdleThree.wav"));

    idleHandler->setLoops(QMediaPlayer::Infinite);
    idleHandler->play();

    connect(keyTimer, SIGNAL(timeout()), this, SLOT(frame()));
    keyTimer->start(7);

    this->setZValue(-3);

}


// Creates turret and sets its point of rotation.
void Tank::createTurret() {
    int rotationPoint = 7; // 7 pixels down the turret is where the hatch on the turret is which is where the rotation point needs to be.
    turret->setPos(x() + this->boundingRect().width() / 2 - turret->boundingRect().width() / 2, y() + this->boundingRect().height() / 2 - turret->boundingRect().height() / 2 - 10);
    turret->setPixmap(QPixmap(":/images/greenTurret.png"));
    turret->setTransformOriginPoint(turret->boundingRect().width() / 2, turret->boundingRect().height() / 2 + rotationPoint);
    scene()->addItem(turret);

    fireFlash->setPixmap(QPixmap(":/images/gunFlash.png"));
    fireFlash->setVisible(false);
    fireFlash->setZValue(-2);
    scene()->addItem(fireFlash);
}

void Tank::keyPressEvent(QKeyEvent* event)
{
    keys[event->key()] = true;
}

void Tank::keyReleaseEvent(QKeyEvent* event) {
    keys[event->key()] = false;
}

void Tank::focusOutEvent(QFocusEvent* event)
{
    this->setFocus();
}

void Tank::frame() {

    // this code is what lets the tank follow the cursor. Every time the frame() function is called (about 144 times per second). 
    // The function declared variables below will be deleted when the function exits so I do not believe they will cause memory issues
    turret->setPos(x() + this->boundingRect().width() / 2 - turret->boundingRect().width() / 2, y() + this->boundingRect().height() / 2 - turret->boundingRect().height() / 2 - 7);
    QPoint cursorPos = QCursor::pos();
    QPointF cursorScenePos = scene()->views().first()->mapFromGlobal(cursorPos);

    float angle = (atan2(cursorScenePos.y() - (y() + (this->boundingRect().height() / 2)), cursorScenePos.x() - (x() + (this->boundingRect().width()) / 2)));
    float angleDegrees = angle * (180 / M_PI);

    turret->setRotation(angleDegrees + 90);

    // Calculate x and y velocity for tank gun muzzle flash
    int dx = 50 * cos(angle);
    int dy = 50 * sin(angle);

    fireFlash->setPos(x() + this->boundingRect().width() / 2 - fireFlash->boundingRect().width()/2, y() + this->boundingRect().height() / 2 - fireFlash->boundingRect().height() / 2);
    fireFlash->moveBy(dx, dy);
    fireFlash->setTransformOriginPoint(fireFlash->boundingRect().width() / 2, fireFlash->boundingRect().height() / 2);
    fireFlash->setRotation(angleDegrees + 90);


    //Makes view camera follow the tank
    //v->centerOn(this);

    // Checking to see how the moving sound should be handled
    if ((isMoving() == true) && (movingHandler->playbackState() != QMediaPlayer::PlayingState)) {
        movingHandler->setPosition(0);
        movingHandler->play();
    }
    else if ((isMoving() == false) && (movingHandler->playbackState() == QMediaPlayer::PlayingState)) {
        movingHandler->setPosition(0);
        movingHandler->stop();
    }

    // Code that handles the animation for the tank treads. Everytime 20 times the move() function is called while the tank is actually moving,
    if ((isMoving() == true)) {
        counter++;
        if (counter % 20 == 0) {
            if (changeTreads == true) {
                setPixmap(QPixmap(":/images/greenChasis2.png"));
                changeTreads = false;
            }
            else {
                setPixmap(QPixmap(":/images/greenChasis.png"));
                changeTreads = true;
            }
        }
    }

    //Movement
    if (keys[Qt::Key_W]) {
        if (pos().y() > 0) {
            setPos(x(), y() - distance);
            setRotation(0); // facing upwards
            direction = 'w';
        }
    }

    else if (keys[Qt::Key_A]) {
        if (pos().x() > 0) {
            setPos(x() - distance, y());
            setRotation(270); // facing left
            direction = 'a';
        }
    }

    else if (keys[Qt::Key_S]) {
        if (pos().y() + distance < scene()->height() - boundingRect().height()) {
            setPos(x(), y() + distance);
            setRotation(180); // facing downwards
            direction = 's';
        }
    }

    else if (keys[Qt::Key_D]) {
        if (pos().x() + distance < scene()->width() - boundingRect().width()) {
            setPos(x() + distance, y());
            setRotation(90); // facing right
            direction = 'd';
        }
    }

    // Shooting (Spacebar)
    if (keys[Qt::Key_Space]) {
        if (!fireRateTimer->isActive()) {
            fireFlash->setVisible(true);

            Bullet* bullet = new Bullet(direction, angle);
            bullet->setPos(x() + this->boundingRect().width() / 2 - bullet->boundingRect().width() / 2, y() + this->boundingRect().height() / 2 - bullet->boundingRect().height() / 2);
            bullet->setZValue(-1);
            scene()->addItem(bullet);

            //This can be set to either 'fire()' for cursor shooting or 'fireAlt()' for directional shooting
            bullet->fireSwivel();

            //Fire Bullet sound
            bulletHandler->setPosition(0);
            bulletHandler->play();

            // Start the fireRateTimer
            fireRateTimer->start();
        }
    }

    if (fireRateTimer->isActive()) {

        if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() < .95) {
            fireFlash->setVisible(false);
        }

        if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() > .8) {
            QCursor cursor = QCursor(QPixmap(":/images/greenCrosshairGearZero.png"));
            v->setCursor(cursor);
        }
        else if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() > .6) {
            QCursor cursor = QCursor(QPixmap(":/images/greenCrosshairGearOne.png"));
            v->setCursor(cursor);
        }
        else if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() > .4) {
            QCursor cursor = QCursor(QPixmap(":/images/greenCrosshairGearTwo.png"));
            v->setCursor(cursor);
        }
        else if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() > .2) {
            QCursor cursor = QCursor(QPixmap(":/images/greenCrosshairGearThree.png"));
            v->setCursor(cursor);
        }
        else {
            QCursor cursor = QCursor(QPixmap(":/images/greenCrosshairGearFour.png"));
            v->setCursor(cursor);
        }

    }
    else {
        QCursor cursor = QCursor(QPixmap(":/images/crosshair.png"));
        v->setCursor(cursor);
    }
}

bool Tank::isMoving() {
    if (keys[Qt::Key_W] || keys[Qt::Key_A] || keys[Qt::Key_S] || keys[Qt::Key_D]) {
        return true;
    }
    else {
        return false;
    }
}

void Tank::spawn() {
    // create an enemy
    Enemy* enemy = new Enemy();
    scene()->addItem(enemy);
}

