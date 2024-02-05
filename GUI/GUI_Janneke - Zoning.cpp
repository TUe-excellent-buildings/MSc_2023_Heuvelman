#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Visualisation/Visualisation.hpp>


typedef void (*ButtonCallback)(int);

struct Button {
    float x, y, width, height;
    ButtonCallback callback;
    const char* text;
    int variable;
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
TextField opinionTF2;
TextField opinionTF3;
TextField opinionTF4;
TextField opinionTF5;
TextField opinionTF6;
TextField opinionTF7;
TextField opinionTF8;
TextField opinionTF9;
TextField opinionTF10;
TextField opinionTF11;
TextField opinionTF12;
TextField opinionTF13;
TextField opinionTF14;
TextField opinionTF15;
TextField opinionTF16;
TextField opinionTF17;
TextField opinionTF18;
TextField opinionTF19;
TextField opinionTF20;
TextField opinionTF21;
TextField opinionTF22;
TextField opinionTF23;

// Global variables for current screen and screen dimensions
int currentScreen = 0;
const int screenWidth = 1800;
const int screenHeight = 1000;

//Global variable to indicate if the confirm button was clicked
int confirmButtonClickFlag = 0;

// Text margin as a percentage of the window width
const float MARGIN_PERCENT = 5.0f; // Margin as a percentage of the window width

// Function prototypes
void display();
void keyboard(unsigned char key, int x, int y);
void reshape(int width, int height);
void mainScreen();
void assignmentDescriptionScreen();
void screen3a();
void screen3b();
void screen3c();
void screen3d();
void screen3e();
void screen4a();
void screen4b();
void screen4c();
void screen4d();
void screen4e();
void screen4f();
void screen5();
void screenCreateZone();
void screenDeleteZone();
void screenCreateZonedDesign();
void screenDeleteZonedDesign();
void screenAddSpace();
void screenDeleteSpace();
void screenMoveSpace();
void screenResizeSpace();
void screenCreateZone2();
void screenDeleteZone2();
void screenCreateZonedDesign2();
void screenDeleteZonedDesign2();
void drawText(const char *text, float x, float y);
void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable);
void drawArrow(float x, float y, bool leftArrow);
void drawUndoRedoButtons();
void drawTextField(int x, int y, int width, int height, TextField& textfield);
void onMouseClick(int button, int state, int x, int y);
void drawBuilding();

//declare outputfile at global scope
std::ofstream outputFile;

//creating output in excel file
void openOutputFile(std::string outputFileName) {
    outputFile.open("output.csv", std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
    }
}
void closeOutputFile() {
    outputFile.close();
}
void writeToOutputFile(std::string outputFileName, std::string question, std::string userAnswer, std::string userExplanation) {
    outputFile << "Question,User Answer\n";
    outputFile << question << "," << userAnswer << "\n";
    outputFile << "User Explanation," << userExplanation << "\n";
}

void buttonClicked(int variable) {
    std::cout << "Button clicked: " << variable << std::endl;
}

