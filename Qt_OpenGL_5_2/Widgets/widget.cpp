#include "widget.h"

Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
    menuIsActive = true;
    miliSecondsFromStart = 0;

    createLayout();
    makeConnection();
    //initializeSoundtrack();
    initializeTimers();
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

    game.getProjectionMatrixRef().setToIdentity();
    game.getProjectionMatrixRef().perspective(60.0, (float) width / (float) height, 1, 500000.0f);
    glViewport(0, 0, width, height);
}

void Widget::paintGL()
{
    game.logic();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    game.render();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    mouseTimer.stop();

    game.getCameraRef().addToDirection((event->pos().x() - mousePos.x())/2, (event->pos().y() - mousePos.y())/2, 0);
    mousePos = event->pos();
    event->accept();
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    mouseTimer.start(1000);
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
    addFromTimer = true;
    if(game.getIsGameEnded() == true)
    {
        game.resume();
    }
    else
    {
        game.start();
        playGameTimer.start();
    }

    paintTimer.start(16);
    layoutTimer.start(10);

    toggleMenuVisibility(false);
    toggleInGameLayoutVisibility(true);
    setFocus();

}
void Widget::pauseGame()
{
    menuIsActive = true;
    game.pause();

    if(game.getIsGameEnded() == false)
    {
        miliSecondsFromStart += playGameTimer.elapsed();
    }

    paintTimer.stop();
    layoutTimer.stop();

    toggleMenuVisibility(true);
    toggleInGameLayoutVisibility(false);
    setFocus();
}

void Widget::restartGame()
{
    menuIsActive = false;
    addFromTimer = true;
    game.restart();

    playGameTimer.start();
    miliSecondsFromStart = 0;

    paintTimer.start(16);
    layoutTimer.start(10);

    toggleMenuVisibility(false);
    toggleInGameLayoutVisibility(true);
    setFocus();
}

void Widget::closeGame()
{
    qApp->quit();
}

void Widget::makeConnection()
{
    connect(&paintTimer, SIGNAL(timeout()), this, SLOT(update()));
    connect(&layoutTimer,SIGNAL(timeout()),this,SLOT(updateLayout()));
    connect(&mouseTimer, SIGNAL(timeout()), this, SLOT(mouseTimerTimeout()));

    connect(startGameButton,SIGNAL(pressed()),this,SLOT(startGame()));
    connect(restartGameButton,SIGNAL(pressed()),this,SLOT(restartGame()));
    connect(closeGameButton,SIGNAL(pressed()),this,SLOT(closeGame()));

    connect(this,SIGNAL(updateSpeedProgressBar(int)),speedProgressBar,SLOT(setValue(int)));
    connect(this,SIGNAL(updateEnginePowerProgressBar(int)),enginePowerProgressBar,SLOT(setValue(int)));

    connect(&game,SIGNAL(endGame(QString)),this,SLOT(showEndGameInfo(QString)));
}

