// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkXMLPVAnimationWriter
 * @brief   Data writer for ParaView
 *
 * vtkXMLPVAnimationWriter is used to save all parts of a current
 * source to a file with pieces spread across other server processes.
 */

#ifndef vtkXMLPVAnimationWriter_h
#define vtkXMLPVAnimationWriter_h

#include "vtkRemotingAnimationModule.h" //needed for exports
#include "vtkXMLPVDWriter.h"

class vtkXMLPVAnimationWriterInternals;

class VTKREMOTINGANIMATION_EXPORT vtkXMLPVAnimationWriter : public vtkXMLPVDWriter
{
public:
  static vtkXMLPVAnimationWriter* New();
  vtkTypeMacro(vtkXMLPVAnimationWriter, vtkXMLPVDWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Add/Remove representations.
   */
  void AddRepresentation(vtkAlgorithm*, const char* groupname);
  void RemoveAllRepresentations();
  ///@}

  /**
   * Start a new animation with the current set of inputs.
   */
  void Start();

  /**
   * Write the current time step.
   */
  void WriteTime(double time);

  /**
   * Finish an animation by writing the collection file.
   */
  void Finish();

protected:
  vtkXMLPVAnimationWriter();
  ~vtkXMLPVAnimationWriter() override;

  // Replace vtkXMLWriter's writing driver method.
  int WriteInternal() override;

  // Status safety check for method call ordering.
  int StartCalled;
  int FinishCalled;

  // Internal implementation details.
  vtkXMLPVAnimationWriterInternals* Internal;

  char** FileNamesCreated;
  int NumberOfFileNamesCreated;
  void AddFileName(const char* fileName);
  void DeleteFileNames();
  void DeleteFiles();

  void AddInputInternal(const char* group);

private:
  vtkXMLPVAnimationWriter(const vtkXMLPVAnimationWriter&) = delete;
  void operator=(const vtkXMLPVAnimationWriter&) = delete;
};

#endif
