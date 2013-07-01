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

#include "vtkMRMLBetaProbeNode.h"
#include "vtkMRMLIGTLConnectorNode.h"

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

  this->BrainLab.IPAddress.assign("127.0.0.1");
  this->BrainLab.Port = 22222;

  this->BetaProbe.IPAddress.assign("192.168.0.207");
  this->BetaProbe.Port = 3000;
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeModuleWidgetPrivate::~qSlicerBetaProbeModuleWidgetPrivate()
{
  if (this->udpTimeout)
    {
    this->udpTimeout->deleteLater();
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
  
  connect(d->udpTimeout, SIGNAL(timeout()),
	  this, SLOT(onCountingNodeDisconnected()));

  
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
  if (d->countingNode)
    {
    d->countingNode->deleteLater();
    }

  // Connect it
  d->countingNode = new QUdpSocket(this);
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
    qvtkConnect(d->trackingNode, vtkMRMLIGTLConnectorNode::ConnectedEvent,
		this, SLOT(onTrackingNodeConnected()));
    qvtkConnect(d->trackingNode, vtkMRMLIGTLConnectorNode::DisconnectedEvent,
		this, SLOT(onTrackingNodeDisconnected()));
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
  
  while(d->countingNode->hasPendingDatagrams())
    {
    // BetaProbe system sends data every 100ms
    // Timeout if no data during 500ms
    d->udpTimeout->start(500);

    // Read data
    QByteArray datagram;
    datagram.resize(d->countingNode->pendingDatagramSize());
    d->countingNode->readDatagram(datagram.data(), datagram.size());
    
    // Write datagrams
    QString dataReceived = QString(datagram);
    QString date = dataReceived.section(',',0,0);
    QString time = dataReceived.section(',',1,1);
    double s = dataReceived.section(',',2,2).toDouble();
    double b = dataReceived.section(',',3,3).toDouble();
    double g = dataReceived.section(',',4,4).toDouble();

    d->betaProbeNode->WriteCountData(date.toStdString(),
				     time.toStdString(),
				     s, b, g);
    }
}