void changeScreen(int screen) {
    currentScreen = screen;
    std::cout << "Changed to screen: " << currentScreen << std::endl;
    glutPostRedisplay();
}


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
        case 2: screen3a(); break;
        case 3: screen3b(); break;
        case 4: screen3c(); break;
        case 5: screen3d(); break;
        case 6: screen3e(); break;
        case 7: screen4a(); break;
        case 8: screen4b(); break;
        case 9: screen4c(); break;
        case 10: screen4d(); break;
        case 11: screen4e(); break;
        case 12: screen4f(); break;
        case 13: screen5(); break;
        case 14: screenCreateZone(); break;
        case 15: screenDeleteZone(); break;
        case 16: screenCreateZonedDesign(); break;
        case 17: screenDeleteZonedDesign(); break;
        case 18: screenAddSpace(); break;
        case 19: screenDeleteSpace(); break;
        case 20: screenMoveSpace(); break;
        case 21: screenResizeSpace(); break;
        case 22: screenCreateZone2(); break;
        case 23: screenDeleteZone2(); break;
        case 24: screenCreateZonedDesign2(); break;
        case 25: screenDeleteZonedDesign2(); break;
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
    if (key == 'q') currentScreen = 0;
    if (key == 'w') currentScreen = 1;
    if (key == 'e') currentScreen = 2;
    if (key == 'r') currentScreen = 3;
    if (key == 't') currentScreen = 4;
    if (key == 'y') currentScreen = 5;
    if (key == 'u') currentScreen = 6;
    if (key == 'i') currentScreen = 7;
    if (key == 'o') currentScreen = 8;
    if (key == 'p') currentScreen = 9;
    if (key == 'a') currentScreen = 10;
    if (key == 's') currentScreen = 11;
    if (key == 'd') currentScreen = 12;
    if (key == 'f') currentScreen = 13;
    if (key == 'g') currentScreen = 14;
    if (key == 'h') currentScreen = 15;
    if (key == 'j') currentScreen = 16;
    if (key == 'k') currentScreen = 17;
    if (key == 'l') currentScreen = 18;
    if (key == 'z') currentScreen = 19;
    if (key == 'x') currentScreen = 20;
    if (key == 'c') currentScreen = 21;
    if (key == 'v') currentScreen = 22;
    if (key == 'b') currentScreen = 23;
    if (key == 'n') currentScreen = 24;
    if (key == 'm') currentScreen = 25;

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

    if (currentScreen == 4) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF2.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF2.text != "") { // Backspace key
            opinionTF2.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF2.text << std::endl;
            opinionTF2.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 7) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF3.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF3.text != "") { // Backspace key
            opinionTF3.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF3.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "1. How much did you enjoy performing this assignment?", "1", opinionTF3.text);
            closeOutputFile();
            opinionTF3.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 8) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF4.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF4.text != "") { // Backspace key
            opinionTF4.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF4.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "2. How would you rate the level of ease in performing this assignment?", "1", opinionTF4.text);
            closeOutputFile();
            opinionTF4.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 9) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF5.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF5.text != "") { // Backspace key
            opinionTF5.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF5.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "3. How well do you think you performed the assignment?", "1", opinionTF5.text);
            closeOutputFile();
            opinionTF5.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 10) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF6.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF6.text != "") { // Backspace key
            opinionTF6.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF6.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "4. Do you think it would have gone better with an AI tool that identifies all zoned designs for you?", "1", opinionTF6.text);
            closeOutputFile();
            opinionTF6.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 11) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF7.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF7.text != "") { // Backspace key
            opinionTF7.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF7.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "5. Do you think the AI tool itself can perform zoning better than you?", "1", opinionTF7.text);
            closeOutputFile();
            opinionTF7.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 12) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF8.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF8.text != "") { // Backspace key
            opinionTF8.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF8.text << std::endl;
            // Write the entered text to the output file
            openOutputFile("output.csv");
            writeToOutputFile("output.csv", "6. What criteria did you keep in mind while performing this assignment?", "1", opinionTF8.text);
            closeOutputFile();
            opinionTF8.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 14) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF9.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF9.text != "") { // Backspace key
            opinionTF9.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF9.text << std::endl;
            opinionTF9.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 15) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF10.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF10.text != "") { // Backspace key
            opinionTF10.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF10.text << std::endl;
            opinionTF10.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 16) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF11.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF11.text != "") { // Backspace key
            opinionTF11.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF11.text << std::endl;
            opinionTF11.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 17) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF12.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF12.text != "") { // Backspace key
            opinionTF12.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF12.text << std::endl;
            opinionTF12.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 18) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF13.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF13.text != "") { // Backspace key
            opinionTF13.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF13.text << std::endl;
            opinionTF13.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 18) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF14.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF14.text != "") { // Backspace key
            opinionTF14.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF14.text << std::endl;
            opinionTF14.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 19) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF15.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF15.text != "") { // Backspace key
            opinionTF15.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF15.text << std::endl;
            opinionTF15.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 20) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF16.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF16.text != "") { // Backspace key
            opinionTF16.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF16.text << std::endl;
            opinionTF16.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 20) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF17.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF17.text != "") { // Backspace key
            opinionTF17.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF17.text << std::endl;
            opinionTF17.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 21) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF18.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF18.text != "") { // Backspace key
            opinionTF18.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF18.text << std::endl;
            opinionTF18.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 21) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF19.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF19.text != "") { // Backspace key
            opinionTF19.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF19.text << std::endl;
            opinionTF19.text = ""; // Clear the input string after processing
        }
    }
    
    if (currentScreen == 22) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF20.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF20.text != "") { // Backspace key
            opinionTF20.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF20.text << std::endl;
            opinionTF20.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 23) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF21.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF21.text != "") { // Backspace key
            opinionTF21.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF21.text << std::endl;
            opinionTF21.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 24) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF22.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF22.text != "") { // Backspace key
            opinionTF22.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF22.text << std::endl;
            opinionTF22.text = ""; // Clear the input string after processing
        }
    }

    if (currentScreen == 25) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF23.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF23.text != "") { // Backspace key
            opinionTF23.text.pop_back(); // Remove the last character from input string
        }
        else if (key == 13) { // Enter key
            // Print the entered text to the terminal
            std::cout << "Entered text: " << opinionTF23.text << std::endl;
            opinionTF23.text = ""; // Clear the input string after processing
        }
    }

    //above is for text fields, below is for the confirm button to also work when 'enter' is pressed on the keyboard
    if (key == 13) {  // ASCII code for Enter key
        // Set the flag to indicate that Enter key was pressed
        confirmButtonClickFlag = 1;
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

void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable) {
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

    Button button = {x, y, width, height, callback, text, variable};
    buttons.push_back(button);

    //std::cout << "Button coordinates: (" << x << ", " << y << "), Dimensions: (" << width << " x " << height << ")" << std::endl;
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
                    btn.callback(btn.variable);
                }
                break;
            }
        }
    }

    std::cout << "Mouse clicked at (" << x << ", " << y << ")" << std::endl;

    for (const auto& btn : buttons) {
        std::cout << "Button: (" << btn.x << ", " << btn.y << "), Dimensions: (" << btn.width << " x " << btn.height << ")" << std::endl;
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
    drawButton("", 10, screenHeight - 60, 50, 50, buttonClicked, 1);
    glColor3f(0, 0, 0); // Arrow color
    drawArrow(10, screenHeight - 60, false); // Left arrow for undo

    // Redo Button
    glColor3f(0.7, 0.7, 0.7); // Button color
    drawButton("", 70, screenHeight - 60, 50, 50, buttonClicked, 1);
    glColor3f(0, 0, 0); // Arrow color
    drawArrow(70, screenHeight - 60, true); // Right arrow for redo

    // Reset Button
    glColor3f(0.7, 0.7, 0.7); // Button color
    drawButton("Reset", 10, screenHeight - 120, 110, 50, buttonClicked, 1);
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

void mainScreen() {
    drawText("Hello and welcome to this MSc project by Janneke Heuvelman. We are glad to have you here and hope you will have a nice experience. In case of any problems, be sure to contact Janneke via email: j.h.heuvelman@student.tue.nl. Please select the Assignment number:",
    900, 800, 400);

    drawButton("Assignment 1", 800, 650, 200, 50, buttonClicked, 1);
    drawButton("Assignment 2", 800, 580, 200, 50, changeScreen, 1);
    drawButton("Assignment 3", 800, 510, 200, 50, buttonClicked, 1);
    drawButton("Assignment 4", 800, 440, 200, 50, buttonClicked, 1);

    drawUndoRedoButtons();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 1);
}

