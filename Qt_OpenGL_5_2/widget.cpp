#include "widget.h"

Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
    menuIsActive = true;
    miliSecondsFromStart = 0;

    createLayout();

    makeConnection();

    mouseTimer.setSingleShot(true);

    //initializeSoundtrack();
}

Widget::~Widget()
{

}

QSize Widget::sizeHint() const
{
    return QSize(640, 480);
}

void Widget::loadShaders()
{
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/LightningVertexShader");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/LightningFragmentShader");
    shaderProgram.link();
}

void Widget::initializeGL()
{
    initializeOpenGLFunctions(); //Odpala funkcję OpenGL'a. Bez tego wywala aplikację zanim ją ujrzysz.
    glEnable(GL_DEPTH_TEST); //Sprawi, że będą wyświetlane tylko te obiekty, które sa bliżej kamery
    glEnable(GL_CULL_FACE); //Obiekty będą renderowane tylko na przedniej stronie
    glClearColor(0,0,0,0); //Ustawienie koloru tła

    loadShaders(); //Ładowanie shaderów
    game.initializeGame(&shaderProgram, &keyboardManager);

}

void Widget::resizeGL(int width, int height)
{
    if (height == 0)
    {
        height = 1; //Aby nie dzielić przez zero
    }

    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);
    glViewport(0, 0, width, height);
}

void Widget::paintGL()
{
    logic();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game.render(camera, light, projectionMatrix);

    updateTimeLabel();
}

void Widget::logic()
{
    game.logic(camera);
    fpsCounterLabel->setText("FPS: " + QString::number(telemetry.getFPS()));
    shipInfo->setText("Informacje o statku<br/>Nazwa statku: Orzeł 1<br/>Prędkość: " +QString::number(static_cast<int>(2444 * game.getPlayerSpeed())) + " m/s<br/>Moc silników: " +QString::number(static_cast<int>(696.8 * game.getPlayerAccelerate())) + "%");
    telemetry.logic();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    mouseTimer.stop();


    camera.setDirection(QVector3D(camera.getDirection().x() + (event->pos().x() - mousePos.x())/2,camera.getDirection().y() + (event->pos().y() - mousePos.y())/2,0));

    mousePos = event->pos();
    event->accept();
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    mouseTimer.start(1000);
    event->accept();
}

void Widget::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        if(menuIsActive == false)
        {
            pauseGame();
        }
    }

    Qt::Key key = (Qt::Key)event->key();

    keyboardManager.keyPressed(key);

    event->accept();
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{
    Qt::Key key = (Qt::Key)event->key();

    keyboardManager.keyReleased(key);

    event->accept();
}

void Widget::startGame()
{
    menuIsActive = false;
    game.start();

    startGameButton->setVisible(false);
    restartGameButton->setVisible(false);
    closeGameButton->setVisible(false);

    playGameTimer.start();

    paintTimer.setTimerType(Qt::PreciseTimer);
    paintTimer.start(16);

    fpsCounterLabel->setVisible(true);
    timerLabel->setVisible(true);
    shipInfo->setVisible(true);
    speedProgressBar->setVisible(true);
    enginePowerProgressBar->setVisible(true);
    setFocus();

}
void Widget::pauseGame()
{
    menuIsActive = true;
    game.pause();

    startGameButton->setVisible(true);
    startGameButton->setText("Wznów");
    restartGameButton->setVisible(true);
    closeGameButton->setVisible(true);

    paintTimer.stop();
    miliSecondsFromStart += playGameTimer.elapsed();

    fpsCounterLabel->setVisible(false);
    timerLabel->setVisible(false);
    shipInfo->setVisible(false);
    speedProgressBar->setVisible(false);
    enginePowerProgressBar->setVisible(false);
    setFocus();
}

void Widget::restartGame()
{
    menuIsActive = false;
    game.restart();

    startGameButton->setVisible(false);
    restartGameButton->setVisible(false);
    closeGameButton->setVisible(false);

    playGameTimer.start();
    miliSecondsFromStart = 0;

    paintTimer.setTimerType(Qt::PreciseTimer);
    paintTimer.start(16);

    fpsCounterLabel->setVisible(true);
    timerLabel->setVisible(true);
    shipInfo->setVisible(true);
    speedProgressBar->setVisible(true);
    enginePowerProgressBar->setVisible(true);
    setFocus();
}

void Widget::closeGame()
{
    qApp->quit();
}


void Widget::updateTimeLabel()
{
    timerLabel->setText("Czas: "+TimeConverter::toQStringFromMsec(playGameTimer.elapsed() + miliSecondsFromStart));
}

void Widget::makeConnection()
{
    connect(&paintTimer, SIGNAL(timeout()), this, SLOT(update()));
    connect(startGameButton,SIGNAL(pressed()),this,SLOT(startGame()));
    connect(restartGameButton,SIGNAL(pressed()),this,SLOT(restartGame()));
    connect(closeGameButton,SIGNAL(pressed()),this,SLOT(closeGame()));
    connect(&mouseTimer, SIGNAL(timeout()), this, SLOT(mouseTimerTimeout()));
}

