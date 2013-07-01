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

// LogRecorder Widgets includes
#include "qSlicerBetaProbeLogRecorderWidget.h"
#include "ui_qSlicerBetaProbeLogRecorderWidget.h"

#include "vtkMRMLBetaProbeNode.h"
#include "vtkMRMLScene.h"

#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_BetaProbe
class qSlicerBetaProbeLogRecorderWidgetPrivate
  : public Ui_qSlicerBetaProbeLogRecorderWidget
{
  Q_DECLARE_PUBLIC(qSlicerBetaProbeLogRecorderWidget);
protected:
  qSlicerBetaProbeLogRecorderWidget* const q_ptr;

  vtkMRMLBetaProbeNode* betaProbeMRMLNode;
  QString currentLogFile;
  std::ofstream recordingFile;
  bool logFileOpen;
  bool recording;

public:
  qSlicerBetaProbeLogRecorderWidgetPrivate(
    qSlicerBetaProbeLogRecorderWidget& object);
  ~qSlicerBetaProbeLogRecorderWidgetPrivate();
  virtual void setupUi(qSlicerBetaProbeLogRecorderWidget*);

};

// --------------------------------------------------------------------------
qSlicerBetaProbeLogRecorderWidgetPrivate
::qSlicerBetaProbeLogRecorderWidgetPrivate(
  qSlicerBetaProbeLogRecorderWidget& object)
  : q_ptr(&object)
{
  this->logFileOpen = false;
  this->recording = false;
}

// --------------------------------------------------------------------------
qSlicerBetaProbeLogRecorderWidgetPrivate
::~qSlicerBetaProbeLogRecorderWidgetPrivate()
{
  if (this->recordingFile.is_open())
    {
    this->recordingFile.close();
    }
}

// --------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidgetPrivate
::setupUi(qSlicerBetaProbeLogRecorderWidget* widget)
{
  this->Ui_qSlicerBetaProbeLogRecorderWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerBetaProbeLogRecorderWidget methods

//-----------------------------------------------------------------------------
qSlicerBetaProbeLogRecorderWidget
::qSlicerBetaProbeLogRecorderWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerBetaProbeLogRecorderWidgetPrivate(*this) )
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);
  d->setupUi(this);

  connect(d->SelectFileButton, SIGNAL(clicked()),
	  this, SLOT(onSelectFileClicked()));

  connect(d->RecordCheckbox, SIGNAL(stateChanged(int)),
	  this, SLOT(onRecordChecked(int)));
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeLogRecorderWidget
::~qSlicerBetaProbeLogRecorderWidget()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (d->recordingFile.is_open())
    {
    this->closeLogFile();
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::onSelectFileClicked()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  // Open Dialog box
  std::string previousPath("");
  if (!d->currentLogFile.isEmpty())
    {
    previousPath.assign(d->currentLogFile.toStdString());
    }    
  QString fileName = QFileDialog::getSaveFileName(this, tr("Select Output File"),
						  previousPath.c_str(),
						  tr("TXT (*.txt)"));
  
  if (!fileName.isEmpty())
    {
    // New file choosen. Close old file.
    this->closeLogFile();
    this->openLogFile(fileName);
    }
  
  if (d->logFileOpen)
    {
    if (!d->RecordCheckbox->isEnabled())
      {
      d->RecordCheckbox->setEnabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::openLogFile(QString filenamePath)
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  // Open recording file
  QFileInfo file(filenamePath);
  bool fileExists = file.exists();
  
  d->SelectFileButton->setText(file.fileName());
  d->recordingFile.open(filenamePath.toStdString().c_str(), ios::out | ios::ate | ios::app);
  
  if (d->recordingFile.is_open())
    {
    if (!fileExists)
      {
      d->recordingFile << "File recorded from BetaProbe Module " << std::endl;
      d->recordingFile << "Creation date: " << QDateTime::currentDateTime().toString().toStdString() << std::endl;
      }
    d->logFileOpen = true;
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::closeLogFile()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  d->SelectFileButton->setText("Select Output File");
  if (d->recordingFile.is_open())
    {
    d->recordingFile.close();
    d->logFileOpen = false;
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::onRecordChecked(int state)
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!d->recordingFile)
    {
    return;
    }

  d->recording = (state == Qt::Checked ? true : false);
  if (d->recording)
    {
    d->recordingFile << std::endl << std::endl
		     << "Start recording at: " << QTime::currentTime().toString().toStdString() << std::endl
		     << "--------------------------------------------------" << std::endl
		     << std::endl;    
    }
  else
    {
    d->recordingFile << std::endl
		     << "--------------------------------------------------" << std::endl
		     << "End recording at: " << QTime::currentTime().toString().toStdString() << std::endl
		     << std::endl;    
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::recordData(const char* string)
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (d->recordingFile && d->logFileOpen)
    {
    d->recordingFile << string << std::endl;
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::setBetaProbeNode(vtkMRMLBetaProbeNode* newBetaProbeNode)
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!newBetaProbeNode)
    {
    return;
    }

  // Observe events
  qvtkReconnect(d->betaProbeMRMLNode, newBetaProbeNode, vtkCommand::ModifiedEvent,
		this, SLOT(onDataNodeModified()));

  d->betaProbeMRMLNode = newBetaProbeNode;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::onDataNodeModified()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);
  
  if (!d->betaProbeMRMLNode)
    {
    return;
    }

  if (!d->recording)
    {
    return;
    }

  // TODO: How to know which data received ?
  vtkMRMLBetaProbeNode::trackingData* curPos =
    d->betaProbeMRMLNode->GetCurrentPosition();
  vtkMRMLBetaProbeNode::countingData* curVal =
    d->betaProbeMRMLNode->GetCurrentCounts();

  if (curPos && curVal)
    {
    std::stringstream dataReceived;
    dataReceived  << curVal->Date.c_str() << "\t" << curVal->Time.c_str() << "\t"
                  << curVal->Smoothed << "\t" << curVal->Beta << "\t" << curVal->Gamma << "\t"
                  << curPos->X << "\t" << curPos->Y << "\t" << curPos->Z << std::endl;
    this->recordData(dataReceived.str().c_str());
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::connectionBroken()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  d->RecordCheckbox->setCheckState(Qt::Unchecked);
}