void Widget::createLayout()
{
    QRect rect = QApplication::desktop()->screen()->rect();
    double width = rect.width();
    double height = rect.height();

    cssLabels = "font-size:"+QString::number(static_cast<int>(30.0/1920.0 * width))+"px;color:white;padding:" + QString::number(static_cast<int>(8.0/1920.0 * width)) +"px;margin:" + QString::number(static_cast<int>(10.0/1920.0 * width)) +"px;background-color: rgba(0,84,210,0.5);border: 1px solid rgba(0,94,220,0.6); border-radius: 10px;";
    cssButtons = "QPushButton {"+cssLabels+"} QPushButton:hover {background-color: rgba(0,54,180,0.5);} QPushButton:pressed {background-color: rgba(0,44,170,0.4);}";
    cssProgressBars = "QProgressBar {" + cssLabels + " text-align: center; } QProgressBar::chunk {background-color: qlineargradient(x1: 0.5, y1: 0, x2: 0.5, y2: 1,stop: 0 rgba(0,211,28,0.8), stop: 1 rgba(255,0,0,0.8));border: 1px solid rgba(0,94,220,0.8); border-radius: 10px;}";

    fpsCounterLabel = new QLabel("FPS: 00");
    fpsCounterLabel->setStyleSheet(cssLabels);
    fpsCounterLabel->setAlignment(Qt::AlignLeft);
    fpsCounterLabel->setVisible(false);

    timerLabel = new QLabel("Czas: 00:00.00");
    timerLabel->setStyleSheet(cssLabels);
    timerLabel->setAlignment(Qt::AlignRight);
    timerLabel->setVisible(false);

    speedLabel = new QLabel("Prędkość");
    speedLabel->setStyleSheet(cssLabels);
    speedLabel->setAlignment(Qt::AlignLeft);
    speedLabel->setVisible(false);

    enginePowerLabel = new QLabel("Moc silników");
    enginePowerLabel->setStyleSheet(cssLabels);
    enginePowerLabel->setAlignment(Qt::AlignRight);
    enginePowerLabel->setVisible(false);

    shipInfo = new QLabel("Informacje o statku<br/>Nazwa statku: Orzeł 1<br/>Prędkość: 0 m/s<br/>Moc silników: 0%");
    shipInfo->setAlignment(Qt::AlignCenter);;
    shipInfo->setStyleSheet(cssLabels);
    shipInfo->setMinimumSize(800.0/1920.0 * width,250.0/1080.0 * height);
    shipInfo->setMaximumSize(800.0/1920.0 * width,250.0/1080.0 * height);
    shipInfo->setVisible(false);

    gameInfoLabel = new QLabel("Dummy");
    gameInfoLabel->setAlignment(Qt::AlignCenter);
    gameInfoLabel->setStyleSheet(cssLabels);
    gameInfoLabel->setVisible(false);

    gameTitleLabel = new QLabel("Game of Drones");
    gameTitleLabel->setAlignment(Qt::AlignCenter);
    gameTitleLabel->setStyleSheet(cssLabels);
    gameTitleLabel->setMaximumWidth(400.0/1920.0 * width);
    gameTitleLabel->setMinimumWidth(400.0/1920.0 * width);
    gameTitleLabel->setVisible(true);

    startGameButton = new QPushButton("Start!");
    startGameButton->setStyleSheet(cssButtons);
    startGameButton->setMaximumWidth(400.0/1920.0 * width);
    startGameButton->setMinimumWidth(400.0/1920.0 * width);

    restartGameButton = new QPushButton("Zacznij od nowa");
    restartGameButton->setStyleSheet(cssButtons);
    restartGameButton->setMaximumWidth(400.0/1920.0 * width);
    restartGameButton->setMinimumWidth(400.0/1920.0 * width);
    restartGameButton->setVisible(false);

    closeGameButton = new QPushButton("Wyjście");
    closeGameButton->setStyleSheet(cssButtons);
    closeGameButton->setMaximumWidth(400.0/1920.0 * width);
    closeGameButton->setMinimumWidth(400.0/1920.0 * width);

    speedProgressBar = new QProgressBar();
    speedProgressBar->setStyleSheet(cssProgressBars);
    speedProgressBar->setMaximumSize(150.0/1920.0 * width,400.0/1080.0 * height);
    speedProgressBar->setMinimumSize(150.0/1920.0 * width,400.0/1080.0 * height);
    speedProgressBar->setOrientation(Qt::Vertical);
    speedProgressBar->setMinimum(0);
    speedProgressBar->setMaximum(35425);
    speedProgressBar->setVisible(false);

    enginePowerProgressBar = new QProgressBar();
    enginePowerProgressBar->setStyleSheet(cssProgressBars);
    enginePowerProgressBar->setMaximumSize(150.0/1920.0 * width,400.0/1080.0 * height);
    enginePowerProgressBar->setMinimumSize(150.0/1920.0 * width,400.0/1080.0 * height);
    enginePowerProgressBar->setOrientation(Qt::Vertical);
    enginePowerProgressBar->setMinimum(0);
    enginePowerProgressBar->setMaximum(1000);
    enginePowerProgressBar->setVisible(false);

    gridMenuLayout = new QGridLayout();

    gridLayout = new QGridLayout(this);
    gridLayout->addWidget(fpsCounterLabel,0,0,Qt::AlignTop | Qt::AlignLeft);
    gridLayout->addWidget(timerLabel,0,2,Qt::AlignTop | Qt::AlignRight);

    gridLayout->addWidget(gameInfoLabel,0,1,Qt::AlignTop | Qt::AlignCenter);

    gridLayout->addWidget(speedLabel,1,0,Qt::AlignBottom | Qt::AlignLeft);
    gridLayout->addWidget(enginePowerLabel,1,2,Qt::AlignBottom | Qt::AlignRight);

    gridLayout->addLayout(gridMenuLayout,1,1, Qt::AlignCenter);

    gridMenuLayout->addWidget(gameTitleLabel,1,1, Qt::AlignCenter);
    gridMenuLayout->addWidget(startGameButton,2,1, Qt::AlignCenter);
    gridMenuLayout->addWidget(restartGameButton,3,1, Qt::AlignCenter);
    gridMenuLayout->addWidget(closeGameButton,4,1, Qt::AlignCenter);

    gridLayout->addWidget(speedProgressBar,2,0,Qt::AlignBottom | Qt::AlignLeft);
    gridLayout->addWidget(enginePowerProgressBar,2,2,Qt::AlignBottom | Qt::AlignRight);

    gridLayout->addWidget(shipInfo,2,1,Qt::AlignBottom);
    gridLayout->setColumnStretch(0,0.1);
    gridLayout->setColumnStretch(1,1.8);
    gridLayout->setColumnStretch(2,0.1);
}

