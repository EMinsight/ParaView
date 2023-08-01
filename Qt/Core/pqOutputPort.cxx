// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation, Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
#include "pqOutputPort.h"

// Server Manager Includes.
#include "vtkPVClassNameInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkSMOutputPort.h"
#include "vtkSMSourceProxy.h"

// Qt Includes.
#include <QList>
#include <QtDebug>

// ParaView Includes.
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqTimeKeeper.h"
#include "pqView.h"

class pqOutputPort::pqInternal
{
public:
  QList<pqPipelineSource*> Consumers;
  QList<pqDataRepresentation*> Representations;
};

//-----------------------------------------------------------------------------
pqOutputPort::pqOutputPort(pqPipelineSource* source, int portno)
  : Superclass(QString(), QString(), source->getSourceProxy()->GetOutputPort(portno),
      source->getServer(), source)
  , Source(source)
  , PortNumber(portno)
{
  this->Internal = new pqInternal();

  /// Fire visibility changed signals when representations are added/removed.
  QObject::connect(this, SIGNAL(representationAdded(pqOutputPort*, pqDataRepresentation*)), this,
    SIGNAL(visibilityChanged(pqOutputPort*, pqDataRepresentation*)));
  QObject::connect(this, SIGNAL(representationRemoved(pqOutputPort*, pqDataRepresentation*)), this,
    SIGNAL(visibilityChanged(pqOutputPort*, pqDataRepresentation*)));
}

//-----------------------------------------------------------------------------
pqOutputPort::~pqOutputPort()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
pqServer* pqOutputPort::getServer() const
{
  return this->Source ? this->Source->getServer() : nullptr;
}

//-----------------------------------------------------------------------------
vtkSMOutputPort* pqOutputPort::getOutputPortProxy() const
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());

  if (!source || !source->GetOutputPortsCreated())
  {
    return nullptr;
  }

  return source->GetOutputPort(this->PortNumber);
}

//-----------------------------------------------------------------------------
vtkPVDataInformation* pqOutputPort::getDataInformation() const
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());

  if (!source)
  {
    return nullptr;
  }

  return source->GetDataInformation(this->PortNumber);
}

//-----------------------------------------------------------------------------
vtkPVDataInformation* pqOutputPort::getRankDataInformation(int rank) const
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());

  if (!source)
  {
    return nullptr;
  }

  return source->GetRankDataInformation(this->PortNumber, rank);
}

//-----------------------------------------------------------------------------
vtkPVTemporalDataInformation* pqOutputPort::getTemporalDataInformation()
{
  return this->getOutputPortProxy()->GetTemporalDataInformation();
}

//-----------------------------------------------------------------------------
vtkPVDataInformation* pqOutputPort::getSelectedDataInformation(int es_port) const
{
  auto source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());
  source->CreateSelectionProxies();
  auto esfilter = source->GetSelectionOutput(this->PortNumber);
  return esfilter->GetDataInformation(es_port);
}

//-----------------------------------------------------------------------------
const char* pqOutputPort::getDataClassName() const
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());

  if (!source)
  {
    return nullptr;
  }

  vtkPVClassNameInformation* ciInfo =
    source->GetOutputPort(this->PortNumber)->GetClassNameInformation();
  return ciInfo ? ciInfo->GetVTKClassName() : nullptr;
}

//-----------------------------------------------------------------------------
QString pqOutputPort::getPortName() const
{
  vtkSMSourceProxy* source = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());
  return QString(source->GetOutputPortName(static_cast<unsigned int>(this->PortNumber)));
}

//-----------------------------------------------------------------------------
/// Called to set the selection input.
void pqOutputPort::setSelectionInput(vtkSMSourceProxy* selSrc, int port)
{
  vtkSMSourceProxy* src = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());
  src->SetSelectionInput(this->getPortNumber(), selSrc, port);
}

//-----------------------------------------------------------------------------
/// Calls vtkSMSourceProxy::GetSelectionInput() on the underlying source
/// proxy.
vtkSMSourceProxy* pqOutputPort::getSelectionInput()
{
  vtkSMSourceProxy* src = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());
  return src->GetSelectionInput(this->getPortNumber());
}

//-----------------------------------------------------------------------------
/// Calls vtkSMSourceProxy::GetSelectionInputPort() on the underlying source
/// proxy.
unsigned int pqOutputPort::getSelectionInputPort()
{
  vtkSMSourceProxy* src = vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy());
  return src->GetSelectionInputPort(this->getPortNumber());
}

