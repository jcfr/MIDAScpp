#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMIDASFlatResourceWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerWindow.h"
#include "vtkSlicerWaitMessageWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWTopLevel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWMessageDialog.h"

#include "vtkFetchMIDASIcons.h"
#include "vtkFetchMIDASTagViewWidget.h"
#include "vtkFetchMIDASHandler.h"

#include "vtkDataIOManager.h"
#include "vtkMRMLScene.h"

#include <vtksys/SystemTools.hxx>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIDASFlatResourceWidget );
vtkCxxRevisionMacro ( vtkFetchMIDASFlatResourceWidget, "$Revision$");


//---------------------------------------------------------------------------
vtkFetchMIDASFlatResourceWidget::vtkFetchMIDASFlatResourceWidget ( )
{

    this->DownloadSelectedButton = NULL;
    this->SelectAllButton = NULL;
    this->DeselectAllButton = NULL;
    this->HelpButton = NULL;
    this->ClearAllButton = NULL;
    this->ClearSelectedButton = NULL;
    this->FetchMIDASIcons = NULL;
    this->DeleteButton = NULL;
    //--- initial (select & result)
    this->NumberOfColumns = 2;
    this->Logic = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIDASFlatResourceWidget::~vtkFetchMIDASFlatResourceWidget ( )
{
  this->RemoveMRMLObservers();
  this->SetLogic ( NULL );

  if ( this->DownloadSelectedButton )
    {
    this->DownloadSelectedButton->SetParent ( NULL );
    this->DownloadSelectedButton->Delete();
    this->DownloadSelectedButton = NULL;    
    }
  if ( this->SelectAllButton )
    {
    this->SelectAllButton->SetParent ( NULL );
    this->SelectAllButton->Delete();
    this->SelectAllButton = NULL;    
    }
  if ( this->DeselectAllButton )
    {
    this->DeselectAllButton->SetParent ( NULL );
    this->DeselectAllButton->Delete();
    this->DeselectAllButton = NULL;
    }
  if ( this->HelpButton )
    {
    this->HelpButton->SetParent ( NULL );
    this->HelpButton->Delete();
    this->HelpButton = NULL;
    }
  if ( this->DeleteButton )
    {
    this->DeleteButton->SetParent ( NULL );
    this->DeleteButton ->Delete();
    this->DeleteButton= NULL;
    }
  if ( this->ClearAllButton )
    {
    this->ClearAllButton->SetParent ( NULL );
    this->ClearAllButton->Delete();
    this->ClearAllButton = NULL;    
    }
  if ( this->ClearSelectedButton )
    {
    this->ClearSelectedButton->SetParent ( NULL );
    this->ClearSelectedButton->Delete();
    this->ClearSelectedButton = NULL;    
    }
  if ( this->FetchMIDASIcons )
    {
    this->FetchMIDASIcons->Delete();
    this->FetchMIDASIcons = NULL;
    }
  this->SetMRMLScene ( NULL );

}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMIDASFlatResourceWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "DownloadSelectedButton: " << this->GetDownloadSelectedButton() << "\n";
    os << indent << "SelectAllButton: " << this->GetSelectAllButton() << "\n";
    os << indent << "ClearSelectedButton: " << this->GetClearSelectedButton() << "\n";
    os << indent << "ClearAllButton: " << this->GetClearAllButton() << "\n";
    os << indent << "DeleteButton: " << this->GetDeleteButton () << "\n";
    os << indent << "DeselectAllButton: " << this->GetDeselectAllButton() << "\n";

}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::RaiseHelpWindow()
{
  int px, py;
  vtkFetchMIDASTagViewWidget *win = vtkFetchMIDASTagViewWidget::New();
  win->SetParent ( this->HelpButton);
  win->Create();
  vtkKWTkUtilities::GetWidgetCoordinates(this->HelpButton, &px, &py);
  win->GetTagViewWindow()->SetPosition ( px + 10, py + 10) ;
  win->SetTagTitle ("Tips on downloading and deleting resources");
  std::stringstream ss;
  ss << "Currently, only MRML scene files can be selected and downloaded directly from a remote host; the datasets a MRML file references will be downloaded implicitly along with the scene. Functionality to select and download image or polygonal data directly is being implemented. To **Download** a MRML scene, select it and choose the **Download** icon -- this will close the current scene and download the selected scene. To **Delete** a MRML scene from the server, select it and choose the **Delete** (trashcan) icon -- this this action is confirmed, the resource will be permanently deleted from the server. Selecting the **Clear All** or **Clear Selected** icons will clean up the display only, without touching the resources on the server.";
  win->SetTagText ( ss.str().c_str() );
  win->DisplayTagViewWindow();
}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  
  if ( this->IsCreated() )
    {
    if ( (b == this->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllItems( );
      }
    else if ( (b == this->GetDeselectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeselectAllItems ( );
      }
    else if ( (b == this->GetDownloadSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
      if ( app )
        {
        if ( app->GetApplicationGUI() )
          {
          if ( app->GetApplicationGUI()->GetMainSlicerWindow() )
            {
            }
          }
        }
      //--- raise the DataIOManager Window.
      if ( this->GetMRMLScene() != NULL )
        {
        if ( this->GetMRMLScene()->GetDataIOManager() != NULL )
          {
          this->GetMRMLScene()->GetDataIOManager()->InvokeEvent(vtkDataIOManager::DisplayManagerWindowEvent );
          }
        }
      std::string msg;
      //--- Request download of each selected resource
      int num = this->GetNumberOfSelectedItems();
      if ( num == 0 )
        {
        vtkWarningMacro ( "No items are selected for download. Download request ignored." );
        return;
        }
      if ( num > 1 )
        {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent ( this->GetParent() );
          dialog->SetStyleToMessage();
          msg = "Currently, only one MRML scene at a time can be downloaded, and there appear to be multiple selections. Please select only scene at a time for download.";
          dialog->SetText ( msg.c_str() );
          dialog->Create();
          dialog->Invoke();
          dialog->Delete();
          return;
        }
      std::string dtype;
      std::string uri;

      //--- try to post a wait message
      if ( this->GetApplication() )
        {
        vtkSlicerApplication* app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
        if ( app )
          {
          vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
          if ( appGUI )
            {
            if (appGUI->GetMainSlicerWindow() )
              {
              vtkSlicerWaitMessageWidget *wm = vtkSlicerWaitMessageWidget::New();
              wm->SetParent ( appGUI->GetMainSlicerWindow() );
              wm->Create();
              wm->SetText ("Downloading requested resources (may take a little while)...");
              this->SetStatusText ( "Downloading requested resources (may take a little while)...");
              wm->DisplayWindow();
              this->Script ("update idletasks");  
              for ( int n=0; n <num; n++)
                {
                dtype = this->GetNthSelectedSlicerDataType (n);
                /// std::cout << "dtype" + dtype + "\n";
                uri = this->GetNthSelectedURI (n);
                /// std::cout << "In" << __FILE__ << " " << __LINE__ << ": " << "uri=" << uri << " dtype=" << dtype << std::endl;
                vtkFetchMIDASHandler *h = vtkFetchMIDASHandler::New();
                h->SetHostName(this->Logic->GetCurrentWebService()->GetName());
                h->SetName("Nom du handler");
                this->Logic->GetCurrentWebService()->SetURIHandler(h);
                this->Logic->RequestResourceDownload (uri.c_str(), dtype.c_str());
                h->Delete();
                }
              wm->SetText ("Downloading requested resources (may take a little while)... done.");
              wm->WithdrawWindow();
              wm->Delete();
              this->SetStatusText ("");
              }
            }
          }
        }
      else
        {
        for ( int n=0; n <num; n++)
          {
          dtype = this->GetNthSelectedSlicerDataType (n);
          uri = this->GetNthSelectedURI (n);
          this->Logic->RequestResourceDownload (uri.c_str(), dtype.c_str());
          }
        }
      }
    else if ( (b == this->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedItems ();
      }
    else if ( (b == this->GetSelectAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SelectAllItems ( );
      }
     else if ( (b == this->GetHelpButton()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->RaiseHelpWindow();
      }
    else if ( (b == this->GetDeleteButton() ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      // Tell the FetchMIGUI to delete all selected resources from the selected server.
      this->InvokeEvent ( vtkFetchMIDASFlatResourceWidget::DeleteResourceEvent );
      }
    }
  this->UpdateMRML();
} 


//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::SetStatusText (const char *txt)
{
  if ( this->GetApplication() )
    {
    if ( (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI() )
      {
      if ( (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI()->GetMainSlicerWindow() )
        {
        (vtkSlicerApplication::SafeDownCast(this->GetApplication()))->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::SelectRow( int i)
{
      this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::SelectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
      this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 1 );
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::DeselectAllItems()
{
  int numrows, i;
  
  numrows = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i = 0; i < numrows; i++ )
    {
      this->GetMultiColumnList()->GetWidget()->SetCellTextAsInt(i, 0, 0 );
      this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);
    }
}



//---------------------------------------------------------------------------
int vtkFetchMIDASFlatResourceWidget::IsItemSelected(int i)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  if ( i >=0 && i < r )
    {
    int sel = this->GetMultiColumnList()->GetWidget()->GetCellTextAsInt (i,0);
    if ( sel == 0 || sel == 1 )
      {
      return (sel);
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::DeleteSelectedItems()
{
 int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      this->GetMultiColumnList()->GetWidget()->DeleteRow(i);
      }
    }
}

//---------------------------------------------------------------------------
int vtkFetchMIDASFlatResourceWidget::GetNumberOfSelectedItems()
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int numSelected = 0;
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      numSelected++;
      }
    }
  return ( numSelected );
}

//---------------------------------------------------------------------------
const char* vtkFetchMIDASFlatResourceWidget::GetNthSelectedSlicerDataType(int n)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;
  
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return (this->GetMultiColumnList()->GetWidget()->GetCellText (i,2) );
        }
      counter++;
      }
    }
  return NULL;
}


//---------------------------------------------------------------------------
const char* vtkFetchMIDASFlatResourceWidget::GetNthSelectedURI (int n)
{
  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  int counter = 0;
  
  for ( int i=0; i < r; i++)
    {
    if ( this->IsItemSelected(i) )
      {
      if ( counter == n )
        {
        return ( this->GetMultiColumnList()->GetWidget()->GetCellText (i,3) );
        }
      counter++;
      }
    }
  return NULL;
}




//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // nothing; handle in parent.
}

//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::AddMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::RemoveMRMLObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::UpdateWidget()
{
}


//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::UpdateMRML()
{
  // nothing for now, not allowing editing
}


//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::RemoveWidgetObservers ( ) {

  this->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetHelpButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDownloadSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeleteButton ( )->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::AddWidgetObservers ( ) {
  this->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeselectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetHelpButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDownloadSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->GetSelectAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GetDeleteButton ()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();
  
  // create the icons
  this->FetchMIDASIcons = vtkFetchMIDASIcons::New();

  // frames for the buttons
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( this->ContainerFrame );
  f->Create();
  vtkKWFrame *f1 = vtkKWFrame::New();
  f1->SetParent ( f );
  f1->Create();
  vtkKWFrame *f2 = vtkKWFrame::New();
  f2->SetParent ( f );
  f2->Create();
  vtkKWFrame *f3 = vtkKWFrame::New();
  f3->SetParent ( f );
  f3->Create();
  
  this->Script ("pack %s -side top -anchor w -expand y -fill x -padx 2 -pady 6", f->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 0 -sticky w -padx 2 -pady 0", f1->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 1 -sticky ew -padx 30 -pady 0", f2->GetWidgetName() );
  this->Script ("grid %s -row 0 -column 2 -sticky e -padx 2 -pady 0", f3->GetWidgetName() );
  this->Script ("grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  this->Script ("grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );
  this->Script ("grid columnconfigure %s 2 -weight 0", f->GetWidgetName() );


  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent ( f1 );
  this->SelectAllButton->Create();
  this->SelectAllButton->SetBorderWidth ( 0 );
  this->SelectAllButton->SetReliefToFlat();  
  this->SelectAllButton->SetImageToIcon ( this->FetchMIDASIcons->GetSelectAllIcon() );
  this->SelectAllButton->SetBalloonHelpString ( "Select all resources for downloading" );

  this->DeselectAllButton = vtkKWPushButton::New();
  this->DeselectAllButton->SetParent ( f1 );
  this->DeselectAllButton->Create();
  this->DeselectAllButton->SetBorderWidth ( 0 );
  this->DeselectAllButton->SetReliefToFlat();  
  this->DeselectAllButton->SetImageToIcon ( this->FetchMIDASIcons->GetDeselectAllIcon() );
  this->DeselectAllButton->SetBalloonHelpString ( "Deselect all resources in list" );

  this->ClearSelectedButton = vtkKWPushButton::New();
  this->ClearSelectedButton->SetParent ( f1 );
  this->ClearSelectedButton->Create();
  this->ClearSelectedButton->SetBorderWidth ( 0 );
  this->ClearSelectedButton->SetReliefToFlat ( );  
  this->ClearSelectedButton->SetImageToIcon ( this->FetchMIDASIcons->GetDeleteSelectedIcon() );
  this->ClearSelectedButton->SetBalloonHelpString ( "Clear selected resources from list" );

  this->ClearAllButton = vtkKWPushButton::New();
  this->ClearAllButton->SetParent ( f1 );
  this->ClearAllButton->Create();
  this->ClearAllButton->SetBorderWidth ( 0 );
  this->ClearAllButton->SetReliefToFlat();  
  this->ClearAllButton->SetImageToIcon ( this->FetchMIDASIcons->GetDeleteAllIcon() );
  this->ClearAllButton->SetBalloonHelpString ( "Clear all resources from list" );

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f2 );
  l1->Create();
  l1->SetText ( "  Download:" );
  this->DownloadSelectedButton = vtkKWPushButton::New();
  this->DownloadSelectedButton->SetParent ( f2 );
  this->DownloadSelectedButton->Create();
  this->DownloadSelectedButton->SetBorderWidth ( 0 );
  this->DownloadSelectedButton->SetReliefToFlat();  
  this->DownloadSelectedButton->SetImageToIcon ( this->FetchMIDASIcons->GetDownloadIcon() );
  this->DownloadSelectedButton->SetBalloonHelpString ( "Download selected resources" );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f2 );
  l2->Create();
  l2->SetText ( "Delete:" );
  this->DeleteButton = vtkKWPushButton::New();
  this->DeleteButton->SetParent ( f2 );
  this->DeleteButton->Create();
  this->DeleteButton->SetBorderWidth ( 0 );
  this->DeleteButton->SetReliefToFlat();
  this->DeleteButton->SetBalloonHelpString ( "Delete selected resources from remote server." );

  this->HelpButton = vtkKWPushButton::New();
  this->HelpButton->SetParent ( f3 );
  this->HelpButton->Create();
  this->HelpButton->SetBorderWidth ( 0 );
  this->HelpButton->SetReliefToFlat();  
  this->HelpButton->SetImageToIcon ( this->FetchMIDASIcons->GetHelpIcon() );
  this->HelpButton->SetBalloonHelpString ( "Tips on using this panel's interface." );

  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
    if ( app->GetApplicationGUI() != NULL )
      {
      this->DeleteButton->SetImageToIcon ( app->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerDeleteIcon() );
      }
    }

  this->Script ("pack %s %s %s %s -side left -anchor w -expand n -padx 2 -pady 2",
                this->SelectAllButton->GetWidgetName(),
                this->DeselectAllButton->GetWidgetName(),
                this->ClearSelectedButton->GetWidgetName(),
                this->ClearAllButton->GetWidgetName() );
  this->Script ("pack %s %s %s %s -side left -anchor w  -expand n -padx 2 -pady 2",
                l1->GetWidgetName(),
                this->DownloadSelectedButton->GetWidgetName(),
                l2->GetWidgetName(),
                this->DeleteButton->GetWidgetName() );
  this->Script ("pack %s -side right -anchor w -expand n -padx 2 -pady 2",
                this->HelpButton->GetWidgetName() );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Select" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOn ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnAlignmentToLeft (0 );
  this->GetMultiColumnList()->GetWidget()->ColumnResizableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->ColumnStretchableOff ( 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnFormatCommandToEmptyOutput(0);
  this->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToCheckButton(0);

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Filename      " );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 1 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 1, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 1 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Slicer Data Type" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 2 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 2, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 2 );

  this->GetMultiColumnList()->GetWidget()->AddColumn ( "Query Result" );
  this->GetMultiColumnList()->GetWidget()->ColumnEditableOff ( 3 );
  this->GetMultiColumnList()->GetWidget()->SetColumnWidth ( 3, 0 );
  this->GetMultiColumnList()->GetWidget()->SetColumnSortModeToAscii ( 3 );

  this->GetMultiColumnList()->GetWidget()->SetHeight ( 22 );
  
  // some problems with editing, so add a call back
  this->GetMultiColumnList()->GetWidget()->SetRightClickCommand(this, "RightClickListCallback");
  
  this->Script ( "pack %s -side top -fill x -pady 0 -expand n", this->GetMultiColumnList()->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  f1->Delete();
  f2->Delete();
  f3->Delete();
  f->Delete();
}


