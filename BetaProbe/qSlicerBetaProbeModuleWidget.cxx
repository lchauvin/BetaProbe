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

#include <sstream>

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>

// SlicerQt includes
#include "qSlicerBetaProbeModuleWidget.h"
#include "ui_qSlicerBetaProbeModuleWidget.h"

#include "vtkMRMLIGTLConnectorNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerBetaProbeModuleWidgetPrivate: public Ui_qSlicerBetaProbeModuleWidget
{
public:
  qSlicerBetaProbeModuleWidgetPrivate();
  ~qSlicerBetaProbeModuleWidgetPrivate();
 
public:
  vtkMRMLIGTLConnectorNode* trackingDeviceNode;
  vtkMRMLIGTLConnectorNode* countingDeviceNode;
};

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::qSlicerBetaProbeModuleWidgetPrivate()
{
  this->trackingDeviceNode = NULL;
  this->countingDeviceNode = NULL;
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::~qSlicerBetaProbeModuleWidgetPrivate()
{
  if (this->trackingDeviceNode)
    {
    this->trackingDeviceNode->Delete();
    }

  if (this->countingDeviceNode)
    {
    this->countingDeviceNode->Delete();
    }
}

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidget::qSlicerBetaProbeModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerBetaProbeModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidget::~qSlicerBetaProbeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::setup()
{
  Q_D(qSlicerBetaProbeModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->NodeSelector, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
	  this, SLOT(onNodeAdded(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onNodeAdded(vtkMRMLNode* nodeAdded)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (nodeAdded)
    {
    // New node created. Create 2 new OpenIGTLink node:
    // 1 for Tracking device, 1 for BetaProbe data
    if (this->mrmlScene())
      {
      // Trackig device node
      vtkMRMLIGTLConnectorNode* trackingNode = 
	vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
      if (trackingNode)
	{
	trackingNode->SetName("BetaProbeTrackingDevice");
	this->mrmlScene()->AddNode(trackingNode);
	d->trackingDeviceNode = trackingNode;
	}

      // Counting device node
      vtkMRMLIGTLConnectorNode* countingNode = 
	vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
      if (countingNode)
	{
	countingNode->SetName("BetaProbeCountingDevice");
	this->mrmlScene()->AddNode(countingNode);
	d->countingDeviceNode = countingNode;
	}
      }
    }
}

