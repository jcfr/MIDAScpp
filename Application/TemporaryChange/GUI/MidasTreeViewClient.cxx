#include "MidasTreeViewClient.h"

#include <QtGui>
#include <QItemSelection>
#include <QContextMenuEvent>
#include <QModelIndex>

#include "MIDASDesktopUI.h"
#include "MidasTreeItem.h"
#include "MidasTreeModelClient.h"
#include "MidasCommunityTreeItem.h"
#include "MidasCollectionTreeItem.h"
#include "MidasItemTreeItem.h"
#include "MidasBitstreamTreeItem.h"
#include "Logger.h"
#include "Utils.h"
#include "MidasClientGlobal.h"
#include "mwsCommunity.h"
#include "mdoBitstream.h"
#include "midasLog.h"
#include <iostream>

/** Constructor */
MidasTreeViewClient::MidasTreeViewClient(QWidget * parent):QTreeView(parent)
{  

  // The tree model
  m_Model = new MidasTreeModelClient;
  this->setModel(m_Model);
  
  this->setSelectionMode( QTreeView::SingleSelection );

  connect (this, SIGNAL( collapsed(const QModelIndex&)),
     this->model(), SLOT(itemCollapsed(const QModelIndex&)) );
  connect (this, SIGNAL( expanded(const QModelIndex&)),
     this->model(), SLOT(itemExpanded (const QModelIndex&)) );

  // define action to be triggered when tree item is selected
  QItemSelectionModel * itemSelectionModel = this->selectionModel();
  connect(itemSelectionModel,
     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
     this, SLOT(updateSelection(const QItemSelection&, const QItemSelection& )));
}

/** Destructor */
MidasTreeViewClient::~MidasTreeViewClient()
{
  delete m_Model;
}

void MidasTreeViewClient::SetDatabaseProxy(midasDatabaseProxy* proxy)
{
  this->m_Model->SetDatabase(proxy);
}

void MidasTreeViewClient::SetLog(midasLog* log)
{
  this->m_Model->SetLog(log);
}

void MidasTreeViewClient::Update()
{
  this->Clear();
  this->Initialize();
}

/** Initialize the tree */
bool MidasTreeViewClient::Initialize()
{
  m_Model->Populate();
  return true;
}

/** Clear */
void MidasTreeViewClient::Clear()
{
  this->m_Model->clear(QModelIndex());
  disconnect(this);
  this->reset();
}

bool MidasTreeViewClient::isModelIndexSelected() const
{
  QItemSelectionModel * selectionModel = this->selectionModel();
  assert(selectionModel != NULL);
  return (selectionModel->selectedIndexes().size() > 0);
}

const QModelIndex MidasTreeViewClient::getSelectedModelIndex() const
{
  QItemSelectionModel * selectionModel = this->selectionModel();
  assert(selectionModel != NULL);
  QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
  assert(selectedIndexes.size() == 1);
  return selectedIndexes.first();
}

const MidasTreeItem * MidasTreeViewClient::getSelectedMidasTreeItem() const
{
  return reinterpret_cast<MidasTreeModelClient*>(
    this->model())->midasTreeItem(this->getSelectedModelIndex());
}

