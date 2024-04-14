#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib> // for exit()


#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Spatial_Design/Zoning.hpp>
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Structural_Design/Stabilization/Stabilize.hpp>
#include <BSO/Visualisation/Visualisation.hpp>
#include <BSO/Performance_Indexing.hpp>
#include <AEI_Grammar/Grammar_stabilize.hpp>


std::shared_ptr <BSO::Spatial_Design::MS_Building> MS = nullptr;
std::shared_ptr <BSO::Spatial_Design::MS_Conformal> CF = nullptr;
std::shared_ptr <BSO::Structural_Design::SD_Analysis_Vars> SD_Building = nullptr;
std::shared_ptr <BSO::Structural_Design::Stabilization::Stabilize> Stab_model = nullptr;

// Global variables for visualisation
bool visualisationActive = false; // Flag to control when to activate visualisation
BSO::Visualisation::viewportmanager vpmanager_local;
BSO::Visualisation::orbitalcamera   cam_local;
int prevx, prevy;

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
TextField opinionTF9;
TextField opinionTF10;
TextField opinionTF11;
TextField opinionTF12;

// Global variables for current screen and screen dimensions
int currentScreen = 0;
const int screenWidth = 1800;
const int screenHeight = 1000;

// Variable to keep track of the number of trusses and beams added and AI suggestions
int TrussCount = 0;
int BeamCount = 0;
int AISuggestionCount = 0;
// Draw a message when input is invalid
bool DrawInvalidInput = false;

// Text margin as a percentage of the window width
const float MARGIN_PERCENT = 5.0f; // Margin as a percentage of the window width

// Function prototypes
void display();
void keyboard(unsigned char key, int x, int y);
void reshape(int width, int height);
void mainScreen();
void assignmentDescriptionScreen();
void screen3();
void screen4a();
void screen4b();
void screen4c();
void screen4d();
void screen4e();
void screen4f();
void screen5();
void screen5b();
void screenAddTrussDiagonally();
void screenReplaceTrussByBeam();
void screenDeleteDiagonalTruss();
void screenReplaceBeamByTruss();
void screenAISuggestion();
void screenCheckNext();
void screenCheckNext1();
void screenCheckNext2();
void drawText(const char *text, float x, float y);
void drawButton(const char *text, float x, float y, float width, float height, ButtonCallback callback, int variable);
void drawButtonWithBackgroundColor(const char* text, float x, float y, float width, float height, ButtonCallback callback, int variable, float r, float g, float b);
void drawArrow(float x, float y, bool leftArrow);
void drawUndoRedoButtons();
void drawTextField(int x, int y, int width, int height, TextField& textfield);
void onMouseClick(int button, int state, int x, int y);
void setup2D();
void setup3D();
void setup_models();

void visualise(BSO::Spatial_Design::MS_Building& ms_building)
{
    vpmanager_local.addviewport(new BSO::Visualisation::viewport(new BSO::Visualisation::MS_Model(ms_building)));
}

void visualise(BSO::Spatial_Design::MS_Conformal& cf_building, std::string type)
{
    vpmanager_local.addviewport(new BSO::Visualisation::viewport(new BSO::Visualisation::Conformal_Model(cf_building, type)));
}

void visualise(BSO::Structural_Design::SD_Analysis_Vars* SD_building, int vis_switch)
{
    vpmanager_local.addviewport(new BSO::Visualisation::viewport(new BSO::Visualisation::Stabilization_Model(SD_building, vis_switch)));
}

