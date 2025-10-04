#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include "game.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Размеры карты Pac-Man
const int N = 21; // Высота (Rows)
const int M = 19; // Ширина (Columns)
const int CELL_SIZE = 30;

Game game(M, N);

void drawCircle(float x, float y, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * i / segments;
        glVertex2f(x + r * cos(theta), y + r * sin(theta));
    }
    glEnd();
}

void drawPacman(float x, float y, float size, float mouthAngle, int dx, int dy) {
    // Рисуем основное тело Пакмана
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Центр Пакмана

    // Определяем начальный угол в зависимости от направления
    float startAngle = 0.0f;
    if (dx == 1) startAngle = 0.0f;      // Вправо
    else if (dx == -1) startAngle = M_PI; // Влево
    else if (dy == 1) startAngle = M_PI / 2.0f; // Вверх
    else if (dy == -1) startAngle = 3.0f * M_PI / 2.0f; // Вниз
    else startAngle = 0.0f; // По умолчанию (если стоит на месте)

    // Рисуем Пакмана с ртом
    const int segments = 50;
    for (int i = 0; i <= segments; i++) {
        float angle = startAngle + mouthAngle + (2.0f * M_PI - 2.0f * mouthAngle) * i / segments;
        glVertex2f(x + size * cos(angle), y + size * sin(angle));
    }
    glEnd();

    // Рисуем глазки
    glColor3f(0.0f, 0.0f, 0.0f); // Черный цвет для глаз

    float eyeOffsetX = 0.0f;
    float eyeOffsetY = 0.0f;
    float eyeSize = size * 0.15f; // Размер глаза

    // Позиция глаза в зависимости от направления
    if (dx == 1) { // Вправо
        eyeOffsetX = size * 0.3f;
        eyeOffsetY = size * 0.3f;
    }
    else if (dx == -1) { // Влево
        eyeOffsetX = -size * 0.3f;
        eyeOffsetY = size * 0.3f;
    }
    else if (dy == 1) { // Вверх
        eyeOffsetX = 0.0f;
        eyeOffsetY = size * 0.4f;
    }
    else if (dy == -1) { // Вниз
        eyeOffsetX = 0.0f;
        eyeOffsetY = -size * 0.2f;
    }
    else { // Стоит на месте (смотрит вправо)
        eyeOffsetX = size * 0.3f;
        eyeOffsetY = size * 0.3f;
    }

    // Рисуем глаз
    drawCircle(x + eyeOffsetX, y + eyeOffsetY, eyeSize, 12);

    // Возвращаем желтый цвет для остальной отрисовки
    glColor3f(1.0f, 1.0f, 0.0f);
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, M * CELL_SIZE, 0, N * CELL_SIZE);
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    const auto& map = game.getMap();
    const auto& grid = map.getGrid();

    // Отрисовка карты
    for (int i = 0; i < map.getHeight(); i++) {
        for (int j = 0; j < map.getWidth(); j++) {
            float x = j * CELL_SIZE;
            float y = i * CELL_SIZE;

            switch (grid[i][j].type) {
            case WALL:
                glColor3f(0.0f, 0.0f, 1.0f);
                glRectf(x, y, x + CELL_SIZE, y + CELL_SIZE);
                break;
            case COIN:
                glColor3f(1.0f, 1.0f, 0.0f);
                drawCircle(x + CELL_SIZE / 2.0f, y + CELL_SIZE / 2.0f, 3, 12);
                break;
            case POWER_POINT:
                glColor3f(1.0f, 1.0f, 1.0f);
                drawCircle(x + CELL_SIZE / 2.0f, y + CELL_SIZE / 2.0f, 6, 16);
                break;
            case EMPTY:
                break;
            }
        }
    }

    // Отрисовка Пакмана с ртом и глазками
    const auto& pacman = game.getPacman();
    glColor3f(1.0f, 1.0f, 0.0f);
    float pacmanX = pacman.getX() * CELL_SIZE + CELL_SIZE / 2.0f;
    float pacmanY = pacman.getY() * CELL_SIZE + CELL_SIZE / 2.0f;
    float pacmanSize = CELL_SIZE / 2.5f;

    drawPacman(pacmanX, pacmanY, pacmanSize,
        pacman.getMouthAngle(),
        pacman.getDirectionX(),
        pacman.getDirectionY());

    // Отрисовка призраков
    for (const auto& ghost : game.getGhosts()) {
        if (ghost.isVulnerable()) {
            if (game.getFlashTimer() < 5) {
                glColor3f(0.0f, 0.0f, 1.0f); // Синий
            }
            else {
                glColor3f(1.0f, 1.0f, 1.0f); // Белый
            }
        }
        else {
            // Разные цвета для призраков
            switch (ghost.getColor()) {
            case RED:    glColor3f(1.0f, 0.0f, 0.0f); break;    // Красный
            case PINK:   glColor3f(1.0f, 0.5f, 0.8f); break;    // Розовый
            case CYAN:   glColor3f(0.0f, 1.0f, 1.0f); break;    // Голубой
            case ORANGE: glColor3f(1.0f, 0.5f, 0.0f); break;    // Оранжевый
            }
        }

        drawCircle(ghost.getX() * CELL_SIZE + CELL_SIZE / 2.0f,
            ghost.getY() * CELL_SIZE + CELL_SIZE / 2.0f,
            CELL_SIZE / 2.5f, 16);
    }

    // Отображение информации
    glColor3f(1.0f, 1.0f, 1.0f);
    const int screenWidth = M * CELL_SIZE;
    const int screenHeight = N * CELL_SIZE;

    drawText(screenWidth / 2.0f - 50, screenHeight - 20, "HIGH SCORE " + std::to_string(game.getHighScore()));
    drawText(10, screenHeight - 20, "SCORE " + std::to_string(game.getScore()));
    drawText(screenWidth - 80, screenHeight - 20, "LEVEL " + std::to_string(game.getLevel()));

    if (!game.isGameStarted()) {
        glColor3f(1.0f, 1.0f, 0.0f);
        drawText(screenWidth / 2.0f - 35, screenHeight / 2.0f, "READY!");
    }

    if (game.isGameOver()) {
        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(screenWidth / 2.0f - 50, screenHeight / 2.0f, "GAME OVER");
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(screenWidth / 2.0f - 80, screenHeight / 2.0f - 30, "Press R to restart");
    }

    if (game.isLevelComplete()) {
        glColor3f(0.0f, 1.0f, 0.0f);
        drawText(screenWidth / 2.0f - 70, screenHeight / 2.0f, "LEVEL COMPLETE!");
        drawText(screenWidth / 2.0f - 100, screenHeight / 2.0f - 30, "Press SPACE to continue");
    }

    glutSwapBuffers();
}

void update(int value) {
    game.update();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key != 27 && key != 'r' && key != 'R') {
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(M * CELL_SIZE, N * CELL_SIZE);
    glutCreateWindow("Pac-Man");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, update, 0);

    std::cout << "Pac-Man Game Started!" << std::endl;
    std::cout << "Move with WASD or Arrow Keys" << std::endl;
    std::cout << "Press R to restart game" << std::endl;

    glutMainLoop();
    return 0;
}