#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Spatial_Design/Zoning.hpp>
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Visualisation/Visualisation.hpp>
#include <BSO/Performance_Indexing.hpp>
#include <AEI_Grammar/Grammar_zoning.hpp>

std::shared_ptr <BSO::Spatial_Design::MS_Building> MS = nullptr;
std::shared_ptr <BSO::Spatial_Design::MS_Conformal> CF = nullptr;
std::shared_ptr <BSO::Spatial_Design::Zoning::Zoned_Design> Zoned = nullptr;
std::shared_ptr <BSO::Structural_Design::SD_Analysis_Vars> SD_Building = nullptr;
std::shared_ptr <BSO::Spatial_Design::Geometry::Space> space = nullptr;
std::shared_ptr <BSO::Spatial_Design::MS_Space> MS_Space = nullptr;

// Global variables for visualisation
bool visualisationActive = false; // Flag to control when to activate visualisation
BSO::Visualisation::viewportmanager vpmanager_local;
BSO::Visualisation::orbitalcamera   cam_local;
int prevx, prevy;

//MS_Space newSpace; 

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
    float x, y, width, height; // Add x, y, width, and height to make them clickable

    // Constructor
    TextField() : cursorPosition(0), isActive(false), x(0), y(0), width(0), height(0) {}

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
TextField opinionTF13;
TextField opinionTF14;
TextField opinionTF15;
TextField opinionTF16;
TextField opinionTF17;
TextField opinionTF18;
TextField opinionTF19;
TextField opinionTF24;
TextField opinionTF25;
TextField opinionTF26;
TextField opinionTF27;

// Global variables for current screen and screen dimensions
int currentScreen = 0;
const int screenWidth = 1800;
const int screenHeight = 1000;

// Text margin as a percentage of the window width
const float MARGIN_PERCENT = 5.0f; // Margin as a percentage of the window width

// Variable to keep track of BSD modifications
int modificationCount = 0;
// Draw a message when input is invalid
bool DrawInvalidInput = false;


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
void screen3f();
void screen3g();
void screen3g2();
void screen4a();
void screen4b();
void screen4c();
void screen4d();
void screen4e();
void screen4f();
void screen4g();
void screen5();
void screen5b();
void screenAddSpace();
void screenDeleteSpace();
void screenMoveSpace();
void screenResizeSpace();
void screenCheckNext();
void screenCheckNext1();
void screenCheckNext2();
void screenCheckNext3();
void screenCheckNext4();
void screenCheckNext5();
void screenCheckNext6();
void screenCheckNext7();
void drawText(const char *text, float x, float y);
void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable);
void drawButtonWithBackgroundColor(const char* text, float x, float y, float width, float height, ButtonCallback callback, int variable, float r, float g, float b);
void drawArrow(float x, float y, bool leftArrow);
void drawTextField(int x, int y, int width, int height, TextField& textfield);
void onMouseClick(int button, int state, int x, int y);
void setup2D();
void setup3D();
void setup_models();

void visualise(BSO::Spatial_Design::MS_Building& ms_building)
{
    vpmanager_local.addviewportzoning(new BSO::Visualisation::viewport(new BSO::Visualisation::MS_Model(ms_building)));
}

void visualise(BSO::Spatial_Design::MS_Conformal& cf_building, std::string type)
{
    vpmanager_local.addviewportzoning(new BSO::Visualisation::viewport(new BSO::Visualisation::Conformal_Model(cf_building, type)));
}

void visualise(BSO::Spatial_Design::MS_Conformal& cf_building, std::string type, unsigned int i)
{
    vpmanager_local.addviewportzoning(new BSO::Visualisation::viewport(new BSO::Visualisation::Zoning_Model(cf_building, type, i)));
}

void setup_pointers() {
    MS = std::make_shared<BSO::Spatial_Design::MS_Building>("JH_Zoning_Assignment_GUI/MS_Input.txt");
    CF = std::make_shared<BSO::Spatial_Design::MS_Conformal>(*MS, &(BSO::Grammar::grammar_zoning));
    (*CF).make_conformal();
    Zoned = std::make_shared<BSO::Spatial_Design::Zoning::Zoned_Design>(CF.get());
    (*Zoned).make_zoning();
    SD_Building = std::make_shared<BSO::Structural_Design::SD_Analysis>(*CF);
    (*SD_Building).analyse();
}

// Function to recreate CF using the updated MS
void update_CF() {
    CF = std::make_shared<BSO::Spatial_Design::MS_Conformal>(*MS, &(BSO::Grammar::grammar_zoning));
    (*CF).make_conformal();
    Zoned = std::make_shared<BSO::Spatial_Design::Zoning::Zoned_Design>(CF.get());
    (*Zoned).make_zoning();
    SD_Building = std::make_shared<BSO::Structural_Design::SD_Analysis>(*CF);
    (*SD_Building).analyse();
}

void checkGLError(const char* action) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error after " << action << ": " << gluErrorString(err) << std::endl;
    }
}

//declare outputfile at global scope
std::ofstream outputFile;
std::ofstream processFile;
std::ofstream IQDFile;

//creating output in excel file
void writeToOutputFile(std::string outputFileName, std::string question, std::string userAnswer, std::string userExplanation) {
    static bool headerPrinted = false;
    outputFile.open("output3.csv", std::ios::app);
    if (!headerPrinted) {
        outputFile << "Question,User Answer,User Explanation\n";
        headerPrinted = true;
    }
    if (!userExplanation.empty()) {
        outputFile << question << "," << userAnswer << "," << userExplanation << "\n";
    }
    else {
        outputFile << question << "," << userAnswer << ",\n";
    }
    outputFile.close();
}

void writeToProcessFile(std::string processFileName, std::string action, std::string userInput) {
    //headers are only printed once, so the static variable for each column
    static bool headerPrinted = false;
    processFile.open("process3.csv", std::ios::app);
    if (!headerPrinted) {
        processFile << "Action,User input,Time\n";
        headerPrinted = true;
    }
    //to print the time
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%H:%M:%S"); // Format time as HH:MM:SS
    std::string timeString = oss.str();

    processFile << action << "," << userInput << "," << timeString << "\n";
    processFile.close();
}

void writeToIQDFile(std::string IQDFileName, std::string question, std::string userAnswer) {
    static bool headerPrinted = false;
    IQDFile.open("IQD assessments.csv", std::ios::app); // Use the provided IQDFileName instead of hardcoding "output3.csv"
    if (!headerPrinted) {
        IQDFile << "Question,User Answer\n";
        headerPrinted = true;
    }
    IQDFile << question << "," << userAnswer << "\n";
    IQDFile.close();
}

//Declare a global variable to store the selected button label
std::string selectedButtonLabel = "";

// Function to get the selected button label
std::string getSelectedButtonLabel() {
    return selectedButtonLabel;
}

void buttonClicked(int variable) {
    std::cout << "Button clicked: " << variable << std::endl;

    // Set the selected button label based on the variable
    switch (variable) {
    case 1:
        selectedButtonLabel = "1";
        break;
    case 2:
        selectedButtonLabel = "2";
        break;
    case 3:
        selectedButtonLabel = "3";
        break;
    case 4:
        selectedButtonLabel = "4";
        break;
    case 5:
        selectedButtonLabel = "5";
        break;
    case 6:
        selectedButtonLabel = "Yes";
        break;
    case 7:
        selectedButtonLabel = "No";
        break;
    case 8:
        selectedButtonLabel = "No idea";
        break;
    case 9:
        selectedButtonLabel = "Limited options";
        break;
    case 10:
        selectedButtonLabel = "All options";
        break;
    }

    if (currentScreen == 7) {
        writeToOutputFile("output3.csv", "1. How much did you enjoy performing this assignment?", getSelectedButtonLabel(), opinionTF3.text);
    }
    if (currentScreen == 8) {
        writeToOutputFile("output3.csv", "2. How would you rate the level of ease in performing this assignment?", getSelectedButtonLabel(), opinionTF4.text);
	}
    if (currentScreen == 9) {
        writeToOutputFile("output3.csv", "3. How well do you think you performed the assignment?", getSelectedButtonLabel(), opinionTF5.text);
    }
    if (currentScreen == 10) {
        writeToOutputFile("output3.csv", "4. Do you think it would have gone better without the AI tool?", getSelectedButtonLabel(), opinionTF6.text);
	}
    if (currentScreen == 11) {
        writeToOutputFile("output3.csv", "5. Do you think the AI tool itself can perform zoning better than you?", getSelectedButtonLabel(), opinionTF7.text);
    }
    if (currentScreen == 12) {
        writeToOutputFile("output3.csv", "6. What criteria did you keep in mind while performing this assignment?", "", opinionTF8.text);
    }
    if (currentScreen == 36) {
        writeToOutputFile("output3.csv", "7.  Did you prefer choosing from the limited zoned design options or from all zoned design options?", getSelectedButtonLabel(), opinionTF26.text);
    }

    if (currentScreen == 35) {
		writeToIQDFile("IQD assessments.csv", "Assessment 1", getSelectedButtonLabel());
	}
}

