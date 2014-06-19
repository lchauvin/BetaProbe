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

// STL
#include <sstream>

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>
#include <QUdpSocket>

// SlicerQt includes
#include "qSlicerBetaProbeModuleWidget.h"
#include "ui_qSlicerBetaProbeModuleWidget.h"

// VTK includes
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkMatrix4x4.h"

// MRML includes
#include "vtkMRMLBetaProbeNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerBetaProbeModuleWidgetPrivate: public Ui_qSlicerBetaProbeModuleWidget
{
public:
  qSlicerBetaProbeModuleWidgetPrivate();
  ~qSlicerBetaProbeModuleWidgetPrivate();

  vtkMRMLBetaProbeNode* betaProbeNode;
  vtkMRMLIGTLConnectorNode* trackingNode;
  QUdpSocket* countingNode;
  QTimer* udpTimeout;
  qSlicerBetaProbeModuleWidget::HostInformation BrainLab;
  qSlicerBetaProbeModuleWidget::HostInformation BetaProbe;
  bool betaProbeStatus;
  bool trackingStatus;
  vtkMRMLScalarVolumeNode* VolumeToMap;
  vtkMRMLColorTableNode* BetaProbeColorNode;
  int PointSize;
};

//-----------------------------------------------------------------------------
// qSlicerBetaProbeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::qSlicerBetaProbeModuleWidgetPrivate()
{
  this->betaProbeNode = NULL;
  this->trackingNode = NULL;
  this->countingNode = NULL;
  this->udpTimeout = new QTimer();

  this->betaProbeStatus = false;
  this->trackingStatus = false;

  this->BrainLab.IPAddress.assign("127.0.0.1");
  this->BrainLab.Port = 22222;

  this->BetaProbe.IPAddress.assign("192.168.0.207");
  this->BetaProbe.Port = 3000;

  this->VolumeToMap = NULL;
  this->BetaProbeColorNode = NULL;

  // Number of voxels to display around real voxel position
  this->PointSize = 1;
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::~qSlicerBetaProbeModuleWidgetPrivate()
{
  if (this->udpTimeout)
    {
    this->udpTimeout->deleteLater();
    }

  if (this->BetaProbeColorNode)
    {
    this->BetaProbeColorNode->Delete();
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

  connect(d->ReconnectButton, SIGNAL(clicked()),
          this, SLOT(StartConnections()));

  connect(d->TransformSelectorNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
	  this, SLOT(onTransformNodeChanged(vtkMRMLNode*)));

  connect(d->udpTimeout, SIGNAL(timeout()),
          this, SLOT(onCountingNodeDisconnected()));

  connect(d->MapButton, SIGNAL(clicked()),
          this, SLOT(onMapButtonClicked()));

  connect(d->VolumeToMapSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onVolumeToMapSelected(vtkMRMLNode*)));

  connect(d->ColorWindowWidget, SIGNAL(valuesChanged(double, double)),
	  this, SLOT(onColorWindowRangeChanged(double, double)));

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
    d->betaProbeNode = nodeAdded;
    d->LogRecorderWidget->setBetaProbeNode(nodeAdded);

    this->StartConnections();
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::StartConnections()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->betaProbeNode)
    {
    return;
    }

  // Create new counting node if not existing
  if (!d->countingNode)
    {
    d->countingNode = new QUdpSocket(this);
    }

  // Connect it
  QHostAddress betaProbeAddress = QHostAddress(QString::fromStdString(d->BetaProbe.IPAddress));
  if (d->countingNode->bind(betaProbeAddress, d->BetaProbe.Port))
    {
    this->onCountingNodeConnected();
    connect(d->countingNode, SIGNAL(readyRead()),
            this, SLOT(onCountsReceived()));
    }

  // Create new tracking node if not existing
  if (!d->trackingNode)
    {
    d->trackingNode =
      vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
    if (d->trackingNode)
      {
      d->trackingNode->SetName("BetaProbeTrackingDevice");
      this->mrmlScene()->AddNode(d->trackingNode);
      d->trackingNode->SetTypeClient(d->BrainLab.IPAddress.c_str(), d->BrainLab.Port);
      d->betaProbeNode->SetTrackingDeviceNode(d->trackingNode);
      }
    }

  // Connect it
  if (d->trackingNode->Start())
    {
    // Connect events
    qvtkConnect(d->trackingNode, vtkMRMLIGTLConnectorNode::ReceiveEvent,
                this, SLOT(onTrackingNodeReceivedData()));
    qvtkConnect(d->trackingNode, vtkMRMLIGTLConnectorNode::ConnectedEvent,
                this, SLOT(onTrackingNodeConnected()));
    qvtkConnect(d->trackingNode, vtkMRMLIGTLConnectorNode::DisconnectedEvent,
                this, SLOT(onTrackingNodeDisconnected()));
    }

}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTransformNodeChanged(vtkMRMLNode* newTransform)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->betaProbeNode)
    {
    return;
    }
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(newTransform);
  d->betaProbeNode->SetTransformNode(transformNode);
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

  d->betaProbeStatus = status;
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

  d->trackingStatus = status;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTrackingNodeConnected()
{
  this->setTrackingStatus(true);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTrackingNodeDisconnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setTrackingStatus(false);

  // Stop recording
  d->LogRecorderWidget->connectionBroken();

  // Close connection
  if (!d->trackingNode)
    {
    return;
    }
  d->trackingNode->Stop();
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onTrackingNodeReceivedData()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->betaProbeNode)
    {
    return;
    }

  // Get counts informations and update widget
  vtkMRMLBetaProbeNode::countingData* newCountingData
    = d->betaProbeNode->GetCurrentCounts();
  if (newCountingData)
    {
    d->SmoothedLine->setText(QString::number(newCountingData->Smoothed));
    d->BetaLine->setText(QString::number(newCountingData->BetaGamma));
    d->GammaLine->setText(QString::number(newCountingData->Gamma));
    }

  // Get position informations and update widget
  vtkMRMLBetaProbeNode::trackingData* newTrackingData
    = d->betaProbeNode->GetCurrentPosition();
  if (newTrackingData)
    {
    d->XLine->setText(QString::number(newTrackingData->X));
    d->YLine->setText(QString::number(newTrackingData->Y));
    d->ZLine->setText(QString::number(newTrackingData->Z));
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onCountingNodeConnected()
{
  this->setBetaProbeStatus(true);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onCountingNodeDisconnected()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  this->setBetaProbeStatus(false);

  // Stop recording
  d->LogRecorderWidget->connectionBroken();
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onCountsReceived()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->countingNode || !d->betaProbeNode || !d->udpTimeout)
    {
    return;
    }

  if (!d->betaProbeStatus)
    {
    // Connection is back
    this->setBetaProbeStatus(true);
    }

  while(d->countingNode->hasPendingDatagrams())
    {
    // BetaProbe system sends data every 100ms
    // Timeout if no data during 1000ms
    d->udpTimeout->start(1000);

    // Read data
    QByteArray datagram;
    datagram.resize(d->countingNode->pendingDatagramSize());
    d->countingNode->readDatagram(datagram.data(), datagram.size());

    // Write datagrams
    QString dataReceived = QString(datagram);
    QString date = dataReceived.section(',',0,0);
    QString time = dataReceived.section(',',1,1);
    double s = dataReceived.section(',',2,2).toDouble();
    double c = dataReceived.section(',',3,3).toDouble();
    double g = dataReceived.section(',',4,4).toDouble();

    d->betaProbeNode->WriteCountData(date.toStdString(),
                                     time.toStdString(),
                                     s, c, g);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onVolumeToMapSelected(vtkMRMLNode* selectedNode)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!selectedNode)
    {
    return;
    }

  d->VolumeToMap = vtkMRMLScalarVolumeNode::SafeDownCast(selectedNode);
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onMapButtonClicked()
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->betaProbeNode || !d->VolumeToMap || !this->mrmlScene())
    {
    return;
    }

  // Get position and counting values
  std::vector<vtkMRMLBetaProbeNode::trackingData> positionData;
  std::vector<vtkMRMLBetaProbeNode::countingData> activityData;
  positionData = d->betaProbeNode->GetTrackerPositions();
  activityData = d->betaProbeNode->GetBetaProbeValues();

  if (positionData.size() == activityData.size())
    {
    std::stringstream mapName;
    mapName << d->VolumeToMap->GetName() << "-BetaProbeMapping";

    // Create new volume with same information as volume to map
    vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> labelMapDisplayNode
      = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
    this->mrmlScene()->AddNode(labelMapDisplayNode);

    vtkSmartPointer<vtkMRMLScalarVolumeNode> mapNode
      = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    mapNode->Copy(d->VolumeToMap);
    mapNode->LabelMapOn();
    mapNode->SetAndObserveDisplayNodeID(labelMapDisplayNode->GetID());
    mapNode->SetName(mapName.str().c_str());

    vtkSmartPointer<vtkImageData> mapData
      = vtkSmartPointer<vtkImageData>::New();
    mapData->SetDimensions(d->VolumeToMap->GetImageData()->GetDimensions());
    mapData->SetNumberOfScalarComponents(1);
    mapData->SetScalarTypeToDouble();
    mapData->AllocateScalars();

    // Get RASToIJK matrix to convert tracking coordinates from RAS to IJK
    vtkSmartPointer<vtkMatrix4x4> RASToIJKMatrix
      = vtkSmartPointer<vtkMatrix4x4>::New();
    mapNode->GetRASToIJKMatrix(RASToIJKMatrix);

    // Map values
    std::cerr << "Number of points: " << positionData.size() << std::endl;

    for (unsigned int pt = 0; pt < positionData.size(); ++pt)
      {
      double curPoint[4] = { positionData[pt].X, positionData[pt].Y, positionData[pt].Z, 1.0 };
      double* registeredPoint;
      registeredPoint = RASToIJKMatrix->MultiplyDoublePoint(curPoint);

      // Also set same activity value to voxels around to make it more visible
      double activityValue = activityData[pt].Gamma;
      for (int k = registeredPoint[2]-d->PointSize; k < registeredPoint[2]+d->PointSize; ++k)
	{
	for (int j = registeredPoint[1]-d->PointSize; j < registeredPoint[1]+d->PointSize; ++j)
	  {
	  for (int i = registeredPoint[0]-d->PointSize; i < registeredPoint[0]+d->PointSize; ++i)
	    {
	    mapData->SetScalarComponentFromDouble(i,j,k,0,activityValue);
	    }
	  }
	}
      }
    mapData->Modified();
    mapNode->SetAndObserveImageData(mapData);

    // Create new lookup table if not already existing
    // value 0: opacity 0
    // From blue to red (Hue: 0.67 -> 0.0)
    if (!d->BetaProbeColorNode)
      {
      d->BetaProbeColorNode = vtkMRMLColorTableNode::New();
      d->BetaProbeColorNode->SetName("BetaProbeColorNode");
      d->BetaProbeColorNode->SetTypeToUser();
      d->BetaProbeColorNode->SetNumberOfColors(256);

      vtkLookupTable* betaProbeLUT = d->BetaProbeColorNode->GetLookupTable();
      betaProbeLUT->SetRampToLinear();
      betaProbeLUT->SetHueRange(0.67, 0.0);
      betaProbeLUT->SetSaturationRange(1.0, 1.0);
      betaProbeLUT->SetValueRange(1.0, 1.0);
      betaProbeLUT->SetAlphaRange(1.0, 1.0);
      betaProbeLUT->Build();
      betaProbeLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
      d->BetaProbeColorNode->HideFromEditorsOff();
      this->mrmlScene()->AddNode(d->BetaProbeColorNode);
      }

    // Set range of data (range of scalar values)
    // Update range even if not recreating color table
    d->BetaProbeColorNode->GetLookupTable()->SetTableRange(mapData->GetScalarRange());
    d->BetaProbeColorNode->Modified();

    // Add node to scene
    labelMapDisplayNode->SetAndObserveColorNodeID(d->BetaProbeColorNode->GetID());
    this->mrmlScene()->AddNode(mapNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::onColorWindowRangeChanged(double min, double max)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (!d->BetaProbeColorNode)
    {
    return;
    }

  vtkLookupTable* betaProbeLUT = d->BetaProbeColorNode->GetLookupTable();
  if (betaProbeLUT)
    {
    // Value 0 should be reset after rebuilding lookup table
    double colorMax = d->ColorWindowWidget->maximum();
    std::cerr << "Range: " << colorMax-min << "-" << colorMax-max << std::endl;
    betaProbeLUT->SetHueRange(colorMax-min,colorMax-max);
    betaProbeLUT->ForceBuild();
    betaProbeLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    d->BetaProbeColorNode->Modified();
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::SetBrainLabIPAddress(const char* brainLabIP)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (brainLabIP)
    {
    d->BrainLab.IPAddress.assign(brainLabIP);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::SetBrainLabPort(int port)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  d->BrainLab.Port = port;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::SetBetaProbeIPAddress(const char* betaProbeIP)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (betaProbeIP)
    {
    d->BetaProbe.IPAddress.assign(betaProbeIP);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::SetBetaProbePort(int port)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  d->BetaProbe.Port = port;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeModuleWidget::SetPointSize(int voxelSize)
{
  Q_D(qSlicerBetaProbeModuleWidget);

  if (voxelSize < 0)
    {
    return;
    }

  d->PointSize = voxelSize;
}
