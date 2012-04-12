/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */


#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qtextstream.h>

QString
KAboutPerson::name() const
{
   return QString::fromUtf8(mName);
}

QString
KAboutPerson::task() const
{
   if (mTask && *mTask)
      return i18n(mTask);
   else
      return QString::null;
}

QString
KAboutPerson::emailAddress() const
{
   return QString::fromUtf8(mEmailAddress);
}


QString
KAboutPerson::webAddress() const
{
   return QString::fromUtf8(mWebAddress);
}


KAboutTranslator::KAboutTranslator(const QString & name,
                const QString & emailAddress)
{
    mName=name;
    mEmail=emailAddress;
}

QString KAboutTranslator::name() const
{
    return mName;
}

QString KAboutTranslator::emailAddress() const
{
    return mEmail;
}

class KAboutDataPrivate
{
public:
    KAboutDataPrivate()
        : translatorName("_: NAME OF TRANSLATORS\nYour names")
        , translatorEmail("_: EMAIL OF TRANSLATORS\nYour emails")
        , productName(0)
        , programLogo(0)
        , customAuthorTextEnabled(false)
        , mTranslatedProgramName( 0 )
        {}
    ~KAboutDataPrivate()
        {
             delete programLogo;
             delete[] mTranslatedProgramName;
        }
    const char *translatorName;
    const char *translatorEmail;
    const char *productName;
    QImage* programLogo;
    QString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;
    const char *mTranslatedProgramName;
};



KAboutData::KAboutData( const char *appName,
                        const char *programName,
                        const char *version,
                        const char *shortDescription,
			int licenseType,
			const char *copyrightStatement,
			const char *text,
			const char *homePageAddress,
			const char *bugsEmailAddress
			) :
  mProgramName( programName ),
  mVersion( version ),
  mShortDescription( shortDescription ),
  mLicenseKey( licenseType ),
  mCopyrightStatement( copyrightStatement ),
  mOtherText( text ),
  mHomepageAddress( homePageAddress ),
  mBugEmailAddress( bugsEmailAddress ),
  mLicenseText (0)
{
   d = new KAboutDataPrivate;

   if( appName ) {
     const char *p = strrchr(appName, '/');
     if( p )
	 mAppName = p+1;
     else
	 mAppName = appName;
   } else
     mAppName = 0;
}

KAboutData::~KAboutData()
{
    if (mLicenseKey == License_File)
        delete [] mLicenseText;
    delete d;
}

