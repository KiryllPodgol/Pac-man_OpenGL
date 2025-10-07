// model.h
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>

struct Vertex {
    float x, y, z;
};

struct TexCoord {
    float u, v;
};

struct Normal {
    float x, y, z;
};

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texCoordIndices;
    std::vector<int> normalIndices;
};

class Model {
private:
    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    bool hasTextureCoords;
    bool hasNormals;

public:
    Model();
    bool loadFromFile(const std::string& filename);
    void render() const;
    void scale(float scaleFactor);
};

#endif