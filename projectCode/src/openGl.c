#include "GUI.h"

float maxTime;

// Function to render text on the screen
void renderText(float x, float y, const char *text, void *font, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

// Function to draw a rectangle (used for both horizontal and vertical bars)
void drawRectangle(float x1, float y1, float x2, float y2, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

// Function to draw a horizontal loading bar
void drawLoadingBar(float x, float y, float width, float height, float progress, const char *currentTimeText, const char *maxTimeText) {
    // Background of the loading bar
    drawRectangle(x, y, x + width, y - height, 0.5f, 0.5f, 0.5f); // Grey color

    // Filled portion of the loading bar
    drawRectangle(x, y, x + width * progress, y - height, 0.0f, 0.0f, 1.0f); // Blue color

    // Display current time in the center of the bar
    if (currentTimeText) {
        renderText(x + width / 2 - 0.05f, y - height / 2 , currentTimeText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }

    // Display max time at the end of the bar
    if (maxTimeText) {
        renderText(x + width + 0.05f, y - height / 2, maxTimeText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }
}

// Function to draw a vertical progress bar (filling from bottom to top)
void drawVerticalBar(float x, float y, float width, float height, float progress, const char *currentText, const char *maxText) {
    // Background of the vertical bar
    drawRectangle(x, y - height, x + width, y, 0.5f, 0.5f, 0.5f); // Grey color

    // Filled portion of the vertical bar (bottom to top)
    drawRectangle(x, y - height, x + width, y - height + height * progress, 0.2f, 0.8f, 0.2f); // Green color

    // Display current time near the center
    if (currentText) {
        renderText(x + width / 2 - 0.05f, y - height / 2, currentText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }

    // Display max time at the bottom
    if (maxText) {
        renderText(x + width / 2 - 0.05f, y + 0.05f, maxText, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);
    }
}

// Timer callback function
void timer(int value) {

    elapsedTime += 0.1f; // Increase elapsed time
    if (elapsedTime > maxTime) {
        elapsedTime = maxTime; // Cap elapsed time at maxTime
    }

    glutPostRedisplay(); // Request a redraw
    if (elapsedTime < maxTime) {
        glutTimerFunc(100, timer, 0); // Call timer function again after 100ms
    }
}



// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

//====================Timer Bar====================
    // Calculate the progress for the bars
    float timeProgress = elapsedTime / maxTime;

    // Create labels for current time and max time
    char currentTimeText[50];
    sprintf(currentTimeText, "%.1f s", elapsedTime);

    char maxTimeText[50];
    sprintf(maxTimeText, "Max: %.1f s", maxTime);

    // Draw horizontal loading bars
    drawLoadingBar(-0.9f, 0.9f, 1.0f, 0.1f, timeProgress, currentTimeText, maxTimeText); //timer bar

//====================Text====================
    char numberGenerated[50];
    sprintf(numberGenerated, "Number of generated files: %d", shared_data->total_csv_generated);//change to real number from shared memory
    renderText(-0.9f, 0.7f, numberGenerated, GLUT_BITMAP_HELVETICA_18, 1.0f, 1.0f, 1.0f);


//====================Horizontal Bars====================

    float barWidth = 0.35f;
    float spacing = 0.1f;
    float startX = -0.9f;
    float startY = 0.5f;

    memset(progress, 0, sizeof(progress));
   

    // calculated bar
    float calculated_ratio = (float)shared_data->total_csv_calculated config.PROCESSED_THRESHOLD;
        drawVerticalBar(startX + 0 * (barWidth + spacing), startY, barWidth, 0.8f, calculated_ratio, currentTimeText, maxTimeText);
        
    // unprocessed bar
    float unprocessed_ratio = (float)shared_data->unprocessed_csv / config.UNPROCESSED_THRESHOLD;
        drawVerticalBar(startX + 1 * (barWidth + spacing), startY, barWidth, 0.8f, progress[1], currentTimeText, maxTimeText);
    
    // backup bar
    float backup_ratio = (float)shared_data->files_moved_to_backup / config.BACKUP_THRESHOLD;
        drawVerticalBar(startX + 2 * (barWidth + spacing), startY, barWidth, 0.8f, progress[2], currentTimeText, maxTimeText);
        
    // deleted bar
    float deleted_ratio = (float)shared_data->files_deleted / config.DELETED_THRESHOLD;
        drawVerticalBar(startX + 3 * (barWidth + spacing), startY, barWidth, 0.8f, progress[3], currentTimeText, maxTimeText);

    glutSwapBuffers();
}

// Function to set up the OpenGL environment
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Set coordinate system
}

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Config.txt>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Load configuration
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    // Get max time from config file (in menitues convert to seconds)
    maxTime = config.MAX_TIME * 60;

    //swnsitive to kill signal
    signal(SIGINT, handle_signal);

    //get shm_data_key from environment
    char *shm_data_key_str = getenv("SHM_DATA_KEY");
    if (shm_data_key_str == NULL) {
        fprintf(stderr, "Error: SHM_DATA_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int shm_data_key = atoi(shm_data_key_str);

    // Get shared memory ID
    shm_data_id = shmget(shm_data_key, sizeof(SharedData), 0666);
    if (shm_data_id == -1) {
        perror("Shared memory retrieval failed");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory to shared_data
    shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Shared memory attach failed");
        exit(EXIT_FAILURE);
    }

    


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Horizontal and Vertical Bars");

    init();
    glutDisplayFunc(display);
    glutTimerFunc(100, timer, 0);
    glutMainLoop();

    return 0;
}


void handle_signal(int signal) {
    if (signal == SIGINT) {
        shmdt(shared_data);
    // Exit loop and wait for the user to press on X button
    glutLeaveMainLoop();
       

    }
}