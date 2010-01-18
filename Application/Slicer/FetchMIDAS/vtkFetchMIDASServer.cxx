#include "vtkObjectFactory.h"

#include "vtkFetchMIDASServer.h"



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIDASServer );
vtkCxxRevisionMacro ( vtkFetchMIDASServer, "$Revision$");


//---------------------------------------------------------------------------
vtkFetchMIDASServer::vtkFetchMIDASServer ( )
{
  this->Name = NULL;
  this->URIHandlerName = NULL;
  this->TagTableName = NULL;
  this->Parser = NULL;
  this->Writer = NULL;
  this->URIHandler = NULL;
  this->WebServicesClient = NULL;
  this->ServiceType = NULL;
  this->TagTable = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIDASServer::~vtkFetchMIDASServer ( )
{
  if ( this->Name )
    {
    delete [] this->Name;
    this->Name = NULL;
    }
  if ( this->URIHandlerName )
    {
    delete [] this->URIHandlerName;
    this->URIHandlerName = NULL;    
    }
  if ( this->TagTableName )
    {
    delete [] this->TagTableName;
    this->TagTableName = NULL;    
    }
  this->Parser = NULL;
  this->Writer = NULL;
  this->URIHandler = NULL;
  this->WebServicesClient = NULL;
  this->ServiceType = NULL;
  this->TagTable = NULL;
}

//---------------------------------------------------------------------------
void vtkFetchMIDASServer::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    if ( this->Name )
      {
      os << indent << "Name: " << this->GetName() << "\n";
      }
    else
      {
      os << indent << "Name: NULL \n";
      }
    if ( this->URIHandlerName )
      {
      os << indent << "URIHandlerName: " << this->GetURIHandlerName() << "\n";
      }
    else
      {
      os << indent << "URIHandlerName: NULL \n";
      }
    if ( this->TagTableName )
      {
      os << indent << "TagTableName: " << this->GetTagTableName() << "\n";
      }
    else
      {
      os << indent << "TagTableName: NULL \n";
      }


}










