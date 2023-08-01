// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-FileCopyrightText: Copyright (c) Menno Deij - van Rijswijk, MARIN, The Netherlands
// SPDX-License-Identifier: BSD-3-Clause
#include "TestFunctions.h"
#include "mpi.h"
#include "vtkCGNSReader.h"
#include "vtkCell.h"
#include "vtkLogger.h"
#include "vtkMPIController.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPCGNSWriter.h"
#include "vtkPVTestUtilities.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtksys/SystemTools.hxx"

int TestPolygonalData(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);
  vtkNew<vtkMPIController> mpiController;
  mpiController->Initialize(&argc, &argv, 1);
  vtkObject::GlobalWarningDisplayOff();
  vtkMultiProcessController::SetGlobalController(mpiController);

  int rank = mpiController->GetCommunicator()->GetLocalProcessId();
  int size = mpiController->GetCommunicator()->GetNumberOfProcesses();
  vtkNew<vtkPolyData> polygonalData;

  CreatePolygonal(polygonalData, rank);

  vtkNew<vtkPVTestUtilities> utilities;
  utilities->Initialize(argc, argv);
  const char* filename = utilities->GetTempFilePath("polygonal-mpi.cgns");
  if (vtksys::SystemTools::FileExists(filename))
  {
    vtksys::SystemTools::RemoveFile(filename);
  }

  vtkNew<vtkPCGNSWriter> writer;
  writer->SetInputData(polygonalData);
  writer->SetFileName(filename);
  int rc = writer->Write();
  mpiController->Finalize();

  if (rc == 1 && rank == 0)
  {
    vtkLogIfF(ERROR, !vtksys::SystemTools::FileExists(filename), "File '%s' not found", filename);

    vtkNew<vtkCGNSReader> reader;
    reader->SetFileName(filename);
    reader->Update();

    unsigned long err = reader->GetErrorCode();
    vtkLogIfF(ERROR, err != 0, "Reading CGNS file failed.");

    vtkMultiBlockDataSet* output = reader->GetOutput();
    vtkLogIfF(ERROR, nullptr == output, "No CGNS reader output.");
    vtkLogIfF(ERROR, 1 != output->GetNumberOfBlocks(), "Expected 1 base block.");

    vtkMultiBlockDataSet* firstBlock = vtkMultiBlockDataSet::SafeDownCast(output->GetBlock(0));
    vtkLogIfF(ERROR, nullptr == firstBlock, "First block is NULL");
    vtkLogIfF(ERROR, 1 != firstBlock->GetNumberOfBlocks(), "Expected 1 zone block.");

    vtkUnstructuredGrid* outputGrid = vtkUnstructuredGrid::SafeDownCast(firstBlock->GetBlock(0));
    vtkLogIfF(ERROR, nullptr == outputGrid, "Read grid is NULL");

    vtkLogIfF(ERROR, size != outputGrid->GetNumberOfCells(), "Expected %d cells, got %lld.", size,
      outputGrid->GetNumberOfCells());

    rc = err == 0 ? 1 : 0;
  }

  delete[] filename;
  return rc == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