//-----------------------------------------------------------------------------
int pqOutputPort::getNumberOfConsumers() const
{
  return this->Internal->Consumers.size();
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqOutputPort::getConsumer(int index) const
{
  if (index < 0 || index >= this->Internal->Consumers.size())
  {
    qCritical() << "Invalid index: " << index;
    return nullptr;
  }

  return this->Internal->Consumers[index];
}

//-----------------------------------------------------------------------------
QList<pqPipelineSource*> pqOutputPort::getConsumers() const
{
  return this->Internal->Consumers;
}

//-----------------------------------------------------------------------------
void pqOutputPort::addConsumer(pqPipelineSource* cons)
{
  if (!this->Internal->Consumers.contains(cons))
  {
    Q_EMIT this->preConnectionAdded(this, cons);
    this->Internal->Consumers.push_back(cons);
    Q_EMIT this->connectionAdded(this, cons);
  }
}

//-----------------------------------------------------------------------------
void pqOutputPort::removeConsumer(pqPipelineSource* cons)
{
  if (this->Internal->Consumers.contains(cons))
  {
    Q_EMIT this->preConnectionRemoved(this, cons);
    this->Internal->Consumers.removeAll(cons);
    Q_EMIT this->connectionRemoved(this, cons);
  }
}

//-----------------------------------------------------------------------------
void pqOutputPort::addRepresentation(pqDataRepresentation* repr)
{
  if (!this->Internal->Representations.contains(repr))
  {
    QObject::connect(
      repr, SIGNAL(visibilityChanged(bool)), this, SLOT(onRepresentationVisibilityChanged()));

    this->Internal->Representations.push_back(repr);
    Q_EMIT this->representationAdded(this, repr);
  }
}

//-----------------------------------------------------------------------------
void pqOutputPort::removeRepresentation(pqDataRepresentation* repr)
{
  this->Internal->Representations.removeAll(repr);
  QObject::disconnect(repr, nullptr, this, nullptr);
  Q_EMIT this->representationRemoved(this, repr);
}

//-----------------------------------------------------------------------------
void pqOutputPort::onRepresentationVisibilityChanged()
{
  Q_EMIT this->visibilityChanged(this, qobject_cast<pqDataRepresentation*>(this->sender()));
}

//-----------------------------------------------------------------------------
pqDataRepresentation* pqOutputPort::getRepresentation(pqView* view) const
{
  if (view)
  {
    Q_FOREACH (pqDataRepresentation* repr, this->Internal->Representations)
    {
      if (repr && (!view || repr->getView() == view))
      {
        return repr;
      }
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
QList<pqDataRepresentation*> pqOutputPort::getRepresentations(pqView* view) const
{
  QList<pqDataRepresentation*> list;
  Q_FOREACH (pqDataRepresentation* repr, this->Internal->Representations)
  {
    if (repr && (!view || repr->getView() == view))
    {
      list.push_back(repr);
    }
  }

  return list;
}

//-----------------------------------------------------------------------------
QList<pqView*> pqOutputPort::getViews() const
{
  QList<pqView*> views;
  Q_FOREACH (pqDataRepresentation* repr, this->Internal->Representations)
  {
    if (repr)
    {
      pqView* view = repr->getView();
      if (view && !views.contains(view))
      {
        views.push_back(view);
      }
    }
  }

  return views;
}

//-----------------------------------------------------------------------------
void pqOutputPort::renderAllViews(bool force /*=false*/)
{
  QList<pqView*> views = this->getViews();
  Q_FOREACH (pqView* view, views)
  {
    if (force)
    {
      view->forceRender();
    }
    else
    {
      view->render();
    }
  }
}

//-----------------------------------------------------------------------------
vtkSMSourceProxy* pqOutputPort::getSourceProxy() const
{
  return this->getSource() ? vtkSMSourceProxy::SafeDownCast(this->getSource()->getProxy())
                           : nullptr;
}

//-----------------------------------------------------------------------------
QString pqOutputPort::prettyName() const
{
  auto source = this->getSource();
  Q_ASSERT(source);
  if (source->getNumberOfOutputPorts() > 1)
  {
    return QString("%1: %2").arg(source->getSMName()).arg(this->getPortName());
  }
  else
  {
    return source->getSMName();
  }
}
