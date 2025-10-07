#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>

#include <iostream>
#include <cmath>
#include "game.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Класс для представления материала
class Material {
public:
    std::string name;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
    float transparency;
    std::string textureFile;

    Material() : shininess(0.0f), transparency(1.0f) {
        ambient[0] = ambient[1] = ambient[2] = 0.2f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.8f;
        specular[0] = specular[1] = specular[2] = 0.0f;
    }
};

static inline std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Улучшенная реализация класса Model для загрузки OBJ с материалами
class Model {
private:
    std::vector<float> vertices;
    std::vector<float> texCoords;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    std::vector<std::string> materialNames;
    std::map<std::string, Material> materials;
    bool hasTextureCoords;
    bool hasNormals;
    bool hasMaterials;

public:
    Model() : hasTextureCoords(false), hasNormals(false), hasMaterials(false) {}

    bool loadMTL(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open MTL file: " << filename << std::endl;
            // Попробуем найти файл в текущей директории
            std::string currentDirFile = filename.substr(filename.find_last_of("/\\") + 1);
            file.open(currentDirFile);
            if (!file.is_open()) {
                std::cerr << "Cannot open MTL file in current directory: " << currentDirFile << std::endl;
                return false;
            }
        }

        Material currentMaterial;
        std::string line;
        int materialsCount = 0;

