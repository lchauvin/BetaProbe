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

#define MAX_DATA_SAVED 50

#include <cmath>

#include "vtkMRMLBetaProbeNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLBetaProbeNode);


//----------------------------------------------------------------------------
vtkMRMLBetaProbeNode::vtkMRMLBetaProbeNode()
{
  this->HideFromEditors = false;
  this->TrackingDeviceNode = NULL;
  this->ToolTransform = NULL;
  this->numberOfTrackingDataReceived = 0;

  this->currentPosition.X = 0.0;
  this->currentPosition.Y = 0.0;
  this->currentPosition.Z = 0.0;

  this->currentValues.Date.assign("");
  this->currentValues.Time.assign("");
  this->currentValues.Smoothed  = 0.0;
  this->currentValues.BetaGamma = 0.0;
  this->currentValues.Gamma     = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLBetaProbeNode::~vtkMRMLBetaProbeNode()
{
  if (this->TrackingDeviceNode)
    {
    this->TrackingDeviceNode->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}


//----------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//-----------------------------------------------------------
void vtkMRMLBetaProbeNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::ProcessMRMLEvents ( vtkObject *caller,
                                               unsigned long event,
                                               void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Tracking node
  /*
  if (vtkMRMLIGTLConnectorNode::SafeDownCast(caller) == this->TrackingDeviceNode)
    {
    // Data received
    if (event == vtkMRMLIGTLConnectorNode::ReceiveEvent)
      {
      vtkMRMLLinearTransformNode* transformReceived =
        vtkMRMLLinearTransformNode::SafeDownCast(this->TrackingDeviceNode->GetIncomingMRMLNode(this->TrackingDeviceNode->GetNumberOfIncomingMRMLNodes()-1));
      if (transformReceived)
        {
        vtkSmartPointer<vtkMatrix4x4> matrixReceived =
          vtkSmartPointer<vtkMatrix4x4>::New();
        transformReceived->GetMatrixTransformToWorld(matrixReceived.GetPointer());

        this->currentPosition.X = std::floor(matrixReceived->GetElement(0,3)*100)/100;
        this->currentPosition.Y = std::floor(matrixReceived->GetElement(1,3)*100)/100;
        this->currentPosition.Z = std::floor(matrixReceived->GetElement(2,3)*100)/100;
        this->TrackingDeviceNode->InvokeEvent(vtkMRMLIGTLConnectorNode::ReceiveEvent);
        this->Modified();
        }
      }
  */
  if (vtkMRMLLinearTransformNode::SafeDownCast(caller) == this->ToolTransform)
    {
    // Data received
    if (event == vtkMRMLLinearTransformNode::TransformModifiedEvent)
      {
      if (this->ToolTransform)
        {
        vtkSmartPointer<vtkMatrix4x4> matrixReceived =
          vtkSmartPointer<vtkMatrix4x4>::New();
        this->ToolTransform->GetMatrixTransformToWorld(matrixReceived.GetPointer());

        this->currentPosition.X = std::floor(matrixReceived->GetElement(0,3)*100)/100;
        this->currentPosition.Y = std::floor(matrixReceived->GetElement(1,3)*100)/100;
        this->currentPosition.Z = std::floor(matrixReceived->GetElement(2,3)*100)/100;
        this->TrackingDeviceNode->InvokeEvent(vtkMRMLIGTLConnectorNode::ReceiveEvent);
        this->Modified();
        }
      }
    else if (event == vtkMRMLIGTLConnectorNode::ConnectedEvent)
      {
      this->InvokeEvent(vtkMRMLIGTLConnectorNode::ConnectedEvent);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::SetTrackingDeviceNode(vtkMRMLIGTLConnectorNode *trackingNode)
{
  if (!trackingNode || (trackingNode == this->TrackingDeviceNode))
    {
    return;
    }

  this->TrackingDeviceNode = trackingNode;
  vtkNew<vtkIntArray> connectorNodeEvents;
  connectorNodeEvents->InsertNextValue(vtkMRMLIGTLConnectorNode::ReceiveEvent);
  connectorNodeEvents->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
  connectorNodeEvents->InsertNextValue(vtkMRMLIGTLConnectorNode::ConnectedEvent);
  connectorNodeEvents->InsertNextValue(vtkMRMLIGTLConnectorNode::DisconnectedEvent);
  vtkObserveMRMLObjectEventsMacro(this->TrackingDeviceNode,
                                  connectorNodeEvents.GetPointer());

}

//---------------------------------------------------------------------------
vtkMRMLBetaProbeNode::trackingData* vtkMRMLBetaProbeNode::GetCurrentPosition()
{
  return &this->currentPosition;
}

//---------------------------------------------------------------------------
vtkMRMLBetaProbeNode::countingData* vtkMRMLBetaProbeNode::GetCurrentCounts()
{
  return &this->currentValues;
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::WriteCountData(std::string date,
                                          std::string time,
                                          double smoothed,
                                          double betaGamma,
                                          double gamma)
{
  if (date.empty() || time.empty())
    {
    return;
    }

  if (!&this->currentValues)
    {
    return;
    }

  this->currentValues.Date.assign(date);
  this->currentValues.Time.assign(time);
  this->currentValues.Smoothed  = smoothed;
  this->currentValues.BetaGamma = betaGamma;
  this->currentValues.Gamma     = gamma;
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLBetaProbeNode::trackingData> vtkMRMLBetaProbeNode::GetTrackerPositions()
{
  return this->trackerPosition;
}

//---------------------------------------------------------------------------
std::vector<vtkMRMLBetaProbeNode::countingData> vtkMRMLBetaProbeNode::GetBetaProbeValues()
{
  return this->countingValues;
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::RecordMappingData()
{
  this->trackerPosition.push_back(this->currentPosition);
  this->countingValues.push_back(this->currentValues);
  this->numberOfCountingDataReceived++;
  this->numberOfTrackingDataReceived++;
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::SetTransformNode(vtkMRMLLinearTransformNode* newTransform)
{
  if (this->ToolTransform == newTransform)
    {
    return;
    }

  this->ToolTransform = newTransform;
}
