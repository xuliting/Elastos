//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "FilePreferencesImpl.h"
#include "CHashSet.h"
#include "XMLParser.h"
#include "CFile.h"
#include "CString.h"

using Elastos::Core::ICharSequence;
using Elastos::Core::CString;
using Elastos::IO::CFile;
using Elastos::IO::EIID_IFilenameFilter;
using Elastos::Utility::CHashSet;

namespace Elastos {
namespace Utility {
namespace Prefs {

CAR_INTERFACE_IMPL(FilePreferencesImpl::FilenameFilter, Object, IFilenameFilter)

FilePreferencesImpl::FilenameFilter::FilenameFilter(
    /* [in] */ FilePreferencesImpl* host)
    : mHost(host)
{
}

ECode FilePreferencesImpl::FilenameFilter::Accept(
    /* [in] */ IFile* dir,
    /* [in] */ const String& filename,
    /* [out] */ Boolean* succeeded)
{
    VALIDATE_NOT_NULL(succeeded);
    AutoPtr<IFile> file;
    CFile::New(mHost->mPath + CFile::sSeparator + filename, (IFile**)&file);
    return file->IsDirectory(succeeded);
}

CAR_INTERFACE_IMPL(FilePreferencesImpl, Object, IAbstractPreferences, IPreferences)

const String FilePreferencesImpl::PREFS_FILE_NAME("prefs.xml");

FilePreferencesImpl::FilePreferencesImpl(
    /* [in] */ const String& path,
    /* [in] */ Boolean isUserNode)
    : AbstractPreferences(NULL, String(""))
    , mPath(path)
{
    mUserNode = isUserNode;
    InitPrefs();
}

FilePreferencesImpl::FilePreferencesImpl(
    /* [in] */ AbstractPreferences* parent,
    /* [in] */ const String& name)
    : AbstractPreferences(parent, name)
{
    mPath = ((FilePreferencesImpl*) parent)->mPath + CFile::sSeparator + name;
    InitPrefs();
}

void FilePreferencesImpl::InitPrefs()
{
    CHashSet::New((ISet**)&mRemoved);
    CHashSet::New((ISet**)&mUpdated);

    CFile::New(mPath, (IFile**)&mDir);
    mDir->Exists(&mNewNode);
    mNewNode = !mNewNode;
    CFile::New(mPath + CFile::sSeparator + PREFS_FILE_NAME, (IFile**)&mPrefsFile);
    XMLParser::ReadXmlPreferences(mPrefsFile, (IProperties**)&mPrefs);
}

ECode FilePreferencesImpl::ChildrenNamesSpi(
    /* [out, callee] */ ArrayOf<String>** list)
{
    VALIDATE_NOT_NULL(list);
    AutoPtr<FilenameFilter> filter = new FilenameFilter(this);
    AutoPtr<ArrayOf<String> > names;
    mDir->List((IFilenameFilter*)filter->Probe(EIID_IFilenameFilter), (ArrayOf<String>**)&names);
    if (names == NULL) {// file is not a directory, exception case
        // throw new BackingStoreException("Cannot get child names for " + toString()
        //         + " (path is " + path + ")");
        return E_BACKING_STORE_EXCEPTION;
    }
    *list = names;
    REFCOUNT_ADD(*list);
    return NOERROR;
}

AutoPtr<AbstractPreferences> FilePreferencesImpl::ChildSpi(
    /* [in] */ const String& name)
{
    AutoPtr<FilePreferencesImpl> child = new FilePreferencesImpl(this, name);
    return child;
}

ECode FilePreferencesImpl::FlushSpi() /*throws BackingStoreException*/
{
    // try {
    //if removed, return
    if(IsRemoved()){
        return NOERROR;
    }
    // reload
    AutoPtr<IProperties> currentPrefs;
    if (FAILED(XMLParser::ReadXmlPreferences(mPrefsFile, (IProperties**)&currentPrefs))) {
        return E_BACKING_STORE_EXCEPTION;
    }
    // merge
    AutoPtr<IIterator> it;
    mRemoved->GetIterator((IIterator**)&it);
    Boolean has = FALSE;
    while (it->HasNext(&has), has) {
        AutoPtr<IInterface> value;
        it->GetNext((IInterface**)&value);
        IMap::Probe(currentPrefs)->Remove(value);
    }
    IMap::Probe(mRemoved)->Clear();

    it = NULL;
    mUpdated->GetIterator((IIterator**)&it);
    while (it->HasNext(&has), has) {
        AutoPtr<IInterface> key;
        it->GetNext((IInterface**)&key);

        AutoPtr<IInterface> value;
        IMap::Probe(mPrefs)->Get(key, (IInterface**)&value);
        IMap::Probe(currentPrefs)->Put(key, value);
    }
    IMap::Probe(mUpdated)->Clear();
    // flush
    mPrefs = currentPrefs;
    if (FAILED(XMLParser::WriteXmlPreferences(mPrefsFile, mPrefs))) {
        return E_BACKING_STORE_EXCEPTION;
    }
    return NOERROR;
    // } catch (Exception e) {
    //     // throw new BackingStoreException(e);
    //     return E_BACKING_STORE_EXCEPTION;
    // }
}

String FilePreferencesImpl::GetSpi(
    /* [in] */ const String& key)
{
    if (mPrefs == NULL) {
        XMLParser::ReadXmlPreferences(mPrefsFile, (IProperties**)&mPrefs);
    }

    String spi;
    ECode ec = mPrefs->GetProperty(key, &spi);
    if (FAILED(ec)) {
        return String(NULL);
    }

    return spi;
}

ECode FilePreferencesImpl::KeysSpi(
    /* [out, callee] */ ArrayOf<String>** spi)
{
    AutoPtr<ISet> ks;
    IMap::Probe(mPrefs)->GetKeySet((ISet**)&ks);
    Int32 size = 0;
    IMap::Probe(ks)->GetSize(&size);
    AutoPtr<ArrayOf<IInterface*> > datas = ArrayOf<IInterface*>::Alloc(size);
    AutoPtr<ArrayOf<String> > strArray = ArrayOf<String>::Alloc(size);
    Int32 i = 0;
    AutoPtr<IIterator> it;
    ks->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        strArray->Set(i++, TO_STR(obj));
    }
    *spi = strArray;
    REFCOUNT_ADD(*spi);
    return NOERROR;
}

ECode FilePreferencesImpl::PutSpi(
    /* [in] */ const String& name,
    /* [in] */ const String& value)
{
    String tmp;
    mPrefs->SetProperty(name, value, &tmp);
    AutoPtr<ICharSequence> obj;
    CString::New(name, (ICharSequence**)&obj);
    return mUpdated->Add(obj);
}

ECode FilePreferencesImpl::RemoveNodeSpi()
{
    mPrefsFile->Delete();
    Boolean removeSucceed = FALSE;
    mDir->Delete(&removeSucceed);
    if (!removeSucceed) {
        // throw new BackingStoreException("Cannot remove " + toString());
        return E_BACKING_STORE_EXCEPTION;
    }
    return NOERROR;
}

ECode FilePreferencesImpl::RemoveSpi(
    /* [in] */ const String& key)
{
    AutoPtr<ICharSequence> obj;
    CString::New(key, (ICharSequence**)&obj);

    IMap::Probe(mPrefs)->Remove(obj);
    IMap::Probe(mUpdated)->Remove(obj);
    return mRemoved->Add(obj);
}

ECode FilePreferencesImpl::SyncSpi()
{
    return FlushSpi();
}

ECode FilePreferencesImpl::ToString(
    /* [out] */ String* info)
{
    VALIDATE_NOT_NULL(info);
    *info = String("Elastos.Utility.Prefs.FilePreferencesImpl");
    return NOERROR;
}

} // namespace Prefs
} // namespace Utility
} // namespace Elastos
