// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkPVDReader
 * @brief   ParaView-specific vtkXMLCollectionReader subclass
 *
 * vtkPVDReader subclasses vtkXMLCollectionReader to add
 * ParaView-specific methods.
 */

#ifndef vtkPVDReader_h
#define vtkPVDReader_h

#include "vtkPVVTKExtensionsIOCoreModule.h" //needed for exports
#include "vtkXMLCollectionReader.h"

class VTKPVVTKEXTENSIONSIOCORE_EXPORT vtkPVDReader : public vtkXMLCollectionReader
{
public:
  static vtkPVDReader* New();
  vtkTypeMacro(vtkPVDReader, vtkXMLCollectionReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Get/Set the required value for the timestep attribute.  The value
   * should be referenced by its index.  Only data sets matching this
   * value will be read.  An out-of-range index will remove the
   * restriction.
   */
  void SetTimeStep(int index) override;
  int GetTimeStep() override;
  ///@}

protected:
  vtkPVDReader();
  ~vtkPVDReader() override;

  void ReadXMLData() override;

  int RequestDataObject(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;
  int RequestInformation(vtkInformation* request, vtkInformationVector** vtkNotUsed(inputVector),
    vtkInformationVector* outputVector) override;

private:
  vtkPVDReader(const vtkPVDReader&) = delete;
  void operator=(const vtkPVDReader&) = delete;
};

#endif
