#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
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

// Вспомогательные функции для работы с матрицами
aiMatrix4x4 multiplyMatrices(const aiMatrix4x4& a, const aiMatrix4x4& b) {
    aiMatrix4x4 result;
    for (unsigned int i = 0; i < 4; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            result[i][j] = 0.0f;
            for (unsigned int k = 0; k < 4; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

aiVector3D transformVector(const aiMatrix4x4& matrix, const aiVector3D& vector) {
    aiVector3D result;
    result.x = matrix.a1 * vector.x + matrix.a2 * vector.y + matrix.a3 * vector.z + matrix.a4;
    result.y = matrix.b1 * vector.x + matrix.b2 * vector.y + matrix.b3 * vector.z + matrix.b4;
    result.z = matrix.c1 * vector.x + matrix.c2 * vector.y + matrix.c3 * vector.z + matrix.c4;
    return result;
}

// Упрощенный класс для загрузки 3D моделей с использованием Assimp
class SimpleModel3DS {
private:
    const aiScene* scene;
    bool loaded;
    float scaleFactor;
    Assimp::Importer importer; // <-- ДОБАВЬ ЭТУ СТРОКУ. Импортер теперь живет вместе с объектом.

public:
    SimpleModel3DS() : scene(nullptr), loaded(false), scaleFactor(1.0f) {}

    bool loadFromFile(const std::string& filename) {
        // Assimp::Importer importer; // <-- УДАЛИ ЭТУ ЛОКАЛЬНУЮ ПЕРЕМЕННУЮ

        // Теперь используется член класса 'importer', который не будет уничтожен
        scene = importer.ReadFile(filename,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_JoinIdenticalVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        loaded = true;
        calculateSimpleScale();

        std::cout << "Model loaded: " << filename << std::endl;
        std::cout << "Meshes: " << scene->mNumMeshes << ", Materials: " << scene->mNumMaterials << std::endl;

        return true;
    }

    void render() const {
        if (!loaded || !scene) return;

        glPushMatrix();
        glScalef(scaleFactor, scaleFactor, scaleFactor);

        // Проходим по всем мешам (частям) модели
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            const aiMesh* mesh = scene->mMeshes[m];

            // Получаем индекс материала для этого меша
            unsigned int materialIndex = mesh->mMaterialIndex;
            if (materialIndex < scene->mNumMaterials) {
                // Получаем сам материал из сцены
                const aiMaterial* material = scene->mMaterials[materialIndex];

                // Получаем диффузный (основной) цвет из материала
                aiColor4D diffuseColor;
                if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor) == AI_SUCCESS) {
                    // Устанавливаем этот цвет как материал в OpenGL
                    GLfloat color[] = { diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
                }

                // Можно также получить и другие свойства, например, фоновый (ambient)
                aiColor4D ambientColor;
                if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor) == AI_SUCCESS) {
                    GLfloat color[] = { ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };
                    glMaterialfv(GL_FRONT, GL_AMBIENT, color);
                }
            }

            // Теперь отрисовываем меш с уже установленным для него материалом
            renderSimpleMesh(mesh);
        }

        glPopMatrix();
    }

private:
    void calculateSimpleScale() {
        // Простое вычисление масштаба
        if (scene->mNumMeshes > 0) {
            const aiMesh* mesh = scene->mMeshes[0];
            float maxSize = 0.0f;

            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                const aiVector3D& vertex = mesh->mVertices[i];
                maxSize = std::max(maxSize, std::abs(vertex.x));
                maxSize = std::max(maxSize, std::abs(vertex.y));
                maxSize = std::max(maxSize, std::abs(vertex.z));
            }

            if (maxSize > 0.0f) {
                scaleFactor = 1.0f / maxSize;
            }
        }
    }

    void renderSimpleMesh(const aiMesh* mesh) const {
        // Рендерим треугольники
        glBegin(GL_TRIANGLES);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                unsigned int index = face.mIndices[j];

                // Нормали
                if (mesh->HasNormals()) {
                    glNormal3f(mesh->mNormals[index].x,
                        mesh->mNormals[index].y,
                        mesh->mNormals[index].z);
                }

                // Вершины
                glVertex3f(mesh->mVertices[index].x,
                    mesh->mVertices[index].y,
                    mesh->mVertices[index].z);
            }
        }
        glEnd();
    }
};

const int N = 21;
const int M = 19;
const float CELL_SIZE_3D = 2.0f;

Game game(M, N);

// Глобальные переменные для моделей
SimpleModel3DS pacmanModel;
bool pacmanModelLoaded = false;
SimpleModel3DS ghostModel;
bool ghostModelLoaded = false;

class Camera {
public:
    float eyeX, eyeY, eyeZ;
    float centerX, centerY, centerZ;
    float upX, upY, upZ;
    bool followMode;
    float angleY;

