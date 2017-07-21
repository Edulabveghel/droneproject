/*
 *
 * documentation for mapbuilder should proceed from there --> http://reference.mrpt.org/devel/classmrpt_1_1slam_1_1_c_metric_map_builder_r_b_p_f.html
 *
 */


#include <mrpt/utils/CImage.h>
#include <mrpt/slam.h>
#include <mrpt/gui.h>
#include <mrpt/maps.h>
#include <mrpt/obs.h>
#include <mrpt/poses.h>
#include <mrpt/opengl.h>
#include <mrpt/math.h>

#include <vector>

#include <sweep/sweep.hpp>


using namespace mrpt::slam;
using namespace mrpt::utils;
using namespace mrpt::gui;
using namespace mrpt::maps;
using namespace mrpt::obs;
using namespace mrpt::poses;
using namespace mrpt::opengl;
using namespace mrpt::math;
using namespace std;

#define PI 3.14159

int main()
{
      // setup the LIDAR units motor speed and sample rate then start the scan

      cout << "Constructing sweep device..." << endl;
      sweep::sweep device{"/dev/ttyUSB0"};

      device.set_motor_speed(10);
      device.set_sample_rate(1000);

      cout << "Motor Speed Setting: " << device.get_motor_speed() << " Hz" << endl;
      cout << "Sample Rate Setting: " << device.get_sample_rate() << " Hz" << endl;

      cout << "Beginning data acquisition as soon as motor speed stabilizes..." << endl;
      device.start_scanning();

      vector<float> distance;


    // make an image to draw the map to
    CImage img(400, 400, CH_GRAY);

    // map used by the mapbuilder
    CSimpleMap simpleMap;

    // options for initializing map builder
    CMetricMapBuilderRBPF::TConstructionOptions constructionOptions;
    constructionOptions.loadFromConfigFileName("gridmapping_optimal_sampling.ini", "MappingApplication");

    // initialize the map builder
    CMetricMapBuilderRBPF mapBuilder(constructionOptions);
    mapBuilder.initialize(simpleMap);

    // setup a window to display the generated map
    CDisplayWindow window("lidar img", 400, 400);

    // setup of the sensory frame
    CSensoryFrame sensoryFrame;

    // create the oberservation where the scan data can be written to
    CObservation2DRangeScanPtr observation = CObservation2DRangeScan::Create();

    // radius of the measurment in radians (360 degrees)
    observation->aperture = 2.0*PI;

    // initialize the position change for the robot (Which is zero)
    CPose2D robotPose;
    CPosePDFGaussian pdf;
    pdf.getMean(robotPose);

    CActionRobotMovement2D movement;
    movement.hasEncodersInfo = true;
    movement.computeFromEncoders(0.0,0.0,0.0);

    // create the action where the movements can be written to
    CActionCollection actionCollection;
    actionCollection.insert(movement);

    // setup for the 3D window
    const CMultiMetricMap *multiMetricMap = mapBuilder.mapPDF.getCurrentMostLikelyMetricMap();

    CDisplayWindow3DPtr win3D = CDisplayWindow3D::Create("RBPF-SLAM @ MRPT C++ Library", 400,
                                                400);
    win3D->setCameraZoom(40);
    win3D->setCameraAzimuthDeg(-50);
    win3D->setCameraElevationDeg(70);

    COpenGLScenePtr scene;


    scene = COpenGLScene::Create();

    // initial scan for the sweep, it should be skipped because it is used for calibration
    sweep::scan scan;
    device.get_scan(); // Skip first

    // loops each scan(rotation) of the lidar unit
    while (window.isOpen()) {
        mapBuilder.clear();

        try {
            // get scan data
            scan = device.get_scan();
        } catch (const sweep::device_error& e) {
            cout << e.what() << endl;
        }
        // clear old data
        distance.clear();

        for (const sweep::sample& sample : scan.samples) {
          // add new distance data obtained by the LIDAR
          distance.push_back(sample.distance / 100.0);
        }

        // set to same size as distance array
        observation->resizeScan(distance.size());

        for (int i =0; i < distance.size(); i++) {
            // set the lenght for each scan in the oberservation
            observation->setScanRange(i, distance[i]);
            observation->setScanRangeValidity(i, 1);
        }
        cout << observation->getScanSize() << endl;

        // insert the scan into the frame
        sensoryFrame.insert(observation);
        // process the map with the new sensory data and the old action that is currently zero(no movement)
        mapBuilder.processActionObservation(actionCollection, sensoryFrame);


        // build the 2D image en 3D map
        // code for doing this is found here http://docs.ros.org/indigo/api/mrpt_rbpf_slam/html/mrpt__rbpf__slam_8cpp_source.html
        mapBuilder.drawCurrentEstimationToImage(&img);
        window.showImage(img);

        scene->clear();

        CSetOfObjectsPtr objs = CSetOfObjects::Create();
        multiMetricMap->getAs3DObject(objs);
        scene->insert(objs);

        size_t M = mapBuilder.mapPDF.particlesCount();
        CSetOfLinesPtr objLines = CSetOfLines::Create();
        objLines->setColor(0, 1, 1);
        for (size_t i = 0; i < M; i++)
        {
            deque<TPose3D> path;
            mapBuilder.mapPDF.getPath(i, path);
            float x0 = 0, y0 = 0, z0 = 0;
            for (size_t k = 0; k < path.size(); k++)
            {
               objLines->appendLine(x0, y0, z0 + 0.001, path[k].x, path[k].y, path[k].z + 0.001);
               x0 = path[k].x;
               y0 = path[k].y;
               z0 = path[k].z;
            }
        }



        CGridPlaneXYPtr groundPlane = CGridPlaneXY::Create(-200, 200, -200, 200, 0, 5);
        groundPlane->setColor(0.4, 0.4, 0.4);
        scene->insert(groundPlane);
        scene->insert(objLines);

        CPose3D lastMeanPose;
        float minDistBtwPoses = -1;
        deque<TPose3D> dummyPath;
        mapBuilder.mapPDF.getPath(0, dummyPath);
        for (int k = (int)dummyPath.size() - 1; k >= 0; k--)
        {
            CPose3DPDFParticles poseParts;
            mapBuilder.mapPDF.getEstimatedPosePDFAtTime(k, poseParts);
            CPose3D meanPose;
            CMatrixDouble66 COV;
            poseParts.getCovarianceAndMean(COV, meanPose);
            if (meanPose.distanceTo(lastMeanPose) > minDistBtwPoses)
            {
                CMatrixDouble33 COV3 = COV.block(0, 0, 3, 3);
                minDistBtwPoses = 6 * sqrt(COV3(0, 0) + COV3(1, 1));
                CEllipsoidPtr objEllip = CEllipsoid::Create();
                objEllip->setLocation(meanPose.x(), meanPose.y(), meanPose.z() + 0.001);
                objEllip->setCovMatrix(COV3, COV3(2, 2) == 0 ? 2 : 3);
                objEllip->setColor(0, 0, 1);
                objEllip->enableDrawSolid3D(false);
                scene->insert(objEllip);
                lastMeanPose = meanPose;
            }
        }

        COpenGLScenePtr &scenePtr = win3D->get3DSceneAndLock();
        scenePtr = scene;
        win3D->unlockAccess3DScene();
        win3D->forceRepaint();

    }
    device.stop_scanning();
}


