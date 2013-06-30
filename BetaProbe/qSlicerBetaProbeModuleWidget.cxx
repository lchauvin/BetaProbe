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

#define BRAINLAB_IP "127.0.0.1"
#define BRAINLAB_PORT 22222

#define BETAPROBE_IP "127.0.0.1"
#define BETAPROBE_PORT 12345

#include <sstream>

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>

// SlicerQt includes
#include "qSlicerBetaProbeModuleWidget.h"
#include "ui_qSlicerBetaProbeModuleWidget.h"

#include "vtkMRMLBetaProbeNode.h"
#include "vtkMRMLIGTLConnectorNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerBetaProbeModuleWidgetPrivate: public Ui_qSlicerBetaProbeModuleWidget
{
public:
  qSlicerBetaProbeModuleWidgetPrivate();
  ~qSlicerBetaProbeModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::qSlicerBetaProbeModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::~qSlicerBetaProbeModuleWidgetPrivate()
{
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

  // Put label status to OFF
  this->setBetaProbeStatus(false);
  this->setTrackingStatus(false);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onNodeAdded(vtkMRMLNode* node)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!this->mrmlScene())
    {
    return;
    }

  vtkMRMLBetaProbeNode* nodeAdded =
    vtkMRMLBetaProbeNode::SafeDownCast(node);
  if (nodeAdded)
    {
    // New node created. Create 2 new OpenIGTLink node:
    // 1 for Tracking device, 1 for BetaProbe data
    // Counting device node
    vtkMRMLIGTLConnectorNode* countingNode = 
      vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
    if (countingNode)
      {
      countingNode->SetName("BetaProbeCountingDevice");
      this->mrmlScene()->AddNode(countingNode);
      countingNode->SetTypeClient(BETAPROBE_IP, BETAPROBE_PORT);
      // Disable as BetaProbe not available 
      //countingNode->Start();
      nodeAdded->SetCountingDeviceNode(countingNode);

      // Connect events
      qvtkConnect(countingNode, vtkMRMLIGTLConnectorNode::ConnectedEvent,
		  this, SLOT(onCountingNodeConnected()));
      qvtkConnect(countingNode, vtkMRMLIGTLConnectorNode::DisconnectedEvent,
		  this, SLOT(onCountingNodeDisconnected()));
      }
    
    // Tracking device node
    vtkMRMLIGTLConnectorNode* trackingNode = 
      vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
    if (trackingNode)
      {
      trackingNode->SetName("BetaProbeTrackingDevice");
      this->mrmlScene()->AddNode(trackingNode);
      trackingNode->SetTypeClient(BRAINLAB_IP, BRAINLAB_PORT);
      trackingNode->Start();
      nodeAdded->SetTrackingDeviceNode(trackingNode);

      // Connect events
      qvtkConnect(trackingNode, vtkMRMLIGTLConnectorNode::ConnectedEvent,
		  this, SLOT(onTrackingNodeConnected()));
      qvtkConnect(trackingNode, vtkMRMLIGTLConnectorNode::DisconnectedEvent,
		  this, SLOT(onTrackingNodeDisconnected()));
      }
    
    d->LogRecorderWidget->setBetaProbeNode(nodeAdded);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::setBetaProbeStatus(bool status)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->BetaProbeStatusLabel)
    {
    return;
    }
  
  QString labelText = QString(status ? 
				   "ON" :
				   "OFF"); 
  QPalette bgColor = d->BetaProbeStatusLabel->palette();
  bgColor.setColor(d->BetaProbeStatusLabel->foregroundRole(),
		    status ? 
		    QColor::fromRgb(0,155,0,255) :
		    QColor::fromRgb(155,0,0,255));

  d->BetaProbeStatusLabel->setPalette(bgColor);
  d->BetaProbeStatusLabel->setText(labelText);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::setTrackingStatus(bool status)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->TrackingStatusLabel)
    {
    return;
    }
  
  QString labelText = QString(status ? 
			      "ON" :
			      "OFF"); 
  QPalette bgColor = d->TrackingStatusLabel->palette();
  bgColor.setColor(d->TrackingStatusLabel->foregroundRole(),
		    status ? 
		    QColor::fromRgb(0,155,0,255) :
		    QColor::fromRgb(155,0,0,255));

  d->TrackingStatusLabel->setPalette(bgColor);
  d->TrackingStatusLabel->setText(labelText);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTrackingNodeConnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setTrackingStatus(true);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTrackingNodeDisconnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setTrackingStatus(false);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onCountingNodeConnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setBetaProbeStatus(true);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onCountingNodeDisconnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setBetaProbeStatus(false);
}