void initializeScreen() {
    // Your initialization code for the screen

    // Set initial active state for opinionTF13
    opinionTF13.isActive = true;
    opinionTF14.isActive = false;
    opinionTF16.isActive = true;
    opinionTF17.isActive = false;
    opinionTF18.isActive = true;
    opinionTF19.isActive = false;
}

//Function to visualize zoned designs, which one can be chosen with the index. 
void visualiseZones(unsigned int indexToVisualize = -1) {
    std::cout << "Total designs in Zoned: " << Zoned->get_designs().size() << std::endl;
    unsigned int designsCount = Zoned->get_designs().size();
    if (indexToVisualize < designsCount) {
        visualise(*CF, "zones", indexToVisualize);
    }
    else {
        for (unsigned int i = 0; i < designsCount; i++) {
            visualise(*CF, "zones", i);
        }
    }
}

//Function to visualize a single zone
//void visualiseZone(unsigned int indexToVisualize = 1) 
//{
 //       visualise(*CF, "zone", indexToVisualize);
//}

bool visualisationActive_3a = false;
bool visualisationActive_3b = false;
bool visualisationActive_3c = false;
bool visualisationActive_3d = false;
bool visualisationActive_3e = false;
bool visualisationActive_3f = false;

void changeScreen(int screen) {
    currentScreen = screen;
    std::cout << "Changing to screen: " << screen << std::endl;
    selectedButtonLabel = "";
    DrawInvalidInput = false;
    initializeScreen();

    // Define separate flags for each group of screens
    visualisationActive_3a = false;
    visualisationActive_3b = false;
    visualisationActive_3c = false;
    visualisationActive_3d = false;
    visualisationActive_3e = false;
    visualisationActive_3f = false;

    if (screen == 2 || (screen >= 14 && screen <= 17) || (screen == 26)) {
        visualisationActive_3a = true; //Screens first time zoning (screen 3a and pop ups)
    }
    else if (screen == 3 || (screen == 27)) {
        visualisationActive_3b = true; //Screens second time zoning (screen 3b and pop ups)
    }
    else if (screen == 4 || (screen == 28)) {
        visualisationActive_3c = true; 	//Screens changing the SD (screen 3c and pop ups)
    }
    else if ((screen == 5) || (screen >= 18 && screen <= 21) || (screen == 29)) {
        visualisationActive_3d = true; //Screens changing the BSD (screen 3d and pop ups)
    }
    else if (screen == 6 || (screen >= 22 && screen <= 25) || screen == 30) {
        visualisationActive_3e = true; //Screens second time zoning (screen 3e and pop ups)
    }
    else if (screen == 32 || screen == 33) {
        visualisationActive_3f = true; //Screens second time zoning (screen 3f and pop ups)
    }

    // Based on the flags, activate/deactivate visualization for each group
    if (visualisationActive_3a) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
        //visualiseZones(2);
        //visualiseZones();
        // visualise(&SD, 1);
        // visualise(*SD_Building, 4);
    }
    else if (visualisationActive_3b) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
        visualiseZones();
    }
    else if (visualisationActive_3c) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
        visualiseZones();
    }
    else if (visualisationActive_3d) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
    }
    else if (visualisationActive_3e) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
        update_CF();
        //visualise(*CF, "rectangles");
        visualiseZones(); //should become only the most diverse 2
    }
    else if (visualisationActive_3f) {
        // Activate visualization for group 3a
        if (MS == nullptr || CF == nullptr || Zoned == nullptr) {
            setup_pointers();
        }
        vpmanager_local.clearviewports();
        visualise(*MS);
        visualiseZones(); //should be all, but based on the new BSD
    }
    else {
        vpmanager_local.clearviewports(); // Deactivate visualization if none of the groups match
        visualisationActive_3a = false;
        visualisationActive_3b = false;
        visualisationActive_3c = false;
        visualisationActive_3d = false;
        visualisationActive_3e = false;
        visualisationActive_3f = false;
    }


    if (screen == 4) {
        writeToOutputFile("output3.csv", "Step 2: Pick one zoned design you would like to continue with and explain why.", "", opinionTF.text);
    }
    if (screen == 33) {
        writeToOutputFile("output3.csv", "Step 3: This time pick the one of which you think its structural design has the highest stiffness. Explain your reasoning.", "", opinionTF2.text);
        writeToOutputFile("output3.csv", "Step 5: Pick one to continue with out of the two most diverse zoned designs. Explain your reasoning.", "", opinionTF25.text);

        BSO::Structural_Design::SD_Building_Results& SD_results = SD_Building.get()->get_results();
        SD_results.obtain_results();
        std::cout << "Total compliance: " << SD_results.m_total_ghost_compliance << SD_results.m_struct_volume << SD_results.m_struct_ghost_volume << std::endl;
        writeToOutputFile("output2.csv", "Total compliance:", std::to_string(SD_results.m_total_compliance), "");
        writeToOutputFile("output2.csv", "Structural volume:", std::to_string(SD_results.m_struct_volume), "");
        writeToOutputFile("output2.csv", "Ghost volume:", std::to_string(SD_results.m_struct_ghost_volume), "");
    }
    if (screen == 34) {
        writeToOutputFile("output3.csv", "Step 6: This time pick again out of all zoned designs. Explain your reasoning.", "", opinionTF27.text);
    }
    if (screen == 8) {
        writeToOutputFile("output3.csv", "1..", getSelectedButtonLabel(), opinionTF3.text);
    }
    if (screen == 9) {
        writeToOutputFile("output3.csv", "2..", getSelectedButtonLabel(), opinionTF4.text);
    }
    if (screen == 10) {
        writeToOutputFile("output3.csv", "3..", getSelectedButtonLabel(), opinionTF5.text);
	}
    if (screen == 11) {
        writeToOutputFile("output3.csv", "4..", getSelectedButtonLabel(), opinionTF6.text);
    }
    if (screen == 12) {
        writeToOutputFile("output3.csv", "5..", getSelectedButtonLabel(), opinionTF7.text);
	}
    if (screen == 36) {
        writeToOutputFile("output3.csv", "6. What criteria did you keep in mind while performing this assignment?", "", opinionTF8.text);
	}
    if (screen == 13) {
        writeToOutputFile("output3.csv", "7..", getSelectedButtonLabel(), opinionTF26.text);
    }
    if (screen == 37) {
        writeToOutputFile("output3.csv", "e-mail adress:", getSelectedButtonLabel(), opinionTF24.text);
    }

    glutPostRedisplay();
    buttons.clear();
}


bool windowOpen = true; // Variable to track whether the window is open or closed

void closeWindowCallback(int) {
    // Close the window
    glutLeaveMainLoop();
    windowOpen = false;
    exit(EXIT_SUCCESS);
}