        while (std::getline(file, line)) {
            line = trim(line);
            // Пропускаем пустые строки и комментарии
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "newmtl") {
                // Сохраняем предыдущий материал и начинаем новый
                if (!currentMaterial.name.empty()) {
                    materials[currentMaterial.name] = currentMaterial;
                    materialsCount++;
                    std::cout << "Saved material: " << currentMaterial.name << std::endl;
                }
                std::string materialName;
                iss >> materialName;
                currentMaterial = Material(); // Сброс к значениям по умолчанию
                currentMaterial.name = materialName;
                std::cout << "Found material: " << currentMaterial.name << std::endl;
            }
            else if (type == "Ka") { // Ambient
                iss >> currentMaterial.ambient[0] >> currentMaterial.ambient[1] >> currentMaterial.ambient[2];
                std::cout << "  Ambient: " << currentMaterial.ambient[0] << " " << currentMaterial.ambient[1] << " " << currentMaterial.ambient[2] << std::endl;
            }
            else if (type == "Kd") { // Diffuse
                iss >> currentMaterial.diffuse[0] >> currentMaterial.diffuse[1] >> currentMaterial.diffuse[2];
                std::cout << "  Diffuse: " << currentMaterial.diffuse[0] << " " << currentMaterial.diffuse[1] << " " << currentMaterial.diffuse[2] << std::endl;
            }
            else if (type == "Ks") { // Specular
                iss >> currentMaterial.specular[0] >> currentMaterial.specular[1] >> currentMaterial.specular[2];
                std::cout << "  Specular: " << currentMaterial.specular[0] << " " << currentMaterial.specular[1] << " " << currentMaterial.specular[2] << std::endl;
            }
            else if (type == "Ns") { // Shininess
                iss >> currentMaterial.shininess;
                std::cout << "  Shininess: " << currentMaterial.shininess << std::endl;
            }
            else if (type == "d" || type == "Tr") { // Transparency
                iss >> currentMaterial.transparency;
                std::cout << "  Transparency: " << currentMaterial.transparency << std::endl;
            }
            else if (type == "map_Kd") { // Diffuse texture
                iss >> currentMaterial.textureFile;
                std::cout << "  Texture: " << currentMaterial.textureFile << std::endl;
            }
            else if (type == "illum") { // Illumination model
                int illum;
                iss >> illum;
                std::cout << "  Illumination: " << illum << std::endl;
            }
            else if (type == "Ni") { // Optical density (игнорируем)
                float ni;
                iss >> ni;
                std::cout << "  Optical density: " << ni << " (ignored)" << std::endl;
            }
            else if (type == "Tf") { // Transmission filter (игнорируем)
                float tf1, tf2, tf3;
                iss >> tf1 >> tf2 >> tf3;
                std::cout << "  Transmission filter: " << tf1 << " " << tf2 << " " << tf3 << " (ignored)" << std::endl;
            }
            else if (type == "Ke") { // Emissive
                float ke1, ke2, ke3;
                iss >> ke1 >> ke2 >> ke3;
                std::cout << "  Emissive: " << ke1 << " " << ke2 << " " << ke3 << " (ignored)" << std::endl;
            }
            else {
                std::cout << "Unknown MTL parameter: " << type << " in line: " << line << std::endl;
            }
        }

        // Не забываем сохранить последний материал
        if (!currentMaterial.name.empty()) {
            materials[currentMaterial.name] = currentMaterial;
            materialsCount++;
            std::cout << "Saved material: " << currentMaterial.name << std::endl;
        }

        file.close();
        std::cout << "Successfully loaded " << materialsCount << " materials from " << filename << std::endl;

        // Отладочная информация о материалах
        std::cout << "=== Loaded Materials ===" << std::endl;
        for (const auto& mat : materials) {
            std::cout << "Material '" << mat.first << "': "
                << "Ka(" << mat.second.ambient[0] << "," << mat.second.ambient[1] << "," << mat.second.ambient[2] << ") "
                << "Kd(" << mat.second.diffuse[0] << "," << mat.second.diffuse[1] << "," << mat.second.diffuse[2] << ") "
                << "Ks(" << mat.second.specular[0] << "," << mat.second.specular[1] << "," << mat.second.specular[2] << ") "
                << "Ns(" << mat.second.shininess << ")" << std::endl;
        }
        std::cout << "========================" << std::endl;

        return !materials.empty();
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open model file: " << filename << std::endl;
            return false;
        }

        vertices.clear();
        texCoords.clear();
        normals.clear();
        indices.clear();
        materialNames.clear();

        std::vector<float> tempVertices;
        std::vector<float> tempTexCoords;
        std::vector<float> tempNormals;
        std::vector<unsigned int> vertexIndices, texIndices, normalIndices;

        std::string currentMaterial = "default";
        std::string line;
        std::string mtlFileToLoad = "";

        while (std::getline(file, line)) {
            line = trim(line);
            // Пропускаем пустые строки и комментарии
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "v") { // Vertex
                float x, y, z;
                iss >> x >> y >> z;
                tempVertices.push_back(x);
                tempVertices.push_back(y);
                tempVertices.push_back(z);
            }
            else if (type == "vt") { // Texture coordinate
                float u, v;
                iss >> u >> v;
                tempTexCoords.push_back(u);
                tempTexCoords.push_back(v);
                hasTextureCoords = true;
            }
            else if (type == "vn") { // Normal
                float x, y, z;
                iss >> x >> y >> z;
                tempNormals.push_back(x);
                tempNormals.push_back(y);
                tempNormals.push_back(z);
                hasNormals = true;
            }
            else if (type == "f") { // Face
                std::string v1, v2, v3;
                iss >> v1 >> v2 >> v3;

                processFaceVertex(v1, tempVertices.size() / 3, vertexIndices, texIndices, normalIndices);
                processFaceVertex(v2, tempVertices.size() / 3, vertexIndices, texIndices, normalIndices);
                processFaceVertex(v3, tempVertices.size() / 3, vertexIndices, texIndices, normalIndices);

                // Сохраняем материал для этого треугольника
                materialNames.push_back(currentMaterial);
            }
            else if (type == "usemtl") { // Use material
                iss >> currentMaterial;
                hasMaterials = true;
                std::cout << "Using material: " << currentMaterial << std::endl;
            }
            else if (type == "mtllib") { // Material library
                std::string mtlFile;
                iss >> mtlFile;
                mtlFileToLoad = mtlFile;
                // Загружаем MTL файл (предполагаем, что он в той же директории)
                if (!mtlFileToLoad.empty()) {
                    std::cout << "Loading MTL file: " << mtlFileToLoad << std::endl;
                    if (!loadMTL(mtlFileToLoad)) {
                        // Попробуем загрузить из той же директории что и OBJ файл
                        std::string objPath = filename.substr(0, filename.find_last_of("/\\") + 1);
                        std::string fullMtlPath = objPath + mtlFileToLoad;
                        std::cout << "Trying alternative path: " << fullMtlPath << std::endl;
                        loadMTL(fullMtlPath);
                    }
                }
            }
        }

        file.close();

        // Process the data for rendering
        for (unsigned int i = 0; i < vertexIndices.size(); i++) {
            unsigned int vertexIndex = vertexIndices[i];

            vertices.push_back(tempVertices[vertexIndex * 3]);
            vertices.push_back(tempVertices[vertexIndex * 3 + 1]);
            vertices.push_back(tempVertices[vertexIndex * 3 + 2]);

            if (hasTextureCoords && i < texIndices.size()) {
                unsigned int texIndex = texIndices[i];
                texCoords.push_back(tempTexCoords[texIndex * 2]);
                texCoords.push_back(tempTexCoords[texIndex * 2 + 1]);
            }

            if (hasNormals && i < normalIndices.size()) {
                unsigned int normalIndex = normalIndices[i];
                normals.push_back(tempNormals[normalIndex * 3]);
                normals.push_back(tempNormals[normalIndex * 3 + 1]);
                normals.push_back(tempNormals[normalIndex * 3 + 2]);
            }

            indices.push_back(i);
        }

        std::cout << "Model loaded: " << vertices.size() / 3 << " vertices, "
            << indices.size() / 3 << " triangles, " << materials.size() << " materials" << std::endl;

        // Проверяем, используются ли материалы
        if (hasMaterials && !materialNames.empty()) {
            std::cout << "First material used: " << materialNames[0] << std::endl;
            // Проверяем, есть ли этот материал в загруженных
            if (materials.find(materialNames[0]) == materials.end()) {
                std::cout << "WARNING: Material '" << materialNames[0] << "' not found in loaded materials!" << std::endl;
            }
        }

        return true;
    }

    void processFaceVertex(const std::string& vertex, unsigned int vertexCount,
        std::vector<unsigned int>& vertexIndices,
        std::vector<unsigned int>& texIndices,
        std::vector<unsigned int>& normalIndices) {
        std::istringstream viss(vertex);
        std::string v, t, n;

        std::getline(viss, v, '/');
        std::getline(viss, t, '/');
        std::getline(viss, n, '/');

        unsigned int vi = std::stoi(v) - 1;
        vertexIndices.push_back(vi);

        if (!t.empty()) {
            unsigned int ti = std::stoi(t) - 1;
            texIndices.push_back(ti);
        }

        if (!n.empty()) {
            unsigned int ni = std::stoi(n) - 1;
            normalIndices.push_back(ni);
        }
    }

    void render() const {
        if (hasMaterials && !materials.empty()) {
            renderWithMaterials();
        }
        else {
            renderSimple();
        }
    }

    void renderWithMaterials() const {
        if (materialNames.empty() || indices.empty()) {
            renderSimple();
            return;
        }

        // Группируем треугольники по материалам
        std::map<std::string, std::vector<unsigned int>> materialGroups;

        for (size_t i = 0; i < indices.size(); i += 3) {
            size_t triangleIndex = i / 3;
            std::string material = (triangleIndex < materialNames.size()) ?
                materialNames[triangleIndex] : "default";
            materialGroups[material].push_back(indices[i]);
            materialGroups[material].push_back(indices[i + 1]);
            materialGroups[material].push_back(indices[i + 2]);
        }

        // Рендерим каждую группу с соответствующим материалом
        for (const auto& group : materialGroups) {
            applyMaterial(group.first);

            glBegin(GL_TRIANGLES);
            for (unsigned int index : group.second) {
                if (hasNormals && index * 3 + 2 < normals.size()) {
                    glNormal3f(normals[index * 3], normals[index * 3 + 1], normals[index * 3 + 2]);
                }

                if (hasTextureCoords && index * 2 + 1 < texCoords.size()) {
                    glTexCoord2f(texCoords[index * 2], texCoords[index * 2 + 1]);
                }

                if (index * 3 + 2 < vertices.size()) {
                    glVertex3f(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]);
                }
            }
            glEnd();
        }
    }

    void renderSimple() const {
        // Простой рендеринг без материалов
        glColor3f(1.0f, 1.0f, 0.0f); // Желтый цвет для Пакмана
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < indices.size(); i++) {
            unsigned int index = indices[i];

            if (hasNormals && index * 3 + 2 < normals.size()) {
                glNormal3f(normals[index * 3], normals[index * 3 + 1], normals[index * 3 + 2]);
            }

            if (hasTextureCoords && index * 2 + 1 < texCoords.size()) {
                glTexCoord2f(texCoords[index * 2], texCoords[index * 2 + 1]);
            }

            if (index * 3 + 2 < vertices.size()) {
                glVertex3f(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]);
            }
        }
        glEnd();
    }

    void applyMaterial(const std::string& materialName) const {
        auto it = materials.find(materialName);
        if (it != materials.end()) {
            const Material& mat = it->second;

            GLfloat ambient[] = { mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.0f };
            GLfloat diffuse[] = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f };
            GLfloat specular[] = { mat.specular[0], mat.specular[1], mat.specular[2], 1.0f };

            glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
            glMaterialf(GL_FRONT, GL_SHININESS, mat.shininess);

            // Также устанавливаем цвет для совместимости
            glColor3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        }
        else {
            // Материал по умолчанию - желтый как оригинальный Пакман
            GLfloat default_ambient[] = { 0.8f, 0.8f, 0.0f, 1.0f };
            GLfloat default_diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
            GLfloat default_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };

            glMaterialfv(GL_FRONT, GL_AMBIENT, default_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, default_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, default_specular);
            glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

            glColor3f(1.0f, 1.0f, 0.0f);

        }
    }

    void scale(float scaleFactor) {
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i] *= scaleFactor;
        }
    }
};

