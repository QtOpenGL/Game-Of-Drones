#ifndef PLAYER_H
#define PLAYER_H

#define _USE_MATH_DEFINES

#include <QVector>
#include "DrawableObject.h"
#include "OBJManager/objmanager.h"
#include "TexturesManager/texturesmanager.h"
#include <cmath>

class Player : public DrawableObject
{
public:
    float accelerate;
    float friction;
    float maxspeed;
    float acceleration;
    float agility;
    float maxturn;
    int currentTarget = 0;
    QVector3D direction;
    QVector2D rotation;

public:
    Player();

    void init(OBJModel* model, QOpenGLTexture* texture, QOpenGLShaderProgram* shader);
    void logic(int deltaTime);
    void draw(Camera camera, Light light, QMatrix4x4 pMatrix);

public:
    float getAccelerate() noexcept {return accelerate;}
    void restoreStartPosition();
    void Accelerate();
    void TurnLeft();
    void TurnRight();
};

#endif