void drawText(const char* text, float centerX, float centerY, float textWidth) {
    float lineHeight = 18; // Approximate line height, adjust as needed
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    // Calculate the starting position (left align within the margin)
    float startX = centerX - effectiveTextWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    for (const char* c = text; *c != '\0'; c++) {
        // Check if we need to wrap the line
        if ((currentX - startX > effectiveTextWidth) && (*c == ' ' || *c == '\n')) {
            currentY -= lineHeight;
            currentX = startX-4;
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

void drawTextRed(const char* text, float centerX, float centerY, float textWidth) {
    float lineHeight = 18; // Approximate line height, adjust as needed
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    // Calculate the starting position (left align within the margin)
    float startX = centerX - effectiveTextWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    for (const char* c = text; *c != '\0'; c++) {
        // Check if we need to wrap the line
        if ((currentX - startX > effectiveTextWidth) && (*c == ' ' || *c == '\n')) {
            currentY -= lineHeight;
            currentX = startX - 4;
        }

        glRasterPos2f(currentX, currentY);

        // Set text color to black
        glColor3f(1.0, 0.0, 0.0); // red color for text

        // Draw the character
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

        // Move to the next character position
        currentX += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

//Function to draw small text (size 12). Same as the ormal drawText function, but with a different font size.
void drawText2(const char* text, float centerX, float centerY, float textWidth) {
    float lineHeight = 14; // Approximate line height for font size 12
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    // Calculate the starting position (left align within the margin)
    float startX = centerX - effectiveTextWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    for (const char* c = text; *c != '\0'; c++) {
        // Check if we need to wrap the line
        if ((currentX - startX > effectiveTextWidth) && (*c == ' ' || *c == '\n')) {
            currentY -= lineHeight;
            currentX = startX;
        }

        glRasterPos2f(currentX, currentY);

        // Set text color to black
        glColor3f(0.0, 0.0, 0.0); // black color for text

        // Draw the character
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

        // Move to the next character position
        currentX += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// Function to draw text centered within a given width, used within the draw button function. 
void drawTextCentered(const char* text, float centerX, float centerY, float textWidth) {
    float lineHeight = 18; // Approximate line height, adjust as needed
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    float totalLineWidth = 0.0f; // Total width of the current line
    float maxLineWidth = 0.0f; // Maximum width among all lines

    for (const char* c = text; *c != '\0'; c++) {
        // Check for line break
        if (*c == '\n') {
            maxLineWidth = fmax(maxLineWidth, totalLineWidth);
            totalLineWidth = 0.0f; // Reset total width for the new line
            continue;
        }

        // Accumulate width of each character
        totalLineWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
    }
    // Update maxLineWidth if needed (in case the last line didn't have a line break)
    maxLineWidth = fmax(maxLineWidth, totalLineWidth);

    // Calculate the starting position (center align within the margin)
    float startX = centerX - maxLineWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    for (const char* c = text; *c != '\0'; c++) {
        // Check for line break
        if (*c == '\n') {
            // Move to the next line
            currentY -= lineHeight;
            currentX = startX; // Reset X position for the new line
            continue;
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

// Function to draw bold text
void drawBoldText(const char* text, float centerX, float centerY, float textWidth, float boldnessOffset) {
    float lineHeight = 18; // Approximate line height, adjust as needed
    float effectiveTextWidth = textWidth - 2 * MARGIN_PERCENT; // Effective width after considering margins

    // Calculate the starting position (left align within the margin)
    float startX = centerX - effectiveTextWidth / 2.0f;
    float currentX = startX;
    float currentY = centerY;

    // Set text color to black
    glColor3f(0.0, 0.0, 0.0); // black color for text

    // First, draw the text normally
    drawText(text, centerX, centerY, textWidth);

    // Then, draw the text with a variable offset to simulate less boldness
    glRasterPos2f(centerX + boldnessOffset, centerY + boldnessOffset); // Adjust the offset to control boldness
    drawText(text, centerX + boldnessOffset, centerY + boldnessOffset, textWidth);
}

void motion(int x, int y)
{
    // Calculate the boundary of the 3D view area
    float viewWidth = screenWidth / 1.7;

    // Only perform rotation if the mouse is within the 3D view area
    if (x <= viewWidth) {
        double dx = prevx - x;
        double dy = prevy - y;

        cam_local.setrotation(cam_local.getrotation() + (dx * 0.5));
        cam_local.setelevation(cam_local.getelevation() + (dy * 0.5));

        prevx = x;
        prevy = y;
    }

    vpmanager_local.mousemove_event(x, y);

    glutPostRedisplay();
}

void passive_motion(int x, int y)
{
    vpmanager_local.mousemove_event(x, y);
}

void setup2D() {
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, screenWidth, 0.0, screenHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    // Disable lighting for 2D
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}


void setup3D() {
    GLint viewportWidth = screenWidth / 1.7;
    GLint viewportHeight = screenHeight;

    vpmanager_local.resize(viewportWidth, viewportHeight);

    // Set the viewport to cover the left part of the screen
    glViewport(0, 0, viewportWidth, viewportHeight);

    // Setup the projection matrix for 3D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Adjust the perspective projection to match the new aspect ratio
    GLfloat aspectRatio = (GLfloat)viewportWidth / (GLfloat)viewportHeight;
    gluPerspective(45.0, aspectRatio, 0.1f, 1000.0f);

    // Switch back to modelview matrix mode
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Enable depth testing, required for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Enable lighting if your visualization uses it
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void display() {
    //regarding closing the window in the end
    if (!windowOpen) {
        return; // Don't render anything if the window is closed
    }

    // Clear the window with white background
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //white
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f); //very light gray
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up 2D projection matrix
    setup2D();
    // visualise(MS);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    bool active = visualisationActive_3a || visualisationActive_3b || visualisationActive_3c || visualisationActive_3d || visualisationActive_3e || visualisationActive_3f;

    if (active) {
        // Set viewport for the left half of the screen
        setup3D();

        // Render the visualisation
        vpmanager_local.render(cam_local);
        checkGLError("render");

        // Reset the viewport to full window size for the rest of your GUI, if necessary
        setup2D();
        checkGLError("setup2D");
    }

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
        case 18: screenAddSpace(); break;
        case 19: screenDeleteSpace(); break;
        case 20: screenMoveSpace(); break;
        case 21: screenResizeSpace(); break;
        case 26: screenCheckNext1(); break;
        case 27: screenCheckNext2(); break;
        case 28: screenCheckNext3(); break;
        case 29: screenCheckNext4(); break;
        case 30: screenCheckNext5(); break;
		case 31: screenCheckNext6(); break;
        case 32: screenCheckNext7(); break;
        case 33: screen3f(); break;
        case 34: screen3g(); break;
        case 35: screen3g2(); break;
        case 36: screen4g(); break;
        case 37: screen5b(); break;
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

// Function to parse a string containing multiple integer values separated by a delimiter
std::vector<int> parseValues(const std::string& input, char delimiter) {
    std::vector<int> values;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        try {
            // Convert the token to an integer and add it to the vector
            int value = std::stoi(token);
            values.push_back(value);
        }
        catch (const std::invalid_argument& e) {
            // Handle invalid input format (non-integer values)
            std::cerr << "Invalid input format: " << e.what() << std::endl;
            return {};
        }
    }

    return values;
}

void keyboard(unsigned char key, int x, int y) {

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
    }

    if(currentScreen == 3) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF.text != "") { // Backspace key
            opinionTF.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 4) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF2.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF2.text != "") { // Backspace key
            opinionTF2.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 6) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF25.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF25.text != "") { // Backspace key
            opinionTF25.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 33) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF27.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF27.text != "") { // Backspace key
            opinionTF27.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 7) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF3.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF3.text != "") { // Backspace key
            opinionTF3.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 8) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF4.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF4.text != "") { // Backspace key
            opinionTF4.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 9) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF5.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF5.text != "") { // Backspace key
            opinionTF5.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 10) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF6.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF6.text != "") { // Backspace key
            opinionTF6.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 11) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF7.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF7.text != "") { // Backspace key
            opinionTF7.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 12) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF8.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF8.text != "") { // Backspace key
            opinionTF8.text.pop_back(); // Remove the last character from input string
        }

    }

    if (currentScreen == 36) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF26.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF26.text != "") { // Backspace key
            opinionTF26.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 13) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF24.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF24.text != "") { // Backspace key
            opinionTF24.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 18) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            if (opinionTF13.isActive) {
                opinionTF13.text += key; // Append the character to the input string
            }
            else if (opinionTF14.isActive) {
                opinionTF14.text += key; // Append the character to the input string
            }
            DrawInvalidInput = false;
        }
        else if (key == 8) { // Backspace key
            if (opinionTF13.isActive && !opinionTF13.text.empty()) {
                opinionTF13.text.pop_back(); // Remove the last character from input string
            }
            else if (opinionTF14.isActive && !opinionTF14.text.empty()) {
                opinionTF14.text.pop_back(); // Remove the last character from input string
            }
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;
            //Needed variables
            std::shared_ptr<BSO::Spatial_Design::MS_Building> msBuilding = MS;
            BSO::Spatial_Design::MS_Space new_space;
            double x1 = 0.0, y1 = 0.0, z1 = 0.0;
            double width1 = 0.0, depth1 = 0.0, height1 = 0.0;

            if (!opinionTF13.text.empty()) {
                // Print the entered text from opinionTF13 to the terminal
                std::cout << "Entered text (opinionTF13): " << opinionTF13.text << std::endl;
                // Write the entered text from opinionTF13 to the process file
                writeToProcessFile("process3.csv", "Add Space: size", opinionTF13.text);

                std::vector<int> sizes = parseValues(opinionTF13.text, ',');
                if (sizes.size() == 3) {
                    width1 = sizes[0] * 100;
                    depth1 = sizes[1] * 100;
                    height1 = sizes[2] * 100;
                }
                else {
                    std::cout << "Invalid input format" << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle empty input gracefully
                std::cout << "Error: input is empty." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
            }

            if (!opinionTF14.text.empty()) {
                // Print the entered text from opinionTF14 to the terminal
                std::cout << "Entered text (opinionTF14): " << opinionTF14.text << std::endl;
                // Write the entered text from opinionTF14 to the process file
                writeToProcessFile("process3.csv", "Add Space: location", opinionTF14.text);

                std::vector<int> location = parseValues(opinionTF14.text, ',');
                if (location.size() == 3) {
                    x1 = location[0] * 100;
                    y1 = location[1] * 100;
                    z1 = location[2] * 100;
                }
                else {
                    std::cout << "Invalid input format" << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle empty input gracefully
                std::cout << "Error: input is empty." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput) {
                DrawInvalidInput = false;

                int nextID = 1;
                for (int i = 1; i <= 20; ++i) {
                    bool spaceExists = false;
                    for (int j = 0; j < MS->obtain_space_count(); ++j) {
                        if (MS->get_space_ID(j) == i) {
                            spaceExists = true;
                            break;
                        }
                    }
                    if (!spaceExists) {
                        // Found an available ID, set it to nextID and exit the loop
                        nextID = i;
                        break;
                    }
                }

                // add space
                new_space.ID = nextID;
                new_space.x = x1;
                new_space.y = y1;
                new_space.z = z1;
                new_space.width = width1;
                new_space.depth = depth1;
                new_space.height = height1;
                // Set other space properties as needed...
                std::cout << "Added space " << new_space.ID << " with location and size: (" << new_space.x << ", " << new_space.y << ", " << new_space.z << ", " << new_space.width << ", " << new_space.depth << ", " << new_space.height << ")" << std::endl;
                // Add the space to the building
                MS->add_space(new_space);

                // Clear the input strings after processing
                opinionTF13.text = "";
                opinionTF14.text = "";
                // Change the screen after processing both text fields
                changeScreen(5);
                modificationCount++;

            }
            else {
                writeToProcessFile("process3.csv", "", "above input invalid");
            }
        }
        else if (key == '\t') { // Tab key
            // Toggle active state between opinionTF13 and opinionTF14
            opinionTF13.isActive = !opinionTF13.isActive;
            opinionTF14.isActive = !opinionTF14.isActive;
        }
    }

    if (currentScreen == 19) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF15.text += key; // Append the character to the input string
            DrawInvalidInput = false;
        }
        else if (key == 8 && opinionTF15.text != "") { // Backspace key
            opinionTF15.text.pop_back(); // Remove the last character from input string
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;
            if (!opinionTF15.text.empty()) {
                // Print the entered text to the terminal
                std::cout << "Entered text: " << opinionTF15.text << std::endl;
                // Write the entered text to the process file
                writeToProcessFile("process3.csv", "Delete Space", opinionTF15.text);

                std::stringstream ss(opinionTF15.text);
                int space_ID;
                if (!(ss >> space_ID)) {
                    // Handle invalid space ID format gracefully
                    std::cout << "Error: Invalid space ID format." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
                else {
                    // Check if the space ID exists in the list of spaces
                    bool spaceFound = false;
                    // Get the index of the space with the specified ID
                    for (int i = 0; i < MS->obtain_space_count(); ++i) {
                        if (MS->get_space_ID(i) == space_ID) {
                            spaceFound = true;
                            int space_index = MS->get_space_index(space_ID);
                            break;
                        }
                    }
                    if (!spaceFound) {
                        // Handle space not found gracefully
                        std::cout << "Error: Space with ID " << space_ID << " not found." << std::endl;
                        validInput = false;
                        DrawInvalidInput = true;
                    }
                }
            }
            else {
                // Handle empty space ID input gracefully
                std::cout << "Error: Space ID input is empty." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput) {
                DrawInvalidInput = false;

                // Delete the space at the specified index
                std::stringstream ss(opinionTF15.text);
                int space_ID;
                ss >> space_ID;

                int space_index = MS->get_space_index(space_ID);
                MS->delete_space(space_index);

                opinionTF15.text = ""; // Clear the input string after processing
                changeScreen(5);
                modificationCount++;

            }
            else {
                // Handle empty space ID input gracefully
                std::cout << "Error: Space ID input is empty." << std::endl;
                writeToProcessFile("process3.csv", "", "above input invalid");
            }
        }
    }

    if (currentScreen == 20) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            DrawInvalidInput = false;
            if (opinionTF16.isActive) {
                opinionTF16.text += key; // Append the character to the input string
            }
            else if (opinionTF17.isActive) {
                opinionTF17.text += key; // Append the character to the input string
            }
        }
        else if (key == 8) { // Backspace key
            DrawInvalidInput = false;
            if (opinionTF16.isActive && !opinionTF16.text.empty()) {
                opinionTF16.text.pop_back(); // Remove the last character from input string
            }
            else if (opinionTF17.isActive && !opinionTF17.text.empty()) {
                opinionTF17.text.pop_back(); // Remove the last character from input string
            }
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;
            //Needed variables
            std::shared_ptr<BSO::Spatial_Design::MS_Building> msBuilding = MS;
            BSO::Spatial_Design::MS_Space new_space;
            double x3 = 0.0, y3 = 0.0, z3 = 0.0;
            double width3 = 0.0, depth3 = 0.0, height3 = 0.0;

            if (!opinionTF16.text.empty()) {
                // Print the entered text from opinionTF16 to the terminal
                std::cout << "Entered text (opinionTF16): " << opinionTF16.text << std::endl;
                // Write the entered text from opinionTF16 to the process file
                writeToProcessFile("process3.csv", "Move Space: space", opinionTF16.text);
                // Clear the input string of opinionTF16 after processing

                std::stringstream ss(opinionTF16.text);
                int space_ID;
                if (!(ss >> space_ID)) {
                    // Handle invalid space ID format gracefully
                    std::cout << "Error: Invalid space ID format." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
                else {
                    // Check if the space ID exists in the list of spaces
                    bool spaceFound = false;
                    // Get the index of the space with the specified ID
                    for (int i = 0; i < MS->obtain_space_count(); ++i) {
                        if (MS->get_space_ID(i) == space_ID) {
                            spaceFound = true;
                            int space_index = MS->get_space_index(space_ID);
                            break;
                        }
                    }
                    if (!spaceFound) {
                        // Handle space not found gracefully
                        std::cout << "Error: Space with ID " << space_ID << " not found." << std::endl;
                        validInput = false;
                        DrawInvalidInput = true;
                    }
                }
            }
            if (!opinionTF17.text.empty()) {
                // Print the entered text from opinionTF17 to the terminal
                std::cout << "Entered text (opinionTF17): " << opinionTF17.text << std::endl;
                // Write the entered text from opinionTF17 to the process file
                writeToProcessFile("process3.csv", "Move Space: new location", opinionTF17.text);
                // Clear the input string of opinionTF17 after processing

                std::vector<int> location = parseValues(opinionTF17.text, ',');
                if (location.size() != 3) {
                    // Handle invalid location format gracefully
                    std::cout << "Error: Invalid location format." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle empty location input gracefully
                std::cout << "Error: Location input is empty." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput) {
                DrawInvalidInput = false;

                std::vector<int> location = parseValues(opinionTF17.text, ',');
                int x3 = location[0] * 100;
                int y3 = location[1] * 100;
                int z3 = location[2] * 100;

                // Delete the space at the specified index
                std::stringstream ss(opinionTF16.text);
                int space_ID;
                ss >> space_ID;

                int space_index = MS->get_space_index(space_ID);
                //current sizes of the space
                BSO::Spatial_Design::MS_Space space = msBuilding->obtain_space(space_index);
                std::cout << "Width: " << space.width << ", Depth: " << space.depth << ", Height: " << space.height << std::endl;

                MS->delete_space(space_index);

                // Create a new space object with the parsed properties
                BSO::Spatial_Design::MS_Space new_space;
                new_space.ID = space_ID;
                new_space.x = x3;
                new_space.y = y3;
                new_space.z = z3;
                new_space.width = space.width; //existing width
                new_space.depth = space.depth; //existing depth
                new_space.height = space.height; //existing height
                // Set other space properties as needed...
                std::cout << "Moved space " << new_space.ID << " to location: (" << new_space.x << ", " << new_space.y << ", " << new_space.z << ")" << std::endl;
                // Add the space to the building
                MS->add_space(new_space);

                // Clear the input strings after processing
                opinionTF16.text = "";
                opinionTF17.text = "";
                // Change the screen after processing both text fields
                changeScreen(5);
                modificationCount++;

            }
            else {
                writeToProcessFile("process3.csv", "", "above input invalid");
            }
        }
        else if (key == '\t') { // Tab key
            // Toggle active state between opinionTF16 and opinionTF17
            opinionTF16.isActive = !opinionTF16.isActive;
            opinionTF17.isActive = !opinionTF17.isActive;
        }
    }

    if (currentScreen == 21) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            DrawInvalidInput = false;
            if (opinionTF18.isActive) {
                opinionTF18.text += key; // Append the character to the input string
            }
            else if (opinionTF19.isActive) {
                opinionTF19.text += key; // Append the character to the input string
            }
        }
        else if (key == 8) { // Backspace key
            DrawInvalidInput = false;
            if (opinionTF18.isActive && !opinionTF18.text.empty()) {
                opinionTF18.text.pop_back(); // Remove the last character from input string
            }
            else if (opinionTF19.isActive && !opinionTF19.text.empty()) {
                opinionTF19.text.pop_back(); // Remove the last character from input string
            }
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;
            //Needed variables
            std::shared_ptr<BSO::Spatial_Design::MS_Building> msBuilding = MS;
            BSO::Spatial_Design::MS_Space new_space;
            double x4 = 0.0, y4 = 0.0, z4 = 0.0;
            double width4 = 0.0, depth4 = 0.0, height4 = 0.0;

            if (!opinionTF18.text.empty()) {
                // Print the entered text from opinionTF18 to the terminal
                std::cout << "Entered text (opinionTF18): " << opinionTF18.text << std::endl;
                // Write the entered text from opinionTF18 to the process file
                writeToProcessFile("process3.csv", "Resize Space: space", opinionTF18.text);

                std::stringstream ss(opinionTF18.text);
                int space_ID;
                if (!(ss >> space_ID)) {
                    // Handle invalid space ID format gracefully
                    std::cout << "Error: Invalid space ID format." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
                else {
                    // Check if the space ID exists in the list of spaces
                    bool spaceFound = false;
                    // Get the index of the space with the specified ID
                    for (int i = 0; i < MS->obtain_space_count(); ++i) {
                        if (MS->get_space_ID(i) == space_ID) {
                            spaceFound = true;
                            int space_index = MS->get_space_index(space_ID);
                            break;
                        }
                    }
                    if (!spaceFound) {
                        // Handle space not found gracefully
                        std::cout << "Error: Space with ID " << space_ID << " not found." << std::endl;
                        validInput = false;
                        DrawInvalidInput = true;
                    }
                }
            }
            if (!opinionTF19.text.empty()) {
                // Print the entered text from opinionTF19 to the terminal
                std::cout << "Entered text (opinionTF19): " << opinionTF19.text << std::endl;
                // Write the entered text from opinionTF19 to the process file
                writeToProcessFile("process3.csv", "Resize Space: new size", opinionTF19.text);

                std::vector<int> sizes = parseValues(opinionTF19.text, ',');
                if (sizes.size() != 3) {
                    // Handle invalid location format gracefully
                    std::cout << "Error: Invalid location format." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle empty location input gracefully
                std::cout << "Error: Location input is empty." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput) {
                DrawInvalidInput = false;

                std::vector<int> sizes = parseValues(opinionTF19.text, ',');
                int width4 = sizes[0] * 100;
                int depth4 = sizes[1] * 100;
                int height4 = sizes[2] * 100;

                // Delete the space at the specified index
                std::stringstream ss(opinionTF18.text);
                int space_ID;
                ss >> space_ID;

                int space_index = MS->get_space_index(space_ID);
                //current sizes of the space
                BSO::Spatial_Design::MS_Space space = msBuilding->obtain_space(space_index);
                std::cout << "Width: " << space.width << ", Depth: " << space.depth << ", Height: " << space.height << std::endl;

                MS->delete_space(space_index);

                // Create a new space object with the parsed properties
                BSO::Spatial_Design::MS_Space new_space;
                new_space.ID = space_ID;
                new_space.x = space.x; //existing x
                new_space.y = space.y; //existing y
                new_space.z = space.z; //existing z
                new_space.width = width4;
                new_space.depth = depth4;
                new_space.height = height4;
                // Set other space properties as needed...
                std::cout << "Resized space " << new_space.ID << " to location: (" << new_space.width << ", " << new_space.depth << ", " << new_space.height << ")" << std::endl;
                // Add the space to the building
                MS->add_space(new_space);

                // Clear the input strings after processing
                opinionTF18.text = "";
                opinionTF19.text = "";
                // Change the screen after processing both text fields
                changeScreen(5);
                modificationCount++;

            }
            else {
                writeToProcessFile("process3.csv", "", "above input invalid");
            }
        }
        else if (key == '\t') { // Tab key
            // Toggle active state between opinionTF18 and opinionTF19
            opinionTF18.isActive = !opinionTF18.isActive;
            opinionTF19.isActive = !opinionTF19.isActive;
        }
    }

  

    // Redraw screen
    glutPostRedisplay();
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

    // Set button background color based on whether it's clicked or not
    if (getSelectedButtonLabel() == text) {
        // Change the background color when clicked
        glColor3f(0.1, 0.75, 0.9); //light blue color for button background
    }
    else {
        //glColor3f(1.0, 1.0, 1.0); // White color for button background
        glColor3f(0.961, 0.961, 0.863); //beige color for button background
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Centered text within the button with margin
    float centerX = x + width / 2;
    float centerY = y + (height - 11) / 2;
    float textWidth = width - 2 * MARGIN_PERCENT; // Text width considering margin

    // Set text color to black
    glColor3f(0.0, 0.0, 0.0);
    drawTextCentered(text, centerX, centerY, textWidth);

    Button button = {x, y, width, height, callback, text, variable};
    buttons.push_back(button);
}

// Function to reset the selected button label
void resetSelectedButtonLabel() {
    selectedButtonLabel = "";
}

void drawButtonWithBackgroundColor(const char* text, float x, float y, float width, float height, ButtonCallback callback, int variable) {
    // Draw button with specified background color
    float borderWidth = 2.0;

    glColor3f(0.0, 0.0, 0.0); // Black color for border
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();

    // Draw button rectangle with any color background
    //glColor3f(0.8, 0.8, 0.8); //for light gray
    //glColor3f(0.5, 0.5, 0.5); //for dark gray
    //glColor3f(1.0, 0.5, 0.5); //for light red
    //glColor3f(1.0, 0.7, 0.4); //for light orange
    //glColor3f(0.678, 0.847, 0.902); //for light blue
    glColor3f(0.1, 0.75, 0.9); //for light blue brighter
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Centered text within the button with margin
    float centerX = x + width / 2;
    float centerY = y + (height - 11) / 2;
    float textWidth = width - 2 * MARGIN_PERCENT; // Text width considering margin

    // Set text color to black
    glColor3f(0.0, 0.0, 0.0);
    drawTextCentered(text, centerX, centerY, textWidth);

    Button button = { x, y, width, height, callback, text, variable };
    buttons.push_back(button);
}

void drawTextField(int x, int y, int width, int height, TextField& textfield) {
    textfield.x = x;
    textfield.y = y;
    textfield.width = width;
    textfield.height = height;
    float borderWidth = 2.0;

    // Calculate the adjusted width and height considering padding
    int adjustedWidth = width - 4 * borderWidth;
    int adjustedHeight = height - 4 * borderWidth;

    glColor3f(0.0, 0.0, 0.0); // Black color for border
    glBegin(GL_QUADS);
    glVertex2f(x - borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y - borderWidth);
    glVertex2f(x + width + borderWidth, y + height + borderWidth);
    glVertex2f(x - borderWidth, y + height + borderWidth);
    glEnd();

    // Draw text field background
    if (textfield.isActive) {
        // Set color for active text field
        //glColor3f(0.8, 0.8, 0.8); // light gray background for active text field
        //glColor3f(1.0, 1.0, 0.91); // light beige background for active text field
        glColor3f(0.75f, 0.9f, 0.75f); // light green background for active text field
    }
    else {
        // Set color for inactive text field
        glColor3f(1.0, 1.0, 1.0); // white background for inactive text field
    }
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
    int maxWidth = adjustedWidth - 4.0; // maximum width for text before wrapping
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

    /*
        // Draw the cursor if the text field is active
        if (textfield.isActive) {
            int cursorX = x + borderWidth + glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)textfield.text.c_str()); // Adjust for left padding
            int cursorY = startY; // Use the same starting Y coordinate as the text
            glColor3f(0.0, 0.0, 0.0); // black cursor
            glLineWidth(1.0);
            glBegin(GL_LINES);
            glVertex2f(cursorX + 2, cursorY + 18); // Adjust the Y coordinate to draw the cursor above the text
            glVertex2f(cursorX + 2, cursorY - 3);  // Adjust the Y coordinate to draw the cursor above the text
            glEnd();
        }
    */
}

void checkTextFieldClick(TextField& textField, float mouseX, float mouseY) {
    if (mouseX >= textField.x && mouseX <= textField.x + textField.width &&
        mouseY >= textField.y && mouseY <= textField.y + textField.height) {
        textField.isActive = true; // Activate the clicked text field
    }
    else {
        textField.isActive = false; // Deactivate the text field if not clicked
    }
}

void onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float mouseY = screenHeight - static_cast<float>(y);
        float mouseX = static_cast<float>(x);

        // Check each text field individually
        checkTextFieldClick(opinionTF, mouseX, mouseY);
        checkTextFieldClick(opinionTF2, mouseX, mouseY);
        checkTextFieldClick(opinionTF3, mouseX, mouseY);
        checkTextFieldClick(opinionTF4, mouseX, mouseY);
        checkTextFieldClick(opinionTF5, mouseX, mouseY);
        checkTextFieldClick(opinionTF6, mouseX, mouseY);
        checkTextFieldClick(opinionTF7, mouseX, mouseY);
        checkTextFieldClick(opinionTF8, mouseX, mouseY);
        checkTextFieldClick(opinionTF13, mouseX, mouseY);
        checkTextFieldClick(opinionTF14, mouseX, mouseY);
        checkTextFieldClick(opinionTF15, mouseX, mouseY);
        checkTextFieldClick(opinionTF16, mouseX, mouseY);
        checkTextFieldClick(opinionTF17, mouseX, mouseY);
        checkTextFieldClick(opinionTF18, mouseX, mouseY);
        checkTextFieldClick(opinionTF19, mouseX, mouseY);
        checkTextFieldClick(opinionTF24, mouseX, mouseY);
        checkTextFieldClick(opinionTF25, mouseX, mouseY);
        checkTextFieldClick(opinionTF26, mouseX, mouseY);
        checkTextFieldClick(opinionTF27, mouseX, mouseY);

        // Check for button clicks

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

void mainScreen() {
    glColor3f(0.0, 0.0, 0.0);
    drawText("Welcome to this experiment for a SED graduation project. We are glad to have you here and hope you will have a nice experience.", 930, 820, 400);
    drawText("In which assignment will you participate?", 930, 740, 400);

    drawButton("Assignment 1", 800, 650, 200, 50, buttonClicked, 1);
    drawButton("Assignment 2", 800, 580, 200, 50, buttonClicked, 1);
    drawButton("Assignment 3", 800, 510, 200, 50, changeScreen, 1);
    drawButton("Assignment 4", 800, 440, 200, 50, buttonClicked, 1);

    // Draw the "Next step" button in the bottom right corner
    //drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 1);
}

void assignmentDescriptionScreen() {
    drawText("Selected Assignment: 3 'Human-AI zoning assignment'​", 900, 740, 400);
    drawText("Expected duration: 25 minutes​", 900, 710, 400);
    drawText("Read the following instructions carefully:​", 900, 650, 400);
    drawText("You will in a moment go through a design task. You are asked to perform this task in the way you are used to go about a commission in your daily practice. It is important that you say aloud everything that you think or do in designing. ​So, in every step, explain what you do and why you do it. Try to keep speaking constantly and not be silent for longer than 20 seconds. ​Please speak English. Good luck!​",
    900, 600, 400);
    //underline ENGLISH
    //glLineWidth(2.0);
    //glColor3f(0.0, 0.0, 0.0);
    //glBegin(GL_LINES);
    //glVertex2f(820.0f, 470.0f);    // Start point of the line at the top
    //glVertex2f(890.0f, 470.0f); // End point of the line at the bottom
    //glEnd();

    drawButton("<- | Previous step", 1380, 50, 200, 50, changeScreen, 0);
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 31);
}

