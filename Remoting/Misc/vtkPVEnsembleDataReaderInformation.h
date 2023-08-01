// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkPVEnsembleDataReaderInformation
 * @brief   Information obeject to
 * collect file information from vtkEnsembleDataReader.
 *
 * Gather information about data files from vtkEnsembleDataReader.
 */

#ifndef vtkPVEnsembleDataReaderInformation_h
#define vtkPVEnsembleDataReaderInformation_h

#include "vtkPVInformation.h"
#include "vtkRemotingMiscModule.h" //needed for exports

#include <string> // for std::string

class VTKREMOTINGMISC_EXPORT vtkPVEnsembleDataReaderInformation : public vtkPVInformation
{
public:
  static vtkPVEnsembleDataReaderInformation* New();
  vtkTypeMacro(vtkPVEnsembleDataReaderInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Transfer information about a single object into this object.
   */
  void CopyFromObject(vtkObject*) override;

  ///@{
  /**
   * Manage a serialized version of the information.
   */
  void CopyToStream(vtkClientServerStream*) override;
  void CopyFromStream(const vtkClientServerStream*) override;
  ///@}

  /**
   * Get number of files contained in the ensemble.
   */
  virtual unsigned int GetFileCount();

  /**
   * Get the file path for the input row index.
   */
  virtual std::string GetFilePath(unsigned int);

protected:
  vtkPVEnsembleDataReaderInformation();
  ~vtkPVEnsembleDataReaderInformation() override;

private:
  vtkPVEnsembleDataReaderInformation(const vtkPVEnsembleDataReaderInformation&) = delete;
  void operator=(const vtkPVEnsembleDataReaderInformation&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
