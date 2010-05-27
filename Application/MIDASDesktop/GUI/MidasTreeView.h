#ifndef __MidasTreeView_H
#define __MidasTreeView_H

#include <QTreeView>
#include <string>

class QItemSelection; 
class MidasTreeItem; 
class MidasCommunityTreeItem; 
class MidasCollectionTreeItem; 
class MidasItemTreeItem; 
class QContextMenuEvent; 
class MidasTreeModel; 

namespace mws{
  class WebAPI;
}

namespace mdo{
  class Object;
}

class MidasTreeView : public QTreeView
{
  Q_OBJECT

public:
  
  /** Constructor */
  MidasTreeView(QWidget * parent = 0); 
  ~MidasTreeView();

  /** Set the web API */
  void SetWebAPI(mws::WebAPI* api);

  /** Initialize the tree */
  bool Initialize();

  /** Clear the tree */
  void Clear(); 

  bool isModelIndexSelected() const; 
  //const MidasTreeItem * getMidasTreeItemParent(QModelIndex& selected) const;
  //const std::string getSelectedMidasTreeItemParentId() const; 
  const QModelIndex getSelectedModelIndex() const;
  const MidasTreeItem * getSelectedMidasTreeItem() const;

public slots:
  
  void updateSelection(const QItemSelection &selected,
    const QItemSelection &deselected);
  void decorateByUuid(std::string uuid);
  void alertFetchedMore();
  void selectByObject(mdo::Object* object);
  void Update();

signals:

  void midasTreeItemSelected(const MidasTreeItem* item);
  void midasCommunityTreeItemSelected(const MidasCommunityTreeItem * item); 
  void midasCollectionTreeItemSelected(const MidasCollectionTreeItem * item); 
  void midasItemTreeItemSelected(const MidasItemTreeItem * item); 
  void midasTreeViewContextMenu( QContextMenuEvent * e ); 
  void midasNoTreeItemSelected();
  void fetchedMore();

protected:
 
  void contextMenuEvent ( QContextMenuEvent * e );
  mws::WebAPI*        m_WebAPI;
  MidasTreeModel*     m_Model;
 };

#endif //__MidasTreeView_H