const int N = 21;
const int M = 19;
const float CELL_SIZE_3D = 2.0f;

Game game(M, N);

// Глобальная переменная для модели Пакмана
Model pacmanModel;
bool pacmanModelLoaded = false;

class Camera {
public:
    float eyeX, eyeY, eyeZ;
    float centerX, centerY, centerZ;
    float upX, upY, upZ;
    bool followMode;
    float angleY; // Угол поворота камеры вокруг вертикальной оси

    Camera() {
        followMode = true;
        angleY = 0.0f; // Начальный угол
        resetToMapView();
    }

    void resetToMapView() {
        // Камера БЛИЖЕ к центру карты
        float baseX = M * CELL_SIZE_3D / 2.0f;
        float baseZ = -15.0f; // ЕЩЕ БЛИЖЕ к карте (было -25.0f)

        // Применяем поворот вокруг вертикальной оси Y
        eyeX = baseX + 15.0f * sin(angleY * M_PI / 180.0f); // Ближе (было 25.0f)
        eyeY = 40.0f; // НИЖЕ над картой для лучшего обзора центра (было 60.0f)
        eyeZ = baseZ + 15.0f * cos(angleY * M_PI / 180.0f); // Ближе (было 25.0f)

        centerX = M * CELL_SIZE_3D / 2.0f;
        centerY = 0.0f;
        centerZ = N * CELL_SIZE_3D / 2.0f;

        upX = 0; upY = 1; upZ = 0; // Вертикальная ось
    }