void LineDivisionScreen() {
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();
}

void ReadInstructions() {
    //Message to summarize most important information and to refer to the full information in the instructions
    drawText("Please refer to the information sheet for an explanation about the concept of zoning.  ", 1550, screenHeight - 130, 250);
    //underline INSTRUCTIONS
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1582.0, 868.0);
    glVertex2f(1678.0, 868.0);
    glVertex2f(1428.0, 850.0);
    glVertex2f(1481.0, 850.0);
    glEnd();
}

void ReadInstructions2() {
    //Message to summarize most important information and to refer to the full information in the instructions
    drawText("Please refer to the information sheet for more information about zoning and SD.  ", 1550, screenHeight - 200, 250);
    //underline INSTRUCTIONS
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1582.0, 798.0);
    glVertex2f(1678.0, 798.0);
    glVertex2f(1428.0, 780.0);
    glVertex2f(1481.0, 780.0);
    glEnd();
}

void ReadInstructions3() {
    //Message to summarize most important information and to refer to the full information in the instructions
    drawText("Please refer to the information sheet for more information about zoning and SD.  ", 1550, screenHeight - 155, 250);
    //underline INSTRUCTIONS
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1582.0, 843.0);
    glVertex2f(1678.0, 843.0);
    glVertex2f(1428.0, 825.0);
    glVertex2f(1481.0, 825.0);
    glEnd();
}

