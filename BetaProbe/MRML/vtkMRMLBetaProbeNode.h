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

#ifndef __vtkMRMLBetaProbeNode_h
#define __vtkMRMLBetaProbeNode_h


#include "vtkSetGet.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkSlicerBetaProbeModuleMRMLExport.h"

class vtkMRMLScene;

class  VTK_SLICER_BETAPROBE_MODULE_MRML_EXPORT vtkMRMLBetaProbeNode : public vtkMRMLNode
{
public:
  static vtkMRMLBetaProbeNode *New();
  vtkTypeMacro(vtkMRMLBetaProbeNode, vtkMRMLNode);

  typedef struct
  {
    double X;
    double Y;
    double Z;
  }trackingData;

  typedef struct
  {
    std::string Date;
    std::string Time;
    double Smoothed;
    double Beta;
    double Gamma;
  }countingData;

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "BetaProbe";};

  virtual const char* GetIcon() {return ":/Icons/BetaProbe.png";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  void SetTrackingDeviceNode(vtkMRMLIGTLConnectorNode* trackingNode);
  vtkGetObjectMacro(TrackingDeviceNode, vtkMRMLIGTLConnectorNode);

  trackingData* GetCurrentPosition();
  countingData* GetCurrentCounts();

  void WriteCountData(std::string date,
		      std::string time,
		      double smoothed,
		      double beta,
		      double gamma);

protected:
  vtkMRMLBetaProbeNode();
  ~vtkMRMLBetaProbeNode();
  vtkMRMLBetaProbeNode(const vtkMRMLBetaProbeNode&);
  void operator=(const vtkMRMLBetaProbeNode&); 


  vtkMRMLIGTLConnectorNode* TrackingDeviceNode;
  vtkMRMLIGTLConnectorNode* CountingDeviceNode;

  std::vector<trackingData> trackerPosition;
  trackingData currentPosition;
  double numberOfTrackingDataReceived;
  std::vector<countingData> countingValues;
  countingData currentValues;
  double numberOfCountingDataReceived;
};

#endif