//----------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::RightClickListCallback(int row, int col, int x, int y)
{
    vtkKWMultiColumnList *list =
        this->GetMultiColumnList()->GetWidget();
    list->EditCell(row, col);
}



//---------------------------------------------------------------------------
int  vtkFetchMIDASFlatResourceWidget::GetRowForURI ( const char *URI )
{

  int r = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  vtksys_stl::string wuri;

  for ( int i=0; i<r; i++ )
    {
    wuri.clear();
    wuri = this->GetMultiColumnList()->GetWidget()->GetCellText (i,3);
    if ( !strcmp ( wuri.c_str(), URI ))
      {
      return i;
      }
    }
  return -1;
}



//---------------------------------------------------------------------------
void vtkFetchMIDASFlatResourceWidget::AddNewItem ( const char *term, const char *dtype )
{
  // default query terms in list
  int i, unique;
  std::string fileName;
  std::string uriString = term;

  unique = 1;
  // check to see if term is unique before adding it
  int n = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  for ( i=0; i<n; i++ )
    {
    if ( !strcmp (this->GetMultiColumnList()->GetWidget()->GetCellText(i, 2), term ) )
      {
      unique = 0;
      break;
      }
    }

  if ( unique )
    {
    if ( !strcmp (term, "") )
      {
      term = "unknown";
      }
    if ( !strcmp (dtype, "") )
      {
      term = "unknown";
      }

    i = this->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    this->GetMultiColumnList()->GetWidget()->AddRow();
    this->GetMultiColumnList()->GetWidget()->RowSelectableOff(i);
    this->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton(i, 0);

    //--- pull out filename from uri, if possible.
    size_t found=uriString.find_last_of("/\\");
    if ( found != std::string::npos )
      {
      fileName = uriString.substr(found+1);
      }
    else
      {
      fileName = "unknown";
      }
    if ( !strcmp(fileName.c_str(), "")  )
      {
      fileName = "unknown";
      }
      

    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 1, fileName.c_str() );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 2, dtype );
    this->GetMultiColumnList()->GetWidget()->SetCellText (i, 3, term );
    this->GetMultiColumnList()->GetWidget()->SetCellBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor (i, 0, 1.0, 1.0, 1.0);
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 1,
                                                                               this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 1) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 2,
                                                                               this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 2) );
    this->GetMultiColumnList()->GetWidget()->SetCellSelectionBackgroundColor ( i, 3,
                                                                               this->GetMultiColumnList()->GetWidget()->GetCellBackgroundColor(i, 3) );

    //--- For now, we can only download MRML files; so if the
    //--- SlicerDataType is something other than MRML, disable the row.
    if ( (strcmp(dtype, "MRML")) )
      {
      int cn = this->GetMultiColumnList()->GetWidget()->GetNumberOfColumns();
      for (int j=0; j < cn; j++ )
        {
        this->GetMultiColumnList()->GetWidget()->SetCellEnabledAttribute ( i, j, 0);
        }
      }
    }
}








