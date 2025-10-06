#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include "game.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <string>

const int N = 21;
const int M = 19;
const float CELL_SIZE_3D = 2.0f;

Game game(M, N);

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

// Функция для настройки освещения
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

// Улучшенный 3D Пакман
void drawPacman3D(float x, float y, float z, float size, float mouthAngle, float rotationY) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotationY, 0, 1, 0);

    glColor3f(1.0f, 1.0f, 0.0f);

    // Используем GLUT для более качественной сферы
    glutSolidSphere(size, 16, 16);

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
        case RED:    r = 1.0f; g = 0.0f; b = 0.0f; break;
        case PINK:   r = 1.0f; g = 0.5f; b = 0.8f; break;
        case CYAN:   r = 0.0f; g = 1.0f; b = 1.0f; break;
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
    glutTimerFunc(16, update, 0);
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
    glutCreateWindow("Pac-Man 3D with Lighting - Centered Camera");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, update, 0);

    std::cout << "Pac-Man 3D with Centered Camera Started!" << std::endl;
    std::cout << "Move with WASD or Arrow Keys" << std::endl;
    std::cout << "Press 'R' to restart game" << std::endl;
    std::cout << "Press 'ESC' to exit" << std::endl;

    glutMainLoop();
    return 0;
}