void Widget::createLayout()
{
    QRect rect = QApplication::desktop()->screen()->rect();
    double width = rect.width();
    double height = rect.height();

    cssFpsAndTimer = "font-size:"+QString::number(static_cast<int>(30.0/1920.0 * width))+"px;color:white;padding:8px;margin:10px;background-color: rgba(0,84,210,0.5);border: 1px solid rgba(0,94,220,0.6); border-radius: 10px;";

    fpsCounterLabel = new QLabel("FPS: 00");
    fpsCounterLabel->setStyleSheet(cssFpsAndTimer);
    fpsCounterLabel->setAlignment(Qt::AlignLeft);
    fpsCounterLabel->setVisible(false);

    timerLabel = new QLabel("Czas: 00:00.00");
    timerLabel->setStyleSheet(cssFpsAndTimer);
    timerLabel->setAlignment(Qt::AlignRight);
    timerLabel->setVisible(false);

    shipInfo = new QLabel("Informacje o statku<br/>Nazwa statku: Orzeł 1<br/>Prędkość: infinity<br/>Moc silników: 0%");
    shipInfo->setAlignment(Qt::AlignCenter);;
    shipInfo->setStyleSheet(cssFpsAndTimer);
    shipInfo->setMinimumSize(800.0/1920.0 * width,250.0/1080.0 * height);
    shipInfo->setMaximumSize(800.0/1920.0 * width,250.0/1080.0 * height);
    shipInfo->setVisible(false);

    startGameButton = new QPushButton("Start!");
    startGameButton->setStyleSheet("QPushButton {"+cssFpsAndTimer+"} QPushButton:hover {background-color: rgba(0,74,200,0.5);} QPushButton:pressed {background-color: rgba(0,54,180,0.4);}");
    startGameButton->setMaximumWidth(400.0/1920.0 * width);
    startGameButton->setMinimumWidth(400.0/1920.0 * width);

    restartGameButton = new QPushButton("Zacznij od nowa");
    restartGameButton->setStyleSheet("QPushButton {"+cssFpsAndTimer+"} QPushButton:hover {background-color: rgba(0,74,200,0.5);} QPushButton:pressed {background-color: rgba(0,54,180,0.4);}");
    restartGameButton->setMaximumWidth(400.0/1920.0 * width);
    restartGameButton->setMinimumWidth(400.0/1920.0 * width);
    restartGameButton->setVisible(false);

    closeGameButton = new QPushButton("Wyjście");
    closeGameButton->setStyleSheet("QPushButton {"+cssFpsAndTimer+"} QPushButton:hover {background-color: rgba(0,74,200,0.5);} QPushButton:pressed {background-color: rgba(0,54,180,0.4);}");
    closeGameButton->setMaximumWidth(400.0/1920.0 * width);
    closeGameButton->setMinimumWidth(400.0/1920.0 * width);

    speedProgressBar = new QProgressBar();
    speedProgressBar->setStyleSheet(cssFpsAndTimer);
    speedProgressBar->setMaximumSize(100.0/1920.0 * width,400.0/1080.0 * height);
    speedProgressBar->setMinimumSize(100.0/1920.0 * width,400.0/1080.0 * height);
    speedProgressBar->setVisible(false);

    enginePowerProgressBar = new QProgressBar();
    enginePowerProgressBar->setStyleSheet(cssFpsAndTimer);
    enginePowerProgressBar->setMaximumSize(100.0/1920.0 * width,400.0/1080.0 * height);
    enginePowerProgressBar->setMinimumSize(100.0/1920.0 * width,400.0/1080.0 * height);
    enginePowerProgressBar->setVisible(false);

    gridMenuLayout = new QGridLayout();

    gridLayout = new QGridLayout(this);
    gridLayout->addWidget(fpsCounterLabel,0,0,Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(timerLabel,0,2,Qt::AlignTop | Qt::AlignRight);
    gridLayout->addLayout(gridMenuLayout,1,1, Qt::AlignCenter);

    gridMenuLayout->addWidget(startGameButton,1,1, Qt::AlignCenter);
    gridMenuLayout->addWidget(restartGameButton,2,1, Qt::AlignCenter);
    gridMenuLayout->addWidget(closeGameButton,3,1, Qt::AlignCenter);

    gridLayout->addWidget(speedProgressBar,2,0,Qt::AlignBottom | Qt::AlignLeft);
    gridLayout->addWidget(enginePowerProgressBar,2,2,Qt::AlignBottom | Qt::AlignRight);

    gridLayout->addWidget(shipInfo,2,1,Qt::AlignBottom);
    gridLayout->setColumnStretch(0,0.1);
    gridLayout->setColumnStretch(1,1.8);
    gridLayout->setColumnStretch(2,0.1);
}

void Widget::initializeSoundtrack()
{
    musicPlayer.setSong("Dave Rodgers - Deja Vu.wma");
    musicPlayer.play(QMediaPlaylist::CurrentItemInLoop);
}

void Widget::mouseTimerTimeout()
{
    camera.resetCamera();
}