void setup_pointers() {
    MS = std::make_shared<BSO::Spatial_Design::MS_Building>("JH_Stabilization_Assignment_GUI_new/MS_Input.txt");
    CF = std::make_shared<BSO::Spatial_Design::MS_Conformal>(*MS, BSO::Grammar::grammar_stabilize);
    (*CF).make_conformal();
    SD_Building = std::make_shared<BSO::Structural_Design::SD_Analysis>(*CF);
    Stab_model = std::make_shared<BSO::Structural_Design::Stabilization::Stabilize>(SD_Building.get(), CF.get());
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

//creating output in excel file
void writeToOutputFile(std::string outputFileName, std::string question, std::string userAnswer, std::string userExplanation) {
    static bool headerPrinted = false;
    outputFile.open("output4.csv", std::ios::app);
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
    processFile.open("process4.csv", std::ios::app);
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

void initializeScreen() {
    // Your initialization code for the screen

    // Set initial active state for opinionTF13
    opinionTF7.isActive = true;
    opinionTF8.isActive = false;
}


//Declare a global variable to store the selected button label
std::string selectedButtonLabel = "";

// Function to get the selected button label
std::string getSelectedButtonLabel() {
    return selectedButtonLabel;
}

void changeScreen(int screen) {
    currentScreen = screen;
    std::cout << "Changing to screen: " << screen << std::endl;
    selectedButtonLabel = "";
    DrawInvalidInput = false;
    initializeScreen();

    if (screen == 2 || (screen >= 10 && screen <= 14) || screen == 16) {
        if (MS == nullptr || CF == nullptr || SD_Building == nullptr) {
            setup_pointers();
        }
        // visualise(*MS);
        // visualise(&SD, 1);
        // visualise(CF, "rectangles");
        visualise(SD_Building.get(), 1);
        visualisationActive = true;
    }
    else {
        vpmanager_local.clearviewports();
    }

    //If AI suggestion screen is reached (so the button is pressed), increment the AI suggestion count
    if (screen == 16) {
        AISuggestionCount++;
        Stab_model->SD_grammar_stabilize3(SD_Building.get(), CF.get());
        visualise(SD_Building.get(), 1);
    }

    if (screen == 1) {
        //write the starting time to be able to have a total time measurement
        writeToProcessFile("process4.csv", "Starting time, assignment chosen", "");
    }

    if (screen == 3) {
        //write the number of added trusses and beams as measurement
        std::string TrussCountStr = std::to_string(TrussCount);
        writeToOutputFile("output4.csv", "Truss count:", TrussCountStr.c_str(), "");
        std::string BeamCountStr = std::to_string(BeamCount);
        writeToOutputFile("output4.csv", "Beam count:", BeamCountStr.c_str(), "");
        std::string AISuggestionCountStr = std::to_string(TrussCount);
        writeToOutputFile("output4.csv", "AI suggestions count:", AISuggestionCountStr.c_str(), "");

        //write toolbox outputs
        //BSO::Structural_Design::SD_Building_Results& SD_results = SD_Building.get()->get_results();
        //BSO::SD_compliance_indexing(SD_results);
        //double initial_volume = SD_results.m_struct_volume;
        double initial_volume = 1.65;

        SD_Building.get()->remesh();
        SD_Building.get()->analyse();
        BSO::Structural_Design::SD_Building_Results& sd_results = SD_Building.get()->get_results();
        BSO::SD_compliance_indexing(sd_results);

        std::cout << "Free DOF's: " << SD_Building->get_points_with_free_dofs(1).size() << std::endl; //free DOF's after stabilization
        std::cout << "Total compliance: " << sd_results.m_total_compliance << std::endl;
        std::cout << "Total structural volume: " << sd_results.m_struct_volume << std::endl;
        std::cout << "Structural volume added for stabilization: " << sd_results.m_struct_volume - initial_volume << std::endl;

        writeToOutputFile("output4.csv", "Free DOFs:", std::to_string(SD_Building->get_points_with_free_dofs(1).size()), "");
        writeToOutputFile("output4.csv", "Total compliance:", std::to_string(sd_results.m_total_compliance), "");
        writeToOutputFile("output4.csv", "Initial volume:", std::to_string(initial_volume), "");
        writeToOutputFile("output4.csv", "Total structural volume:", std::to_string(sd_results.m_struct_volume), "");
        writeToOutputFile("output4.csv", "Structural volume added for stabilization:", std::to_string(sd_results.m_struct_volume - initial_volume), "");
    }

    if (screen == 4) {
        writeToOutputFile("output4.csv", "1..", getSelectedButtonLabel(), opinionTF.text);
    }
    if (screen == 5) {
        writeToOutputFile("output4.csv", "2..", getSelectedButtonLabel(), opinionTF2.text);
    }
    if (screen == 6) {
        writeToOutputFile("output4.csv", "3..", getSelectedButtonLabel(), opinionTF3.text);
    }
    if (screen == 7) {
        writeToOutputFile("output4.csv", "4..", getSelectedButtonLabel(), opinionTF4.text);
    }
    if (screen == 8) {
        writeToOutputFile("output4.csv", "5..", getSelectedButtonLabel(), opinionTF5.text);
    }
    if (screen == 9) {
        writeToOutputFile("output4.csv", "6. What criteria did you keep in mind while performing this assignment?", "", opinionTF6.text);
    }
    if (screen == 17) {
        writeToOutputFile("output4.csv", "e-mail adress:", getSelectedButtonLabel(), opinionTF12.text);
    }

    glutPostRedisplay();
    buttons.clear();
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
        selectedButtonLabel = "Accept";
		break;
    case 10:
		selectedButtonLabel = "Discard";
        break;
    }

    if (selectedButtonLabel == "Accept") {
        writeToProcessFile("process4.csv", "AI suggestion", getSelectedButtonLabel());
        changeScreen(2);
    }
    else if (selectedButtonLabel == "Discard") {
        writeToProcessFile("process4.csv", "AI suggestion", getSelectedButtonLabel());
        changeScreen(2);
    }

    // to print the selected button label in the actual screen.
    if (currentScreen == 3) {
        writeToOutputFile("output4.csv", "1. How much did you enjoy performing this assignment?", getSelectedButtonLabel(), opinionTF.text);
    }
    if (currentScreen == 4) {
        writeToOutputFile("output4.csv", "2. How would you rate the level of ease in performing this assignment?", getSelectedButtonLabel(), opinionTF2.text);
    }
    if (currentScreen == 5) {
        writeToOutputFile("output4.csv", "3. How well do you think you performed the assignment?", getSelectedButtonLabel(), opinionTF3.text);
    }
    if (currentScreen == 6) {
        writeToOutputFile("output4.csv", "4. Do you think it would have gone better without the AI tool?", getSelectedButtonLabel(), opinionTF4.text);
    }
    if (currentScreen == 7) {
        writeToOutputFile("output4.csv", "5. Do you think the AI tool itself can perform stabilization better than you?", getSelectedButtonLabel(), opinionTF5.text);
    }
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

    if (visualisationActive) {
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
        case 2: screen3(); break;
        case 3: screen4a(); break;
        case 4: screen4b(); break;
        case 5: screen4c(); break;
        case 6: screen4d(); break;
        case 7: screen4e(); break;
        case 8: screen4f(); break;
        case 9: screen5(); break;
        case 10: screenAddTrussDiagonally(); break;
        case 11: screenReplaceTrussByBeam(); break;
        case 12: screenDeleteDiagonalTruss(); break;
        case 13: screenReplaceBeamByTruss(); break;
        case 14: screenCheckNext1(); break;
        case 15: screenCheckNext2(); break;
        case 16: screenAISuggestion(); break;
        case 17: screen5b(); break;
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

std::string clean_str(const std::string& input) {
    std::string result;
    for (char ch : input) {
        if (isdigit(ch)) {
            result += ch;
        }
    }
    return result;
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

    if (currentScreen == 5) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF3.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF3.text != "") { // Backspace key
            opinionTF3.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 6) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF4.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF4.text != "") { // Backspace key
            opinionTF4.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 7) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF5.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF5.text != "") { // Backspace key
            opinionTF5.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 8) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF6.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF6.text != "") { // Backspace key
            opinionTF6.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 9) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF12.text += key; // Append the character to the input string
        }
        else if (key == 8 && opinionTF12.text != "") { // Backspace key
            opinionTF12.text.pop_back(); // Remove the last character from input string
        }
    }

    if (currentScreen == 10) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            if (opinionTF7.isActive) {
                opinionTF7.text += key; // Append the character to the input string
            }
            else if (opinionTF8.isActive) {
                opinionTF8.text += key; // Append the character to the input string
            }
            DrawInvalidInput = false;
        }
        else if (key == 8) { // Backspace key
            if (opinionTF7.isActive && !opinionTF7.text.empty()) {
                opinionTF7.text.pop_back(); // Remove the last character from input string
            }
            else if (opinionTF8.isActive && !opinionTF8.text.empty()) {
                opinionTF8.text.pop_back(); // Remove the last character from input string
            }
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;
            //Needed variables for the truss
            std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p1;
            std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p2;
            std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p3;
            bool TF7 = 0;
            bool TF8 = 0;

            if (!opinionTF7.text.empty()) {
                // Print the entered text from opinionTF7 to the terminal
                std::cout << "Entered text (opinionTF7): " << opinionTF7.text << std::endl;
                //p1 = Stab_model->getBoundaryPoints(std::stoi(clean_str(opinionTF7.text)));
                // Write the entered text from opinionTF7 to the process file
                writeToProcessFile("process2.csv", "Add diagonal: member 1", opinionTF7.text);
                // Clear the input string of opinionTF7 after processing

                try {
                    bool isBeam;
                    bool isTruss;
                    bool isGhost;
                    bool isShell;
                    int componentCount;
                    int pointIndex = std::stoi(clean_str(opinionTF7.text));
                    p1 = Stab_model->getBoundaryPoints(pointIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                    TF7 = true;
                }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid input for opinionTF7: " << e.what() << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
				// Handle invalid input gracefully
				std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
				validInput = false;
				DrawInvalidInput = true;
			}

            if (!opinionTF8.text.empty()) {
                // Print the entered text from opinionTF8 to the terminal
                std::cout << "Entered text (opinionTF8): " << opinionTF8.text << std::endl;
                //p2 = Stab_model->getBoundaryPoints(std::stoi(clean_str(opinionTF8.text)));
                // Write the entered text from opinionTF8 to the process file
                writeToProcessFile("process2.csv", "Add diagonal: member 2", opinionTF8.text);
                // Clear the input string of opinionTF8 after processing

                try {
                    bool isBeam;
                    bool isTruss;
                    bool isGhost;
                    bool isShell;
                    int componentCount;
                    int pointIndex = std::stoi(clean_str(opinionTF8.text));
                    p2 = Stab_model->getBoundaryPoints(pointIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                    TF8 = true;
                }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid input for opinionTF8: " << e.what() << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput && TF7 && TF8) {
                double p1_first_x = p1.first->get_coords()(0);
                double p1_first_y = p1.first->get_coords()[1];
                double p2_second_x = p2.second->get_coords()(0);
                double p2_second_y = p2.second->get_coords()[1];
                double p1_first_z = p1.first->get_coords()[2];
                double p2_second_z = p2.second->get_coords()[2];

                if (validInput && TF7 && TF8 && ((p1_first_x != p2_second_x && p1_first_y != p2_second_y) ||
                    (p1_first_y != p2_second_y && p1_first_z != p2_second_z) ||
                    (p1_first_x != p2_second_x && p1_first_z != p2_second_z))) {

                    p3.first = p1.second;
                    p3.second = p2.first;
                    Stab_model->create_manual_truss(p3);
                    Stab_model = std::make_shared<BSO::Structural_Design::Stabilization::Stabilize>(SD_Building.get(), CF.get());
                    opinionTF7.text = ""; // Clear the input string of opinionTF7
                    opinionTF8.text = ""; // Clear the input string of opinionTF8
                    // Change the screen after processing both text fields
                    changeScreen(2);
                    TrussCount++;
                }
            }
            else {
                // Handle invalid input gracefully
                std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
                writeToProcessFile("process4.csv", "", "above input invalid");
            }

        }
        else if (key == '\t') { // Tab key
            // Toggle active state between opinionTF7 and opinionTF8
            opinionTF7.isActive = !opinionTF7.isActive;
            opinionTF8.isActive = !opinionTF8.isActive;
        }
    }

    if (currentScreen == 11) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF9.text += key; // Append the character to the input string
            DrawInvalidInput = false;
        }
        else if (key == 8 && opinionTF9.text != "") { // Backspace key
            opinionTF9.text.pop_back(); // Remove the last character from input string
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;

            if (!opinionTF9.text.empty()) {
                try {
                    // Print the entered text to the terminal
                    std::cout << "Entered text: " << opinionTF9.text << std::endl;
                    int elementIndex = std::stoi(clean_str(opinionTF9.text));
                    bool isBeam;
                    bool isTruss;
                    bool isGhost;
                    bool isShell;
                    int componentCount;
                    std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                    validInput = true;
                }
                catch (const std::invalid_argument& e) {
                    // Handle invalid input gracefully
                    std::cerr << "Invalid input: " << e.what() << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
				validInput = false;
				DrawInvalidInput = true;
			}

            if (validInput) {
                int elementIndex = std::stoi(clean_str(opinionTF9.text));
                bool isBeam;
                bool isTruss;
                bool isGhost;
                bool isShell;
                int componentCount;
                std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);

                if (isTruss) {
                    Stab_model->delete_element(elementIndex);
                    Stab_model->create_manual_beam(p_delete.first, p_delete.second);

                    // Write the entered text to the process file
                    writeToProcessFile("process2.csv", "Replace rod by beam", opinionTF9.text);

                    opinionTF9.text = ""; // Clear the input string after processing
                    changeScreen(2);
                    BeamCount++;
                }
                else {
                    // Handle invalid input gracefully
                    std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle invalid input gracefully
                std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
                writeToProcessFile("process4.csv", "", "above input invalid");
            }
        }
    }

    if (currentScreen == 12) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF10.text += key; // Append the character to the input string
            DrawInvalidInput = false;
        }
        else if (key == 8 && opinionTF10.text != "") { // Backspace key
            opinionTF10.text.pop_back(); // Remove the last character from input string
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;

            if (!opinionTF10.text.empty()) {
                // Print the entered text to the terminal
                std::cout << "Entered text: " << opinionTF10.text << std::endl;
                try {
                    int elementIndex = std::stoi(clean_str(opinionTF10.text));
                    bool isBeam;
                    bool isTruss;
                    bool isGhost;
                    bool isShell;
                    int componentCount;
                    std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                    BSO::Structural_Design::Components::Point* smallest = p_delete.first;
                    BSO::Structural_Design::Components::Point* largest = p_delete.second;
                    validInput = true;
                }
                catch (const std::invalid_argument& e) {
                    // Handle invalid input gracefully
                    std::cerr << "Invalid input: " << e.what() << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
				validInput = false;
				DrawInvalidInput = true;
			}

            if (validInput) {
                int elementIndex = std::stoi(clean_str(opinionTF10.text));
                bool isBeam;
                bool isTruss;
                bool isGhost;
                bool isShell;
                int componentCount;
                std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                BSO::Structural_Design::Components::Point* smallest = p_delete.first;
                BSO::Structural_Design::Components::Point* largest = p_delete.second;

                double p_delete_first_x = p_delete.first->get_coords()(0);
                double p_delete_first_y = p_delete.first->get_coords()[1];
                double p_delete_first_z = p_delete.first->get_coords()[2];
                double p_delete_second_x = p_delete.second->get_coords()(0);
                double p_delete_second_y = p_delete.second->get_coords()[1];
                double p_delete_second_z = p_delete.second->get_coords()[2];
                std::cout << isTruss << std::endl;

                if ((isTruss) && (p_delete_first_x != p_delete_second_x && p_delete_first_y != p_delete_second_y) ||
                    (p_delete_first_y != p_delete_second_y && p_delete_first_z != p_delete_second_z) ||
                    (p_delete_first_x != p_delete_second_x && p_delete_first_z != p_delete_second_z)) {

                    Stab_model->delete_element(elementIndex);

                    // Write the entered text to the process file
                    writeToProcessFile("process2.csv", "Delete diagonal rod", opinionTF10.text);

                    opinionTF10.text = ""; // Clear the input string after processing
                    changeScreen(2);
                    TrussCount--;
                }
                else {
                    // Handle invalid input gracefully
                    std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle invalid input gracefully
                std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
                writeToProcessFile("process4.csv", "", "above input invalid");
            }
        }
    }

    if (currentScreen == 13) {
        if (key >= 32 && key <= 126) { // Check if it's a printable ASCII character
            opinionTF11.text += key; // Append the character to the input string
            DrawInvalidInput = false;
        }
        else if (key == 8 && opinionTF11.text != "") { // Backspace key
            opinionTF11.text.pop_back(); // Remove the last character from input string
            DrawInvalidInput = false;
        }
        else if (key == 13) { // Enter key
            bool validInput = true; // Flag to track if the input is valid
            DrawInvalidInput = false;

            if (!opinionTF11.text.empty()) {
                // Print the entered text to the terminal
                std::cout << "Entered text: " << opinionTF11.text << std::endl;
                try {
                    int elementIndex = std::stoi(clean_str(opinionTF11.text));
                    bool isBeam;
                    bool isTruss;
                    bool isGhost;
                    bool isShell;
                    int componentCount;
                    std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);
                    validInput = true;
                }
                catch (const std::invalid_argument& e) {
                    // Handle invalid input gracefully
                    std::cerr << "Invalid input: " << e.what() << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                validInput = false;
                DrawInvalidInput = true;
            }

            if (validInput) {
                int elementIndex = std::stoi(clean_str(opinionTF11.text));
                bool isBeam;
                bool isTruss;
                bool isGhost;
                bool isShell;
                int componentCount;
                std::pair<BSO::Structural_Design::Components::Point*, BSO::Structural_Design::Components::Point*> p_delete = Stab_model->getBoundaryPoints(elementIndex, isBeam, isGhost, isShell, isTruss, componentCount);

                if (isBeam) {
                    Stab_model->delete_element(elementIndex);
                    Stab_model->create_manual_truss(p_delete);
                    // Write the entered text to the process file
                    writeToProcessFile("process2.csv", "Replace beam by rod", opinionTF11.text);
                    opinionTF11.text = ""; // Clear the input string after processing
                    changeScreen(2);
                    BeamCount--;
                }
                else {
                    // Handle invalid input gracefully
                    std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                    validInput = false;
                    DrawInvalidInput = true;
                }
            }
            else {
                // Handle invalid input gracefully
                std::cout << "Invalid input. Please make sure both inputs are valid." << std::endl;
                validInput = false;
                DrawInvalidInput = true;
                writeToProcessFile("process4.csv", "", "above input invalid");
            }
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

void drawButtonWithBackgroundColor(const char* text, float x, float y, float width, float height, ButtonCallback callback, int variable, float red, float green, float blue) {
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
    //glColor3f(0.1, 0.75, 0.9); //for light blue brighter
    glColor3f(red, green, blue); //for custom color
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
        checkTextFieldClick(opinionTF9, mouseX, mouseY);
        checkTextFieldClick(opinionTF10, mouseX, mouseY);
        checkTextFieldClick(opinionTF11, mouseX, mouseY);
        checkTextFieldClick(opinionTF12, mouseX, mouseY);

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


// ID 0: Main screen
void mainScreen() {
    glColor3f(0.0, 0.0, 0.0);
    drawText("Welcome to this experiment for a SED graduation project. We are glad to have you here and hope you will have a nice experience.", 930, 820, 400);
    drawText("In which assignment will you participate?", 930, 740, 400);

    drawButton("Assignment 1", 800, 650, 200, 50, buttonClicked, 1);
    drawButton("Assignment 2", 800, 580, 200, 50, buttonClicked, 1);
    drawButton("Assignment 3", 800, 510, 200, 50, buttonClicked, 1);
    drawButton("Assignment 4", 800, 440, 200, 50, changeScreen, 1);

    // Draw the "Next step" button in the bottom right corner
    //drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 1);
}

// ID 1: Assignment description screen
void assignmentDescriptionScreen() {
    drawText("Selected Assignment: 4 'Human-AI stabilization assignment'​", 900, 740, 400);
    drawText("Expected duration: 20 minutes​", 900, 710, 400);
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
    drawButton("-> | Next step", 1590, 50, 200, 50, changeScreen, 15);
}

void LineDivisionScreen() {
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2f(1400.0f, 0.0f);    // Start point of the line at the top
    glVertex2f(1400.0f, screenHeight); // End point of the line at the bottom
    glEnd();
}

// ID 2: Screen 3
void screen3() {
    LineDivisionScreen();

    //draw a message when input is invalid. it is handled in the keyboard function
    if (DrawInvalidInput == true) {
        drawText("Invalid input.", 1645, 115, 200);
    }

    // Draw the counter area
    std::string AISuggestionCountStr = "AI suggestions: " + std::to_string(AISuggestionCount) + "/7";
    drawText(AISuggestionCountStr.c_str(), 1200, screenHeight - 60, 200);
    std::string TrussCountStr = "Number of diagonals: " + std::to_string(TrussCount);
    drawText(TrussCountStr.c_str(), 1200, screenHeight - 100, 200);
    std::string BeamCountStr = "Number of beams: " + std::to_string(BeamCount);
    drawText(BeamCountStr.c_str(), 1200, screenHeight - 120, 200);

    // Draw control buttons (right side)
    drawText("Add elements", screenWidth - 170, 710, 200);
    drawButton("Add rod diagonally", screenWidth - 310, 650, 200, 50, changeScreen, 10);
    drawButton("Replace rod by beam", screenWidth - 310, 590, 200, 50, changeScreen, 11);
    drawText("Remove elements", screenWidth - 180, 550, 200);
    drawButton("Delete diagonal rod", screenWidth - 310, 490, 200, 50, changeScreen, 12);
    drawButton("Replace beam by rod", screenWidth - 310, 430, 200, 50, changeScreen, 13);

    if (AISuggestionCount == 7) {
    drawText("7/7 AI suggestions reached", 1570, 395, 200);
    drawButton("AI suggestion", screenWidth - 310, 330, 200, 50, buttonClicked, 1);
	}
    else {
    drawButton("AI suggestion", screenWidth - 310, 330, 200, 50, changeScreen, 16);
    }

    //drawButton("Hide member numbers", 1100, 50, 200, 50, buttonClicked, 1);

    // Draw the message at the top of the structure illustration
    drawBoldText("Stabilize the structural design with minimal structural adjustments. You may use AI suggestions up to a maximum of seven times. Say aloud everything you think and do; thus, explain your reasoning.", 1550, screenHeight - 35, 280, 1);

    //Message to summarize most important information and to refer to the full information in the instructions
    drawText("The structure consists of rods connected by hinges. Displacements are constrained at ground level. A rod is always connected to the structure with a hinged connection, and a beam with a fixed connection. Please refer to the information sheet for the whole explanation.", 1550, screenHeight - 150, 280);
    //underline STABILIZE
    glLineWidth(1.4);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1413.0, 964.0);
    glVertex2f(1488.0, 964.0);
    glEnd();
    //underline INSTRUCTIONS
    glBegin(GL_LINES);
    glVertex2f(1568.0, 759.0);
    glVertex2f(1716.0, 759.0);
    glEnd();

    // Draw the "Next step" button in the bottom right corner
    drawButton("-> | Finished", 1590, 50, 200, 50, changeScreen, 14);
}

// ID 3: Screen 4a
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
    drawTextField(300, 270, 500, 200, opinionTF);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 1/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 4);
}

// ID 4: Screen 4b
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
    drawTextField(300, 270, 500, 200, opinionTF2);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 2/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 5);
}

