#include "widget.h"

Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{

}

Widget::~Widget()
{

}

QSize Widget::sizeHint() const
{
    return QSize(640, 480);
}

void Widget::initializeGL()
{
    initializeOpenGLFunctions(); //Odpala funkcję OpenGL'a. Bez tego wywala aplikację zanim ją ujrzysz.
    glEnable(GL_DEPTH_TEST); //Sprawi, że będą wyświetlane tylko te obiekty, które sa bliżej kamery
    glEnable(GL_CULL_FACE); //Obiekty będą renderowane tylko na przedniej stronie
    glClearColor(0,0,0,0); //Ustawienie koloru tła

    //Ładowanie shaderów
    cubeShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/lightningvertexshader.vert");
    cubeShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/lightingfragmentshader.fsh");
    cubeShaderProgram.link();

    lightSourceShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/coloringvertexshader.vert");
    lightSourceShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/coloringfragmentshader.frag");
    lightSourceShaderProgram.link();

    spotlightVertices << QVector3D(   0,    1,    0) << QVector3D(-0.5,    0,  0.5) << QVector3D( 0.5,    0,  0.5) // Front
                      << QVector3D(   0,    1,    0) << QVector3D( 0.5,    0, -0.5) << QVector3D(-0.5,    0, -0.5) // Back
                      << QVector3D(   0,    1,    0) << QVector3D(-0.5,    0, -0.5) << QVector3D(-0.5,    0,  0.5) // Left
                      << QVector3D(   0,    1,    0) << QVector3D( 0.5,    0,  0.5) << QVector3D( 0.5,    0, -0.5) // Right
                      << QVector3D(-0.5,    0, -0.5) << QVector3D( 0.5,    0, -0.5) << QVector3D( 0.5,    0,  0.5) // Bottom
                      << QVector3D( 0.5,    0,  0.5) << QVector3D(-0.5,    0,  0.5) << QVector3D(-0.5,    0, -0.5);

    spotlightColors << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) // Front
                    << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) // Back
                    << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) // Left
                    << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) << QVector3D(0.2, 0.2, 0.2) // Right
                    << QVector3D(  1,   1,   1) << QVector3D(  1,   1,   1) << QVector3D(  1,   1,   1) // Bottom
                    << QVector3D(  1,   1,   1) << QVector3D(  1,   1,   1) << QVector3D(  1,   1,   1);


    skybox.Init(&cubeShaderProgram, ":/Content/skybox.obj", ":/Content/skybox.png");
    skybox.SpecularReflection = 0;

    light.Position.setZ(2);

    //Game.Current = Game();
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glDisable(GL_CULL_FACE);
    skybox.Draw(camera, light, projectionMatrix);
    glEnable(GL_CULL_FACE);

    QMatrix4x4 modelMatrix = light.GetMatrix();

    lightSourceShaderProgram.bind();
    lightSourceShaderProgram.setUniformValue("mvpMatrix", projectionMatrix * camera.GetMatrix() * modelMatrix); //mvpMatrix = projection * view * model
    lightSourceShaderProgram.setAttributeArray("vertex", spotlightVertices.constData());
    lightSourceShaderProgram.enableAttributeArray("vertex");
    lightSourceShaderProgram.setAttributeArray("color", spotlightColors.constData());
    lightSourceShaderProgram.enableAttributeArray("color");

    glDrawArrays(GL_TRIANGLES, 0, spotlightVertices.size());

    lightSourceShaderProgram.disableAttributeArray("vertex");
    lightSourceShaderProgram.disableAttributeArray("color");
    lightSourceShaderProgram.release();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    lastMousePosition = event->pos();
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - lastMousePosition.x();
    int deltaY = event->y() - lastMousePosition.y();
    if (event->buttons() & Qt::LeftButton)
    {
        float cameraRotY = camera.Rotation.y();
        cameraRotY -= deltaX;

        while (cameraRotY < 0)
        {
            cameraRotY += 360;
        }
        while (cameraRotY >= 360)
        {
            cameraRotY -= 360;
        }
        camera.Rotation.setY(cameraRotY);

        float cameraRotX = camera.Rotation.x();
        cameraRotX -= deltaY;

        while (cameraRotX < 0)
        {
            cameraRotX += 360;
        }
        while (cameraRotX >= 360)
        {
            cameraRotX -= 360;
        }
        camera.Rotation.setX(cameraRotX);

        update();
    }
    lastMousePosition = event->pos();
    event->accept();
}

void Widget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();
    if (event->orientation() == Qt::Vertical)
    {
        if (delta < 0)
        {
            camera.Distance *= 1.1;
        }
        else if (delta > 0)
        {
            camera.Distance *= 0.9;
        }

        update();
    }

    event->accept();
}

void Widget::keyPressEvent(QKeyEvent *event)
{

}
