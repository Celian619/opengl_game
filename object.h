#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include "texture.h"
#include "shader.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

//if texture inversed add aiProcess_FlipUVs
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals| aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace)
#define INVALID_MATERIAL 0xFFFFFFFF

/* Variables in vertex shader should be defined as:
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal; 
layout(location = 2) in vec2 textureCoord; 
layout(location = 3) in vec3 tangent; 
*/
#define POSITION_LOC 0
#define NORMAL_LOC 1
#define TEXTURECOORD_LOC 2
#define TANGENT_LOC 3

class Material {

 public:

    Texture* pDiffuse = NULL;
    Texture* pNormal = NULL;
    Texture* pSpecularExponent = NULL;
};

//class based on tutorial "Loading Models Using Assimp": https://www.youtube.com/watch?v=sP_kiODC25Q
class Object{
public:

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB       = 1,
        TEXCOORD_VB  = 2,
        NORMAL_VB    = 3,
        TANGENT_VB    = 4,
        NUM_BUFFERS  = 5
    };

    struct BasicMeshEntry {
        BasicMeshEntry(){
            numIndices = 0;
            baseVertex = 0;
            baseIndex = 0;
            materialIndex = INVALID_MATERIAL;
        }
        unsigned int numIndices;
        unsigned int baseVertex;
        unsigned int baseIndex;
        unsigned int materialIndex;
    };
    
    GLuint VAO;
    GLuint buffers[NUM_BUFFERS] = {0};
    std::vector<BasicMeshEntry> meshes;

    std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textCoords;
	std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
	std::vector<unsigned int > indices;
	std::vector<Material> materials;

    Object(const char* path) {

        std::cout << "Loading object" << path << std::endl;
        Assimp::Importer importer;

        const aiScene* pScene = importer.ReadFile(path, ASSIMP_LOAD_FLAGS);

        if(pScene){

            meshes.resize(pScene->mNumMeshes);
            materials.resize(pScene->mNumMaterials);

            //find total number of vertices/indices over all meshes to reserve space
            unsigned int numVertices = 0;
            unsigned int numIndices = 0;

            countVerticesAndIndices(pScene, numVertices, numIndices);

            positions.reserve(numVertices);
            materials.reserve(numVertices);
            normals.reserve(numVertices);
            tangents.reserve(numVertices);
            indices.reserve(numIndices);

            //populate buffers for every meshes
            for(unsigned int meshIdx =0; meshIdx < meshes.size(); meshIdx++){
                const aiMesh* paiMesh = pScene->mMeshes[meshIdx];
                const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

                //populate vertex attribute buffers for this mesh
                for(unsigned int i = 0; i < paiMesh->mNumVertices; i++){
                    const aiVector3D& pPos = paiMesh->mVertices[i];
                    positions.push_back(glm::vec3(pPos.x,pPos.y,pPos.z));

                    const aiVector3D& pNormal = paiMesh->mNormals[i];
                    normals.push_back(glm::vec3(pNormal.x,pNormal.y,pNormal.z));

                    
                    if(paiMesh->HasTangentsAndBitangents()){
                        const aiVector3D& pTangent = paiMesh->mTangents[i];
                        tangents.push_back(glm::vec3(pTangent.x,pTangent.y,pTangent.z));
                    }
                    

                    const aiVector3D& pTexture = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : zero3D;
                    textCoords.push_back(glm::vec2(pTexture.x,pTexture.y));
                }

                //populate index buffers for this mesh
                for(unsigned int i = 0; i < paiMesh->mNumFaces; i++){
                    const aiFace& face = paiMesh->mFaces[i];
                    assert(face.mNumIndices == 3);
                    indices.push_back(face.mIndices[0]);
                    indices.push_back(face.mIndices[1]);
                    indices.push_back(face.mIndices[2]);
                }
            }

            //now, init all materials of this scene
            initMaterials(pScene, path);


        }else{
            std::cerr << "Error importing file " << path << ": " << importer.GetErrorString() << std::endl;
        }
    }

    void makeObject(Shader shader) {

		//Create the VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        //Create the buffers containing vertices attributes
        glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(buffers), buffers);

        
        glBindBuffer(GL_ARRAY_BUFFER, buffers[POS_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0])* positions.size(), &positions[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION_LOC);
        glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
        
        glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXCOORD_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textCoords[0])* textCoords.size(), &textCoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEXTURECOORD_LOC);
        glVertexAttribPointer(TEXTURECOORD_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
       

        glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0])* normals.size(), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOC);
        glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[TANGENT_VB]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tangents[0])* tangents.size(), &tangents[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TANGENT_LOC);
        glVertexAttribPointer(TANGENT_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0])* indices.size(), &indices[0], GL_STATIC_DRAW);

        //unbind VAO
        glBindVertexArray(0);

		//unbind the buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        this->hasTextureLocation = glGetUniformLocation(shader.ID, "hasTexture");
        this->hasSpecularMapLocation = glGetUniformLocation(shader.ID, "hasSpecularMap");
        this->hasNormalMapLocation = glGetUniformLocation(shader.ID, "hasNormalMap");
    }


	void draw() {
		glBindVertexArray(this->VAO);
		for(unsigned int i=0; i< meshes.size(); i++){
            Material& mat = materials[meshes[i].materialIndex];
                
            if(mat.pDiffuse){
                mat.pDiffuse->bind(GL_TEXTURE0);
                if(hasTextureLocation != -1)
                    glUniform1i(hasTextureLocation, 1);
            }else if(hasTextureLocation != -1)
                glUniform1i(hasTextureLocation, 0);
            
            
            
            if(mat.pSpecularExponent){
                mat.pSpecularExponent->bind(GL_TEXTURE1);
                if(hasSpecularMapLocation != -1)
                    glUniform1i(hasSpecularMapLocation, 1);
            }else if(hasSpecularMapLocation != -1)
                glUniform1i(hasSpecularMapLocation, 0);
            
            
            if(mat.pNormal){ 
                mat.pNormal->bind(GL_TEXTURE2);
                if(hasNormalMapLocation != -1)
                    glUniform1i(hasNormalMapLocation, 1);
            }else if(hasNormalMapLocation != -1)
                glUniform1i(hasNormalMapLocation, 0);
            
           
             glDrawElementsBaseVertex(GL_TRIANGLES,
                                 meshes[i].numIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * meshes[i].baseIndex),
                                 meshes[i].baseVertex);
        }

        // unbind VAO
        glBindVertexArray(0);
	}