// ID 5: Screen 4c
void screen4c() {
    drawText("3. How well do you think you performed the assignment?", 600, 800, 600);
    drawButton("1", 300, 725, 50, 30, buttonClicked, 1);
    drawButton("2", 350, 725, 50, 30, buttonClicked, 2);
    drawButton("3", 400, 725, 50, 30, buttonClicked, 3);
    drawButton("4", 450, 725, 50, 30, buttonClicked, 4);
    drawButton("5", 500, 725, 50, 30, buttonClicked, 5);

    drawText("1: I have no idea what I am doing, probably unstable, redundant members. ", 700, 700, 800);
    drawText("5: Confident, stable, little redundancy.", 700, 670, 800);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF3);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 3/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 6);
}

// ID 6: Screen 4d
void screen4d() {
    drawText("4. Do you think it would have gone better without the assistance of the AI tool?", 620, 800, 650);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 6);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 7);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 8);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF4);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 4/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 7);
}

// ID 7: Screen 4e
void screen4e() {
    drawText("5. Do you think the AI tool itself can perform stabilization better than you?", 600, 800, 600);
    drawButton("Yes", 300, 725, 75, 30, buttonClicked, 6);
    drawButton("No", 375, 725, 75, 30, buttonClicked, 7);
    drawButton("No idea", 450, 725, 75, 30, buttonClicked, 8);

    drawText("Please explain your answer:", 600, 500, 600);
    drawTextField(300, 270, 500, 200, opinionTF5);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 5/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 8);
}

