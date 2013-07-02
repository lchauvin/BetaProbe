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
 
  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898
 
==============================================================================*/

#ifndef __qSlicerBetaProbeLogRecorderWidget_h
#define __qSlicerBetaProbeLogRecorderWidget_h

// Qt includes
#include <qSlicerWidget.h>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

// Standard includes
#include <sstream>

// LogRecorder Widgets includes
#include "qSlicerBetaProbeModuleWidgetsExport.h"

#include <ctkVTKObject.h>

class qSlicerBetaProbeLogRecorderWidgetPrivate;
class vtkMRMLBetaProbeNode;

/// \ingroup Slicer_QtModules_BetaProbe
class Q_SLICER_MODULE_BETAPROBE_WIDGETS_EXPORT qSlicerBetaProbeLogRecorderWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerBetaProbeLogRecorderWidget(QWidget *parent=0);
  virtual ~qSlicerBetaProbeLogRecorderWidget();
  void openLogFile(QString filenamePath);
  void closeLogFile();
  void recordData();
  void setBetaProbeNode(vtkMRMLBetaProbeNode* newBetaProbeNode);
  void connectionBroken();

protected slots:
  void onSelectFileClicked();
  void onDataNodeModified();
  void onRecordModeChanged(bool singleMode);
  void onRecordButtonClicked();

  void beginSingleShotRecording();
  void endSingleShotRecording();
  void beginContinuousRecording();
  void endContinuousRecording();

protected:
  QScopedPointer<qSlicerBetaProbeLogRecorderWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerBetaProbeLogRecorderWidget);
  Q_DISABLE_COPY(qSlicerBetaProbeLogRecorderWidget);
};

#endif