private:
    GLuint hasTextureLocation;
    GLuint hasSpecularMapLocation;
    GLuint hasNormalMapLocation;

    void countVerticesAndIndices(const aiScene* pScene, unsigned int& numVertices, unsigned int& numIndices){
        for (unsigned int i = 0 ; i < meshes.size() ; i++) {
            meshes[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
            meshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3;//flag aiProcess_Triangulate force triangles only
            meshes[i].baseVertex = numVertices;
            meshes[i].baseIndex = numIndices;

            numVertices += pScene->mMeshes[i]->mNumVertices;
            numIndices  += meshes[i].numIndices;
        }
    }

    void initMaterials(const aiScene* pScene, const char* path){
      
        //loop over every material
        for(unsigned int i = 0; i < pScene->mNumMaterials; i ++){
            const aiMaterial* pMaterial = pScene->mMaterials[i];
            //load diffuse texture
            if(pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0){

                aiString path;
                if(pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){   
                   
                    std::string p(path.data);
                    std::string::size_type slashIndex = p.find_last_of("/") + 1;
                    std::string fullPath = PATH_TO_OBJECTS  "/textures/" + p.substr(slashIndex);

                    materials[i].pDiffuse = new Texture(fullPath.c_str());
                    if (!materials[i].pDiffuse->load(GL_TEXTURE0)) {
                        std::cout << "Error loading diffuse texture "  << fullPath.c_str() << std::endl;
                    } else {
                        std::cout << "Loaded diffuse texture " << fullPath.c_str() << " at index " << i << std::endl;
                    }
        
                }
            }else{
                std::cout << "No diffuse texture"   << std::endl;
            }
           
            //load normal texture
            if(pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0){

                aiString path;
                if(pMaterial->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){   
                   
                    std::string p(path.data);
                    std::string::size_type slashIndex = p.find_last_of("/") + 1;
                    std::string fullPath = PATH_TO_OBJECTS  "/textures/" + p.substr(slashIndex);

                    materials[i].pNormal = new Texture(fullPath.c_str());
                    if (!materials[i].pNormal->load(GL_TEXTURE1)) {
                        std::cout << "Error loading normal texture "  << fullPath.c_str() << std::endl;
                    } else {
                        std::cout << "Loaded normal texture " << fullPath.c_str() << " at index " << i << std::endl;
                    }
        
                }
            }else{
                std::cout << "No normal texture" << std::endl;
            }

            //load specular map
            if(pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0){

                aiString path;
                if(pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){   
                   
                    std::string p(path.data);
                    std::string::size_type slashIndex = p.find_last_of("/") + 1;
                    std::string fullPath = PATH_TO_OBJECTS  "/textures/" + p.substr(slashIndex);

                    materials[i].pSpecularExponent = new Texture(fullPath.c_str());
                    if (!materials[i].pSpecularExponent->load(GL_TEXTURE2)) {
                        std::cout << "Error loading specular texture "  << fullPath.c_str() << std::endl;
                    } else {
                        std::cout << "Loaded specular texture " << fullPath.c_str() << " at index " << i << std::endl;
                    }
        
                }
            }else{
                std::cout << "No specular texture"   << std::endl;
            }
        }
    }
};

#endif