    void followPacman(const Pacman& pacman) {
        if (!followMode) return;

        float pacmanX3D = pacman.getX() * CELL_SIZE_3D;
        float pacmanZ3D = (N - pacman.getY()) * CELL_SIZE_3D; // ИНВЕРСИЯ Y

        // Камера следует за Пакманом - БЛИЖЕ к нему
        eyeX = pacmanX3D + 12.0f * sin(angleY * M_PI / 180.0f); // Ближе (было 20.0f)
        eyeY = 25.0f; // НИЖЕ над Пакманом (было 45.0f)
        eyeZ = pacmanZ3D + 10.0f * cos(angleY * M_PI / 180.0f); // Ближе (было 15.0f)

        centerX = pacmanX3D;
        centerY = 0.0f;
        centerZ = pacmanZ3D;
    }

    void rotate(float deltaAngle) {
        angleY += deltaAngle;
        if (angleY > 360.0f) angleY -= 360.0f;
        if (angleY < 0.0f) angleY += 360.0f;
    }

    void toggleView() {
        followMode = !followMode;
        if (!followMode) {
            resetToMapView();
        }
    }
};

Camera camera;


void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Основной источник света (как солнце)
    GLfloat light0_position[] = { M * CELL_SIZE_3D / 2.0f, 30.0f, N * CELL_SIZE_3D / 2.0f, 1.0f };
    GLfloat light0_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light0_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light0_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Заполняющий свет (рассеянный)
    GLfloat light1_position[] = { 0.0f, 20.0f, 0.0f, 1.0f };
    GLfloat light1_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light1_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };

    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

    // Настройки материала
    GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// Рисуем видимую лампочку (источник света)
void drawLightBulb(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Временно отключаем освещение для самой лампочки
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.8f); // Светло-желтый
    glutSolidSphere(0.5f, 16, 16);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

