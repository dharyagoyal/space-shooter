#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_ENEMIES 5
#define MAX_BULLETS 5

// -------- STRUCTS --------
struct Player {
    int x;
    int lives;
};

struct Enemy {
    int x, y;
    int active;
};

struct Bullet {
    int x, y;
    int active;
};

// -------- GLOBALS --------
struct Player player;
struct Enemy enemies[MAX_ENEMIES];
struct Bullet bullets[MAX_BULLETS];

int score = 0;
int highScore = 0;
int gameOver = 0;
int frameCount = 0;

// -------- INPUT HANDLING --------
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

char getch() {
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// -------- FILE HANDLING --------
void loadHighScore() {
    FILE *f = fopen("highscore.txt", "r");
    if (f != NULL) {
        fscanf(f, "%d", &highScore);
        fclose(f);
    }
}

void saveHighScore() {
    FILE *f = fopen("highscore.txt", "w");
    if (f != NULL) {
        fprintf(f, "%d", score);
        fclose(f);
    }
}

// -------- GAME INIT --------
void initGame() {
    player.x = WIDTH / 2;
    player.lives = 3;

    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = 0;

    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;

    score = 0;
    gameOver = 0;
}

// -------- RENDER --------
void render() {
    printf("\033[H\033[J"); // clear screen

    for (int i = 0; i <= WIDTH; i++) printf("-");
    printf("\n");

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int printed = 0;

            if (y == HEIGHT - 1 && x == player.x) {
                printf("^");
                printed = 1;
            }

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active && bullets[i].x == x && bullets[i].y == y) {
                    printf("|");
                    printed = 1;
                }
            }

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active && enemies[i].x == x && enemies[i].y == y) {
                    printf("V");
                    printed = 1;
                }
            }

            if (!printed) printf(" ");
        }
        printf("\n");
    }

    for (int i = 0; i <= WIDTH; i++) printf("-");
    printf("\n");

    printf("Score: %d   High Score: %d   Lives: %d\n", score, highScore, player.lives);
}

// -------- GAME LOGIC --------
void movePlayer(char input) {
    if (input == 'a' && player.x > 0) player.x--;
    if (input == 'd' && player.x < WIDTH - 1) player.x++;
}

void shootBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            bullets[i].x = player.x;
            bullets[i].y = HEIGHT - 2;
            break;
        }
    }
}

void moveBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y--;
            if (bullets[i].y < 0)
                bullets[i].active = 0;
        }
    }
}

void spawnEnemy() {
    if (rand() % 10 < 3) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = 1;
                enemies[i].x = rand() % WIDTH;
                enemies[i].y = 0;
                break;
            }
        }
    }
}

void moveEnemies() {
    if (frameCount % 3 != 0) return; // move only every 3 frames

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].y++;

            if (enemies[i].y == HEIGHT - 1) {
                player.lives--;
                enemies[i].active = 0;

                if (player.lives <= 0)
                    gameOver = 1;
            }
        }
    }
}

void detectCollision() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (bullets[i].active && enemies[j].active &&
                bullets[i].x == enemies[j].x &&
                bullets[i].y == enemies[j].y) {

                bullets[i].active = 0;
                enemies[j].active = 0;
                score++;
            }
        }
    }
}

// -------- MAIN --------
int main() {
    srand(time(0));
    loadHighScore();
    initGame();

    while (!gameOver) {
        if (kbhit()) {
            char ch = getch();
            if (ch == 'a' || ch == 'd')
                movePlayer(ch);
            if (ch == ' ')
                shootBullet();
        }

        moveBullets();
        spawnEnemy();
        if (frameCount % 3 == 0){
            moveEnemies();
        }
        detectCollision();

        render();
        frameCount++;
        usleep(100000);
    }

    printf("\nGAME OVER! Final Score: %d\n", score);

    if (score > highScore) {
        printf("New High Score!\n");
        saveHighScore();
    }

    return 0;
}
