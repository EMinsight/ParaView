/*
   ____    _ __           ____               __    ____
  / __/___(_) /  ___ ____/ __ \__ _____ ___ / /_  /  _/__  ____
 _\ \/ __/ / _ \/ -_) __/ /_/ / // / -_|_-</ __/ _/ // _ \/ __/
/___/\__/_/_.__/\__/_/  \___\_\_,_/\__/___/\__/ /___/_//_/\__(_)

Copyright 2012 SciberQuest Inc.
*/
#include "vtkMultiProcessController.h"
#if defined(PARAVIEW_USE_MPI) && !defined(WIN32)
#include "vtkMPIController.h"
#else
#include "vtkDummyController.h"
#endif
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkAppendPolyData.h"
#include "vtkProcessIdScalars.h"
#include "vtkSphereSource.h"
#include "vtkPlaneSource.h"
#include "vtkSQBOVMetaReader.h"
#include "vtkSQBOVReader.h"
#include "vtkSQFieldTracer.h"
#include "vtkSQVolumeSource.h"
#include "vtkSQPlaneSource.h"
#include "vtkSQLineSource.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "vtkGlyph3D.h"
#include "vtkSphereSource.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCamera.h"
#include "vtkPolyDataWriter.h"
#include "vtkTesting.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "TestUtils.h"

#include <iostream>
using std::cerr;

#include <string>
using std::string;

