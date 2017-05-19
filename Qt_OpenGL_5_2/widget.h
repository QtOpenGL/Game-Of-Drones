#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QTimer>

#include "camera.h"
#include "light.h"
#include "drawableobject.h"

class Widget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

private:
    QMatrix4x4 pMatrix;

    QOpenGLShaderProgram cubeShaderProgram;
    QOpenGLShaderProgram lightSourceShaderProgram;
    QVector<QVector3D> spotlightVertices;
    QVector<QVector3D> spotlightColors;
    double droneRotate;

    QPoint lastMousePosition;

    Camera camera;
    Light light;
    DrawableObject object, object2, object3;

public:
    Widget(QWidget *parent = 0);
    ~Widget();
    QSize sizeHint() const;
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

public slots:
    void timeout();
    void keyPressEvent(QKeyEvent *event);
};

#endif // WIDGET_H
