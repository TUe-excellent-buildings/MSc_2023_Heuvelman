#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>


typedef void (*ButtonCallback)(void);

struct Button {
    float x, y, width, height;
    ButtonCallback callback;
    const char* text;
};

struct TextField {
    std::string text;
    int cursorPosition;
    bool isActive;

    // Constructor
    TextField() : cursorPosition(0), isActive(false) {}

    // Add a character where the cursor is
    void addChar(char c) {
        if (cursorPosition < text.length()) {
            text.insert(text.begin() + cursorPosition, c);
        } else {
            text.push_back(c);
        }
        cursorPosition++;
    }

    // Remove a character at the cursor
    void removeChar() {
        if (!text.empty() && cursorPosition > 0) {
            text.erase(text.begin() + cursorPosition - 1);
            cursorPosition--;
        }
    }

    // Handle the Enter key
    void submit() {
        std::cout << text << std::endl;
        // Clear text if needed
        text.clear();
        cursorPosition = 0;
    }
};

std::vector<Button> buttons;
TextField opinionTF;

// Global variables for current screen and screen dimensions
int currentScreen = 0;
const int screenWidth = 1800;
const int screenHeight = 1000;

// Text margin as a percentage of the window width
const float MARGIN_PERCENT = 5.0f; // Margin as a percentage of the window width

// Function prototypes
void display();
void keyboard(unsigned char key, int x, int y);
void reshape(int width, int height);
void mainScreen();
void assignmentDescriptionScreen();
void screen3();
void screen4();
void drawText(const char *text, float x, float y);
void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback);
void drawArrow(float x, float y, bool leftArrow);
void drawUndoRedoButtons();
void drawTextField(int x, int y, int width, int height, TextField& textfield);
void onMouseClick(int button, int state, int x, int y);


void display() { 
    // Clear the window with white background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Set up 2D projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, screenWidth, 0.0, screenHeight);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render the current screen
    switch (currentScreen) {
        case 0: mainScreen(); break;
        case 1: assignmentDescriptionScreen(); break;
        case 2: screen3(); break;
        case 3: screen4(); break;
        // Ensure you have a default case, even if it does nothing,
        // to handle any unexpected values of currentScreen
        default: break;
    }

    // Swap buffers
    glutSwapBuffers();

    // Check for any OpenGL errors
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }
}

void reshape(int width, int height) {
    // Prevent a divide by zero error by making height equal to one
    if (height == 0) {
        height = 1;
    }

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Set the viewport to cover the new window size
    glViewport(0, 0, width, height);

    // Set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set up a perspective projection matrix or an orthographic one depending on your needs
    gluPerspective(45.0, aspectRatio, 0.1, 100.0);
    // For 2D GUI you may want to use an orthographic projection instead
    // gluOrtho2D(0.0, width, 0.0, height);

    // Return to the modelview matrix mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void keyboard(unsigned char key, int x, int y) {
    // Change screens based on key press
    if (key == '!') currentScreen = 0;
    if (key == '@') currentScreen = 1;
    if (key == '#') currentScreen = 2;
    if (key == '$') currentScreen = 3;

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }

    if(currentScreen == 3) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF.text += key; // Append the character to the input string
        } else if (key == 8 && opinionTF.text != "") { // Backspace key
            opinionTF.text.pop_back(); // Remove the last character from input string
        } else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF.text << std::endl;
            opinionTF.text = ""; // Clear the input string after processing
        }
    }


    // Redraw screen
    glutPostRedisplay();
}

void drawText(const char *text, float centerX, float centerY, float textWidth) {
    float lineHeight = 18; // Approximate line height, adjust as needed
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    // Calculate the starting position (left align within the margin)
    float startX = centerX - effectiveTextWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    for (const char *c = text; *c != '\0'; c++) {
        // Check if we need to wrap the line
        if ((currentX - startX > effectiveTextWidth) && (*c == ' ' || *c == '\n')) {
            currentY -= lineHeight;
            currentX = startX;
        }

        glRasterPos2f(currentX, currentY);

        // Set text color to black
        glColor3f(0.0, 0.0, 0.0); // black color for text

        // Draw the character
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // Move to the next character position
        currentX += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void buttonClicked() {
    std::cout << "Button clicked" << std::endl;
}


void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback) {
    float borderWidth = 2.0;

    glColor3f(0.0, 0.0, 0.0); // Black color for border
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();

    // Draw button rectangle with white background
    glColor3f(1.0, 1.0, 1.0); // white color for button background
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Centered text within the button with margin
    float centerX = x + width / 2;
    float centerY = y + height / 2;
    float textWidth = width - 2 * MARGIN_PERCENT; // Text width considering margin

    // Set text color to black
    glColor3f(0.0, 0.0, 0.0);
    drawText(text, centerX, centerY, textWidth);

    Button button = {x, y, width, height, callback, text};
    buttons.push_back(button);
}

void onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float mouseY = screenHeight - static_cast<float>(y);
        float mouseX = static_cast<float>(x);

        for (const auto& btn : buttons) {
            if (mouseX >= btn.x && mouseX <= btn.x + btn.width &&
                mouseY >= btn.y && mouseY <= btn.y + btn.height) {
                // Button was clicked
                if (btn.callback) {
                    btn.callback();
                }
                break;
            }
        }
    }
}

