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

// 
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
  this->CountingDeviceNode = NULL;
  this->numberOfTrackingDataReceived = 0;
  
  this->currentPosition.x = 0.0;
  this->currentPosition.y = 0.0;
  this->currentPosition.z = 0.0;

  this->currentValues.beta     = 0.0;
  this->currentValues.gamma    = 0.0;
  this->currentValues.smoothed = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLBetaProbeNode::~vtkMRMLBetaProbeNode()
{
  if (this->TrackingDeviceNode)
    {
    this->TrackingDeviceNode->Delete();
    }

  if (this->CountingDeviceNode)
    {
    this->CountingDeviceNode->Delete();
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
  if (vtkMRMLIGTLConnectorNode::SafeDownCast(caller) == this->TrackingDeviceNode)
    {
    // Data received
    if (event == vtkMRMLIGTLConnectorNode::ReceiveEvent)
      {
      vtkMRMLIGTLConnectorNode* connector =
	vtkMRMLIGTLConnectorNode::SafeDownCast(caller);
      vtkMRMLLinearTransformNode* transformReceived =
	vtkMRMLLinearTransformNode::SafeDownCast(this->TrackingDeviceNode->GetIncomingMRMLNode(this->TrackingDeviceNode->GetNumberOfIncomingMRMLNodes()-1));
      if (transformReceived)
	{
	vtkSmartPointer<vtkMatrix4x4> matrixReceived =
	  vtkSmartPointer<vtkMatrix4x4>::New();
	transformReceived->GetMatrixTransformToWorld(matrixReceived.GetPointer());
	
	this->currentPosition.x = matrixReceived->GetElement(0,3);
	this->currentPosition.y = matrixReceived->GetElement(1,3);
	this->currentPosition.z = matrixReceived->GetElement(2,3);
	
	if (this->numberOfTrackingDataReceived < MAX_DATA_SAVED)
	  {
	  this->trackerPosition.push_back(this->currentPosition);
	  }
	else
	  {
	  this->trackerPosition[std::fmod(this->numberOfTrackingDataReceived, MAX_DATA_SAVED)] = this->currentPosition;
	  }
	this->numberOfTrackingDataReceived++;
	this->Modified();
	}
      }
    else if (event == vtkMRMLIGTLConnectorNode::ConnectedEvent)
      {
      this->InvokeEvent(vtkMRMLIGTLConnectorNode::ConnectedEvent);
      }
    }

  // Counting node
  else if (vtkMRMLIGTLConnectorNode::SafeDownCast(caller) == this->CountingDeviceNode)
    {
    // Data received
    if (event == vtkMRMLIGTLConnectorNode::ReceiveEvent)
      {
      vtkMRMLIGTLConnectorNode* connector =
	vtkMRMLIGTLConnectorNode::SafeDownCast(caller);
      vtkMRMLLinearTransformNode* transformReceived =
	vtkMRMLLinearTransformNode::SafeDownCast(this->CountingDeviceNode->GetIncomingMRMLNode(this->CountingDeviceNode->GetNumberOfIncomingMRMLNodes()-1));
      if (transformReceived)
	{
	vtkSmartPointer<vtkMatrix4x4> matrixReceived =
	  vtkSmartPointer<vtkMatrix4x4>::New();
	transformReceived->GetMatrixTransformToWorld(matrixReceived.GetPointer());
	
	// TODO: BetaProbe message ?
	this->currentValues.beta	   = matrixReceived->GetElement(0,3);
	this->currentValues.gamma	   = matrixReceived->GetElement(1,3);
	this->currentValues.smoothed = matrixReceived->GetElement(2,3);
	
	if (this->numberOfCountingDataReceived < MAX_DATA_SAVED)
	  {
	  this->countingValues.push_back(this->currentValues);
	  }
	else
	  {
	  this->countingValues[std::fmod(this->numberOfCountingDataReceived, MAX_DATA_SAVED)] = this->currentValues;
	  }
	this->numberOfCountingDataReceived++;
	this->Modified();
	}					       
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
   connectorNodeEvents->InsertNextValue(vtkMRMLIGTLConnectorNode::ConnectedEvent);
   connectorNodeEvents->InsertNextValue(vtkMRMLIGTLConnectorNode::DisconnectedEvent);
   vtkObserveMRMLObjectEventsMacro(this->TrackingDeviceNode,
				  connectorNodeEvents.GetPointer());
  
}

//---------------------------------------------------------------------------
void vtkMRMLBetaProbeNode::SetCountingDeviceNode(vtkMRMLIGTLConnectorNode* countingNode)
{
  if (!countingNode || (countingNode == this->CountingDeviceNode))
    {
    return;
    }

  this->CountingDeviceNode = countingNode;
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