// Рисуем полную 3D коробку (стены со всеми гранями)
void drawCube(float x, float y, float z, float width, float height, float depth) {
    float hw = width / 2;
    float hh = height / 2;
    float hd = depth / 2;

    glBegin(GL_QUADS);

    // Передняя грань
    glColor3f(0.0f, 0.0f, 1.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x - hw, y - hh, z + hd);
    glVertex3f(x + hw, y - hh, z + hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x - hw, y + hh, z + hd);

    // Задняя грань
    glColor3f(0.0f, 0.0f, 0.8f);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x - hw, y + hh, z - hd);
    glVertex3f(x + hw, y + hh, z - hd);
    glVertex3f(x + hw, y - hh, z - hd);

    // Верхняя грань
    glColor3f(0.3f, 0.3f, 1.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x - hw, y + hh, z - hd);
    glVertex3f(x - hw, y + hh, z + hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x + hw, y + hh, z - hd);

    // Нижняя грань (ДНО)
    glColor3f(0.1f, 0.1f, 0.6f);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x + hw, y - hh, z - hd);
    glVertex3f(x + hw, y - hh, z + hd);
    glVertex3f(x - hw, y - hh, z + hd);

    // Левая грань
    glColor3f(0.2f, 0.2f, 0.9f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x - hw, y - hh, z + hd);
    glVertex3f(x - hw, y + hh, z + hd);
    glVertex3f(x - hw, y + hh, z - hd);

    // Правая грань
    glColor3f(0.2f, 0.2f, 0.9f);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x + hw, y - hh, z - hd);
    glVertex3f(x + hw, y + hh, z - hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x + hw, y - hh, z + hd);

    glEnd();
}

// Рисуем пол (основание карты)
void drawFloor() {
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-5.0f, -1.0f, -5.0f);
    glVertex3f(M * CELL_SIZE_3D + 5.0f, -1.0f, -5.0f);
    glVertex3f(M * CELL_SIZE_3D + 5.0f, -1.0f, N * CELL_SIZE_3D + 5.0f);
    glVertex3f(-5.0f, -1.0f, N * CELL_SIZE_3D + 5.0f);
    glEnd();
}

// Улучшенная сфера с нормалями
void drawSphere(float x, float y, float z, float radius, int segments) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(radius, segments, segments);
    glPopMatrix();
}

// Улучшенный 3D Пакман с OBJ моделью и материалами
void drawPacman3D(float x, float y, float z, float size, float mouthAngle, float rotationY) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotationY, 0, 1, 0);
    glScalef(size, size, size);

    if (pacmanModelLoaded) {
        pacmanModel.render();
    }
    else {
        // Fallback: стандартная сфера с желтым материалом
        GLfloat yellow_ambient[] = { 0.8f, 0.8f, 0.0f, 1.0f };
        GLfloat yellow_diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        GLfloat yellow_specular[] = { 1.0f, 1.0f, 0.8f, 1.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, yellow_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, yellow_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

        glutSolidSphere(1.0f, 16, 16);
    }

    glPopMatrix();
}

// Улучшенный 3D Призрак
void drawGhost3D(float x, float y, float z, float size, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, size, z);

    glColor3f(r, g, b);
    glutSolidSphere(size, 12, 12);

    glPopMatrix();
}

void drawMap3D() {
    const auto& map = game.getMap();
    const auto& grid = map.getGrid();

    // Сначала рисуем пол
    drawFloor();

    // Затем стены и объекты с ИНВЕРТИРОВАННОЙ Z координатой
    for (int i = 0; i < map.getHeight(); i++) {
        for (int j = 0; j < map.getWidth(); j++) {
            float x = j * CELL_SIZE_3D;
            float z = (N - i) * CELL_SIZE_3D; // ИНВЕРСИЯ Y в Z

            switch (grid[i][j].type) {
            case WALL:
                drawCube(x, 1.0f, z, 1.8f, 2.0f, 1.8f);
                break;
            case COIN:
                glColor3f(1.0f, 1.0f, 0.0f);
                drawSphere(x, 0.5f, z, 0.2f, 8);
                break;
            case POWER_POINT:
                glColor3f(1.0f, 1.0f, 1.0f);
                drawSphere(x, 0.8f, z, 0.3f, 12);
                break;
            case EMPTY:
                // Пустое пространство
                break;
            }
        }
    }
}

