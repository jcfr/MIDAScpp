#ifndef __vtkMRMLFetchMIDASNode_h
#define __vtkMRMLFetchMIDASNode_h

#include "vtkStringArray.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkTagTable.h"
#include "vtkTagTableCollection.h"

#include "vtkFetchMIDASWin32Header.h"
#include <string>
#include <vector>


class VTK_FETCHMIDAS_EXPORT vtkMRMLFetchMIDASNode : public vtkMRMLNode
{
  public:
  static vtkMRMLFetchMIDASNode *New();
  vtkTypeRevisionMacro(vtkMRMLFetchMIDASNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( ResourceDescription, vtkTagTable );

  vtkGetObjectMacro ( TagTableCollection, vtkTagTableCollection );
  vtkGetObjectMacro ( SelectedTagTable, vtkTagTable );
  vtkSetObjectMacro ( SelectedTagTable, vtkTagTable );

  // Description:
  // This method sets the SelectedServer, the SelectedTagTable,
  // and the ServiceType, then invokes an vtkMRMLFetchMIDASNode::SelectedServerModifiedEvent 
  // Developers: as new servers are supported, add them in this method.
  void SetServer ( const char* );

  vtkGetStringMacro ( SelectedServer );
  vtkSetStringMacro ( SelectedServer );

  // Description:
  // This method sets the service type without invoking
  // an event on the node.
  void SetServiceType ( const char* );
  vtkGetStringMacro ( SelectedServiceType );
  vtkSetStringMacro ( SelectedServiceType );

  vtkGetStringMacro ( ErrorMessage);
  vtkSetStringMacro ( ErrorMessage);

  // Description:
  // Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  
  
  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FetchMIDAS"; };

  virtual void RaiseErrorEvent ();
  virtual void RaiseErrorChoice ();
  
  // contains uri and SlicerDataType from queries
  vtkTagTable *ResourceDescription;

  // Description:
  // This method adds a tag table to contain metadata
  // for each new web service type. DEVELOPERS NOTE:
  // as new web services are added, this method should
  // be extended.
  void AddTagTablesForWebServices();

  // contains a tag table from each type of webservice.
  vtkTagTableCollection *TagTableCollection;
  // contains currently selected tag table
  vtkTagTable *SelectedTagTable;

  char *SelectedServer;
  char *SelectedServiceType;
  char *ErrorMessage;

  //BTX
  enum
    {
      KnownServersModifiedEvent = 11000,
      SelectedServerModifiedEvent,
      TagsModifiedEvent,
      ResourceResponseReadyEvent,
      TagResponseReadyEvent,
      TagValueResponseReadyEvent,
      NewNodeTaggedEvent,
      SaveSelectionEvent,
      RemoteIOErrorEvent,
      RemoteIODirectoryErrorEvent,
      RemoteIOErrorChoiceEvent,
    };
  //ETX

 protected:
  vtkMRMLFetchMIDASNode();
  ~vtkMRMLFetchMIDASNode();
  vtkMRMLFetchMIDASNode(const vtkMRMLFetchMIDASNode&);
  void operator=(const vtkMRMLFetchMIDASNode&);

};

#endif

