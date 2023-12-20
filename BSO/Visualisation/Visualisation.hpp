#ifndef VISUALISATION_HPP
#define VISUALISATION_HPP


//#include <iostream>
#include <stdlib.h>
//#define _USE_MATH_DEFINES
//#include <math.h>

#ifdef __unix__
// see http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring
#include <pthread.h>
void* simpleFunc(void*) {return NULL;}
void forcePThreadLink() {pthread_t t1; pthread_create(&t1,NULL,&simpleFunc,NULL);}

#elif defined(_WIN32) || defined(WIN32)
#include <time.h>
#endif



//#include <iostream>
//#include <string>
//#include <sstream>

#ifdef BP_SIMULATION_HPP
#include <BSO/Visualisation/Model_Module/BP_Model.hpp>
#endif // BP_SIMULATION_HPP

#ifdef MS_CONFORMAL_HPP
#include <BSO/Visualisation/Model_Module/Conformal_Model.hpp>
#endif // MS_CONFORMAL_HPP

#ifdef ZONING_HPP
#include <BSO/Visualisation/Model_Module/Zoning_Model.hpp>
#endif // ZONING_HPP

#ifdef SD_ANALYSIS_HPP
#include <BSO/Visualisation/Model_Module/SD_Model.hpp>
#endif // SD_ANALYSIS_HPP

#ifdef STABILIZE_HPP
#include <BSO/Visualisation/Model_Module/Stabilization_Model.hpp>
#endif // STABILIZE_HPP

#include <BSO/Visualisation/UI_Module/Viewportmanager.hpp>
#include <BSO/Visualisation/Utility_Module/Camera.hpp>
#include <BSO/Visualisation/Model_Module/Model.hpp>

// Visualisation models
#include <BSO/Visualisation/Model_Module/MS_Model.hpp>
#include <BSO/Visualisation/Model_Module/SC_Model.hpp>


namespace BSO { namespace Visualisation
{

    viewportmanager vpmanager;
    orbitalcamera   cam;
    int prevx, prevy;



    void reshape(int w, int h)
    {
        vpmanager.resize(w,h);
    }

    void display()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vpmanager.render(cam);

        glutSwapBuffers();
    }

    void mouse(int button, int state, int x, int y)
    {
        prevx = x;
        prevy = y;

        vpmanager.mouse_event(button, state,
                x, y, glutGet(GLUT_ELAPSED_TIME));
    }

    void motion(int x, int y)
    {
        double dx = prevx-x,
               dy = prevy-y;

        cam.setrotation(cam.getrotation() + (dx*0.5));
        cam.setelevation(cam.getelevation() + (dy*0.5));

        prevx = x;
        prevy = y;

        vpmanager.mousemove_event(x, y);

        glutPostRedisplay();
    }

    void passive_motion(int x, int y)
    {
        vpmanager.mousemove_event(x, y);
    }

    void keyboard(unsigned char key, int x, int y)
    {
        (void)x; //does nothing
        (void)y; //does nothing
        switch (key) {
        case ' ' :
            //next conversion

            break;
        case 'q' :
        case 'Q' :
            exit(0);

        default:
            if (vpmanager.key_event(key, 0))
                glutPostRedisplay();
                break;
        };
    }


    void init_visualisation(int argc, char *argv[])
    {


        /*
        //trace memory allocation (leak testing)
        setenv("MALLOC_TRACE", "mtrace.log", 1);
        mtrace();

        createbsp();

        cout << "deleting" << endl;
        delete_bsp(testbsp);
        */

        //init random seed with the current time
        srand(time(NULL));

        //init glut
        glutInit(&argc, argv);
        //glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
        glutInitWindowSize(600, 300);
        glutInitWindowPosition(100, 100);
        glutCreateWindow(argv[0]);
        //maximize window (not fullscreen)
        int scrw = glutGet(GLUT_SCREEN_WIDTH)/1.5,
            scrh = glutGet(GLUT_SCREEN_HEIGHT)/1.5;
        if (scrw > 0 && scrh > 0)
            glutReshapeWindow(scrw, scrh);

        glutReshapeFunc(reshape);
        glutDisplayFunc(display);
        glutKeyboardFunc(keyboard);
        glutMouseFunc(mouse);
        glutMotionFunc(motion);
        glutPassiveMotionFunc(passive_motion);

        //init gl
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);



