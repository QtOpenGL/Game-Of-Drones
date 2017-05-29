#ifndef PLAYER_H
#define PLAYER_H

#include <QVector>
#include "DrawableObject.h"
#include "OBJManager/objmanager.h"
#include "TexturesManager/texturesmanager.h"
#include <cmath>

class Player : public DrawableObject
{
private:
    GLfloat speed;
    QVector2D direction;

public:
    Player();

    void init(OBJManager* objManager, TexturesManager* texturesManager, QOpenGLShaderProgram* shader);
    void logic(int deltaTime);
    void draw(Camera camera, Light light, QMatrix4x4 pMatrix);
    void input(Qt::Key key);
};

#endif