void MidasTreeViewClient::updateSelection(const QItemSelection &selected,
                                    const QItemSelection &deselected)
{
  assert(this->selectionMode() == QTreeView::SingleSelection);

  QModelIndex index;
  QModelIndexList items = selected.indexes();
  if (items.size() > 0)
    {
    MidasTreeItem * item = const_cast<MidasTreeItem*>( m_Model->midasTreeItem(items.first()) );
    emit midasTreeItemSelected(item);

    MidasCommunityTreeItem * communityTreeItem = NULL;
    MidasCollectionTreeItem * collectionTreeItem = NULL;
    MidasItemTreeItem * itemTreeItem = NULL;
    MidasBitstreamTreeItem * bitstreamTreeItem = NULL;

    if ((communityTreeItem = dynamic_cast<MidasCommunityTreeItem*>(item)) != NULL)
      {
      emit midasCommunityTreeItemSelected(communityTreeItem);
      }
    else if ((collectionTreeItem = dynamic_cast<MidasCollectionTreeItem*>(item)) != NULL)
      {
      emit midasCollectionTreeItemSelected(collectionTreeItem);
      }
    else if ((itemTreeItem = dynamic_cast<MidasItemTreeItem*>(item)) != NULL)
      {
      emit midasItemTreeItemSelected(itemTreeItem);
      }
    else if ((bitstreamTreeItem = dynamic_cast<MidasBitstreamTreeItem*>(item)) != NULL )
      {
      emit midasBitstreamTreeItemSelected(bitstreamTreeItem);
      }
    }
  else 
    {
    emit midasNoTreeItemSelected(); 
    }
}

void MidasTreeViewClient::contextMenuEvent( QContextMenuEvent * e )
{
  emit midasTreeViewContextMenu( e );
}

MidasItemTreeItem * MidasTreeViewClient::selectedMidasItemTreeItem()
{
  MidasItemTreeItem * itemTreeItem = NULL; 
  QModelIndexList selected = this->selectedIndexes(); 
  if(selected.size() > 0)
    {
//    MidasTreeItem * item = const_cast<MidasTreeItem*>( this->model()->midasTreeItem(selected.first()) ); 
//    itemTreeItem = dynamic_cast<MidasItemTreeItem*>(item); 
    }
  return itemTreeItem; 
}

void MidasTreeViewClient::dragEnterEvent( QDragEnterEvent * event )
{
  if (event && event->mimeData())
    {
    const QMimeData * md = event->mimeData();
    if ( md->hasUrls() )
      {
      event->acceptProposedAction();
      }
    }
}

void MidasTreeViewClient::mouseDoubleClickEvent(QMouseEvent *event)
{
  MidasBitstreamTreeItem* bitstream = NULL;
  MidasTreeItem* node = const_cast<MidasTreeItem*>(
    m_Model->midasTreeItem(this->indexAt(event->pos())));

  if ((bitstream = dynamic_cast<MidasBitstreamTreeItem*>(node)) != NULL)
    {
    emit bitstreamOpenRequest(bitstream->getBitstream()->GetId());
    }
  else
    {
    QTreeView::mouseDoubleClickEvent(event);
    }
}

void MidasTreeViewClient::dragLeaveEvent( QDragLeaveEvent * event )
{
}

void MidasTreeViewClient::dragMoveEvent( QDragMoveEvent * event )
{
  MidasItemTreeItem* item = NULL;
  MidasTreeItem* node = const_cast<MidasTreeItem*>(
    m_Model->midasTreeItem(this->indexAt(event->pos())));

  selectionModel()->clearSelection();
  selectionModel()->select(this->indexAt(event->pos()), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  
  if ((item = dynamic_cast<MidasItemTreeItem*>(node)) != NULL)
    {
    event->acceptProposedAction();
    }
  else
    {
    event->setAccepted(false);
    }
}

void MidasTreeViewClient::dropEvent( QDropEvent * event )
{
  if (event && event->mimeData())
    {
    MidasItemTreeItem* item = NULL;
    MidasTreeItem* node = const_cast<MidasTreeItem*>(
      m_Model->midasTreeItem(this->indexAt(event->pos())));

    if ((item = dynamic_cast<MidasItemTreeItem*>(node)) != NULL)
      {
      const QMimeData * md = event->mimeData();
      if ( md->hasUrls() )
        {
        QStringList files;
        foreach (QUrl url, md->urls())
          {
          QFileInfo info(url.toLocalFile());
          if (info.exists() && info.isFile())
            {
            files << url.toLocalFile();
            }
          }
        emit bitstreamsDropped( item, files );
        event->acceptProposedAction();
        }
      }
    }
}
