#include "Tank.h"
#include <QKeyEvent>
#include "Bullet.h"
#include <QGraphicsScene>
#include "Enemy.h"
#include "GameRunner.h"
#include "PauseMenu.h"
#include <stdlib.h>
#include "Obstacles.h"

extern QTimer* enemyTimer;
Tank::Tank(QGraphicsView* view, QGraphicsItem* parent) : QGraphicsPixmapItem(parent)
{
    isPauseActive = false;
    v = view;
    this->setFocus();
    turret = new QGraphicsPixmapItem();
    fireFlash = new QGraphicsPixmapItem();
    health1 = new QGraphicsPixmapItem();
    health2 = new QGraphicsPixmapItem();
    health3 = new QGraphicsPixmapItem();
    bulletHUD = new QGraphicsPixmapItem();
    bulletCounterHUD = new QGraphicsTextItem();

    //Initialize bullet counter
    bulletCounter = 3;

    //Sets up Key Map
    keys.insert(Qt::Key_W, false);
    keys.insert(Qt::Key_A, false);
    keys.insert(Qt::Key_S, false);
    keys.insert(Qt::Key_D, false);
    keys.insert(Qt::Key_Space, false);
    keys.insert(Qt::Key_Escape, false);

    setPixmap(QPixmap(":/images/greenChasis.png"));
    setTransformOriginPoint(boundingRect().width() / 2, boundingRect().height() / 2);

    traversalSpeed = .3;
    rotationSpeed = .3;
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

    connect(enemyTimer, SIGNAL(timeout()), this, SLOT(frame()));
    //keyTimer->start(7);
    //createHUD();
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
void Tank::createHUD() {
    //Setting up health bar
    QPixmap health(":/images/gHealth.png");

    health1->setPixmap(health);
    health2->setPixmap(health);
    health3->setPixmap(health);

    scene()->addItem(health1);
    scene()->addItem(health2);
    scene()->addItem(health3);

    health1->setPos(v->mapToScene(10, 10));
    health2->setPos(v->mapToScene(70, 10));
    health3->setPos(v->mapToScene(130, 10));

    health1->setZValue(10);
    health2->setZValue(10);
    health3->setZValue(10);

    health1->show();
    health2->show();
    health3->show();

    //Creating bullet counter aspects of the HUD
    bulletHUD->setPixmap(QPixmap(":/images/bulletHUD.png"));
    scene()->addItem(bulletHUD);
    bulletHUD->setPos(v->mapToScene(10,60));
    bulletHUD->setZValue(10);

    QFont font("Times", 50);
    
    scene()->addItem(bulletCounterHUD);
    bulletCounterHUD->setDefaultTextColor(Qt::white);
    bulletCounterHUD->adjustSize();
    bulletCounterHUD->setFont(font);
    bulletCounterHUD->setZValue(10);



}

void Tank::keyPressEvent(QKeyEvent* event)
{
    keys[event->key()] = true;
    emit positionChanged();
    //Pause menu implementation
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "Pausing";
        if (pause.isHidden()) {
            GameRunner::pauseTimer();
            pause.show();
        }
    }
}

void Tank::keyReleaseEvent(QKeyEvent* event) {
    keys[event->key()] = false;
}

void Tank::focusOutEvent(QFocusEvent* event)
{
    this->setFocus();  
}

float Tank::calculateAngleCos(float speed, float angle) {
    float dx = speed * cos(angle);
    return dx;
}

float Tank::calculateAngleSin(float speed, float angle) {
    float dy = speed * sin(angle);
    return dy;
}

void Tank::frame() {

    // this code is what lets the tank turret, the muzzle flash, and the bullets follow the cursor. 
    // Every time the frame() function is called (about 144 times per second) the angle from the cursor to the center of the tank is calculated.
    // The function declared variables below will be deleted when the function exits so I do not believe they will cause memory issues
    turret->setPos(x() + this->boundingRect().width() / 2 - turret->boundingRect().width() / 2, y() + this->boundingRect().height() / 2 - turret->boundingRect().height() / 2 - 7);

    //Set HUD Position to update every frame
    health1->setPos(v->mapToScene(10, 10));
    health2->setPos(v->mapToScene(70, 10));
    health3->setPos(v->mapToScene(130, 10));
    bulletHUD->setPos(v->mapToScene(35,75));
    bulletCounterHUD->setPlainText(QString::number(bulletCounter));
    bulletCounterHUD->setPos(v->mapToScene(60, 47));



    QPointF cursorPos = QCursor::pos();
    QPointF cursorViewPos = v->mapFromGlobal(cursorPos);
    QPointF tankPos = this->pos();
    QPointF tankViewPos = v->mapFromScene(tankPos);

    float angle = (atan2(cursorViewPos.y() - (tankViewPos.y() + (this->boundingRect().height() / 2)), cursorViewPos.x() - (tankViewPos.x() + (this->boundingRect().width()) / 2)));
    float angleDegrees = angle * (180 / M_PI);
    turret->setRotation(angleDegrees + 90);

    // Calculate x and y velocity for tank gun muzzle flash
    int dxMuzzleFlash = calculateAngleCos(50, angle);
    int dyMuzzleFlash = calculateAngleSin(50, angle);


    fireFlash->setPos(x() + this->boundingRect().width() / 2 - fireFlash->boundingRect().width() / 2, y() + this->boundingRect().height() / 2 - fireFlash->boundingRect().height() / 2);
    fireFlash->moveBy(dxMuzzleFlash, dyMuzzleFlash);
    fireFlash->setTransformOriginPoint(fireFlash->boundingRect().width() / 2, fireFlash->boundingRect().height() / 2);
    fireFlash->setRotation(angleDegrees + 90);


    //Makes view camera follow the tank
    v->centerOn(this);

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

 // Movement & Shooting:

    float angleTank = (rotation() - 90) * (M_PI / 180);
    float dxTank = calculateAngleCos(traversalSpeed, angleTank);
    float dyTank = calculateAngleSin(traversalSpeed, angleTank);

    // traverse
    if (keys[Qt::Key_W]) {
        setPos(x() + dxTank, y() + dyTank);
        direction = 'w';
    }

    // rotate left
    if (keys[Qt::Key_A]) {
        setRotation(rotation() - rotationSpeed);
        direction = 'a';
    }

    // reverse
    if (keys[Qt::Key_S]) {
        setPos(x() - dxTank, y() - dyTank);
        direction = 's';
    }

    // rotate right
    if (keys[Qt::Key_D]) {
        setRotation(rotation() + rotationSpeed);
        direction = 'd';
    }

    // Shooting (Spacebar)
    if (keys[Qt::Key_Space]) {
        if (!fireRateTimer->isActive() && bulletCounter > 0) {
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
            // Decrement the bullet counter
            bulletCounter--;

            // Start the fireRateTimer
            fireRateTimer->start();
        }
    }

    if (fireRateTimer->isActive()) {

        // Sets how long the muzzle flush graphic lasts after a shot
        if (fireRateTimer->remainingTime() / (float)fireRateTimer->interval() < .95) {
            fireFlash->setVisible(false);
        }

        // Sets times intervals for dynamic crosshair graphics
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
    Obstacles* obstacle = new Obstacles();
    scene()->addItem(obstacle);
}