void ReadInstructions4() {
    //Message to summarize most important information and to refer to the full information in the instructions
    drawText("Please refer to the information sheet for more information about zoning and SD.  ", 1550, screenHeight - 235, 250);
    //underline INSTRUCTIONS
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1582.0, 763.0);
    glVertex2f(1678.0, 763.0);
    glVertex2f(1428.0, 745.0);
    glVertex2f(1481.0, 745.0);
    glEnd();
}

void screen3a() {
    // Screen layout and colors should be adjusted as necessary.

    // Draw structural design illustration placeholder (left side)
    // BSO::Spatial_Design::MS_Building MS("MS_Input.txt");

    // BSO::Visualisation::init_visualisation_without();
    // BSO::Visualisation::visualise(MS);
    // BSO::Visualisation::end_visualisation();

    LineDivisionScreen();

    // Draw the bottom area where zones and zoned designs are displayed
    std::string number_zones = std::string("Zones: ") + std::to_string(Zoned->get_designs().size());
    //drawText(number_zones.c_str(), 100, 300, 200);
    //drawText("Zoned designs: 0", 100, 150, 200);

    // Draw the message at the top of the structure illustration
    drawBoldText("Step 1: The visualization on the left shows a BSD. AI has found all zones and zoned designs for this BSD; you can continue to the next step.", 1550, screenHeight - 50, 250, 1);

    //step vs steps to go as a time indication for the user
    drawText("Step 1/8", screenWidth, screenHeight - 25, 180);

    ReadInstructions2();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 26);
}

