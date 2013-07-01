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

#ifndef __qSlicerBetaProbeModuleWidget_h
#define __qSlicerBetaProbeModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBetaProbeModuleExport.h"

#include "ctkVTKObject.h"

class qSlicerBetaProbeModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_BETAPROBE_EXPORT qSlicerBetaProbeModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerBetaProbeModuleWidget(QWidget *parent=0);
  virtual ~qSlicerBetaProbeModuleWidget();

  typedef struct
  {
    std::string IPAddress;
    int Port;
  }HostInformation;

public slots:
  void onNodeAdded(vtkMRMLNode* node);
  void onTrackingNodeConnected();
  void onTrackingNodeDisconnected();
  void onCountingNodeConnected();
  void onCountingNodeDisconnected();
  void onCountsReceived();
  void StartConnections();

protected:
  QScopedPointer<qSlicerBetaProbeModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  void setBetaProbeStatus(bool status);
  void setTrackingStatus(bool status);

private:
  Q_DECLARE_PRIVATE(qSlicerBetaProbeModuleWidget);
  Q_DISABLE_COPY(qSlicerBetaProbeModuleWidget);
};

#endif
