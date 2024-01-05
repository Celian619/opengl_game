#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include "particles.h"

//Default plane values
const float PLANE_YAW = 70.0f;
const float PLANE_PITCH = 0.0f;
const float PLANE_SPEED = 0.5f;
const float PLANE_ROLL = 0.0f;
const float SHOOTING_COOLDOWN = 0.1f;


// Defines several possible options for plane movement. Used as abstraction to stay away from window-system specific input methods
enum movementDirection {
    UPWARD,
    DOWNWARD,
    LEFT,
    RIGHT
};

class Plane {
public:
    Particles *particles;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    // euler Angles
    float yaw;
    float pitch;
    float roll;
    // camera options
    float speed;

    // constructor with vectors
    Plane(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = PLANE_YAW, float pitch = PLANE_PITCH, float roll = PLANE_ROLL) : front(glm::vec3(1.0f, 0.0f, 0.0f)), speed(PLANE_SPEED)    {
        this->position = position;
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateFront();
    }


 // processes input received from any keyboard-like input system. Accepts input parameter in the form of plane defined ENUM (to abstract it from windowing systems)
    void processKeyboardMovement(movementDirection direction){
        switch (direction){
        case UPWARD:
            pitch -= 0.2f;
            break;
        case DOWNWARD:
            pitch += 0.2f;
            break;
        case LEFT:
            roll += 0.4f;
            break;
        case RIGHT:
            roll -= 0.4f;
            break;
        }

        if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;

        if (this->roll > 89.0f)
            this->roll = 89.0f;
        if (this->roll < -89.0f)
            this->roll = -89.0f;
    }

    void shoot(double time){
        if(time - lastShoot > SHOOTING_COOLDOWN){
            glm::vec3 positionCanon = glm::vec3(this->position) +  this->up * 1.0f + this->front; 
            this->particles->addNew(glm::vec3(this->front), positionCanon, getModelMatrix());

            lastShoot = time;
        }
    }

    void updateState(){
        //slowly go back to neutral position
        if(pitch != 0)
            pitch -= pitch/200;

        if(roll != 0){
            float delta = roll/200;
            roll -= delta;
            yaw -= delta;
        }

        this->updateFront();
        this->position += this->front * speed;
    }


    /*
    The calculus to get the model matrix from the plane rotation angles:
    p = pitch, y = yaw, r = roll

    roll: {{1,0,0},{0,cos(r),-sin(r)},{0,sin(r),cos(r)}}
    pitch: {{cos(p),sin(p),0},{-sin(p),cos(p),0},{0,0,1}}
    yaw: {{cos(y),0,sin(y)},{0,1,0},{-sin(y),0,cos(y)}}

    Combined rotation matrix:
    pitch * roll * yaw:
    sin(p) sin(r) sin(y) + cos(p) cos(y) | sin(p) cos(r) | cos(p) sin(y) - sin(p) sin(r) cos(y)
    cos(p) sin(r) sin(y) - sin(p) cos(y) | cos(p) cos(r) | -cos(p) sin(r) cos(y) - sin(p) sin(y)
    -cos(r) sin(y)                       | sin(r)        | cos(r) cos(y)
    */
    glm::mat4 getModelMatrix(){

        float cosp = cos(glm::radians(pitch));
        float cosy = cos(glm::radians(yaw));
        float cosr = cos(glm::radians(roll));
        float sinp = sin(glm::radians(pitch));
        float siny = sin(glm::radians(yaw));
        float sinr = sin(glm::radians(roll));

        float matrixArray[16] = {
            sinp*sinr*siny + cosp*cosy,  sinp*cosr,    cosp*siny - sinp*sinr*cosy,  0,
            cosp*sinr*siny - sinp*cosy,  cosp*cosr,    -cosp*sinr*cosy - sinp*siny, 0,
            -cosr*siny, sinr,            cosr*cosy,    0,
            position.x,                  position.y,   position.z,                  1
        };
        return glm::make_mat4(matrixArray);
    }


private:

    double lastShoot = 0;
    /* calculates the front and up vectors using the same method than getModelMatrix
        front: (1, 0, 0) => (cos(p) cos(y),    cos(p) sin(r) sin(y) + sin(p) cos(r),    cos(p) cos(r) sin(y) - sin(p) sin(r))
        up: (0, 1, 0) => (sin(p) (-cos(y)),    cos(p) cos(r) - sin(p) sin(r) sin(y),    -sin(p) cos(r) sin(y) - cos(p) sin(r))
    */
    void updateFront()   {
        // calculate the new Front vector
        float cosp = cos(glm::radians(pitch));
        float cosy = cos(glm::radians(yaw));
        float cosr = cos(glm::radians(roll));
        float sinp = sin(glm::radians(pitch));
        float siny = sin(glm::radians(yaw));
        float sinr = sin(glm::radians(roll));

        glm::vec3 calcFront;
        calcFront.x = sinp*sinr*siny + cosp*cosy;
        calcFront.y = sinp*cosr;
        calcFront.z = cosp*siny - sinp*sinr*cosy;
        this->front = glm::normalize(calcFront);

        glm::vec3 calcUp;
        calcUp.x = cosp*sinr*siny - sinp*cosy;
        calcUp.y = cosp*cosr;
        calcUp.z = -cosp*sinr*cosy - sinp*siny;
        this->up = glm::normalize(calcUp);
    }
};


#endif