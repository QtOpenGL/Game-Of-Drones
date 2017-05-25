#include "envgenerator.h"

EnvGenerator::EnvGenerator()
{

}

EnvGenerator::~EnvGenerator()
{
    for(int i=0; i<objects.size(); i++)
    {
        delete objects[i];
    }

    objects.clear();
}

void EnvGenerator::Init(OBJManager* objManager, QOpenGLShaderProgram* shader)
{
    this->objManager = objManager;
    this->shader = shader;

    DrawableObject* test = new DrawableObject();
    test->Init(shader, objManager->GetModel(":/Objects/planetoid"), ":/Textures/planetoid-texture");
    test->GetRotationSpeed().setX(1);
    test->GetRotationSpeed().setY(3);
    test->GetRotationSpeed().setZ(2);
    objects.push_back(test);
}

void EnvGenerator::Logic()
{
    for(int i=0; i<objects.size(); i++)
    {
        objects[i]->Logic();
    }
}

void EnvGenerator::Draw(Camera camera, Light light, QMatrix4x4 pMatrix)
{
    for(int i=0; i<objects.size(); i++)
    {
        objects[i]->Draw(camera, light, pMatrix);
    }
}