#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>

//Default plane values
const float PLANE_YAW = 0.0f;
const float PLANE_PITCH = 0.0f;
const float PLANE_SPEED = 0.1f;//TODO change speed when the rest is working
const float PLANE_ROLL = 0.0f;



// Defines several possible options for plane movement. Used as abstraction to stay away from window-system specific input methods
enum movementDirection {
    UPWARD,
    DOWNWARD,
    LEFT,
    RIGHT
};

class Plane {
public:
    // camera Attributes
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
    void processKeyboardMovement(movementDirection direction, float deltaTime){
        if (direction == UPWARD){
            pitch -= deltaTime;
            //TODO
        }else if (direction == DOWNWARD){
            pitch += deltaTime;
            //TODO
        }else if (direction == LEFT){
            roll += deltaTime;
            //TODO
        }else if (direction == RIGHT){
            roll -= deltaTime;
            //TODO
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

    void updateState(){
        this->updateFront();
        this->position += this->front * speed;
    }

    /*
    p = pitch, y = yaw, r = roll

    calculate with https://www.wolframalpha.com/input?i=matrix+multiplication+calculator

    roll: {{1,0,0},{0,cos(r),-sin(r)},{0,sin(r),cos(r)}}

    pitch: {{cos(p),sin(p),0},{-sin(p),cos(p),0},{0,0,1}}

    yaw: {{cos(y),0,sin(y)},{0,1,0},{-sin(y),0,cos(y)}}

    roll * pitch * yaw:
(cos(p) cos(y) | sin(p) | cos(p) sin(y)
sin(r) sin(y) - sin(p) cos(r) cos(y) | cos(p) cos(r) | sin(r) (-cos(y)) - sin(p) cos(r) sin(y)
sin(p) sin(r) (-cos(y)) - cos(r) sin(y) | cos(p) sin(r) | cos(r) cos(y) - sin(p) sin(r) sin(y))

*/
    glm::mat4 getModelMatrix(){

        float cosp = cos(glm::radians(pitch));
        float cosy = cos(glm::radians(yaw));
        float cosr = cos(glm::radians(roll));
        float sinp = sin(glm::radians(pitch));
        float siny = sin(glm::radians(yaw));
        float sinr = sin(glm::radians(roll));


        float matrixArray[16] = {
    cosp*cosy,    sinp,   cosp*siny , 0,
    sinr*siny - sinp*cosr*cosy,  cosp*cosr,   -sinr*cosy -sinp*cosr*siny, 0,
    -sinp*sinr*cosy-cosr*siny, cosp*sinr,  cosr*cosy-sinp*sinr*siny, 0,
    position.x, position.y,position.z,       1
};

        return glm::make_mat4(matrixArray);
    }


private:
    /* calculates the front vector from the Plane's (updated) Euler Angles
        front: (1, 0, 0) => (cos(p) cos(y), cos(p) sin(r) sin(y) + sin(p) cos(r), cos(p) cos(r) sin(y) - sin(p) sin(r))
         up: (0, 1, 0) => (sin(p) (-cos(y)), cos(p) cos(r) - sin(p) sin(r) sin(y), -sin(p) cos(r) sin(y) - cos(p) sin(r))

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
        calcFront.x = cosp*cosy;
        calcFront.y = sinp;
        calcFront.z = cosp*siny;
        this->front = glm::normalize(calcFront);

        glm::vec3 calcUp;
        calcUp.x = sinr*siny - sinp*cosr*cosy;
        calcUp.y = cosp*cosr;
        calcUp.z = -sinr*cosy -sinp*cosr*siny;
        this->up = glm::normalize(calcUp);
    }
};


#endif