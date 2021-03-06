#include "vtkObjectFactory.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vtksys/SystemTools.hxx>

#include "vtkFetchMIDASParserHID.h"
#include "vtkSlicerApplication.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIDASParserHID );
vtkCxxRevisionMacro ( vtkFetchMIDASParserHID, "$Revision$");


//---------------------------------------------------------------------------
vtkFetchMIDASParserHID::vtkFetchMIDASParserHID ( )
{
  this->Parser = vtkXMLDataParser::New();
}


//---------------------------------------------------------------------------
vtkFetchMIDASParserHID::~vtkFetchMIDASParserHID ( )
{
  if ( this->Parser )
    {
    this->Parser->Delete();
    this->Parser = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIDASParserHID::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    this->vtkObject::PrintSelf ( os, indent );
    if ( this->Parser )
      {      
      os << indent << "Parser: " << this->GetParser ( ) << "\n";
      }
    else
      {
      os << indent << "Parser: NULL\n";
      }
}



//---------------------------------------------------------------------------
void vtkFetchMIDASParserHID::GetXMLEntry ( vtkXMLDataElement *element )
{

  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMIDASLogic: GetXMLEntry called with null vtkXMLDataElement.");
    return;
    }

  const char *name = element->GetName();
  const char *attName;
  int numAtts;
  const char *value;
  const char *dtype = "unknown";
  std::string currentURI;

  if ( name == NULL )
    {
    return;
    }
  if ( this->GetFetchMIDASNode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIDASLogic: FetchMIDASNode is NULL.");
    return;
    }

  // check to see if this is an item of interest.
  // do different things based on what elements we find.
  //---
  // TAGS
  //---
  if ( (!(strcmp(name, "ul" ))) ||
       (!(strcmp(name, "UL" ))) )
    {
    // see if the attribute in this tag is called Label, and if its value is SlicerDataType
    numAtts = element->GetNumberOfAttributes();
    for ( int i=0; i < numAtts; i++ )
      {
      attName = element->GetAttributeName(i);        
      value = element->GetAttributeValue(i);
      if ( attName != NULL && value != NULL )
        {            
        if ( (!strcmp(value, "mrml" )) ||
             (!strcmp(value, "MRML")) )
          {
          //=== if this says 1, then good. if 3, then we can simplify ths metod.
          int nnested = element->GetNumberOfNestedElements();
          vtkXMLDataElement *nnestedElement = element;
          for ( int j=0; j < nnested; j++)
            {
            nnestedElement = nnestedElement->GetNestedElement ( j );
            if ( nnestedElement == NULL )
              {
              break;
              }
            name = nnestedElement->GetName();
            if ( name != NULL )
              {
              if ( !(strcmp (name, "li" )) ||
                   !(strcmp (name, "LI" )) )
                {
                //--- go in deeper.
                int nnnested = nnestedElement->GetNumberOfNestedElements();
                vtkXMLDataElement *nnnestedElement = nnestedElement;
                for ( int k=0; k < nnnested; k++ )
                  {
                  nnnestedElement = nnnestedElement->GetNestedElement ( k );
                  if ( nnnestedElement == NULL )
                    {
                    break;
                    }
                  name = nnnestedElement->GetName();
                  int numatts2 = nnnestedElement->GetNumberOfAttributes();
                  for (int n=0; n<numatts2; n++)
                    {
                    if ( name != NULL )
                      {
                      if ( !(strcmp (name, "a" )) ||
                           !(strcmp (name, "A" )) )
                        {
                        attName = nnnestedElement->GetAttributeName(n);        
                        value = nnnestedElement->GetAttributeValue(n);
                        if ( attName != NULL && value != NULL )
                          {
                          if ( !(strcmp(attName, "href")) ||
                               !(strcmp(attName, "HREF")))
                            {
                            currentURI.clear();
                            currentURI = value;
                            this->FetchMIDASNode->GetResourceDescription()->AddOrUpdateTag ( currentURI.c_str(), dtype, 0 );
                            break;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}





