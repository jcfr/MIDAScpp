#ifndef __MIDASDesktopUI_H
#define __MIDASDesktopUI_H

#include <QFlags>
#include <QProgressBar>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QTimer>

#include "ui_MIDASDesktopUI.h"

class UploadAgreementUI;
class UpDownLoadUI; 
class CreateMidasCommunityUI; 
class CreateMidasCollectionUI; 
class CreateMidasItemUI; 
class SignInUI;
class PullUI;
class CreateProfileUI;
class DeleteResourceUI;
class AboutUI;
class AutoRefreshUI;
class midasSynchronizer;
class midasAuthenticator;
class midasDatabaseProxy;
class midasLog;
class midasProgressReporter;

class RefreshServerTreeThread;
class SynchronizerThread;
class SearchThread;

class QContextMenuEvent;
class MidasTreeItem;

namespace mdo {
  class Object;
}

extern "C" {
  static int progress_transfer_callback(void* data, double dltotal, double dlnow, double ultotal, double ulnow); 
}

class MIDASDesktopUI :  public QMainWindow, private Ui::MIDASDesktopWindow
{
  Q_OBJECT

public:

  enum ActivateAction
    {
    ACTION_ALL                   = 0xFF,
    ACTION_CONNECTED             = 0x1,
    ACTION_ALL_CONNECTED         = 0xFF - 0x1,
    ACTION_COMMUNITY             = 0x2,
    ACTION_COLLECTION            = 0x4,
    ACTION_ITEM                  = 0x8,
    ACTION_BITSTREAM             = 0x30, // 0x30 = 0x10 + 0x20 = (110000)b
    ACTION_BITSTREAM_LIST        = 0x10,
    ACTION_BITSTREAM_COMPUTER    = 0x20,
    ACTION_BITSTREAM_TRANSFERING = 0x40,
    ACTION_CLIENT_COMMUNITY      = 0x80,
    ACTION_CLIENT_COLLECTION     = 0x100,
    ACTION_CLIENT_ITEM           = 0x200,
    ACTION_CLIENT_BITSTREAM      = 0x400,
    ACTION_CLIENT_RESOURCE       = 0x780, //0x80 + 0x100 + 0x200 + 0x400 (any client side resource)
    ACTION_LOCAL_DATABASE        = 0x800
    }; 
  Q_DECLARE_FLAGS(ActivateActions, ActivateAction)
     
  MIDASDesktopUI();
  ~MIDASDesktopUI();

  void activateActions(bool value, ActivateActions activateAction); 

  MidasTreeView * getTreeView() { return treeView; }
  MidasTreeViewClient * getTreeViewClient() { return treeViewClient; }
  midasDatabaseProxy* getDatabaseProxy() { return m_database; }
  midasAuthenticator* getAuthenticator() { return m_auth; }
  midasSynchronizer* getSynchronizer() { return m_synch; }
  midasLog* getLog() { return m_logger; }
  QTextEdit* getLogTextEdit() { return log; }

protected:
  void closeEvent(QCloseEvent *event);

public slots:
  void showNormal();

  // ------------- status bar -------------
  void displayStatus(const QString& message);
  void resetStatus();
  // ------------- status bar -------------

  void signInOrOut();
  void signIn(bool ok);
  void signOut();
  void createProfile(std::string name, std::string email,
                     std::string apiName, std::string apiKey );
  void chooseLocalDatabase();
  void setLocalDatabase(std::string file);
  void deleteLocalResource(bool deleteFiles);
  void updateClientTreeView();
  void updateServerTreeView();
  void decorateServerTree();
  void decorateTheObject();

  void startedExpandingTree();
  void finishedExpandingTree();

  void enableActions(bool val);

  void setTreeTabIndex(int index);

  // ------------- settings -------------
  void setTimerInterval();
  void adjustTimerSettings();
  // ------------- settings -------------

  // -------------- progress bar ----------
  void setProgressIndeterminate();
  void setProgressEmpty();
  // -------------- progress bar ----------

  // ------------- tray icon -------------
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void alertNewResources();
  // ------------- tray icon -------------