// ID 8: Screen 4f
void screen4f() {
    drawText("6. What criteria did you keep in mind while performing this assignment?", 600, 800, 600);
    drawText("(For example, structural, aesthetical, functional, and stability requirements.)", 600, 770, 600);
    drawTextField(300, 270, 500, 200, opinionTF6);
    //drawText("Press enter to submit. Feel free to resubmit as needed; only your last submission will count.", 650, 530, 700);

    // Draw the message at the top of the structure illustration
    drawBoldText("Finally, please complete the questionnaire. You no longer need to speak aloud; simply provide your opinion in the designated fields.", 1550, screenHeight - 50, 250, 1);
    //step vs steps to go as a time indication for the user
    drawText("Question 6/6", screenWidth - 50, screenHeight - 25, 180);

    LineDivisionScreen();

    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 9);
}

// ID 9: Screen 4g
void screen5() {
    drawText("Please leave your email below if you want us to send you the results from this research and include you in the acknowledgments. Nevertheless, no results will be linked to your name since all results are pseudomized.", 600, 520, 600);
    drawTextField(300, 420, 500, 50, opinionTF12);

    LineDivisionScreen();
    drawButton("-> | Next", 1590, 50, 200, 50, changeScreen, 17);
}

//ID 17
void screen5b() {
    drawText("Thank you for your participation, this is the end of the assignment.", 600, 800, 600);
    LineDivisionScreen();
    drawButton("-> | End", 1590, 50, 200, 50, closeWindowCallback, 0);
}