void drawTextField(int x, int y, int width, int height, TextField& textfield) {
    float borderWidth = 2.0;

    // Calculate the adjusted width and height considering padding
    int adjustedWidth = width - 2 * borderWidth;
    int adjustedHeight = height - 2 * borderWidth;

    glColor3f(0.0, 0.0, 0.0); // Black color for border
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();

    // Draw text field background
    glColor3f(1.0, 1.0, 1.0); // white background for text field
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Set the color for the text
    glColor3f(0.0, 0.0, 0.0); // black text

    // Implement text wrapping within the width of the text field
    // This is a simplistic approach and might need adjustment for different font widths
    int maxWidth = adjustedWidth; // maximum width for text before wrapping
    int currentWidth = 0;
    std::string line;
    std::vector<std::string> lines;

    for (char c : textfield.text) {
        if (c == '\n' || currentWidth > maxWidth) {
            lines.push_back(line);
            line.clear();
            currentWidth = 0;
        }
        if (c != '\n') {
            line += c;
            // Estimate the width of the character, adjust for the new font size (18)
            currentWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
    // Add the last line if it's not empty
    if (!line.empty()) {
        lines.push_back(line);
    }

    // Now draw the text line by line from the top
    int lineHeight = 20; // Adjusted for the new font size (18) and additional padding
    int textHeight = lines.size() * lineHeight; // Total height of the text
    int startY = y + height - lineHeight; // Calculate the starting Y coordinate from the top with padding

    for (size_t i = 0; i < lines.size(); ++i) {
        int currentY = startY - i * lineHeight; // Calculate the Y coordinate for this line
        glRasterPos2f(x + borderWidth, currentY); // Adjust for the left padding
        for (char c : lines[i]) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Use GLUT_BITMAP_HELVETICA_18 for the new font size
        }
    }

    // Draw the cursor if the text field is active
    if (textfield.isActive) {
        int cursorX = x + borderWidth + glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)textfield.text.c_str()); // Adjust for left padding
        int cursorY = startY; // Use the same starting Y coordinate as the text
        glColor3f(0.0, 0.0, 0.0); // black cursor
        glBegin(GL_LINES);
        glVertex2f(cursorX, cursorY - 18); // Adjust the Y coordinate to draw the cursor above the text
        glVertex2f(cursorX, cursorY - 3);  // Adjust the Y coordinate to draw the cursor above the text
        glEnd();
    }
}





// Function to draw an arrow inside a button
void drawArrow(float x, float y, bool leftArrow) {
    glBegin(GL_TRIANGLES);
    if (leftArrow) {
        glVertex2f(x + 10, y + 10);
        glVertex2f(x + 30, y + 25);
        glVertex2f(x + 10, y + 40);
    } else {
        glVertex2f(x + 30, y + 10);
        glVertex2f(x + 10, y + 25);
        glVertex2f(x + 30, y + 40);
    }
    glEnd();
}

// Function to draw undo and redo buttons
void drawUndoRedoButtons() {
    // Undo Button
    glColor3f(0.7, 0.7, 0.7); // Button color
    drawButton("", 10, screenHeight - 60, 50, 50, buttonClicked);
    glColor3f(0, 0, 0); // Arrow color
    drawArrow(10, screenHeight - 60, false); // Left arrow for undo

    // Redo Button
    glColor3f(0.7, 0.7, 0.7); // Button color
    drawButton("", 70, screenHeight - 60, 50, 50, buttonClicked);
    glColor3f(0, 0, 0); // Arrow color
    drawArrow(70, screenHeight - 60, true); // Right arrow for redo

    // Reset Button
    glColor3f(0.7, 0.7, 0.7); // Button color
    drawButton("Reset", 10, screenHeight - 120, 110, 50, buttonClicked);
}