int main(int argc, char **argv)
{
  vtkMultiProcessController *controller=NULL;
  int worldRank=0;
  int worldSize=1;
  #if defined(PARAVIEW_USE_MPI) && !defined(WIN32)
  controller=vtkMPIController::New();
  controller->Initialize(&argc,&argv,0);
  worldRank=controller->GetLocalProcessId();
  worldSize=controller->GetNumberOfProcesses();
  #else
  controller=vtkDummyController::New();
  #endif

  vtkCompositeDataPipeline* cexec=vtkCompositeDataPipeline::New();
  vtkAlgorithm::SetDefaultExecutivePrototype(cexec);
  cexec->Delete();

  // configure
  string dataRoot;
  string tempDir;
  string baseline;
  BroadcastConfiguration(controller,argc,argv,dataRoot,tempDir,baseline);

  string inputFileName;
  inputFileName=dataRoot+"/Data/SciberQuestToolKit/MagneticIslands/MagneticIslands.bov";

  // pipeline 1
  // ooc reader
  vtkSQBOVMetaReader *mr=vtkSQBOVMetaReader::New();
  mr->SetFileName(inputFileName.c_str());
  mr->SetPointArrayStatus("b",1);
  mr->SetNumberOfGhostCells(2);
  mr->SetXHasPeriodicBC(1);
  mr->SetYHasPeriodicBC(1);
  mr->SetZHasPeriodicBC(1);

  // seed points
  vtkSQLineSource *l1=vtkSQLineSource::New();
  l1->SetPoint1(-3.0,0.0,0.0);
  l1->SetPoint2(3.0,0.0,0.0);
  l1->SetResolution(5);

  vtkSQLineSource *l2=vtkSQLineSource::New();
  l2->SetPoint1(0.0,0.0,-3.0);
  l2->SetPoint2(0.0,0.0,3.0);
  l2->SetResolution(5);

  vtkAppendPolyData *lines=vtkAppendPolyData::New();
  lines->AddInputConnection(l1->GetOutputPort(0));
  lines->AddInputConnection(l2->GetOutputPort(0));
  l1->Delete();
  l2->Delete();

  // map plane
  vtkSQPlaneSource *map=vtkSQPlaneSource::New();
  map->SetOrigin(-3,0,-3);
  map->SetPoint1(3,0,-3);
  map->SetPoint2(-3,0,3);
  map->SetXResolution(1);
  map->SetYResolution(1);

  // poincare mapper
  vtkSQFieldTracer *pm=vtkSQFieldTracer::New();
  pm->SetMode(vtkSQFieldTracer::MODE_POINCARE);
  pm->SetIntegratorType(vtkSQFieldTracer::INTEGRATOR_RK4);
  pm->SetMinStep(0.01);
  pm->SetMaxStep(0.01);
  pm->SetMaxNumberOfSteps(10000);
  pm->SetMaxLineLength(10000);
  pm->SetNullThreshold(0.001);
  pm->SetForwardOnly(1);
  pm->SetUseDynamicScheduler(1);
  pm->SetMasterBlockSize(1);
  pm->SetWorkerBlockSize(2);
  pm->AddInputConnection(0,mr->GetOutputPort(0));
  pm->AddInputConnection(1,lines->GetOutputPort(0));
  pm->AddInputConnection(2,map->GetOutputPort(0));
  pm->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,"b");

  mr->Delete();
  lines->Delete();
  map->Delete();

  vtkProcessIdScalars *pid=vtkProcessIdScalars::New();
  pid->SetInputConnection(0,pm->GetOutputPort(0));
  pm->Delete();

  vtkSphereSource *s1=vtkSphereSource::New();
  s1->SetThetaResolution(8);
  s1->SetPhiResolution(8);
  s1->SetRadius(0.08);

  vtkGlyph3D *g1=vtkGlyph3D::New();
  g1->ScalingOff();
  g1->SetInputConnection(0,pid->GetOutputPort(0));
  g1->SetSourceConnection(0,s1->GetOutputPort(0));
  pid->Delete();
  s1->Delete();

  // execute
  GetParallelExec(worldRank,worldSize,g1,0.0);
  g1->Update();

  // gather
  vtkPolyData *pd1=Gather(controller,0,g1->GetOutput(),100);
  g1->Delete();

  // pipeline 2
  // reader
  vtkSQBOVReader *r=vtkSQBOVReader::New();
  r->SetFileName(inputFileName.c_str());
  r->SetPointArrayStatus("b",1);

  vtkPlane *p=vtkPlane::New();
  p->SetOrigin(0.0,0.0,0.0);
  p->SetNormal(0.0,1.0,0.0);
  vtkCutter *c=vtkCutter::New();
  c->SetInputConnection(r->GetOutputPort(0));
  c->SetCutFunction(p);
  r->Delete();
  p->Delete();

  vtkDataSetSurfaceFilter *surf=vtkDataSetSurfaceFilter::New();
  surf->SetInputConnection(0,c->GetOutputPort(0));
  c->Delete();

  // execute
  GetParallelExec(worldRank, worldSize, surf, 0.0);
  surf->Update();

  // gather
  vtkPolyData *pd2=Gather(controller,0,surf->GetOutput(),101);
  surf->Delete();

  int testStatus=0; // passed

  // serial render
  if (worldRank==0)
    {
    vtkRenderer *ren=vtkRenderer::New();

    MapArrayToActor(ren,pd1,CELL_ARRAY,0);
    pd1->Delete();

    MapArrayToActor(ren,pd2,POINT_ARRAY,"b");
    pd2->Delete();

    vtkRenderWindow *rwin;
    rwin=vtkRenderWindow::New();
    rwin->AddRenderer(ren);
    rwin->SetSize(600,600);
    ren->Delete();

    vtkCamera *cam=ren->GetActiveCamera();
    cam->SetFocalPoint(0,0,0);
    cam->SetPosition(0,1,0);
    cam->ComputeViewPlaneNormal();
    cam->SetViewUp(0,0,1);
    cam->OrthogonalizeViewUp();
    ren->ResetCamera();
    cam->Zoom(1.25);

    // Perform the regression test.
    string base;
    base+=baseline;
    base+=".png";

    vtkTesting *testHelper = vtkTesting::New();
    testHelper->AddArgument("-T");
    testHelper->AddArgument(tempDir.c_str());
    testHelper->AddArgument("-V");
    testHelper->AddArgument(base.c_str());
    testHelper->SetRenderWindow(rwin);
    int result=testHelper->RegressionTest(5.0);
    testStatus=result==vtkTesting::PASSED?0:1;
    testHelper->Delete();

    rwin->Delete();
    }

  vtkAlgorithm::SetDefaultExecutivePrototype(0);

  controller->Finalize();
  controller->Delete();

  return testStatus;
}