void assignmentDescriptionScreen() {
    drawText("You will in a moment go through a design task. You are asked to perform this task in the way you are used to go about a commission in your daily practice. It is important that you say aloud everything that you think or do in designing. ​So, in every step, explain what you do and why you do it. Try to keep speaking constantly and not be silent for longer than 20 seconds. ​Good luck!​",
    900, 600, 400);

    drawButton("<- | Previous step", 1380, 50, 200, 50, changeScreen, 0);
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 2);

    drawUndoRedoButtons();
}

void screen3a() {
    // Screen layout and colors should be adjusted as necessary.

    // Draw structural design illustration placeholder (left side)
    drawBuilding();
    // BSO::Spatial_Design::MS_Building MS("MS_Input.txt");

    // BSO::Visualisation::init_visualisation_without();
    // BSO::Visualisation::visualise(MS);
    // BSO::Visualisation::end_visualisation();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw the bottom area where zones and zoned designs are displayed
    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    // Draw control buttons (right side)
    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 14);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 15);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 16);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 17);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 1: Find all zoned designs.", 1550, 150, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 3);
}

void screen3b() {
    // Draw structural design illustration placeholder (left side)
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw the bottom area where zones and zoned designs are displayed
    drawText("Zoned designs: ...", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1490, 500, 200, 150, opinionTF);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 2: Pick the design you would like to continue with.", 1550, 150, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 4);
}