void screen3b() {
    // Draw structural design illustration placeholder (left side)

    LineDivisionScreen();

    // Draw the bottom area where zones and zoned designs are displayed
    //drawText("Zoned designs:", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1510, 600, 150, 50, opinionTF);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 1570, 750, 275);

    // Draw the message at the bottom of the structure illustration
    drawBoldText("Step 2: Pick one zoned design you would like to continue with. Say aloud what you think.", 1550, screenHeight - 50, 250, 1);

    //step vs steps to go as a time indication for the user
    drawText("Step 2/8", screenWidth, screenHeight - 25, 180);

    ReadInstructions();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 27);
}

void screen3c() {
    // Draw structural design illustration placeholder (left side)

    LineDivisionScreen();

    // Draw the bottom area where zones and zoned designs are displayed
    //drawText("Zoned designs:", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1510, 600, 150, 50, opinionTF2);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 1570, 750, 275);

    // Draw the message at the bottom of the structure illustration
    drawBoldText("Step 3: This time, pick one based on the expected structural performace of the zoned designs. Say aloud what your reasoning is.", 1550, screenHeight - 50, 250, 1);

    //step vs steps to go as a time indication for the user
    drawText("Step 3/8", screenWidth, screenHeight - 25, 180);

    ReadInstructions3();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 28);
}

