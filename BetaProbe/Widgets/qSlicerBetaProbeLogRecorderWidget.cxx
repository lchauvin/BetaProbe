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
  bool singleModeRecording;
  int singleShotStreak;

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
  this->singleModeRecording = true;
  this->singleShotStreak = 0;
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

  connect(d->SingleModeRadio, SIGNAL(toggled(bool)),
	  this, SLOT(onRecordModeChanged(bool)));

  connect(d->RecordButton, SIGNAL(clicked()),
	  this, SLOT(onRecordButtonClicked()));
}

//-----------------------------------------------------------------------------
qSlicerBetaProbeLogRecorderWidget
::~qSlicerBetaProbeLogRecorderWidget()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (d->recordingFile.is_open())
    {
    this->endSingleShotRecording();
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
  

  bool continousRecordingStatus = !d->singleModeRecording & d->recording;
  
  // Add footers to currently open file
  if (d->logFileOpen)
    {
    d->recording = false;
    this->endSingleShotRecording();
    if (continousRecordingStatus)
      {
      this->endContinuousRecording();
      }
    }

  // Close file and open new one
  if (!fileName.isEmpty())
    {
    // New file choosen. Close old file.
    this->closeLogFile();
    this->openLogFile(fileName);
    }
  
  if (d->logFileOpen)
    {
    // Put button state back
    if (continousRecordingStatus)
      {
      this->beginContinuousRecording();
      d->recording = true;
      }
    d->RecordGroupBox->setEnabled(true);
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
::recordData()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (d->recordingFile && d->logFileOpen)
    {
    vtkMRMLBetaProbeNode::trackingData* curPos =
      d->betaProbeMRMLNode->GetCurrentPosition();
    vtkMRMLBetaProbeNode::countingData* curVal =
      d->betaProbeMRMLNode->GetCurrentCounts();
    
    if (curPos && curVal)
      {
      std::stringstream dataReceived;
      dataReceived  << curVal->Date.c_str() << "\t" << curVal->Time.c_str() << "\t"
		    << curVal->Smoothed << "\t" << curVal->Beta << "\t" << curVal->Gamma << "\t"
		    << curPos->X << "\t" << curPos->Y << "\t" << curPos->Z;

      d->recordingFile << dataReceived.str() << std::endl;
      }
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

  this->recordData();
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::connectionBroken()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!d->singleModeRecording && d->RecordButton->isChecked())
    {
    d->RecordButton->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::onRecordModeChanged(bool singleMode)
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);
  
  if (singleMode)
    {
    d->RecordButton->setCheckable(false);
    }
  else
    {
    // Continous Mode
    d->RecordButton->setCheckable(true);    
    }

  d->singleModeRecording = singleMode;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::onRecordButtonClicked()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);
  
  if (d->singleModeRecording)
    {
    this->beginSingleShotRecording();
    }
  else
    {
    d->recording = d->RecordButton->isChecked();
    if (d->recording)
      {
      // Do not allow changing mode while recording
      d->SingleModeRadio->setEnabled(false);
      d->ContinuousModeRadio->setEnabled(false);
      this->beginContinuousRecording();
      }
    else
      {
      this->endContinuousRecording();
      d->SingleModeRadio->setEnabled(true);
      d->ContinuousModeRadio->setEnabled(true);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::beginSingleShotRecording()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);
  
  if (!d->recordingFile)
    {
    return;
    }

  if (d->singleShotStreak == 0)
    {
    d->recordingFile << std::endl
		     << "Single shots data" << std::endl
		     << "--------------------------------------------------" << std::endl;
    }
  
  this->recordData();
  d->singleShotStreak++;
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::endSingleShotRecording()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!d->recordingFile)
    {
    return;
    }
  
  if (d->singleShotStreak != 0)
    {
    d->recordingFile << "--------------------------------------------------" << std::endl
		     << "End of single shots" << std::endl
		     << std::endl;
    d->singleShotStreak = 0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::beginContinuousRecording()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!d->recordingFile)
    {
    return;
    }

  this->endSingleShotRecording();

  d->recordingFile << std::endl
		   << "Start recording at: " << QTime::currentTime().toString().toStdString() << std::endl
		   << "--------------------------------------------------" << std::endl;
  
  // Observe modified event
  qvtkConnect(d->betaProbeMRMLNode, vtkCommand::ModifiedEvent,
	      this, SLOT(onDataNodeModified()));
}

//-----------------------------------------------------------------------------
void qSlicerBetaProbeLogRecorderWidget
::endContinuousRecording()
{
  Q_D(qSlicerBetaProbeLogRecorderWidget);

  if (!d->recordingFile)
    {
    return;
    }

  if (!d->singleModeRecording)
    {
    d->recordingFile << "--------------------------------------------------" << std::endl
		     << "End recording at: " << QTime::currentTime().toString().toStdString() << std::endl
		     << std::endl;
    }

  // Stop observing modified event
  qvtkDisconnect(d->betaProbeMRMLNode, vtkCommand::ModifiedEvent,
		 this, SLOT(onDataNodeModified()));
}