  // ------------- UI updates -------------
  void updateInfoPanel( const MidasCommunityTreeItem* communityTreeItem );
  void updateInfoPanel( const MidasCollectionTreeItem* collectionTreeItem );
  void updateInfoPanel( const MidasItemTreeItem* itemTreeItem );
  void updateInfoPanel( const MidasBitstreamTreeItem* bitstreamTreeItem );
  void clearInfoPanel();

  void updateActionState(const MidasTreeItem* item);
  void updateActionStateClient(const MidasTreeItem* item);

  void displayServerResourceContextMenu(QContextMenuEvent* e);
  void displayClientResourceContextMenu(QContextMenuEvent* e);
  // ------------- UI updates -------------

  // ------------- resource manipulation -------------

  void addCommunity();
  void addSubcommunity();
  void addCollection();
  void addItem();
  void addBitstream();
  void addBitstreams(const MidasItemTreeItem* parentItem,
                     const QStringList & files);
  void openBitstream(int id);
  void viewInBrowser();
  void storeLastPollTime();
  // ------------- resource manipulation -------------

  // ------------- synchronizer ----------------------
  void pushResources();
  void pushReturned(bool val);
  // ------------- synchronizer ----------------------

  // ------------- search -------------
  void search();
  void showSearchResults();
  void searchItemClicked(QListWidgetItemMidasItem * item);
  void searchItemContextMenu(QContextMenuEvent * e);
  // ------------- search -------------

  void setServerURL(std::string);

  // ------------- log ----------------
  void showLogTab();
  void alertErrorInLog();
  void clearLogTabIcon(int index);
  // ------------- log ----------------

private:

  // ------------- UI Dialogs -------------
  CreateMidasCommunityUI *    dlg_createMidasCommunityUI;
  CreateMidasCommunityUI *    dlg_createMidasSubCommunityUI;
  CreateMidasCommunityUI *    dlg_addMidasCommunityUI;
  CreateMidasCommunityUI *    dlg_addMidasSubCommunityUI;
  CreateMidasCollectionUI *   dlg_createMidasCollectionUI;
  CreateMidasCollectionUI *   dlg_addMidasCollectionUI;
  CreateMidasItemUI *         dlg_createMidasItemUI;
  CreateMidasItemUI *         dlg_addMidasItemUI;
  CreateProfileUI *           dlg_createProfileUI;
  SignInUI *                  dlg_signInUI;
  UpDownLoadUI *              dlg_upDownLoadUI;
  UploadAgreementUI *         dlg_uploadAgreementUI;
  AboutUI *                   dlg_aboutUI;
  AutoRefreshUI *             dlg_autoRefreshUI;
  PullUI *                    dlg_pullUI;
  DeleteResourceUI*           dlg_deleteResourceUI;
  // ------------- UI Dialogs -------------

  // ------------- status bar -------------
  QLabel *                    stateLabel;
  QLabel *                    connectLabel;
  QProgressBar *              progressBar;
  // ------------- status bar -------------

  // ------------- tray ----------------
  QAction *                   showAction;
  QSystemTrayIcon *           trayIcon;
  QMenu *                     trayIconMenu;
  // ------------- tray ----------------

  // ------------- auto-refresh -----------
  QTimer *                    refreshTimer;
  // ------------- auto-refresh -----------


  bool                        m_signIn;
  midasDatabaseProxy*         m_database;
  midasAuthenticator*         m_auth;
  midasSynchronizer*          m_synch;
  std::string                 m_url;
  midasLog*                   m_logger;
  midasProgressReporter*      m_progress;
  std::vector<std::string>    m_dirtyUuids;
  std::vector<mdo::Object*>   m_SearchResults;

  // ----------- threads -----------------
  RefreshServerTreeThread*    m_RefreshThread;
  SynchronizerThread*         m_SynchronizerThread;
  SearchThread*               m_SearchThread;
  // ----------- threads -----------------

  mdo::Object*                m_DecorateObject;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( MIDASDesktopUI::ActivateActions )

#endif //__MIDASDesktopUI_H
