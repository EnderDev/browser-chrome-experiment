/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_SessionStoreListener_h
#define mozilla_dom_SessionStoreListener_h

#include "SessionStoreData.h"
#include "nsIDOMEventListener.h"
#include "nsIObserver.h"
#include "nsIPrivacyTransitionObserver.h"
#include "nsIWebProgressListener.h"
#include "nsWeakReference.h"

class nsITimer;

namespace mozilla {
namespace dom {

class StorageEvent;

class ContentSessionStore {
 public:
  explicit ContentSessionStore(nsIDocShell* aDocShell);
  NS_INLINE_DECL_CYCLE_COLLECTING_NATIVE_REFCOUNTING(ContentSessionStore)
  NS_DECL_CYCLE_COLLECTION_NATIVE_CLASS(ContentSessionStore)

  void OnPrivateModeChanged(bool aEnabled);
  bool IsDocCapChanged() { return mDocCapChanged; }
  nsCString GetDocShellCaps();
  bool IsPrivateChanged() { return mPrivateChanged; }
  bool GetPrivateModeEnabled();

  // Use "mStorageStatus" to manage the status of storageChanges
  bool IsStorageUpdated() { return mStorageStatus != NO_STORAGE; }
  void ResetStorage() { mStorageStatus = RESET; }
  /*
    There are three situations we need entire session storage:
    1. OnDocumentStart: PageLoad started
    2. OnDocumentEnd: PageLoad completed
    3. receive "browser:purge-sessionStorage" event
    Use SetFullStorageNeeded() to set correct "mStorageStatus" and
    reset the pending individual change.
   */
  void SetFullStorageNeeded();
  void ResetStorageChanges();
  // GetAndClearStorageChanges() is used for getting storageChanges.
  // It clears the stored storage changes before returning.
  // It will return true if it is a entire session storage.
  // Otherwise, it will return false.
  bool GetAndClearStorageChanges(nsTArray<nsCString>& aOrigins,
                                 nsTArray<nsString>& aKeys,
                                 nsTArray<nsString>& aValues);
  // Using AppendSessionStorageChange() to append session storage change when
  // receiving "MozSessionStorageChanged".
  // Return true if there is a new storage change which is appended.
  bool AppendSessionStorageChange(StorageEvent* aEvent);

  void SetSHistoryChanged();
  // request "collect sessionHistory" which is happened in the parent process
  void SetSHistoryFromParentChanged();
  bool GetAndClearSHistoryChanged() {
    bool ret = mSHistoryChanged;
    mSHistoryChanged = false;
    mSHistoryChangedFromParent = false;
    return ret;
  }

  void OnDocumentStart();
  void OnDocumentEnd();
  bool UpdateNeeded() {
    return mPrivateChanged || mDocCapChanged || IsStorageUpdated() ||
           mSHistoryChanged || mSHistoryChangedFromParent;
  }

 private:
  virtual ~ContentSessionStore() = default;
  nsCString CollectDocShellCapabilities();

  nsCOMPtr<nsIDocShell> mDocShell;
  bool mPrivateChanged;
  bool mIsPrivate;
  enum {
    NO_STORAGE,
    RESET,
    FULLSTORAGE,
    STORAGECHANGE,
  } mStorageStatus;
  bool mDocCapChanged;
  nsCString mDocCaps;
  // mOrigins, mKeys, mValues are for sessionStorage partial changes
  nsTArray<nsCString> mOrigins;
  nsTArray<nsString> mKeys;
  nsTArray<nsString> mValues;
  // mSHistoryChanged means there are history changes which are found
  // in the child process. The flag is set when
  //    1. webProgress changes to STATE_START
  //    2. webProgress changes to STATE_STOP
  //    3. receiving "DOMTitleChanged" event
  bool mSHistoryChanged;
  // mSHistoryChangedFromParent means there are history changes which
  // are found by session history listener in the parent process.
  bool mSHistoryChangedFromParent;
};

class TabListener : public nsIDOMEventListener,
                    public nsIObserver,
                    public nsIPrivacyTransitionObserver,
                    public nsIWebProgressListener,
                    public nsSupportsWeakReference {
 public:
  explicit TabListener(nsIDocShell* aDocShell, Element* aElement);
  EventTarget* GetEventTarget();
  nsresult Init();
  ContentSessionStore* GetSessionStore() { return mSessionStore; }
  // the function is called only when TabListener is in parent process
  bool ForceFlushFromParent(bool aIsFinal);
  void RemoveListeners();
  void SetEpoch(uint32_t aEpoch) { mEpoch = aEpoch; }
  uint32_t GetEpoch() { return mEpoch; }
  void UpdateSHistoryChanges(bool aImmediately);
  void SetOwnerContent(Element* aElement);

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(TabListener, nsIDOMEventListener)

  NS_DECL_NSIDOMEVENTLISTENER
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIPRIVACYTRANSITIONOBSERVER
  NS_DECL_NSIWEBPROGRESSLISTENER

 private:
  static void TimerCallback(nsITimer* aTimer, void* aClosure);
  void AddTimerForUpdate();
  void StopTimerForUpdate();
  void AddEventListeners();
  void RemoveEventListeners();
  bool UpdateSessionStore(bool aIsFlush = false, bool aIsFinal = false);
  void ResetStorageChangeListener();
  void RemoveStorageChangeListener();
  virtual ~TabListener();

  nsCOMPtr<nsIDocShell> mDocShell;
  RefPtr<ContentSessionStore> mSessionStore;
  RefPtr<mozilla::dom::Element> mOwnerContent;
  bool mProgressListenerRegistered;
  bool mEventListenerRegistered;
  bool mPrefObserverRegistered;
  bool mStorageObserverRegistered;
  bool mStorageChangeListenerRegistered;
  // Timer used to update data
  nsCOMPtr<nsITimer> mUpdatedTimer;
  bool mTimeoutDisabled;
  int32_t mUpdateInterval;
  uint32_t mEpoch;
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_SessionStoreListener_h