// 2D текст поверх 3D сцены
void drawText(float x, float y, const std::string& text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1200, 0, 800);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // УВЕЛИЧИВАЕМ поле обзора чтобы видеть БОЛЬШЕ карты (было 60.0)
    gluPerspective(75.0, 1200.0 / 800.0, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.eyeX, camera.eyeY, camera.eyeZ,
        camera.centerX, camera.centerY, camera.centerZ,
        camera.upX, camera.upY, camera.upZ);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Настраиваем освещение
    setupLighting();

    // Настраиваем 3D камеру
    setupCamera();

    // Рисуем видимые лампочки (источники света)
    drawLightBulb(M * CELL_SIZE_3D / 2.0f, 30.0f, N * CELL_SIZE_3D / 2.0f);
    drawLightBulb(0.0f, 20.0f, 0.0f);

    // Рендерим 3D мир
    drawMap3D();

    // Рендерим Пакмана с ИНВЕРТИРОВАННОЙ Z координатой
    const auto& pacman = game.getPacman();
    float pacmanX = pacman.getX() * CELL_SIZE_3D;
    float pacmanZ = (N - pacman.getY()) * CELL_SIZE_3D; // ИНВЕРСИЯ Y в Z

    drawPacman3D(pacmanX, 1.0f, pacmanZ, 0.8f, pacman.getMouthAngle(), pacman.getRotationY());

    // Рендерим призраков с ИНВЕРТИРОВАННОЙ Z координатой
    for (const auto& ghost : game.getGhosts()) {
        float ghostX = ghost.getX() * CELL_SIZE_3D;
        float ghostZ = (N - ghost.getY()) * CELL_SIZE_3D; // ИНВЕРСИЯ Y в Z

        float r = 1.0f, g = 0.0f, b = 0.0f;
        switch (ghost.getColor()) {
        case RED:r = 1.0f; g = 0.0f; b = 0.0f; break;
        case PINK:r = 1.0f; g = 0.5f; b = 0.8f; break;
        case CYAN:r = 0.0f; g = 1.0f; b = 1.0f; break;
        case ORANGE: r = 1.0f; g = 0.5f; b = 0.0f; break;
        }

        if (ghost.isVulnerable()) {
            r = 0.0f; g = 0.0f; b = 1.0f;
        }

        drawGhost3D(ghostX, 0, ghostZ, 0.7f, r, g, b);
    }

    // 2D UI поверх всего
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    drawText(10, 750, "SCORE: " + std::to_string(game.getScore()));
    drawText(500, 750, "HIGH SCORE: " + std::to_string(game.getHighScore()));
    drawText(1000, 750, "LEVEL: " + std::to_string(game.getLevel()));
    drawText(10, 730, "Press 'C' to toggle camera");

    if (!game.isGameStarted()) {
        drawText(550, 400, "READY!");
    }

    if (game.isGameOver()) {
        drawText(520, 400, "GAME OVER");
        drawText(480, 370, "Press R to restart");
    }

    if (game.isLevelComplete()) {
        drawText(500, 400, "LEVEL COMPLETE!");
        drawText(470, 370, "Press SPACE to continue");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void update(int value) {
    game.update();
    camera.followPacman(game.getPacman());
    glutPostRedisplay();
    glutTimerFunc(1, update, 0);
}

// Улучшенное управление с переключением камеры
void keyboard(unsigned char key, int x, int y) {
    if (key != 27 && key != 'r' && key != 'R' && key != 'c' && key != 'C' && key != 'q' && key != 'e') {
        game.startGame();
    }

    switch (key) {
    case 'w': case 'W': game.setPacmanDirection(0, 1); break;
    case 's': case 'S': game.setPacmanDirection(0, -1); break;
    case 'a': case 'A': game.setPacmanDirection(-1, 0); break;
    case 'd': case 'D': game.setPacmanDirection(1, 0); break;
    case ' ': if (game.isLevelComplete()) game.nextLevel(); break;
    case 'r': case 'R': game.restart(); break;
    case 'c': case 'C': camera.toggleView(); break;
    case 27: exit(0); break;
    }
}

void specialKeys(int key, int x, int y) {
    game.startGame();
    switch (key) {
    case GLUT_KEY_UP: game.setPacmanDirection(0, 1); break;
    case GLUT_KEY_DOWN: game.setPacmanDirection(0, -1); break;
    case GLUT_KEY_LEFT: game.setPacmanDirection(-1, 0); break;
    case GLUT_KEY_RIGHT: game.setPacmanDirection(1, 0); break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Pac-Man 3D with OBJ Model and Materials");

    // Загружаем модель Пакмана
    pacmanModelLoaded = pacmanModel.loadFromFile("pacman.obj");
    if (pacmanModelLoaded) {
        pacmanModel.scale(1.0f);
        std::cout << "Pacman OBJ model with materials loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to load Pacman OBJ model, using default sphere." << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(10, update, 0);

    std::cout << "Pac-Man 3D with OBJ Model and Materials Started!" << std::endl;
    std::cout << "Move with WASD or Arrow Keys" << std::endl;
    std::cout << "Press 'R' to restart game" << std::endl;
    std::cout << "Press 'ESC' to exit" << std::endl;

    glutMainLoop();
    return 0;
}