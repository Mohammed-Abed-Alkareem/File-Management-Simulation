#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 100     // Maximum number of lines
#define MAX_LINE_LENGTH 256 // Maximum length of each line
#define TEXT_HEIGHT 20.0f  // Height of each line
#define VISIBLE_LINES 5    // Number of visible lines in the text field

// Text buffer
char textBuffer[MAX_LINES][MAX_LINE_LENGTH];
int lineCount = 0;         // Number of lines in the buffer
float scrollOffset = 0.0f; // Current scroll offset

void addText(const char *newText) {
    if (lineCount < MAX_LINES) {
        strncpy(textBuffer[lineCount], newText, MAX_LINE_LENGTH - 1);
        textBuffer[lineCount][MAX_LINE_LENGTH - 1] = '\0'; // Ensure null termination
        lineCount++;

        // Auto-scroll if at the bottom
        if (scrollOffset > (lineCount - VISIBLE_LINES) * TEXT_HEIGHT) {
            scrollOffset = (lineCount - VISIBLE_LINES) * TEXT_HEIGHT;
        }

        glutPostRedisplay();
    } else {
        printf("Text buffer full!\n");
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable scissor test to define the visible area
    glEnable(GL_SCISSOR_TEST);
    glScissor(50, 50, 300, VISIBLE_LINES * TEXT_HEIGHT); // x, y, width, height

    glPushMatrix();
    glTranslatef(0.0f, -scrollOffset, 0.0f); // Apply scrolling offset

    // Render each visible line
    for (int i = 0; i < lineCount; i++) {
        float yPos = 130.0f - i * TEXT_HEIGHT;
        if (yPos + TEXT_HEIGHT >= 50 && yPos <= 50 + VISIBLE_LINES * TEXT_HEIGHT) {
            glRasterPos2f(60.0f, yPos);

            // Render characters with dynamic color
            for (int j = 0; textBuffer[i][j] != '\0'; j++) {
                // Change color based on specific condition
                if (textBuffer[i][j] == ':') {
                    glColor3f(1.0f, 0.0f, 0.0f); // Red color for colons
                } else if (j % 2 == 0) {
                    glColor3f(0.0f, 1.0f, 0.0f); // Green color for even-index characters
                } else {
                    glColor3f(1.0f, 1.0f, 1.0f); // White for others
                }
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, textBuffer[i][j]);
            }
        }
    }

    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);

    glutSwapBuffers();
}

void scroll(int direction) {
    if (direction > 0) {
        scrollOffset -= TEXT_HEIGHT; // Scroll up
        if (scrollOffset < 0.0f) scrollOffset = 0.0f;
    } else if (direction < 0) {
        scrollOffset += TEXT_HEIGHT; // Scroll down
        if (scrollOffset > (lineCount - VISIBLE_LINES) * TEXT_HEIGHT) {
            scrollOffset = (lineCount - VISIBLE_LINES) * TEXT_HEIGHT;
        }
    }
    glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y) {
    if (dir > 0) {
        scroll(1); // Scroll up
    } else if (dir < 0) {
        scroll(-1); // Scroll down
    }
}

void timer(int value) {
    // Simulate appending text periodically
    char newLine[MAX_LINE_LENGTH];
    snprintf(newLine, MAX_LINE_LENGTH, "Line %d: New text appended", lineCount + 1);
    addText(newLine);

    glutTimerFunc(2000, timer, 0); // Schedule the next text append in 2 seconds
}

// ... [Keep existing constants and buffer declarations] ...

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h); // Set coordinate system to match window size
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Modify main():
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 300);
    glutCreateWindow("Scrollable Text Field with Colored Text");

    init();
    
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(mouseWheel);
    glutTimerFunc(2000, timer, 0);

    // Initialize with some text
    addText("Welcome to the Scrollable Text Field!");
    addText("This text field updates periodically.");
    addText("You can scroll up and down.");

    glutMainLoop();
    return 0;
}