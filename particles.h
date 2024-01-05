#ifndef PARTICLES_H
#define PARTICLES_H

#include <vector>
#include <glm/glm.hpp>
#include "object.h"
#include "shader.h"

struct Particle {
    glm::vec3 direction;
    glm::vec3 position;
    glm::mat4 model;
    float life;
};

const float PARTICLE_SPEED = 300.0f;
const float PARTICLE_LIFE = 5.0f;

class Particles {

public:

    Particles(Shader *particleShader, Object *particleObject){
        this->particleShader = particleShader;
        this->particleObject = particleObject;
    }

    void addNew(glm::vec3 direction, glm::vec3 position, glm::mat4 model){
        Particle * part = new Particle();
        part->direction = direction;
        part->position = position;
        part->model = model;
        part->life = PARTICLE_LIFE;
        particles.push_back(*part);
    }

    void update(float deltaTime){
        //remove every dead particles
        while(!particles.empty() && particles.front().life <= deltaTime)
                particles.erase(particles.begin());
        
        //update particles life and position
        for(auto partIt = particles.begin(); partIt != particles.end(); ++ partIt){
            partIt->life -= deltaTime;
            partIt->position += (deltaTime * PARTICLE_SPEED) * partIt->direction;
        }
    }

    void draw(){
        for(auto partIt = particles.begin(); partIt != particles.end(); ++ partIt){
            glm::mat4 modelParticle = glm::mat4(1.0f);
            modelParticle = glm::scale(modelParticle, glm::vec3(0.4f, 0.04f, 0.04f));

            glm::mat4 modelTransform = partIt->model;
            modelTransform[3] = glm::vec4(partIt->position,1);
            modelParticle =  modelTransform * modelParticle;
            particleShader->setMatrix4("M", modelParticle);
            particleObject->draw();
        }
    }

private:
    std::vector<Particle> particles;
    Shader *particleShader;
    Object *particleObject;
};
#endif