void mainScreen() {
    drawText("Hello and welcome to this MSc project by Janneke Heuvelman. We are glad to have you here and hope you will have a nice experience. In case of any problems, be sure to contact Janneke via email: j.h.heuvelman@student.tue.nl. Please select the Assignment number:",
    900, 800, 400);

    drawButton("Assignment 1", 800, 650, 200, 50, buttonClicked);
    drawButton("Assignment 2", 800, 580, 200, 50, buttonClicked);
    drawButton("Assignment 3", 800, 510, 200, 50, buttonClicked);
    drawButton("Assignment 4", 800, 440, 200, 50, buttonClicked);
    drawButton("Assignment 5", 800, 370, 200, 50, buttonClicked);
    drawButton("Assignment 6", 800, 300, 200, 50, buttonClicked);

    drawUndoRedoButtons();
}

void assignmentDescriptionScreen() {
    drawText("You will in a moment go through a design task. You are asked to perform this task in the way you are used to go about a commission in your daily practice. It is important that you say aloud everything that you think or do in designing. ​So, in every step, explain what you do and why you do it. Try to keep speaking constantly and not be silent for longer than 20 seconds. ​Good luck!​",
    900, 600, 400);

    drawButton("<- | Previous step", 1380, 50, 200, 50, buttonClicked);
    drawButton("-> | Next step", 1590, 50, 200, 50, buttonClicked);

    drawUndoRedoButtons();
}

void drawBuilding() {
    glColor3f(0.0, 0.0, 0.0); // Blue color for the building structure
    glBegin(GL_LINES);

    // Base of the building
    glVertex2f(100.0f, 300.0f);
    glVertex2f(600.0f, 300.0f);

    glVertex2f(600.0f, 300.0f);
    glVertex2f(600.0f, 800.0f);

    glVertex2f(600.0f, 800.0f);
    glVertex2f(100.0f, 800.0f);

    glVertex2f(100.0f, 800.0f);
    glVertex2f(100.0f, 300.0f);

    // Top of the building
    glVertex2f(100.0f, 200.0f);
    glVertex2f(600.0f, 200.0f);

    glVertex2f(600.0f, 200.0f);
    glVertex2f(600.0f, 700.0f);

    glVertex2f(600.0f, 700.0f);
    glVertex2f(100.0f, 700.0f);

    glVertex2f(100.0f, 700.0f);
    glVertex2f(100.0f, 200.0f);

    // Vertical lines
    glVertex2f(100.0f, 200.0f);
    glVertex2f(100.0f, 300.0f);

    glVertex2f(600.0f, 200.0f);
    glVertex2f(600.0f, 300.0f);

    glVertex2f(600.0f, 700.0f);
    glVertex2f(600.0f, 800.0f);

    glVertex2f(100.0f, 700.0f);
    glVertex2f(100.0f, 800.0f);

    // Interior lines - for simplicity, just a couple are drawn here
    glVertex2f(350.0f, 200.0f);
    glVertex2f(350.0f, 700.0f);

    glVertex2f(350.0f, 250.0f);
    glVertex2f(600.0f, 250.0f);

    glEnd();
}

void screen3() {
    // Screen layout and colors should be adjusted as necessary.

    // Draw structural design illustration placeholder (left side)
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw the counter area
    drawText("Number of diagonals: 0", 1600, screenHeight - 220, 200);
    drawText("Number of beams: 0", 1600, screenHeight - 240, 200);

    // Draw control buttons (right side)
    drawButton("Add diagonal", screenWidth - 310, 500, 200, 50, buttonClicked);
    drawButton("Add beam", screenWidth - 310, 560, 200, 50, buttonClicked);
    drawButton("Remove diagonal", screenWidth - 310, 620, 200, 50, buttonClicked);
    drawButton("Remove beam", screenWidth - 310, 680, 200, 50, buttonClicked);

    // Draw the message at the bottom of the structure illustration
    drawText("Stabilize the structural design while trying to achieve high stiffness with minimal adjustments.", 1550, 150, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, buttonClicked);
}

void screen4() {
    drawText("1. Did you like the assignment?", 400, 800, 400);
    drawButton("1", 300, 750, 50, 30, buttonClicked);
    drawButton("2", 350, 750, 50, 30, buttonClicked);
    drawButton("3", 400, 750, 50, 30, buttonClicked);
    drawButton("4", 450, 750, 50, 30, buttonClicked);
    drawButton("5", 500, 750, 50, 30, buttonClicked);
    drawButton("6", 550, 750, 50, 30, buttonClicked);
    drawButton("7", 600, 750, 50, 30, buttonClicked);
    drawButton("8", 650, 750, 50, 30, buttonClicked);
    drawButton("9", 700, 750, 50, 30, buttonClicked);
    drawButton("10", 750, 750, 50, 30, buttonClicked);

    drawText("Please explain:", 300, 630, 200);
    drawTextField(300, 400, 300, 200, opinionTF);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, buttonClicked);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Menu Interface");

    // Set callback functions
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(onMouseClick);

    // Main loop
    glutMainLoop();
    return 0;
}
