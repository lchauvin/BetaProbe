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

// BetaProbe Logic includes
#include "vtkSlicerBetaProbeLogic.h"

// MRML includes
#include "vtkMRMLBetaProbeNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerBetaProbeLogic);

//----------------------------------------------------------------------------
vtkSlicerBetaProbeLogic::vtkSlicerBetaProbeLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerBetaProbeLogic::~vtkSlicerBetaProbeLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  vtkMRMLBetaProbeNode* betaProbeNode 
    = vtkMRMLBetaProbeNode::New();
  this->GetMRMLScene()->RegisterNodeClass(betaProbeNode);
  betaProbeNode->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerBetaProbeLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