void screen3d() {
    // Draw structural design illustration placeholder (left side)
    LineDivisionScreen();

/*
    // Draw counter area
    //drawText("Modifications: 0/7", 1300, screenHeight - 100, 200);
    std::string modificationCountStr = "Modifications: " + std::to_string(modificationCount) + "/7";
    drawText(modificationCountStr.c_str(), 1300, screenHeight - 100, 200);

    // Draw the buttons, they work until the modification count reaches 7
    if (modificationCount == 7) {
        // If modification count reaches 7, change the message on screen3d
        drawText("7/7 modifications reached", 1590, 300, 200);
        // Draw control buttons without them working
        drawButton("Add space", screenWidth - 310, 610, 200, 50, buttonClicked, 1);
        drawButton("Delete space", screenWidth - 310, 550, 200, 50, buttonClicked, 1);
        drawButton("Move space", screenWidth - 310, 490, 200, 50, buttonClicked, 1);
        drawButton("Resize space", screenWidth - 310, 430, 200, 50, buttonClicked, 1);
    }
*/

    //draw a message when input is invalid. it is handled in the keyboard function
    if (DrawInvalidInput == true) {
        drawText("Invalid input.", 1645, 190, 200);
    }

    // Draw control buttons (right side)
    drawButton("Add space", screenWidth - 310, 610, 200, 50, changeScreen, 18);
    drawButton("Delete space", screenWidth - 310, 550, 200, 50, changeScreen, 19);
    drawButton("Move space", screenWidth - 310, 490, 200, 50, changeScreen, 20);
    drawButton("Resize space", screenWidth - 310, 430, 200, 50, changeScreen, 21);

    // Draw the message at the bottom of the structure illustration
    drawBoldText("Step 4: Adapt the BSD with two changes to create a new BSD you desire. You can do this by adding, deleting, moving, and resizing spaces (one change can consist of more than one action). In the next step, AI will create zoned designs for your new BSD. Say aloud everything you think and do.", 1550, screenHeight - 50, 250, 1);
    glLineWidth(2.0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1658.0, 949.0);
    glVertex2f(1698.0, 949.0);
    glVertex2f(1428.0, 930.0);
    glVertex2f(1510.0, 930.0);
    glEnd();


    //step vs steps to go as a time indication for the user
    drawText("Step 4/8", screenWidth, screenHeight - 25, 180);

    ReadInstructions4();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 29);


    //draw a table with the spaces, to make it easier for the user to keep track of the spaces
    // Draw the table lines
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    // Vertical lines
    glVertex2f(1150.0, 668.0);
    glVertex2f(1150.0, 400); // Adjust the y-coordinate based on your screen layout
    glVertex2f(1280.0, 668.0);
    glVertex2f(1280.0, 400); // Adjust the y-coordinate based on your screen layout
    // Horizontal lines
    glVertex2f(1090.0, 650.0);
    glVertex2f(1390.0, 650.0);
    // Repeat for each horizontal line
    glEnd();

    // Draw the text for table headers
    drawText2("Space ID", 1170, 655, 180); // Adjust coordinates and textWidth as needed
    drawText2("Size (x,y,z)", 1250, 655, 180); // Adjust coordinates and textWidth as needed
    drawText2("Location (x,y,z)", 1380, 655, 180); // Adjust coordinates and textWidth as needed

    std::shared_ptr<BSO::Spatial_Design::MS_Building> msBuilding = MS;
    BSO::Spatial_Design::MS_Space new_space;

    float y = 630.0; // Starting y-coordinate for the first row
    for (int i = 0; i < MS->obtain_space_count(); ++i) {
        // Obtain the space at the current index
        BSO::Spatial_Design::MS_Space space = msBuilding->obtain_space(i);

        int width_int = static_cast<int>(space.width) /100;
        int depth_int = static_cast<int>(space.depth) /100;
        int height_int = static_cast<int>(space.height) /100;
        int x_int = static_cast<int>(space.x) /100;
        int y_int = static_cast<int>(space.y) /100;
        int z_int = static_cast<int>(space.z) /100;

        // Draw space ID
        drawText2((std::to_string(space.ID)).c_str(), 1190, y, 180);

        // Draw space size (width, depth, height)
        std::string sizeStr = std::to_string(width_int) + "," + std::to_string(depth_int) + "," + std::to_string(height_int); // size
        drawText2(sizeStr.c_str(), 1250, y, 180);

        // Draw space location (x, y, z)
        std::string locationStr = std::to_string(x_int) + "," + std::to_string(y_int) + "," + std::to_string(z_int); // location
        drawText2(locationStr.c_str(), 1370, y, 180);

        // Move to the next row
        y -= 20.0; // Adjust this value to set the vertical gap between rows
    }
}

void screen3e() {
    // Draw structural design illustration placeholder (left side)

    LineDivisionScreen();

    // Draw the bottom area where zones and zoned designs are displayed
    //drawText("Zoned designs: 0", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1510, 600, 150, 50, opinionTF25);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 1565, 740, 275);

    // Draw the message at the top of the structure illustration
    drawBoldText("Step 5: The top screens shows your new BSD. AI found all zoned designs. In the bottom screen only the two most diverse zoned designs are shown, selected with IQD. Pick one zoned design you would like to continue with. Say aloud what you think.", 1550, screenHeight - 50, 250, 1);
    //underline two
    glLineWidth(2.0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1501.0f, 895.0f);
    glVertex2f(1535.0f, 895.0f);
    glEnd();


    drawText("Please refer to the information sheet for more information about zoning, SD and IQD.  ", 1550, screenHeight - 200, 250);
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1582.0, 798.0);
    glVertex2f(1678.0, 798.0);
    glVertex2f(1428.0, 780.0);
    glVertex2f(1481.0, 780.0);
    glEnd();

    //step vs steps to go as a time indication for the user
    drawText("Step 5/8", screenWidth, screenHeight - 25, 180);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 30);
}

void screen3f() {
    // Draw structural design illustration placeholder (left side)
    LineDivisionScreen();

    // Draw the bottom area where zones and zoned designs are displayed
    //drawText("Zoned designs: 0", 100, 150, 200);

    //Draw text and a textfield(textbox)
    drawText("Zoned design:", screenWidth - 180, 660, 200);
    drawTextField(1510, 600, 150, 50, opinionTF27);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 1565, 740, 275);

    // Draw the message at the top of the structure illustration
    drawBoldText("Step 6: The top screens shows your new BSD. The bottom screen shows all zoned designs that AI found. Again, pick one zoned design you would like to continue with. Say aloud what you think.", 1550, screenHeight - 50, 250, 1);
    //underline ALL
    glLineWidth(2.0);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1487.0f, 914.0f);
    glVertex2f(1509.0f, 914.0f);
    glEnd();

    ReadInstructions2();

    //step vs steps to go as a time indication for the user
    drawText("Step 6/8", screenWidth, screenHeight - 25, 180);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 32);
}

void screen3g() {
    // Draw structural design illustration placeholder (left side)
    LineDivisionScreen();

    // Draw the message at the top of the structure illustration
    drawBoldText("Step 7: In the next steps, you will need to assess the similarity between these BSDs. Spend max. 5-10 seconds per assessment. You no longer need to speak aloud.", 1550, screenHeight - 50, 250, 1);

    //VISUALS OF ALL BUIDLINGS TO BE ASSESSED

    //step vs steps to go as a time indication for the user
    drawText("Step 7/8, Assessment 0/21", screenWidth - 160, screenHeight - 25, 200);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 35);
}

void screen3g2() {
    // Draw structural design illustration placeholder (left side)
    LineDivisionScreen();

    // Draw the message at the top of the structure illustration
    drawBoldText("Step 7: Assess the similarity between these two BSDs. Spend max. 5-10 seconds per assessment. You no longer need to speak aloud.", 1550, screenHeight - 50, 250, 1);

    drawButton("1", 1450, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 1500, 725, 50, 30, buttonClicked, 2);
    drawButton("3", 1550, 725, 50, 30, buttonClicked, 3);
    drawButton("4", 1600, 725, 50, 30, buttonClicked, 4);
    drawButton("5", 1650, 725, 50, 30, buttonClicked, 5);

    drawText("1: Very different", 1550, 700, 200);
    drawText("5: Very similar", 1550, 670, 200);

    //step vs steps to go as a time indication for the user
    drawText("Step 7/8, Assessment 1/21", screenWidth - 160, screenHeight - 25, 200);

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 7);
}

void screen4a() {
    //draw buttons 1 to 5
    drawText("1. How much did you enjoy performing this assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 2);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 3);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 4);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 5);

    drawText("1: Not at all", 600, 700, 600);
    drawText("5: Very much", 600, 670, 600);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF3);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the bottom of the structure illustration
    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 1/7", screenWidth - 115, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 8);
}

void screen4b() {
    drawText("2. How would you rate the level of ease in performing this assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 2);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 3);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 4);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 5);

    drawText("1: Very hard", 600, 700, 600);
    drawText("5: Very easy", 600, 670, 600);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF4);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 2/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 9);
}

void screen4c() {
    drawText("3. How well do you think you performed the assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 2);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 3);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 4);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 5);

    drawText("1: I have no idea what I am doing, and unable to identify zoned designs and high stiffness.", 700, 700, 800);
    drawText("5: Confident, correct answers, and able to identify zoned designs and high stiffness.", 700, 670, 800);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF5);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 3/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 10);
}

void screen4d() {
    drawText("4. Do you think it would have gone better without the AI tool that identified all zoned designs for you?", 600, 800, 600);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 6);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 7);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 8);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF6);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 4/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 11);
}

void screen4e() {
    drawText("5. Do you think the AI tool itself can perform zoning better than you?", 600, 800, 600);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 6);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 7);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 8);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF7);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 5/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 12);
}