void screen3c() {
    // Draw structural design illustration placeholder (left side)
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw the bottom area where zones and zoned designs are displayed
    drawText("Zoned designs: ...", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1490, 500, 200, 150, opinionTF2);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 3: This time, pick the one of which you think its structural design has the highest stiffness.", 1550, 150, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 5);
}

void screen3d() {
    // Draw structural design illustration placeholder (left side)
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw control buttons (right side)
    drawButton("Add space", screenWidth - 310, 760, 200, 50, changeScreen, 18);
    drawButton("Delete space", screenWidth - 310, 700, 200, 50, changeScreen, 19);
    drawButton("Move space", screenWidth - 310, 640, 200, 50, changeScreen, 20);
    drawButton("Resize space", screenWidth - 310, 580, 200, 50, changeScreen, 21);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 4: You may adapt the BSD you started with to create any new BSD you desire. Keep the function of the building in mind, as well as the resulting zoned designs and their stiffnesses.", 1550, 200, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 6);
}

void screen3e() {
    // Draw structural design illustration placeholder (left side)
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    // Draw the undo and redo buttons with arrows in the top left corner
    drawUndoRedoButtons();

    // Draw the bottom area where zones and zoned designs are displayed
    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    // Draw control buttons (right side)
    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 22);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 23);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 24);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 25);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 5: Find all zoned designs.", 1550, 150, 250);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);
}

void screen4a() {
    drawText("1. How much did you enjoy performing this assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 1);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 1);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 1);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 1);

    drawText("1: Not at all", 600, 700, 600);
    drawText("5: Very much", 600, 670, 600);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF3);

    // Draw the message at the bottom of the structure illustration
    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 8);
}

void screen4b() {
    drawText("2. How would you rate the level of ease in performing this assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 1);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 1);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 1);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 1);

    drawText("1: Very hard", 600, 700, 600);
    drawText("5: Very easy", 600, 670, 600);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF4);

    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 9);
}

void screen4c() {
    drawText("3. How well do you think you performed the assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 1);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 1);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 1);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 1);

    drawText("1: I have no idea what I am doing, and unable to identify zoned designs and high stiffness.", 700, 700, 800);
    drawText("5: Confident, correct answers, and able to identify zoned designs and high stiffness.", 700, 670, 800);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF5);

    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 10);
}

void screen4d() {
    drawText("4. Do you think it would have gone better with an AI tool that identifies all zoned designs for you?", 600, 800, 600);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 1);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 1);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 1);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF6);

    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 11);
}

void screen4e() {
    drawText("5. Do you think the AI tool itself can perform zoning better than you?", 600, 800, 600);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 1);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 1);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 1);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF7);

    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 12);
}

void screen4f() {
    drawText("6. What criteria did you keep in mind while performing this assignment?", 600, 800, 600);
    drawText("(For example, structural, aesthetical, functional, and zoning requirements.)", 600, 770, 600);
    drawTextField(300, 530, 500, 200, opinionTF8);

    drawText("Step 6: Questionnaire.", 1550, 150, 250);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 13);
}

