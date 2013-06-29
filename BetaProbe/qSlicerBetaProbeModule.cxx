/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// BetaProbe Logic includes
#include <vtkSlicerBetaProbeLogic.h>

// BetaProbe includes
#include "qSlicerBetaProbeModule.h"
#include "qSlicerBetaProbeModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerBetaProbeModule, qSlicerBetaProbeModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerBetaProbeModulePrivate
{
public:
  qSlicerBetaProbeModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModulePrivate
::qSlicerBetaProbeModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModule methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModule
::qSlicerBetaProbeModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerBetaProbeModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModule::~qSlicerBetaProbeModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerBetaProbeModule::helpText()const
{
  return "This is a loadable module bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerBetaProbeModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBetaProbeModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerBetaProbeModule::icon()const
{
  return QIcon(":/Icons/BetaProbe.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerBetaProbeModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBetaProbeModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerBetaProbeModule
::createWidgetRepresentation()
{
  return new qSlicerBetaProbeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerBetaProbeModule::createLogic()
{
  return vtkSlicerBetaProbeLogic::New();
}