void boxAroundPopUp() {
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(1420.0f, 260.0f);
    glVertex2f(1420.0f, 135.0f);
    glVertex2f(1420.0f, 260.0f);
    glVertex2f(1780.0f, 260.0f);
    glVertex2f(1780.0f, 260.0f);
    glVertex2f(1780.0f, 135.0f);
    glVertex2f(1780.0f, 135.0f);
    glVertex2f(1420.0f, 135.0f);
    glEnd();
}

void screenAddTrussDiagonally() {
    screen3();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Add rod diagonally", screenWidth - 310, 650, 200, 50, buttonClicked, 1, 0.1, 0.75, 0.9);

    //draw text and input adding a rod diagonally
    drawText("Enter two opposite members to place the diagonal between:", 1575, 240, 275);
    drawTextField(screenWidth - 355, 160, 150, 50, opinionTF7);
    drawTextField(screenWidth - 195, 160, 150, 50, opinionTF8);
    //drawText("Use the 'Tab' key to swith input fields", screenWidth - 110, 300, 500);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 280, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color
    drawText("Reminder: diagonals should not span through a space nor more than one increment.", 1200, 185, 200);

    //draw lines around it
    boxAroundPopUp();
}

// ID 11: Screen 4i
void screenReplaceTrussByBeam() {
    screen3();
    //same as previous screen
    // BSO::Spatial_Design::MS_Building MS("MS_Input.txt");

    // BSO::Visualisation::init_visualisation_without();
    // BSO::Visualisation::visualise(MS);
    // BSO::Visualisation::end_visualisation();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Replace rod by beam", screenWidth - 310, 590, 200, 50, buttonClicked, 1, 0.1, 0.75, 0.9);

    //draw text and input adding a beam
    drawText("Member to replace:", 1600, 230, 350);
    drawTextField(screenWidth - 355, 160, 150, 50, opinionTF9);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 280, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color

    //draw lines around it
    boxAroundPopUp();
}