void
KAboutData::addAuthor( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mAuthorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::addCredit( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mCreditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::setTranslator( const char *name, const char *emailAddress)
{
  d->translatorName=name;
  d->translatorEmail=emailAddress;
}

void
KAboutData::setLicenseText( const char *licenseText )
{
  mLicenseText = licenseText;
  mLicenseKey = License_Custom;
}

void
KAboutData::setLicenseTextFile( const QString &file )
{
  mLicenseText = qstrdup(QFile::encodeName(file));
  mLicenseKey = License_File;
}

void
KAboutData::setAppName( const char *appName )
{
  mAppName = appName;
}

void
KAboutData::setProgramName( const char* programName )
{
  mProgramName = programName;
  translateInternalProgramName();
}

void
KAboutData::setVersion( const char* version )
{
  mVersion = version;
}

void
KAboutData::setShortDescription( const char *shortDescription )
{
  mShortDescription = shortDescription;
}

void
KAboutData::setLicense( LicenseKey licenseKey)
{
  mLicenseKey = licenseKey;
}

void
KAboutData::setCopyrightStatement( const char *copyrightStatement )
{
  mCopyrightStatement = copyrightStatement;
}

void
KAboutData::setOtherText( const char *otherText )
{
  mOtherText = otherText;
}

void
KAboutData::setHomepage( const char *homepage )
{
  mHomepageAddress = homepage;
}

void
KAboutData::setBugAddress( const char *bugAddress )
{
  mBugEmailAddress = bugAddress;
}

void
KAboutData::setProductName( const char *productName )
{
  d->productName = productName;
}

const char *
KAboutData::appName() const
{
   return mAppName;
}

const char *
KAboutData::productName() const
{
   if (d->productName)
      return d->productName;
   else
      return appName();
}

QString
KAboutData::programName() const
{
   if (mProgramName && *mProgramName)
      return i18n(mProgramName);
   else
      return QString::null;
}

const char*
KAboutData::internalProgramName() const
{
   if (d->mTranslatedProgramName)
      return d->mTranslatedProgramName;
   else
      return mProgramName;
}

// KCrash should call as few things as possible and should avoid e.g. malloc()
// because it may deadlock. Since i18n() needs it, when KLocale is available
// the i18n() call will be done here in advance.
void
KAboutData::translateInternalProgramName() const
{
  delete[] d->mTranslatedProgramName;
  d->mTranslatedProgramName = 0;
  if( KGlobal::locale() )
      d->mTranslatedProgramName = qstrdup( programName().utf8());
}

QImage
KAboutData::programLogo() const
{
    return d->programLogo ? (*d->programLogo) : QImage();
}

void
KAboutData::setProgramLogo(const QImage& image)
{
    if (!d->programLogo)
       d->programLogo = new QImage( image );
    else
       *d->programLogo = image;
}

QString
KAboutData::version() const
{
   return QString::fromLatin1(mVersion);
}

QString
KAboutData::shortDescription() const
{
   if (mShortDescription && *mShortDescription)
      return i18n(mShortDescription);
   else
      return QString::null;
}

QString
KAboutData::homepage() const
{
   return QString::fromLatin1(mHomepageAddress);
}

QString
KAboutData::bugAddress() const
{
   return QString::fromLatin1(mBugEmailAddress);
}

const QValueList<KAboutPerson>
KAboutData::authors() const
{
   return mAuthorList;
}

const QValueList<KAboutPerson>
KAboutData::credits() const
{
   return mCreditList;
}

const QValueList<KAboutTranslator>
KAboutData::translators() const
{
    QValueList<KAboutTranslator> personList;

    if(d->translatorName == 0)
        return personList;

    QStringList nameList;
    QStringList emailList;

    QString names = i18n(d->translatorName);
    if(names != QString::fromUtf8(d->translatorName))
    {
        nameList = QStringList::split(',',names);
    }


    if(d->translatorEmail)
    {
        QString emails = i18n(d->translatorEmail);

        if(emails != QString::fromUtf8(d->translatorEmail))
        {
            emailList = QStringList::split(',',emails,true);
        }
    }


    QStringList::Iterator nit;
    QStringList::Iterator eit=emailList.begin();

    for(nit = nameList.begin(); nit != nameList.end(); ++nit)
    {
        QString email;
        if(eit != emailList.end())
        {
            email=*eit;
            ++eit;
        }

        QString name=*nit;

        personList.append(KAboutTranslator(name.stripWhiteSpace(), email.stripWhiteSpace()));
    }

    return personList;
}

QString
KAboutData::aboutTranslationTeam()
{
    return i18n("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            );
}

QString
KAboutData::otherText() const
{
   if (mOtherText && *mOtherText)
      return i18n(mOtherText);
   else
      return QString::null;
}


QString
KAboutData::license() const
{
  QString result;
  if (!copyrightStatement().isEmpty())
    result = copyrightStatement() + "\n\n";

  QString l;
  QString f;
  switch ( mLicenseKey )
  {
    case License_File:
       f = QFile::decodeName(mLicenseText);
       break;
    case License_GPL_V2:
       l = "GPL v2";
       f = locate("data", "LICENSES/GPL_V2");
       break;
    case License_LGPL_V2:
       l = "LGPL v2";
       f = locate("data", "LICENSES/LGPL_V2");
       break;
    case License_BSD:
       l = "BSD License";
       f = locate("data", "LICENSES/BSD");
       break;
    case License_Artistic:
       l = "Artistic License";
       f = locate("data", "LICENSES/ARTISTIC");
       break;
    case License_QPL_V1_0:
       l = "QPL v1.0";
       f = locate("data", "LICENSES/QPL_V1.0");
       break;
    case License_Custom:
       if (mLicenseText && *mLicenseText)
          return( i18n(mLicenseText) );
       // fall through
    default:
       result += i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
       return result;
      }

  if (!l.isEmpty())
     result += i18n("This program is distributed under the terms of the %1.").arg( l );

  if (!f.isEmpty())
  {
     QFile file(f);
     if (file.open(IO_ReadOnly))
     {
        result += '\n';
        result += '\n';
        QTextStream str(&file);
        result += str.read();
     }
  }

  return result;
}

QString
KAboutData::copyrightStatement() const
{
  if (mCopyrightStatement && *mCopyrightStatement)
     return i18n(mCopyrightStatement);
  else
     return QString::null;
}

QString
KAboutData::customAuthorPlainText() const
{
  return d->customAuthorPlainText;
}

QString
KAboutData::customAuthorRichText() const
{
  return d->customAuthorRichText;
}

bool
KAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}
    
void
KAboutData::setCustomAuthorText(const QString &plainText, const QString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;
}
    
void
KAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = QString::null;
  d->customAuthorRichText = QString::null;

  d->customAuthorTextEnabled = false;
}