    Camera() {
        followMode = true;
        angleY = 0.0f;
        resetToMapView();
    }

    void resetToMapView() {
        float baseX = M * CELL_SIZE_3D / 2.0f;
        float baseZ = -15.0f;

        eyeX = baseX + 15.0f * sin(angleY * M_PI / 180.0f);
        eyeY = 40.0f;
        eyeZ = baseZ + 15.0f * cos(angleY * M_PI / 180.0f);

        centerX = M * CELL_SIZE_3D / 2.0f;
        centerY = 0.0f;
        centerZ = N * CELL_SIZE_3D / 2.0f;

        upX = 0; upY = 1; upZ = 0;
    }

    void followPacman(const Pacman& pacman) {
        if (!followMode) return;

        float pacmanX3D = pacman.getX() * CELL_SIZE_3D;
        float pacmanZ3D = (N - pacman.getY()) * CELL_SIZE_3D;

        eyeX = pacmanX3D + 12.0f * sin(angleY * M_PI / 180.0f);
        eyeY = 25.0f;
        eyeZ = pacmanZ3D + 10.0f * cos(angleY * M_PI / 180.0f);

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

// Класс для сохранения и восстановления материалов
class MaterialSaver {
private:
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess;

public:
    MaterialSaver() {
        glGetMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
        glGetMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glGetMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glGetMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
    }

    ~MaterialSaver() {
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    }
};

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    /*glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);*/

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



void drawCube(float x, float y, float z, float width, float height, float depth) {
    MaterialSaver saver;
    float hw = width / 2;
    float hh = height / 2;
    float hd = depth / 2;

    // Материал для стен
    GLfloat wall_ambient[] = { 0.1f, 0.1f, 0.4f, 1.0f };
    GLfloat wall_diffuse[] = { 0.2f, 0.2f, 0.8f, 1.0f };
    GLfloat wall_specular[] = { 0.3f, 0.3f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, wall_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, wall_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wall_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);

    glBegin(GL_QUADS);
    // Передняя грань
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x - hw, y - hh, z + hd);
    glVertex3f(x + hw, y - hh, z + hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x - hw, y + hh, z + hd);

    // Задняя грань
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x - hw, y + hh, z - hd);
    glVertex3f(x + hw, y + hh, z - hd);
    glVertex3f(x + hw, y - hh, z - hd);

    // Верхняя грань
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x - hw, y + hh, z - hd);
    glVertex3f(x - hw, y + hh, z + hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x + hw, y + hh, z - hd);

    // Нижняя грань
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x + hw, y - hh, z - hd);
    glVertex3f(x + hw, y - hh, z + hd);
    glVertex3f(x - hw, y - hh, z + hd);

    // Левая грань
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x - hw, y - hh, z - hd);
    glVertex3f(x - hw, y - hh, z + hd);
    glVertex3f(x - hw, y + hh, z + hd);
    glVertex3f(x - hw, y + hh, z - hd);

    // Правая грань
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x + hw, y - hh, z - hd);
    glVertex3f(x + hw, y + hh, z - hd);
    glVertex3f(x + hw, y + hh, z + hd);
    glVertex3f(x + hw, y - hh, z + hd);
    glEnd();
}

void drawFloor() {
    MaterialSaver saver;
    GLfloat floor_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat floor_diffuse[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, floor_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, floor_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-5.0f, -1.0f, -5.0f);
    glVertex3f(M * CELL_SIZE_3D + 5.0f, -1.0f, -5.0f);
    glVertex3f(M * CELL_SIZE_3D + 5.0f, -1.0f, N * CELL_SIZE_3D + 5.0f);
    glVertex3f(-5.0f, -1.0f, N * CELL_SIZE_3D + 5.0f);
    glEnd();
}

void drawSphere(float x, float y, float z, float radius, int segments) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(radius, segments, segments);
    glPopMatrix();
}

void drawPacman3D(float x, float y, float z, float size, float mouthAngle, float rotationY) {
    MaterialSaver saver; // Оставляем, чтобы сохранить состояние материала ДО отрисовки модели
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotationY, 0, 1, 0);

   

    if (pacmanModelLoaded) {
        pacmanModel.render();
    }
    else {
        // Fallback можно оставить и задать ему цвет здесь, если нужно
        GLfloat yellow_diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow_diffuse);
        glutSolidSphere(size, 16, 16);
    }

    glPopMatrix();
}