void screen5() {
    drawText("Thank you for your participation, this is the end of the assignment.", 600, 800, 600);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawButton("-> | Next", 1590, 50, 200, 50, buttonClicked, 1);
}

void screenCreateZone() {
    //same as previous screen
    drawBuilding();
    
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, buttonClicked, 1);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 15);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 16);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 17);

    drawText("Step 1: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 3);
    
    //draw text and input for creating a zone
    drawText("Space(s) to include:", screenWidth, 420, 600);
	drawTextField(screenWidth - 310, 350, 200, 50, opinionTF9);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 2); //go back to main zoning screen AND a zone should be created

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(2);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);   
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenDeleteZone() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 14);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, buttonClicked, 1);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 16);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 17);

    drawText("Step 1: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 3);

    //draw text and input for deleting a zone
    drawText("Zone to delete:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF10);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 2); //go back to main zoning screen AND a zone should be deleted

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(2);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenCreateZonedDesign() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 14);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 15);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, buttonClicked, 1);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 17);

    drawText("Step 1: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 3);

    //draw text and input for creating a zoned design
    drawText("Zone(s) to include:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF11);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 2); //go back to main zoning screen AND a zoned design should be created

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(2);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenDeleteZonedDesign() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 14);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 15);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 16);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, buttonClicked, 1);

    drawText("Step 1: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 3);

    //draw text and input for deleting a zoned design
    drawText("Zoned design to delete:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF12);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 2); //go back to main zoning screen AND a zoned design should be deleted

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(2);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenAddSpace() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawButton("Add space", screenWidth - 310, 760, 200, 50, buttonClicked, 1);
    drawButton("Delete space", screenWidth - 310, 700, 200, 50, changeScreen, 19);
    drawButton("Move space", screenWidth - 310, 640, 200, 50, changeScreen, 20);
    drawButton("Resize space", screenWidth - 310, 580, 200, 50, changeScreen, 21);

    drawText("Step 4: You may adapt the BSD you started with to create any new BSD you desire. Keep the function of the building in mind, as well as the resulting zoned designs and their stiffnesses.", 1550, 200, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 6);

    //draw text and input for adding a space
    drawText("Size (x,y,z):", 1515, 420, 150);
    drawText("Location (x,y,z):", 1680, 420, 150);
    drawTextField(screenWidth - 355, 350, 150, 50, opinionTF13);
    drawTextField(screenWidth - 195, 350, 150, 50, opinionTF14);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 5); //go back to main BSD apting screen AND a space should be added

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(5);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1420.0f, 275.0f);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1420.0f, 275.0f);
    glEnd();
}

void screenDeleteSpace() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawButton("Add space", screenWidth - 310, 760, 200, 50, changeScreen, 18);
    drawButton("Delete space", screenWidth - 310, 700, 200, 50, buttonClicked, 1);
    drawButton("Move space", screenWidth - 310, 640, 200, 50, changeScreen, 20);
    drawButton("Resize space", screenWidth - 310, 580, 200, 50, changeScreen, 21);

    drawText("Step 4: You may adapt the BSD you started with to create any new BSD you desire. Keep the function of the building in mind, as well as the resulting zoned designs and their stiffnesses.", 1550, 200, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 6);

    //draw text and input for deleting a space
    drawText("Space(s) to delete:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF15);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 5); //go back to main BSD apting screen AND a space should be deleted

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(5);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenMoveSpace() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawButton("Add space", screenWidth - 310, 760, 200, 50, changeScreen, 18);
    drawButton("Delete space", screenWidth - 310, 700, 200, 50, changeScreen, 19);
    drawButton("Move space", screenWidth - 310, 640, 200, 50, buttonClicked, 1);
    drawButton("Resize space", screenWidth - 310, 580, 200, 50, changeScreen, 21);

    drawText("Step 4: You may adapt the BSD you started with to create any new BSD you desire. Keep the function of the building in mind, as well as the resulting zoned designs and their stiffnesses.", 1550, 200, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 6);

    //draw text and input for moving a space
    drawText("Space:", 1515, 420, 150);
    drawText("New location (x,y,z):", 1680, 420, 150);
    drawTextField(screenWidth - 355, 350, 150, 50, opinionTF16);
    drawTextField(screenWidth - 195, 350, 150, 50, opinionTF17);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 5); //go back to main BSD apting screen AND a space should be moved

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(5);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1420.0f, 275.0f);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1420.0f, 275.0f);
    glEnd();
}