        if(1 == 1){glClearColor(1.0, 1.0, 1.0, 1.0);} // toggles white background, I deleted the input possibility
        else      {glClearColor(0.8, 0.8, 0.8, 1.0);}


        glClearDepth(1000.0);


    } // init_visualisation()


    void end_visualisation()
    {
        glutMainLoop();
    } // end_visualisation()

    void visualise(Spatial_Design::MS_Building& ms_building)
    {
        vpmanager.addviewport(new viewport(new MS_Model(ms_building)));
    }

    void visualise(Spatial_Design::MS_Building& ms_building, std::string type)
    {
        vpmanager.addviewport(new viewport(new MS_Model(ms_building, type)));
    }

    void visualise(Spatial_Design::SC_Building& sc_building)
    {
        vpmanager.addviewport(new viewport(new SC_Model(sc_building)));
    }

    #ifdef MS_CONFORMAL_HPP
    void visualise(Spatial_Design::MS_Conformal& cf_building, std::string type)
    {
        vpmanager.addviewport(new viewport(new Conformal_Model(cf_building, type)));
    }
    //void visualise(Spatial_Design::MS_Conformal& cf_building, std::string type, unsigned int i)
    //{
        //vpmanager.addviewport(new viewport(new Conformal_Model(cf_building, type, i)));
    //}
    #endif // MS_CONFORMAL_HPP

    #ifdef ZONING_HPP
    void visualise(Spatial_Design::MS_Conformal& cf_building, std::string type, unsigned int i)
    {
        vpmanager.addviewport(new viewport(new Zoning_Model(cf_building, type, i)));
    }
    #endif // ZONING_HPP

    #ifdef BP_SIMULATION_HPP
    void visualise(Building_Physics::BP_Simulation& BPS)
    {
        vpmanager.addviewport(new viewport(new BP_Model(BPS)));
    }

    void visualise(Spatial_Design::MS_Building& ms_building, Building_Physics::BP_Building_Results& bp_results)
    {
        vpmanager.addviewport(new viewport(new MS_Model(ms_building, bp_results)));
    }
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void visualise(Spatial_Design::MS_Building& ms_building, Structural_Design::SD_Building_Results& sd_results)
    {
        vpmanager.addviewport(new viewport(new MS_Model(ms_building, sd_results)));
    }

    void visualise(Structural_Design::SD_Analysis& SD_building, int vis_switch)
    {
        vpmanager.addviewport(new viewport(new SD_Model(SD_building, vis_switch)));
    }

    void visualise(Structural_Design::SD_Analysis& SD_building, int vis_switch, bool ghosty)
    {
        vpmanager.addviewport(new viewport(new SD_Model(SD_building, vis_switch, ghosty)));
    }
    #endif // SD_ANALYSIS_HPP

    #ifdef STABILIZE_HPP

    void visualise(Structural_Design::SD_Analysis_Vars*& SD_building, int vis_switch)
    {
        vpmanager.addviewport(new viewport(new Stabilization_Model(SD_building, vis_switch)));
    }

    void visualise(Structural_Design::SD_Analysis_Vars*& SD_building, int vis_switch, bool ghosty)
    {
        vpmanager.addviewport(new viewport(new Stabilization_Model(SD_building, vis_switch, ghosty)));
    }
    #endif // STABILIZE_HPP

} // namespace Visualisation
} // namespace BSO

#endif // VISUALISATION_HPP