void drawGhost3D(float x, float y, float z, float size, GhostColor color, bool isVulnerable, int ghostIndex) {
    MaterialSaver saver;
    glPushMatrix();
    glTranslatef(x, y + size * 0.5f, z);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    float r, g, b;
    if (isVulnerable) {
        r = 0.0f; g = 0.0f; b = 1.0f; // Синий для уязвимых
    }
    else {
        switch (color) {
        case RED: r = 1.0f; g = 0.0f; b = 0.0f; break;
        case PINK: r = 1.0f; g = 0.5f; b = 0.8f; break;
        case CYAN: r = 0.0f; g = 1.0f; b = 1.0f; break;
        case ORANGE: r = 1.0f; g = 0.5f; b = 0.0f; break;
        default: r = 1.0f; g = 0.0f; b = 0.0f; break;
        }
    }

    // Материал для призрака
    GLfloat ghost_ambient[] = { r * 0.4f, g * 0.4f, b * 0.4f, 1.0f };
    GLfloat ghost_diffuse[] = { r, g, b, 1.0f };
    GLfloat ghost_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ghost_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, ghost_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ghost_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.0f);

    if (ghostModelLoaded) {
        ghostModel.render();
    }
    else {
        // Fallback: стандартная сфера
        glutSolidSphere(size, 16, 16);
    }

    glPopMatrix();
}

void drawMap3D() {
    const auto& map = game.getMap();
    const auto& grid = map.getGrid();

    drawFloor();

    for (int i = 0; i < map.getHeight(); i++) {
        for (int j = 0; j < map.getWidth(); j++) {
            float x = j * CELL_SIZE_3D;
            float z = (N - i) * CELL_SIZE_3D;

            switch (grid[i][j].type) {
            case WALL:
                drawCube(x, 1.0f, z, 1.8f, 2.0f, 1.8f);
                break;
            case COIN: {
                MaterialSaver saver;
                GLfloat coin_ambient[] = { 0.8f, 0.8f, 0.0f, 1.0f };
                GLfloat coin_diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
                GLfloat coin_specular[] = { 1.0f, 1.0f, 0.5f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT, coin_ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, coin_diffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, coin_specular);
                glMaterialf(GL_FRONT, GL_SHININESS, 30.0f);
                drawSphere(x, 0.5f, z, 0.2f, 8);
                break;
            }
            case POWER_POINT: {
                MaterialSaver saver;
                GLfloat power_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
                GLfloat power_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                GLfloat power_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                glMaterialfv(GL_FRONT, GL_AMBIENT, power_ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, power_diffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, power_specular);
                glMaterialf(GL_FRONT, GL_SHININESS, 60.0f);
                drawSphere(x, 0.8f, z, 0.3f, 12);
                break;
            }
            case EMPTY:
                break;
            }
        }
    }
}

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
    gluPerspective(75.0, 1200.0 / 800.0, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.eyeX, camera.eyeY, camera.eyeZ,
        camera.centerX, camera.centerY, camera.centerZ,
        camera.upX, camera.upY, camera.upZ);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setupLighting();
    setupCamera();

    glEnable(GL_DEPTH_TEST);

    drawLightBulb(M * CELL_SIZE_3D / 2.0f, 30.0f, N * CELL_SIZE_3D / 2.0f);
    drawLightBulb(0.0f, 20.0f, 0.0f);

    drawMap3D();

    const auto& pacman = game.getPacman();
    float pacmanX = pacman.getX() * CELL_SIZE_3D;
    float pacmanZ = (N - pacman.getY()) * CELL_SIZE_3D;

    drawPacman3D(pacmanX, 1.0f, pacmanZ, 0.8f, pacman.getMouthAngle(), pacman.getRotationY());

    int ghostIndex = 0;
    for (const auto& ghost : game.getGhosts()) {
        float ghostX = ghost.getX() * CELL_SIZE_3D;
        float ghostZ = (N - ghost.getY()) * CELL_SIZE_3D;
        drawGhost3D(ghostX, 0, ghostZ, 0.7f, ghost.getColor(), ghost.isVulnerable(), ghostIndex);
        ghostIndex++;
    }

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
    glutTimerFunc(10, update, 0);
}

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
    glutCreateWindow("Pac-Man 3D with Assimp Models");

    // Загружаем модели через Assimp
    std::cout << "--- Loading Pacman Model ---" << std::endl;
    pacmanModelLoaded = pacmanModel.loadFromFile("pacman.3ds");
    if (pacmanModelLoaded) {
        std::cout << "Pacman 3DS model loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to load Pacman 3DS model, using default sphere." << std::endl;
    }

    std::cout << "\n--- Loading Ghost Model ---" << std::endl;
    ghostModelLoaded = ghostModel.loadFromFile("ghost.3ds");
    if (ghostModelLoaded) {
        std::cout << "Ghost 3DS model loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to load Ghost 3DS model, using default sphere." << std::endl;
    }

    std::cout << "\n---------------------------\n" << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(10, update, 0);

    std::cout << "Pac-Man 3D with Assimp Models Started!" << std::endl;
    std::cout << "Move with WASD or Arrow Keys" << std::endl;
    std::cout << "Press 'R' to restart game" << std::endl;
    std::cout << "Press 'ESC' to exit" << std::endl;

    glutMainLoop();
    return 0;
}