void Widget::initializeSoundtrack()
{
    musicPlayer.addSong("Artist - Song.wma");
    musicPlayer.play(QMediaPlaylist::CurrentItemInLoop);
}

void Widget::toggleMenuVisibility(bool value)
{
    startGameButton->setVisible(value);
    restartGameButton->setVisible(value);
    closeGameButton->setVisible(value);
    gameTitleLabel->setVisible(value);
    if(game.getIsGamePaused() == true)
    {
        startGameButton->setText("Wznów");
    }
}

void Widget::toggleInGameLayoutVisibility(bool value)
{
    fpsCounterLabel->setVisible(value);
    timerLabel->setVisible(value);
    speedLabel->setVisible(value);
    enginePowerLabel->setVisible(value);
    shipInfo->setVisible(value);
    speedProgressBar->setVisible(value);
    enginePowerProgressBar->setVisible(value);
    if(game.getIsGameEnded() == true)
    {
        gameInfoLabel->setVisible(value);
    }
}

void Widget::initializeTimers()
{
    paintTimer.setTimerType(Qt::PreciseTimer);
    layoutTimer.setTimerType(Qt::PreciseTimer);
    mouseTimer.setSingleShot(true);
}

void Widget::mouseTimerTimeout()
{
    game.getCameraRef().resetCamera();
}

void Widget::updateLayout()
{
    telemetry.logic();
    int speed = static_cast<int>(2444 * game.getPlayerSpeed());
    int enginePower = static_cast<int>(6968 * game.getPlayerAccelerate());
    if(game.getIsGameEnded() == true)
    {
        timerLabel->setText("Czas: "+TimeConverter::toQStringFromMsec(miliSecondsFromStart));
    }
    else
    {
        timerLabel->setText("Czas: "+TimeConverter::toQStringFromMsec(playGameTimer.elapsed() + miliSecondsFromStart));
    }
    fpsCounterLabel->setText("FPS: " + QString::number(telemetry.getFPS()));
    shipInfo->setText("Informacje o statku<br/>Nazwa statku: Orzeł 1<br/>Prędkość: " +QString::number(speed) + " m/s<br/>Moc silników: " +QString::number(enginePower/10) + "%");
    updateSpeedProgressBar(speed);
    updateEnginePowerProgressBar(enginePower);
}

void Widget::showEndGameInfo(QString message)
{
    gameInfoLabel->setVisible(true);
    gameInfoLabel->setText(message);
    if(addFromTimer == true)
    {
        miliSecondsFromStart += playGameTimer.elapsed();
        addFromTimer = false;
    }
}