// ID 12: screenDeleteDiagonalTruss
void screenDeleteDiagonalTruss() {
    screen3();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Delete diagonal rod", screenWidth - 310, 490, 200, 50, buttonClicked, 1, 0.1, 0.75, 0.9);

    //draw text and input deleting a diagonal rod element
    drawText("Member to delete:", 1600, 230, 350);
    drawTextField(screenWidth - 355, 160, 150, 50, opinionTF10);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 280, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color

    //draw lines around it
    boxAroundPopUp();
}

void screenReplaceBeamByTruss() {
    screen3();

    //repeat button with a background color
    drawButtonWithBackgroundColor("Replace beam by rod", screenWidth - 310, 430, 200, 50, buttonClicked, 1, 0.1, 0.75, 0.9);

    //draw text and input adding a beam
    drawText("Member to replace:", 1600, 230, 350);
    drawTextField(screenWidth - 355, 160, 150, 50, opinionTF11);
    glColor3f(1.0, 0.0, 0.0); //red color)
    drawTextRed("Press enter to submit", screenWidth - 60, 280, 500);
    glColor3f(0.0, 0.0, 0.0); //back to black color

    //draw lines around it
    boxAroundPopUp();
}

void screenAISuggestion() {
    screen3();

    //repeat button with a background color
    drawButtonWithBackgroundColor("AI suggestion", screenWidth - 310, 330, 200, 50, buttonClicked, 1, 0.1, 0.75, 0.9);

    SD_Building.get()->remesh();
    SD_Building.get()->analyse();
    BSO::Structural_Design::SD_Building_Results& sd_results = SD_Building.get()->get_results();
    BSO::SD_compliance_indexing(sd_results);
    std::cout << "Free DOF's: " << SD_Building->get_points_with_free_dofs(1).size() << std::endl; //free DOF's after stabilization

    if (SD_Building->get_points_with_free_dofs(1).size() == 0) {
		drawText("The structure is stable.", 1630, 200, 275);
	}
	else {
        drawText("Choose to accept or discard the suggestion:", 1575, 240, 275);
        drawButtonWithBackgroundColor("Accept", 1445, 160, 150, 50, buttonClicked, 9, 0.5, 0.8, 0.5);
        drawButtonWithBackgroundColor("Discard", 1605, 160, 150, 50, buttonClicked, 10, 1.0, 0.5, 0.5);
        boxAroundPopUp();
	}
}

void screenCheckNext() {
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
    drawText("Are you sure you want to continue? Once you continue to the next step, you cannot go back to this step.      Continuing can take a few seconds.", 880, 620, 200);
}

void screenCheckNext1() {
    screen3();
    screenCheckNext();
    drawButton("Yes", 790, 460, 100, 30, changeScreen, 3);
    drawButton("No", 910, 460, 100, 30, changeScreen, 2);
}

void screenCheckNext2() {
    assignmentDescriptionScreen();
    screenCheckNext();
    drawButton("Yes", 790, 460, 100, 30, changeScreen, 2);
    drawButton("No", 910, 460, 100, 30, changeScreen, 1);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Stabilization assignment; MSc graduation project");

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