void screenResizeSpace() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawButton("Add space", screenWidth - 310, 760, 200, 50, changeScreen, 18);
    drawButton("Delete space", screenWidth - 310, 700, 200, 50, changeScreen, 19);
    drawButton("Move space", screenWidth - 310, 640, 200, 50, changeScreen, 20);
    drawButton("Resize space", screenWidth - 310, 580, 200, 50, buttonClicked, 1);

    drawText("Step 4: You may adapt the BSD you started with to create any new BSD you desire. Keep the function of the building in mind, as well as the resulting zoned designs and their stiffnesses.", 1550, 200, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 6);

    //draw text and input for resizing a space
    drawText("Space:", 1515, 420, 150);
    drawText("New size (x,y,z):", 1680, 420, 150);
    drawTextField(screenWidth - 355, 350, 150, 50, opinionTF18);
    drawTextField(screenWidth - 195, 350, 150, 50, opinionTF19);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 5); //go back to main BSD apting screen AND a space should be resized

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(5);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1420.0f, 275.0f);
    glVertex2f(1420.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 450.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1780.0f, 275.0f);
    glVertex2f(1420.0f, 275.0f);
    glEnd();
}

void screenCreateZone2() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, buttonClicked, 1);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 23);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 24);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 25);

    drawText("Step 5: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);

    //draw text and input for creating a zone
    drawText("Space(s) to include:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF20);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 6); //go back to zoning screen 2 AND a zone should be created

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(6);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenDeleteZone2() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 22);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, buttonClicked, 1);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 24);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 25);

    drawText("Step 5: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);

    //draw text and input for deleting a zone
    drawText("Zone to delete:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF21);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 6); //go back to zoning screen 2  AND a zone should be deleted

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(6);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenCreateZonedDesign2() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 22);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 23);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, buttonClicked, 1);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, changeScreen, 25);

    drawText("Step 5: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);

    //draw text and input for creating a zoned design
    drawText("Zone(s) to include:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF22);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 6); //go back to zoning screen 2 AND a zoned design should be created

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(6);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
}

void screenDeleteZonedDesign2() {
    //same as previous screen
    drawBuilding();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();

    drawUndoRedoButtons();

    drawText("Zones: 0", 100, 300, 200);
    drawText("Zoned designs: 0", 100, 150, 200);

    drawText("Zones", screenWidth - 150, 820, 200);
    drawButton("Create zone", screenWidth - 310, 760, 200, 50, changeScreen, 22);
    drawButton("Delete zone", screenWidth - 310, 700, 200, 50, changeScreen, 23);
    drawText("Zoned designs", screenWidth - 180, 660, 200);
    drawButton("Create zoned design", screenWidth - 310, 600, 200, 50, changeScreen, 24);
    drawButton("Delete zoned design", screenWidth - 310, 540, 200, 50, buttonClicked, 1);

    drawText("Step 5: Find all zoned designs.", 1550, 150, 250);

    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);

    //draw text and input for deleting a zoned design
    drawText("Zoned design to delete:", screenWidth, 420, 600);
    drawTextField(screenWidth - 310, 350, 200, 50, opinionTF23);
    drawButton("Confirm", screenWidth - 260, 300, 100, 30, changeScreen, 6); //go back to zoning screen 2 AND a zoned design should be deleted

    // Check if the Enter key was pressed
    if (confirmButtonClickFlag) {
        // Reset the flag
        confirmButtonClickFlag = 0;

        // Perform the action corresponding to the "Confirm" button
        changeScreen(6);
    }

    //draw lines around it
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1470.0f, 275.0f);
    glVertex2f(1470.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 450.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1710.0f, 275.0f);
    glVertex2f(1470.0f, 275.0f);
    glEnd();
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
