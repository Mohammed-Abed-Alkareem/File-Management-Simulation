#include "GUI.h"
#include <GL/glut.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

// Data structures and global variables
float maxTime;
float elapsedTime = 0.0f;
SharedData *shared_data;
Config config;
int shm_data_id;

// Function prototypes
void handle_signal(int signal);
void renderText(float x, float y, const char *text, void *font, float r, float g, float b);
void drawRectangle(float x1, float y1, float x2, float y2, float r, float g, float b);
void drawLoadingBar(float x, float y, float width, float height, float progress, const char *currentTimeText, const char *maxTimeText);
void drawVerticalBar(float x, float y, float width, float height, float progress, const char *currentText, const char *maxText);
void display();
void timer(int value);
void init();



// Render text on the screen
void renderText(float x, float y, const char *text, void *font, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

// Draw a rectangle
void drawRectangle(float x1, float y1, float x2, float y2, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// Draw a horizontal loading bar
void drawLoadingBar(float x, float y, float width, float height, float progress, const char *currentTimeText, const char *maxTimeText) {
    drawRectangle(x, y, x + width, y - height, 0.5f, 0.5f, 0.5f); // Background
    drawRectangle(x, y, x + width * progress, y - height, 0.0f, 0.0f, 1.0f); // Filled part

    if (currentTimeText) {
        renderText(x + width / 2 - 0.05f, y - height / 2, currentTimeText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }

    if (maxTimeText) {
        renderText(x + width + 0.05f, y - height / 2, maxTimeText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }
}

// Draw a vertical progress bar
void drawVerticalBar(float x, float y, float width, float height, float progress, const char *currentText, const char *maxText) {
    drawRectangle(x, y - height, x + width, y, 0.5f, 0.5f, 0.5f); // Background
    drawRectangle(x, y - height, x + width, y - height + height * progress, 0.2f, 0.8f, 0.2f); // Filled part

    if (currentText) {
        renderText(x + width / 2 - 0.05f, y - height / 2, currentText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }

    if (maxText) {
        renderText(x + width / 2 - 0.05f, y + 0.05f, maxText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }
}

// Timer function
void timer(int value) {
    elapsedTime += 0.1f; // Increment elapsed time
    if (elapsedTime > maxTime) elapsedTime = maxTime;

    glutPostRedisplay(); // Request redraw
    if (elapsedTime < maxTime) {
        glutTimerFunc(100, timer, 0); // Schedule next timer
    }
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Timer bar
    float timeProgress = elapsedTime / maxTime;
    char currentTimeText[50];
    sprintf(currentTimeText, "%.1f s", elapsedTime);
    char maxTimeText[50];
    sprintf(maxTimeText, "Max: %.1f s", maxTime);
    drawLoadingBar(-0.9f, 0.9f, 1.0f, 0.1f, timeProgress, currentTimeText, maxTimeText);

    // Generated files
    char numberGenerated[50];
    sprintf(numberGenerated, "Number of generated files: %d", shared_data->total_csv_generated);
    renderText(-0.9f, 0.7f, numberGenerated, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);

    // Bar properties
    float barWidth = 0.35f;
    float spacing = 0.1f;
    float startX = -0.9f;
    float startY = 0.5f;

    // Calculated bar
    float calculated_ratio = fminf((float)shared_data->total_csv_calculated / config.PROCESSED_THRESHOLD, 1.0f);
    char currentCalculatedText[50];
    sprintf(currentCalculatedText, "%d files", shared_data->total_csv_calculated);
    char maxCalculatedText[50];
    sprintf(maxCalculatedText, "Max: %d files", config.PROCESSED_THRESHOLD);
    drawVerticalBar(startX, startY, barWidth, 0.8f, calculated_ratio, currentCalculatedText, maxCalculatedText);

    // Unprocessed bar
    float unprocessed_ratio = fminf((float)shared_data->unprocessed_csv / config.UNPROCESSED_THRESHOLD, 1.0f);
    char currentUnprocessedText[50];
    sprintf(currentUnprocessedText, "%d files", shared_data->unprocessed_csv);
    char maxUnprocessedText[50];
    sprintf(maxUnprocessedText, "Max: %d files", config.UNPROCESSED_THRESHOLD);
    drawVerticalBar(startX + 1 * (barWidth + spacing), startY, barWidth, 0.8f, unprocessed_ratio, currentUnprocessedText, maxUnprocessedText);

    // Backup bar
    float backup_ratio = fminf((float)shared_data->files_moved_to_backup / config.BACKUP_THRESHOLD, 1.0f);
    char currentBackupText[50];
    sprintf(currentBackupText, "%d files", shared_data->files_moved_to_backup);
    char maxBackupText[50];
    sprintf(maxBackupText, "Max: %d files", config.BACKUP_THRESHOLD);
    drawVerticalBar(startX + 2 * (barWidth + spacing), startY, barWidth, 0.8f, backup_ratio, currentBackupText, maxBackupText);

    // Deleted bar
    float deleted_ratio = fminf((float)shared_data->files_deleted / config.DELETED_THRESHOLD, 1.0f);
    char currentDeletedText[50];
    sprintf(currentDeletedText, "%d files", shared_data->files_deleted);
    char maxDeletedText[50];
    sprintf(maxDeletedText, "Max: %d files", config.DELETED_THRESHOLD);
    drawVerticalBar(startX + 3 * (barWidth + spacing), startY, barWidth, 0.8f, deleted_ratio, currentDeletedText, maxDeletedText);

    glutSwapBuffers();
}

// Initialization
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

// Signal handler
void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nCaught SIGINT. Detaching shared memory and exiting...\n");
        shmdt(shared_data);
        glutLeaveMainLoop();
    }
    else if(signal == SIGUSR1){
        printf("start\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Config.txt>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    maxTime = config.MAX_TIME * 60;

    signal(SIGINT, handle_signal);
    signal(SIGUSR1, handle_signal);

    char *shm_data_key_str = getenv("SHM_DATA_KEY");
    if (!shm_data_key_str) {
        fprintf(stderr, "Error: SHM_DATA_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int shm_data_key = atoi(shm_data_key_str);
    shm_data_id = shmget(shm_data_key, sizeof(SharedData), 0666);
    if (shm_data_id == -1) {
        perror("Shared memory retrieval failed");
        exit(EXIT_FAILURE);
    }

    shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Shared memory attach failed");
        exit(EXIT_FAILURE);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("GUI Visualization");

    init();

    kill(getppid(), SIGUSR2);

    pause();

    glutDisplayFunc(display);
    glutTimerFunc(100, timer, 0);
    glutMainLoop();

    return 0;
}