void screen4f() {
    drawText("6. What criteria did you keep in mind while performing this assignment?", 600, 800, 600);
    drawText("(For example, structural, aesthetical, functional, and zoning requirements.)", 600, 770, 600);
    drawTextField(300, 270, 500, 200, opinionTF8);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 6/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 36);
}

void screen4g() {
    drawText("7. In step 5 a limited number of zoned designs (only the two most diverse ones) were shown to you to choose from. Did you prefer choosing from the limited zoned design options or from all zoned design options?", 600, 800, 600);
    drawButton("Limited options", 300, 715, 150, 30, buttonClicked, 9);
    drawButton("All options", 450, 715, 150, 30, buttonClicked, 10);
    drawButton("No idea", 600, 715, 150, 30, buttonClicked, 8);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF26);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    drawBoldText("Step 8: Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Step 8/8, Question 6/7", screenWidth - 115, screenHeight - 25, 180);
    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 13);
}

void screen5() {
    drawText("Please leave your email below if you want us to send you the results from this research and include you in the acknowledgments. Nevertheless, no results will be linked to your name since all results are pseudomized.", 600, 520, 600);
    drawTextField(300, 420, 500, 50, opinionTF24);

    LineDivisionScreen();
    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 37);
}

void screen5b() {
    drawText("Thank you for your participation, this is the end of the assignment.", 600, 800, 600);
    LineDivisionScreen();
    drawButton("-> | End", 1590, 50, 200, 50, closeWindowCallback, 0);
}

void boxAroundPopUp() {
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1470.0f, 350.0f);
    glVertex2f(1470.0f, 225.0f);
    glVertex2f(1470.0f, 350.0f);
    glVertex2f(1710.0f, 350.0f);
    glVertex2f(1710.0f, 350.0f);
    glVertex2f(1710.0f, 225.0f);
    glVertex2f(1710.0f, 225.0f);
    glVertex2f(1470.0f, 225.0f);
    glEnd();
}

void boxAroundPopUp2() {
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1420.0f, 350.0f);
    glVertex2f(1420.0f, 225.0f);
    glVertex2f(1420.0f, 350.0f);
    glVertex2f(1780.0f, 350.0f);
    glVertex2f(1780.0f, 350.0f);
    glVertex2f(1780.0f, 225.0f);
    glVertex2f(1780.0f, 225.0f);
    glVertex2f(1420.0f, 225.0f);
    glEnd();
}

void screenAddSpace() {
    screen3d();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Add space", screenWidth - 310, 610, 200, 50, buttonClicked, 1);

    //draw text and input for adding a space
    drawText("Size (x,y,z):", 1515, 320, 150);
    drawText("Location (x,y,z):", 1680, 320, 150);
    drawTextField(screenWidth - 355, 250, 150, 50, opinionTF13);
    drawTextField(screenWidth - 195, 250, 150, 50, opinionTF14);
    //drawText("Use the 'Tab' key to swith input fields", screenWidth - 110, 390, 500);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 370, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color
    drawText("Warning: make sure that spaces do not overlap and are not detached.", 1550, 165, 250);

    //draw lines around it
    boxAroundPopUp2();
}

void screenDeleteSpace() {
    screen3d();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Delete space", screenWidth - 310, 550, 200, 50, buttonClicked, 1);

    //draw text and input for deleting a space
    drawText("Space to delete:", screenWidth, 320, 600);
    drawTextField(screenWidth - 310, 250, 200, 50, opinionTF15);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 370, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color
    drawText("Warning: make sure that spaces do not overlap and are not detached.", 1550, 165, 250);

    //draw lines around it
    boxAroundPopUp2();
}

void screenMoveSpace() {
    screen3d();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Move space", screenWidth - 310, 490, 200, 50, buttonClicked, 1);

    //draw text and input for moving a space
    drawText("Space:", 1515, 320, 150);
    drawText("New location (x,y,z):", 1680, 320, 150);
    drawTextField(screenWidth - 355, 250, 150, 50, opinionTF16);
    drawTextField(screenWidth - 195, 250, 150, 50, opinionTF17);
    //drawText("Use the 'Tab' key to swith input fields", screenWidth - 110, 390, 500);
    glColor3f(1.0, 0.0, 0.0); //red color
    drawTextRed("Press enter to submit", screenWidth - 60, 370, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color
    drawText("Warning: make sure that spaces do not overlap and are not detached.", 1550, 165, 250);

    //draw lines around it
    boxAroundPopUp2();
}

void screenResizeSpace() {
    screen3d();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Resize space", screenWidth - 310, 430, 200, 50, buttonClicked, 1);

    //draw text and input for resizing a space
    drawText("Space:", 1515, 320, 150);
    drawText("New size (x,y,z):", 1680, 320, 150);
    drawTextField(screenWidth - 355, 250, 150, 50, opinionTF18);
    drawTextField(screenWidth - 195, 250, 150, 50, opinionTF19);
    //drawText("Use the 'Tab' key to swith input fields", screenWidth - 110, 390, 500);
    glColor3f(1.0, 0.0, 0.0); //red color
    drawTextRed("Press enter to submit", screenWidth - 60, 370, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color
    drawText("Warning: make sure that spaces do not overlap and are not detached.", 1550, 165, 250);

    //draw lines around it
    boxAroundPopUp2();
}

void screenCheckNext() {
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    glRectf(750.0f, 500.0f, 1050.0f, 650.0f); // Draw rectangle covering the entire screen

    //draw box of lines
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(750.0f, 650.0f);
    glVertex2f(750.0f, 500.0f);
    glVertex2f(750.0f, 650.0f);
    glVertex2f(1050.0f, 650.0f);
    glVertex2f(1050.0f, 650.0f);
    glVertex2f(1050.0f, 500.0f);
    glVertex2f(1050.0f, 500.0f);
    glVertex2f(750.0f, 500.0f);
    glEnd();

    //draw text within the box
    glColor3f(0.0, 0.0, 0.0);
    drawText("Are you sure you want to continue? Once you continue to the next step, you cannot go back to this step.", 880, 620, 200);
}

void screenCheckNextLonger() {
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    glRectf(750.0f, 450.0f, 1050.0f, 650.0f); // Draw rectangle covering the entire screen

    //draw box of lines
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(750.0f, 650.0f);
    glVertex2f(750.0f, 450.0f);
    glVertex2f(750.0f, 650.0f);
    glVertex2f(1050.0f, 650.0f);
    glVertex2f(1050.0f, 650.0f);
    glVertex2f(1050.0f, 450.0f);
    glVertex2f(1050.0f, 450.0f);
    glVertex2f(750.0f, 450.0f);
    glEnd();

    //draw text within the box
    glColor3f(0.0, 0.0, 0.0);
    drawText("Are you sure you want to continue? Once you continue to the next step, you cannot go back to this step.      Continuing can take a minute.", 880, 620, 200);
}

void screenCheckNext1() {
    screen3a();
    screenCheckNext();
    drawButton("Yes", 790, 510, 100, 30, changeScreen, 3);
    drawButton("No", 910, 510, 100, 30, changeScreen, 2);
}

void screenCheckNext2() {
    screen3b();
    screenCheckNext();
    drawButton("Yes", 790, 510, 100, 30, changeScreen, 4);
    drawButton("No", 910, 510, 100, 30, changeScreen, 3);
}

void screenCheckNext3() {
    screen3c();
	screenCheckNext();
	drawButton("Yes", 790, 510, 100, 30, changeScreen, 5);
	drawButton("No", 910, 510, 100, 30, changeScreen, 4);
}

void screenCheckNext4() {
    screen3d();
	screenCheckNextLonger();
	drawButton("Yes", 790, 460, 100, 30, changeScreen, 6);
	drawButton("No", 910, 460, 100, 30, changeScreen, 5);
}

void screenCheckNext5() {
    screen3e();
	screenCheckNext();
	drawButton("Yes", 790, 510, 100, 30, changeScreen, 33);
	drawButton("No", 910, 510, 100, 30, changeScreen, 6);
}

void screenCheckNext6() {
    assignmentDescriptionScreen();
    screenCheckNext();
    drawButton("Yes", 790, 510, 100, 30, changeScreen, 2);
    drawButton("No", 910, 510, 100, 30, changeScreen, 1);
}

void screenCheckNext7() {
    screen3f();
    screenCheckNext();
    drawButton("Yes", 790, 510, 100, 30, changeScreen, 34);
    drawButton("No", 910, 510, 100, 30, changeScreen, 33);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Zoning assignment; MSc graduation project");

    // Set callback functions
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(onMouseClick);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive_motion);

    //init gl
    glShadeModel(GL_SMOOTH);

    //Set up code
    initializeScreen();

    // Main loop
    glutMainLoop();
    //return 0;

    // At this point, the window is closed, so you can exit the application
    exit(EXIT_SUCCESS);
}
