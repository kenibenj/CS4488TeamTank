#include "GameRunner.h"
#include "StartMenu.h"
#include "MainHeader.h"
#include "Tank.h"
#include "map_creator.h"

QTimer* enemyTimer = new QTimer();
GameRunner::GameRunner() {
    enemyTimer->start(17);
    QScreen* primaryScreen = QApplication::primaryScreen();


    MapCreator map_creator;
    //map_creator.setFile(":/sounds/my_map.txt"); NOT SURE why this doesn't work
    map_creator.setFile("C:\\Users\\pawan\\OneDrive\\Documents\\GitHub\\CS4488TeamTank\\TankTest\\my_map.txt");
    //Create scene on the heap
    QGraphicsScene* scene = new QGraphicsScene();

    int w = primaryScreen->geometry().width() / 2;
    int h = primaryScreen->geometry().height() / 2;

    //add a view, this is what displays the graphics
    QGraphicsView* view = new QGraphicsView(scene);

    //Create rectangle for scene
    //MyRect* player = new MyRect();
    Tank* tank = new Tank(view);

    scene->addItem(tank);
    scene->setSceneRect(0, 0, 2400, 1800);

    //Create map using map_creator
    map_creator.CreateMap(scene);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setBackgroundBrush(QBrush(Qt::black));
    view->setAlignment(Qt::AlignCenter);
    view->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    view->setRenderHint(QPainter::Antialiasing);
    view->setGeometry(400, 50, 1200, 900);
    QCursor cursor = QCursor(QPixmap(":/images/crosshair.png"));
    view->setCursor(cursor);
    //view->resize(w * 2 - 40, h * 2 - 100);


    QBrush back_brush(QColor(255, 243, 240)); //bricks & box
    //QBrush back_brush(QColor(224, 255, 224)); //forest

    QPixmap backgroundImage(":/images/bg.png");
    QGraphicsPixmapItem* background = new QGraphicsPixmapItem();
    background->setPixmap(backgroundImage.scaled(scene->width(), scene->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    background->setZValue(-10);
    scene->addItem(background);

    //  scene->addLine(-w, -h + 75, w, -h + 75, QPen(Qt::black));//upper bound
      //scene->addLine(-w, h, w, h, QPen(Qt::black));//lower bound
      //scene->addLine(-w, -h + 75, -w, h, QPen(Qt::black));//left bound
      //scene->addLine(w, -h + 75, w, h, QPen(Qt::black));//right bo


      //makes focuable
    tank->setFlag(QGraphicsItem::ItemIsFocusable);
    tank->setFocus();
    tank->setPos(view->width() / 2 - 50, view->height() - 50);
    tank->createTurret();
    //Play music
    QMediaPlayer music;
    QAudioOutput audioPlayer;
    audioPlayer.setVolume(.05);
    music.setAudioOutput(&audioPlayer);
    music.setSource(QUrl("qrc:/sounds/backgroundMusic.mp3"));
    music.play();

    view->show();
    view->setFixedSize(1200, 900);

    tank->setPos(view->width() / 2 - 50, view->height() - 50);

    // spawn enemies
    QTimer* timer = new QTimer();
    QObject::connect(timer, SIGNAL(timeout()), tank, SLOT(spawn()));
    timer->start(10000);
}